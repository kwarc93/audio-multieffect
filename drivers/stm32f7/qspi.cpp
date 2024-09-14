/*
 * qspi.cpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#include "qspi.hpp"

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/delay.hpp>

#include <cmath>
#include <algorithm>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

void qspi::configure(const config &cfg)
{
    constexpr auto mbit_to_addr_bits = [](uint32_t mbit) -> uint8_t { return std::log2f(mbit * 1024UL * 1024UL / 8UL - 1); };

    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB3, QSPI), true);

    QUADSPI->CR = 0;
    QUADSPI->DCR = 0;

    while (QUADSPI->SR & QUADSPI_SR_BUSY);

    QUADSPI->CCR = cfg.ddr << QUADSPI_CCR_DDRM_Pos;

    QUADSPI->DCR |= (mbit_to_addr_bits(cfg.size) - 1) << QUADSPI_DCR_FSIZE_Pos;
    QUADSPI->DCR |= (std::clamp((uint32_t)cfg.cs_ht, 1ul, 8ul) - 1) << QUADSPI_DCR_CSHT_Pos;
    QUADSPI->DCR |= static_cast<uint8_t>(cfg.mode) << QUADSPI_DCR_CKMODE_Pos;

    QUADSPI->CR |= cfg.dual << QUADSPI_CR_DFM_Pos;
    QUADSPI->CR |= (std::clamp((uint32_t)cfg.clk_div, 1ul, 256ul) - 1) << QUADSPI_CR_PRESCALER_Pos;
    QUADSPI->CR |= QUADSPI_CR_SSHIFT;
}

bool qspi::send(const command &cmd, uint32_t timeout_ms)
{
    constexpr auto bits_to_size = [](uint8_t bits) -> uint8_t { return (bits > 0) ? ((bits - 1) >> 3) & 0b11 : 0; };
    constexpr auto wait_for_flag = [](uint32_t flag, uint32_t start, uint32_t timeout)
                                   {
                                        while (!(QUADSPI->SR & flag))
                                        {
                                            if (timeout && (DWT->CYCCNT - start) >= timeout)
                                                return false;
                                        }

                                        return true;
                                   };

    /* Fill Communication Configuration Register */
    uint32_t ccr = QUADSPI->CCR;
    ccr &= ~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE | QUADSPI_CCR_DCYC | QUADSPI_CCR_ABSIZE |
             QUADSPI_CCR_ABMODE | QUADSPI_CCR_ABSIZE | QUADSPI_CCR_ADMODE | QUADSPI_CCR_IMODE | QUADSPI_CCR_INSTRUCTION);
    ccr |= (static_cast<uint32_t>(cmd.mode) << QUADSPI_CCR_FMODE_Pos);
    ccr |= (static_cast<uint32_t>(cmd.data.mode) << QUADSPI_CCR_DMODE_Pos);
    ccr |= (bits_to_size(cmd.address.bits) << QUADSPI_CCR_ADSIZE_Pos) |
           (static_cast<uint32_t>(cmd.address.mode) << QUADSPI_CCR_ADMODE_Pos);
    ccr |= (cmd.instruction.value << QUADSPI_CCR_INSTRUCTION_Pos) |
           (static_cast<uint32_t>(cmd.instruction.mode) << QUADSPI_CCR_IMODE_Pos);
    ccr |= (bits_to_size(cmd.alt_bytes.bits) << QUADSPI_CCR_ABSIZE_Pos) |
           (static_cast<uint32_t>(cmd.alt_bytes.mode) << QUADSPI_CCR_ABMODE_Pos);

    if (cmd.mode != functional_mode::indirect_write)
        ccr |= (cmd.dummy_cycles << QUADSPI_CCR_DCYC_Pos);

    if (cmd.mode == functional_mode::auto_polling)
    {
        QUADSPI->PSMKR = cmd.auto_polling.mask;
        QUADSPI->PSMAR = cmd.auto_polling.match;
        QUADSPI->PIR = cmd.auto_polling.interval;

        QUADSPI->CR |= QUADSPI_CR_APMS;
    }

    QUADSPI->DLR = cmd.data.size - 1;
    QUADSPI->ABR = cmd.alt_bytes.value;
    QUADSPI->CR |= QUADSPI_CR_EN;
    QUADSPI->CCR = ccr;
    QUADSPI->AR = cmd.address.value;

    bool result = false;

    /* Timeout handling */
    const uint32_t cycles_start = DWT->CYCCNT;
    const uint32_t cycles_timeout = timeout_ms * delay::cycles_per_ms;

    std::byte *data = cmd.data.value;
    size_t data_size = cmd.data.size;

    switch (cmd.mode)
    {
        case functional_mode::indirect_write:
            while (data_size--)
            {
                if (!wait_for_flag(QUADSPI_SR_FTF, cycles_start, cycles_timeout))
                    break;

                *reinterpret_cast<volatile std::byte*>(&QUADSPI->DR) = *data++;
            }
            result = wait_for_flag(QUADSPI_SR_TCF, cycles_start, cycles_timeout);
            QUADSPI->FCR |= result << QUADSPI_FCR_CTCF_Pos;
            break;
        case functional_mode::indirect_read:
            while (data_size--)
            {
                if (!wait_for_flag(QUADSPI_SR_FTF, cycles_start, cycles_timeout))
                    break;

                *data++ = *reinterpret_cast<volatile std::byte*>(&QUADSPI->DR);
            }
            result = wait_for_flag(QUADSPI_SR_TCF, cycles_start, cycles_timeout);
            QUADSPI->FCR |= result << QUADSPI_FCR_CTCF_Pos;
            break;
        case functional_mode::auto_polling:
            result = wait_for_flag(QUADSPI_SR_SMF, cycles_start, cycles_timeout);
            QUADSPI->FCR |= result << QUADSPI_FCR_CSMF_Pos;
            break;
        case functional_mode::memory_mapped:
            /* TODO: Write implementation */
            result = false;
            break;
        default:
            break;
    }

    if (!result)
        QUADSPI->CR |= QUADSPI_CR_ABORT;

    while (QUADSPI->SR & QUADSPI_SR_BUSY);

    if (QUADSPI->SR & QUADSPI_SR_TEF)
    {
        /* Transfer error */
        QUADSPI->FCR |= QUADSPI_FCR_CTEF;
        result = false;
    }

    QUADSPI->CR &= ~QUADSPI_CR_EN;

    return result;
}
