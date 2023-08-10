/*
 * sai.cpp
 *
 *  Created on: 9 sie 2023
 *      Author: kwarc
 */

#include "sai.hpp"

#include <map>
#include <optional>
#include <cassert>

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/gpio.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

using af = sai_base::block::audio_freq;
uint8_t sai_mclk_divide(af audio_freq)
{
    // 1. sai_ker_ck_freq should be set to around 256 x audio_freq
    // 2. sck = mclk x (bit_ck_cycl) / 256

    uint8_t div = 0;
    switch (audio_freq)
    {
    case af::_192kHz:
    case af::_44_1kHz:
    default:
        break;
    case af::_96kHz:
    case af::_22_05kHz:
        div = 0b0001;
        break;
    case af::_48kHz:
    case af::_11_025kHz:
        div = 0b0010;
        break;
    case af::_16kHz:
        div = 0b0110;
        break;
    case af::_8kHz:
            div = 0b1100;
            break;
    }

    return div;
}

DMA_Stream_TypeDef *get_dma_stream_reg(sai_base::block::id id)
{
    if (id == sai_base::block::id::a)
        return DMA2_Stream4;
    else
        return DMA2_Stream6;
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

static const std::map<sai_base::id, sai_base::base_hw> saix
{
    { sai_base::id::sai2, { sai_base::id::sai2, SAI2, RCC_PERIPH_BUS(APB2, SAI2), gpio::af::af10,
                          // BLOCK A
                          {
                            sai_base::block::id::a, SAI2_Block_A,
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin4 }),
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin5 }),
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin6 }),
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin7 })
                          },
                          // BLOCK B
                          {
                            sai_base::block::id::b, SAI2_Block_B,
                            { /* unused */ },
                            { /* unused */ },
                            std::make_optional<gpio::io>({ gpio::port::portg, gpio::pin::pin10 }),
                            { /* unused */ }}
                          }
    },
};

//-----------------------------------------------------------------------------
/* public */

sai_base::sai_base(id id) : block_a {block::id::a, this}, block_b {block::id::a, this}, hw {saix.at(id)}
{
    rcc::enable_periph_clock(this->hw.pbus, true);
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, DMA2), true);

    uint8_t object_id = static_cast<uint8_t>(id);
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

    if (id == id::a)
    {
        /* TODO: DMA IRQ: SAI2A: DMA2 CH3 Stream4 */
        IRQn_Type nvic_irq = DMA2_Stream4_IRQn;
        NVIC_SetPriority(DMA2_Stream4_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 5, 0 ));
        NVIC_EnableIRQ(nvic_irq);
    }
}

void sai_base::sync_with(id id)
{
    /* TODO */
}

void sai_base::block::enable(bool state)
{
    if (state)
        this->hw.reg->CR1 |= SAI_xCR1_SAIEN;
    else
        this->hw.reg->CR1 &= ~SAI_xCR1_SAIEN;
}

void sai_base::block::configure(const config &cfg)
{
    if (cfg.sync != sync_type::none && (cfg.mode == mode_type::master_rx || cfg.mode == mode_type::master_tx))
        assert(!"SAI block must be in slave mode when sync is enabled");

    /* Configure SAI_Block_x */
    this->hw.reg->CR1 &= ~SAI_xCR1_SAIEN; // SAI disable

    this->hw.reg->CR1 = 0;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.mode) << SAI_xCR1_MODE_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.protocol) << SAI_xCR1_PRTCFG_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.data) << SAI_xCR1_DS_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.sync) << SAI_xCR1_SYNCEN_Pos;
    this->hw.reg->CR1 |= static_cast<uint8_t>(cfg.frame) << SAI_xCR1_MONO_Pos;
    this->hw.reg->CR1 |= (sai_mclk_divide(cfg.frequency) << SAI_xCR1_MCKDIV_Pos);
    this->hw.reg->CR1 |= SAI_xCR1_CKSTR; // falling clock strobing edge

    this->hw.reg->CR2 = 0;
    this->hw.reg->CR2 |= SAI_xCR2_FFLUSH | SAI_xCR2_FTH_1; // Flush FIFO, FIFO threshold 1/2

    /* Configure SAI_Block_x Frame */
    this->hw.reg->FRCR = 0;
    this->hw.reg->FRCR |= ((64 - 1) << SAI_xFRCR_FRL_Pos); // Frame length: 64
    this->hw.reg->FRCR |= ((16 - 1) << SAI_xFRCR_FSALL_Pos); // Frame active Length: 16
    this->hw.reg->FRCR |= SAI_xFRCR_FSDEF; // FS Definition: Start frame + Channel Side identification
    this->hw.reg->FRCR |= SAI_xFRCR_FSOFF; // FS Offset: FS asserted one bit before the first bit of slot 0

    /* Configure SAI Block_x Slot */
    this->hw.reg->SLOTR = 0;
    this->hw.reg->SLOTR |= SAI_xSLOTR_NBSLOT_1; // Slot number: 2
    this->hw.reg->SLOTR |= (3 << SAI_xSLOTR_SLOTEN_Pos); // Enable slot 0,1
}

void sai_base::block::configure_dma(void *data, uint16_t data_len, std::size_t data_width, bool circular)
{
    /* Enable DMA requests */
    this->hw.reg->CR1 |= SAI_xCR1_DMAEN;

    /* Configure DMA */
    auto dma_stream = get_dma_stream_reg(this->hw.id);

    dma_stream->CR &= ~DMA_SxCR_EN;
    while (dma_stream->CR & DMA_SxCR_EN);

    dma_stream->PAR = reinterpret_cast<uint32_t>(&this->hw.reg->DR);
    dma_stream->M0AR = reinterpret_cast<uint32_t>(data);
    dma_stream->NDTR = data_len;
    dma_stream->CR |= 3 << DMA_SxCR_CHSEL_Pos; // Channel 3
    dma_stream->CR |= DMA_SxCR_PFCTRL; // SAI is flow controller
    dma_stream->CR |= 0b11 << DMA_SxCR_PL_Pos; // Very high priority
    dma_stream->CR |= DMA_SxCR_HTIE | DMA_SxCR_TCIE | circular << DMA_SxCR_CIRC_Pos | DMA_SxCR_MINC; // HT & TC IRQ
    dma_stream->CR |= (data_width >> 1) << DMA_SxCR_MSIZE_Pos | (data_width >> 1) << DMA_SxCR_PSIZE_Pos;

    mode_type mode = static_cast<mode_type>((this->hw.reg->CR1 & SAI_xCR1_MODE_Msk) >> SAI_xCR1_MODE_Pos);
    if (mode == mode_type::master_tx || mode == mode_type::slave_tx)
        dma_stream->CR |= 0b01 << DMA_SxCR_DIR_Pos; // Memory to peripheral

    dma_stream->CR |= DMA_SxCR_EN; // DMA enable
}

template<typename T>
sai<T>::sai(id id) : sai_base {id}
{

}

template<typename T>
sai<T>::~sai()
{

}

template<typename T>
void sai<T>::transfer(const typename hal::interface::i2s<T>::transfer_desc &transfer,
                      const typename hal::interface::i2s<T>::transfer_cb_t &callback,
                      bool loop)
{
    this->block_b.configure_dma(transfer.rx_data, transfer.rx_size, sizeof(*transfer.rx_data), loop);
    this->block_a.configure_dma(transfer.tx_data, transfer.tx_size, sizeof(*transfer.tx_data), loop);

    this->block_b.enable(true);
    this->block_a.enable(true);
}
