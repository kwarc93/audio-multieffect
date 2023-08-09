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

/* SCK(kHz) = SAI_CK_x/(SAIClockDivider*2*256) */
#define SAIClockDivider(__FREQUENCY__) \
        (__FREQUENCY__ == 8000)  ? 12 \
      : (__FREQUENCY__ == 11025) ? 2 \
      : (__FREQUENCY__ == 16000) ? 6 \
      : (__FREQUENCY__ == 22050) ? 1 \
      : (__FREQUENCY__ == 32000) ? 3 \
      : (__FREQUENCY__ == 44100) ? 0 \
      : (__FREQUENCY__ == 48000) ? 2 : 1  \

//-----------------------------------------------------------------------------
/* private */

struct sai_base::block::block_hw
{
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

static const std::map<sai_base::id, sai_base::base_hw> saix =
{
    { sai_base::id::sai2, { sai_base::id::sai2, SAI2, RCC_PERIPH_BUS(APB2, SAI2), gpio::af::af10,
                          // BLOCK A
                          {
                            SAI2_Block_A,
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin4 }),
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin5 }),
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin6 }),
                            std::make_optional<gpio::io>({ gpio::port::porti, gpio::pin::pin7 })
                          },
                          // BLOCK B
                          {
                            SAI2_Block_B,
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
}

sai_base::~sai_base()
{
    rcc::enable_periph_clock(this->hw.pbus, false);
}

sai_base::block::block(id id) : hw{this->hw}
{
    switch (id)
    {
    case id::a:
        this->reg = base->hw.block_a.reg;
        break;
    case id::b:
        this->reg = base->hw.block_b.reg;
        break;
    default:
        this->reg = nullptr;
        break;
    }
}

void sai_base::sync_with(id id)
{

}

void sai_base::block::configure(const config &cfg)
{
    /* Configure SAI_Block_x */
    this->reg->CR1 &= ~SAI_xCR1_SAIEN;   // SAI disable

    this->reg->CR1 = 0;
    this->reg->CR1 |= SAI_xCR1_OUTDRIV | SAI_xCR1_DMAEN;  // Output drive enable, DMA enable
    this->reg->CR1 |= (SAIClockDivider(cfg.frequency) << SAI_xCR1_MCKDIV_Pos); // MCLK divider
    this->reg->CR1 |= SAI_xCR1_DS_2 | SAI_xCR1_CKSTR;// 16Bit data size, falling clockstrobing edge
    this->reg->CR2 = 0;
    this->reg->CR2 |= SAI_xCR2_FFLUSH | SAI_xCR2_FTH_1;  // Flush FIFO, FIFO threshold 1/2

    /* Configure SAI_Block_x Frame */
    this->reg->FRCR = 0;
    this->reg->FRCR |= ((64 - 1) << SAI_xFRCR_FRL_Pos);  // Frame length: 64
    this->reg->FRCR |= ((16 - 1) << SAI_xFRCR_FSALL_Pos); // Frame active Length: 16
    this->reg->FRCR |= SAI_xFRCR_FSDEF;// FS Definition: Start frame + Channel Side identification
    this->reg->FRCR |= SAI_xFRCR_FSOFF;// FS Offset: FS asserted one bit before the first bit of slot 0

    /* Configure SAI Block_x Slot */
    this->reg->SLOTR = 0;
    this->reg->SLOTR |= SAI_xSLOTR_NBSLOT_1;     // Slot number: 2
    this->reg->SLOTR |= (3 << SAI_xSLOTR_SLOTEN_Pos);  // Enable slot 0,1
    this->reg->CR1 |= SAI_xCR1_SAIEN;      // SAI enable
}

void sai_base::block::sync_with(id id)
{

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

}
