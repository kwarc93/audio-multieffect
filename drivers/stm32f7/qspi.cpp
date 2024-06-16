/*
 * qspi.cpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#include "qspi.hpp"

#include <cmsis/stm32f7xx.h>
#include <drivers/stm32f7/rcc.hpp>

#include <cmath>
#include <algorithm>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
    constexpr uint32_t mbit_to_addr_bits(uint32_t mbit)
    {
        return std::log2f(mbit * 1024UL * 1024UL / 8UL - 1);
    }
}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

void qspi::configure(const config &cfg)
{
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB3, QSPI), true);

    QUADSPI->CR = 0;
    QUADSPI->DCR = 0;

    while (QUADSPI->SR & QUADSPI_SR_BUSY);

    QUADSPI->CCR = cfg.ddr << QUADSPI_CCR_DDRM_Pos;

    QUADSPI->DCR |= mbit_to_addr_bits(cfg.size) << QUADSPI_DCR_FSIZE_Pos;
    QUADSPI->DCR |= (std::clamp((uint32_t)cfg.cs_ht, 1ul, 8ul) - 1) << QUADSPI_DCR_CSHT_Pos;
    QUADSPI->DCR |= static_cast<uint8_t>(cfg.mode) << QUADSPI_DCR_CKMODE_Pos;

    QUADSPI->CR |= (cfg.dual - 1) << QUADSPI_CR_DFM_Pos;
    QUADSPI->CR |= (std::clamp((uint32_t)cfg.clk_div, 1ul, 256ul) - 1) << QUADSPI_CR_PRESCALER_Pos;
    QUADSPI->CR |= QUADSPI_CR_SSHIFT | QUADSPI_CR_EN;
}

bool qspi::write(uint8_t *data, uint32_t timeout)
{
    // TODO
    return false;
}

bool qspi::read(uint8_t *data, uint32_t timeout)
{
    // TODO
    return false;
}


