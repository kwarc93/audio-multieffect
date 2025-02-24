/*
 * sai.cpp
 *
 *  Created on: 9 sie 2023
 *      Author: kwarc
 */

#include "sai.hpp"

#include <array>
#include <optional>
#include <cassert>

#include <cmsis/stm32h7xx.h>

#include <drivers/stm32h7/rcc.hpp>
#include <drivers/stm32h7/gpio.hpp>
#include <drivers/stm32h7/delay.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

using af = sai_base::block::audio_freq;
uint8_t sai_mclk_divide(af audio_freq)
{
    // 1. sai_ker_ck_freq should be set to around 256 x audio_freq
    // 2. fs_clk = sai_ker_ck_freq / (MCKDIV x (OSR + 1) x 256)

    uint8_t div = 0;
    switch (audio_freq)
    {
    case af::_192kHz:
        div = 1;
        break;
    case af::_96kHz:
        div = 2;
        break;
    case af::_48kHz:
        div = 4;
        break;
    case af::_16kHz:
        div = 12;
        break;
    case af::_8kHz:
        div = 24;
        break;
    case af::_44_1kHz:
    case af::_22_05kHz:
    case af::_11_025kHz:
    default:
        assert(!"Uunsupported audio frequency");
    }

    return div;
}

void configure_dmamux(sai_base::block::id id, DMA_Stream_TypeDef *dma)
{
    /* TODO Create driver for DMAMUX */

    /* DMA1/DMA2 Streams are connected to DMAMUX1 channels */
    uint32_t stream_baseaddress = (uint32_t)((uint32_t*)dma);
    uint32_t stream_number = (((uint32_t)((uint32_t*)dma) & 0xFFU) - 16U) / 24U;

    if((stream_baseaddress <= ((uint32_t)DMA2_Stream7) ) &&
       (stream_baseaddress >= ((uint32_t)DMA2_Stream0)))
    {
        stream_number += 8U;
    }

    auto dma_mux = (DMAMUX_Channel_TypeDef *)((uint32_t)(((uint32_t)DMAMUX1_Channel0) + (stream_number * 4U)));

    /* Set peripheral request to DMAMUX channel */
    const uint32_t dma_mux_request = (id == sai_base::block::id::a) ? 89 : 90;
    dma_mux->CCR = (dma_mux_request & DMAMUX_CxCR_DMAREQ_ID);

    /* Clear the DMAMUX synchro overrun flag */
    DMAMUX1_ChannelStatus->CFR = 1 << (stream_number & 0x1FU);
}

DMA_Stream_TypeDef *get_dma_stream_reg(sai_base::block::id id)
{
    if (id == sai_base::block::id::a)
        return DMA2_Stream4;
    else // id::b
        return DMA2_Stream6;
}

IRQn_Type get_dma_stream_irq(sai_base::block::id id)
{
    if (id == sai_base::block::id::a)
        return DMA2_Stream4_IRQn;
    else // id::b
        return DMA2_Stream6_IRQn;
}

}
//-----------------------------------------------------------------------------
/* private */

struct sai_base::block::block_hw
{
    block::id id;
    SAI_Block_TypeDef *const reg;
    std::optional<gpio::io> io_mckl;
    std::optional<gpio::io> io_sdclk;
    std::optional<gpio::io> io_sd;
    std::optional<gpio::io> io_fs;
};

struct sai_base::base_hw
{
    sai_base::id id;
    SAI_TypeDef *const reg;
    rcc::periph_bus pbus;
    gpio::af io_af;

    block::block_hw block_a;
    block::block_hw block_b;
};

static constexpr std::array<sai_base::base_hw, 1> saix
{
    { sai_base::id::sai2, SAI2, RCC_PERIPH_BUS(APB2, SAI2), gpio::af::af10,
    // BLOCK A
    {
      sai_base::block::id::a, SAI2_Block_A,
      std::optional<gpio::io>({ gpio::port::porti, gpio::pin::pin4 }),
      std::optional<gpio::io>({ gpio::port::porti, gpio::pin::pin5 }),
      std::optional<gpio::io>({ gpio::port::porti, gpio::pin::pin6 }),
      std::optional<gpio::io>({ gpio::port::porti, gpio::pin::pin7 })
    },
    // BLOCK B
    {
      sai_base::block::id::b, SAI2_Block_B,
      { /* unused */ },
      { /* unused */ },
      std::optional<gpio::io>({ gpio::port::portg, gpio::pin::pin10 }),
      { /* unused */ }
    }
    }
};

//-----------------------------------------------------------------------------
/* public */

sai_base::sai_base(id hw_id) :
hw {saix.at(static_cast<std::underlying_type_t<id>>(hw_id))}, block_a {block::id::a, this}, block_b {block::id::b, this}
{
    /*
     * Configure clock source for SAI at 192kHz
     * @note 1. Clock source should be set to around 256 x desired max audio frequency
     *       2. SAI clock source is PLL2 output P
     */

    static const rcc::pll_cfg pll2_cfg
    {
        6,
        189,
        16, // 49.21875MHz, close to: 192kHz x 256 = 49.152MHz
        128,
        128,
    };

    rcc::set_2nd_pll(pll2_cfg);

    /* Select PLL2 P output as SAI clock */
    RCC->D2CCIP1R |= RCC_D2CCIP1R_SAI23SEL_0;

    rcc::enable_periph_clock(this->hw.pbus, true);
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, DMA2), true);

    size_t object_id = static_cast<std::underlying_type_t<id>>(hw_id);
    if (object_id < this->instance.size())
        this->instance[object_id] = this;
}

sai_base::~sai_base()
{
    uint8_t object_id = static_cast<uint8_t>(this->hw.id);
    this->instance[object_id] = nullptr;
    rcc::enable_periph_clock(this->hw.pbus, false);
}

sai_base::block::block(id id, sai_base *base) : hw{id == id::a ? base->hw.block_a : base->hw.block_b}
{
    if (this->hw.io_mckl)
        gpio::configure(this->hw.io_mckl.value(), gpio::mode::af, base->hw.io_af);
    if (this->hw.io_sdclk)
        gpio::configure(this->hw.io_sdclk.value(), gpio::mode::af, base->hw.io_af);
    if (this->hw.io_sd)
        gpio::configure(this->hw.io_sd.value(), gpio::mode::af, base->hw.io_af);
    if (this->hw.io_fs)
        gpio::configure(this->hw.io_fs.value(), gpio::mode::af, base->hw.io_af);
}

void sai_base::sync_with(id id)
{
    /* TODO */
}

void sai_base::block::enable(bool state)
{
    if (state)
    {
        this->hw.reg->CR1 |= SAI_xCR1_SAIEN;
    }
    else
    {
        this->hw.reg->CR1 &= ~SAI_xCR1_SAIEN;
        while (this->hw.reg->CR1 & SAI_xCR1_SAIEN);
    }
}

bool sai_base::block::is_enabled(void)
{
    return this->hw.reg->CR1 & SAI_xCR1_SAIEN;
}

void sai_base::block::configure(const config &cfg)
{
    if (cfg.sync != sync_type::none && (cfg.mode == mode_type::master_rx || cfg.mode == mode_type::master_tx))
        assert(!"SAI block must be in slave mode when sync is enabled");

    /* Configure SAI_Block_x */

    this->hw.reg->CR1 = 0;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.mode) << SAI_xCR1_MODE_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.protocol) << SAI_xCR1_PRTCFG_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.data) << SAI_xCR1_DS_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.sync) << SAI_xCR1_SYNCEN_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.frame) << SAI_xCR1_MONO_Pos;
    this->hw.reg->CR1 |= (sai_mclk_divide(cfg.frequency) << SAI_xCR1_MCKDIV_Pos);
    this->hw.reg->CR1 |= SAI_xCR1_CKSTR; // Falling clock strobing edge

    this->hw.reg->CR2 = 0;
    this->hw.reg->CR2 |= SAI_xCR2_FFLUSH | SAI_xCR2_FTH_1; // Flush FIFO, FIFO threshold 1/2

    /* Configure SAI_Block_x Frame (assumming stereo mode) */
    uint8_t frame_len = 0;
    switch (cfg.data)
    {
        case data_size::_8bit:
            frame_len = 8 * cfg.slots;
            break;

        case data_size::_10bit:
        case data_size::_16bit:
            frame_len = 16 * cfg.slots;
            break;

        case data_size::_20bit:
        case data_size::_24bit:
        case data_size::_32bit:
            frame_len = 32 * cfg.slots;
            break;
        default:
            break;
    }

    this->hw.reg->FRCR = 0;
    this->hw.reg->FRCR |= ((frame_len - 1) << SAI_xFRCR_FRL_Pos); // Frame length
    this->hw.reg->FRCR |= (((frame_len / 2) - 1) << SAI_xFRCR_FSALL_Pos); // Frame active length
    this->hw.reg->FRCR |= SAI_xFRCR_FSDEF; // FS Definition: Start frame + Channel Side identification
    this->hw.reg->FRCR |= SAI_xFRCR_FSOFF; // FS Offset: FS asserted one bit before the first bit of slot 0

    /* Configure SAI Block_x Slot */
    this->hw.reg->SLOTR = 0;
    this->hw.reg->SLOTR |= ((cfg.slots - 1) << SAI_xSLOTR_NBSLOT_Pos); // Slot number
    this->hw.reg->SLOTR |= (cfg.active_slots << SAI_xSLOTR_SLOTEN_Pos); // Active slots

    /* Enable DMA requests */
    this->hw.reg->CR1 |= SAI_xCR1_DMAEN;
}

void sai_base::block::configure_dma(void *data, uint16_t data_len, std::size_t data_width, const dma_cb_t &cb, bool circular)
{
    /* Configure DMA */
    auto dma_stream = get_dma_stream_reg(this->hw.id);

    dma_stream->CR = 0;
    while (dma_stream->CR & DMA_SxCR_EN);

    dma_stream->PAR = reinterpret_cast<uint32_t>(&this->hw.reg->DR);
    dma_stream->M0AR = reinterpret_cast<uint32_t>(data);
    dma_stream->NDTR = data_len;

    dma_stream->CR |= 0b11 << DMA_SxCR_PL_Pos; // Very high priority
    dma_stream->CR |= DMA_SxCR_DMEIE | DMA_SxCR_HTIE | DMA_SxCR_TCIE | circular << DMA_SxCR_CIRC_Pos | DMA_SxCR_MINC;
    dma_stream->CR |= (data_width >> 1) << DMA_SxCR_MSIZE_Pos | (data_width >> 1) << DMA_SxCR_PSIZE_Pos;

    mode_type mode = static_cast<mode_type>((this->hw.reg->CR1 & SAI_xCR1_MODE_Msk) >> SAI_xCR1_MODE_Pos);
    if (mode == mode_type::master_tx || mode == mode_type::slave_tx)
    {
        dma_stream->CR |= 0b01 << DMA_SxCR_DIR_Pos; // Memory to peripheral
    }

    /* Configure DMAMUX1 */
    configure_dmamux(this->hw.id, dma_stream);

    this->dma_callback = cb;

    /* Enable NVIC interrupt */
    IRQn_Type nvic_irq = get_dma_stream_irq(this->hw.id);
    NVIC_SetPriority(nvic_irq, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 5, 0 ));
    NVIC_EnableIRQ(nvic_irq);

    dma_stream->CR |= DMA_SxCR_EN; // DMA enable
}

void sai_base::block::dma_irq_handler(void)
{
    if (this->hw.id == sai_base::block::id::a)
    {
        /* Half-transfer */
        if (DMA2->HISR & DMA_HISR_HTIF4)
        {
            DMA2->HIFCR = DMA_HIFCR_CHTIF4;
            if (this->dma_callback)
                this->dma_callback(dma_evt::transfer_half);
        }

        /* Transfer complete */
        if (DMA2->HISR & DMA_HISR_TCIF4)
        {
            DMA2->HIFCR = DMA_HIFCR_CTCIF4;
            if (this->dma_callback)
                this->dma_callback(dma_evt::transfer_complete);
        }

        /* Direct mode error */
        if (DMA2->HISR & DMA_HISR_DMEIF4)
        {
            DMA2->HIFCR = DMA_HIFCR_CDMEIF4;

            assert(!"SAI_A DMA direct mode error");
        }
    }
    else // id::b
    {
        /* Half-transfer */
        if (DMA2->HISR & DMA_HISR_HTIF6)
        {
            DMA2->HIFCR = DMA_HIFCR_CHTIF6;
            if (this->dma_callback)
                this->dma_callback(dma_evt::transfer_half);
        }

        /* Transfer complete */
        if (DMA2->HISR & DMA_HISR_TCIF6)
        {
            DMA2->HIFCR = DMA_HIFCR_CTCIF6;
            if (this->dma_callback)
                this->dma_callback(dma_evt::transfer_complete);
        }

        /* Direct mode error */
        if (DMA2->HISR & DMA_HISR_DMEIF6)
        {
            DMA2->HIFCR = DMA_HIFCR_CDMEIF6;

            assert(!"SAI_B DMA direct mode error");
        }
    }
}
