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

    constexpr uint8_t value_to_size(uint32_t value)
    {
        return value > 0x00FFFFFF ? 3 : value > UINT16_MAX ? 2 : value > UINT8_MAX ? 1 : 0;
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

    QUADSPI->DCR |= (mbit_to_addr_bits(cfg.size) - 1) << QUADSPI_DCR_FSIZE_Pos;
    QUADSPI->DCR |= (std::clamp((uint32_t)cfg.cs_ht, 1ul, 8ul) - 1) << QUADSPI_DCR_CSHT_Pos;
    QUADSPI->DCR |= static_cast<uint8_t>(cfg.mode) << QUADSPI_DCR_CKMODE_Pos;

    volatile uint32_t dcr = QUADSPI->DCR;

    QUADSPI->CR |= cfg.dual << QUADSPI_CR_DFM_Pos;
    QUADSPI->CR |= (std::clamp((uint32_t)cfg.clk_div, 1ul, 256ul) - 1) << QUADSPI_CR_PRESCALER_Pos;
    QUADSPI->CR |= QUADSPI_CR_SSHIFT;

    volatile uint32_t cr = QUADSPI->CR;
    cr = QUADSPI->CR;
}

// When writing the control register (QUADSPI_CR) the user specifies the following settings:
// •The enable bit (EN) set to ‘1’
// •The DMA enable bit (DMAEN) for transferring data to/from RAM
// •Timeout counter enable bit (TCEN)
// •Sample shift setting (SSHIFT)
// •FIFO threshold level (FTRHES) to indicate when the FTF flag should be set
// •Interrupt enables
// •Automatic polling mode parameters: match mode and stop mode (valid when
// FMODE = 11)
// •Clock prescaler

// When writing the communication configuration register (QUADSPI_CCR) the user specifies
// the following parameters:
// •The instruction byte through the INSTRUCTION bits
// •The way the instruction has to be sent through the IMODE bits (1/2/4 lines)
// •The way the address has to be sent through the ADMODE bits (None/1/2/4 lines)
// •The address size (8/16/24/32-bit) through the ADSIZE bits
// •The way the alternate bytes have to be sent through the ABMODE (None/1/2/4 lines)
// •The alternate bytes number (1/2/3/4) through the ABSIZE bits
// •The presence or not of dummy bytes through the DBMODE bit
// •The number of dummy bytes through the DCYC bits
// •The way the data have to be sent/received (None/1/2/4 lines) through the DMODE bits

// If neither the address register (QUADSPI_AR) nor the data register (QUADSPI_DR) need to
// be updated for a particular command, then the command sequence starts as soon as
// QUADSPI_CCR is written. This is the case when both ADMODE and DMODE are 00, or if
// just ADMODE = 00 when in indirect read mode (FMODE = 01).
// When an address is required (ADMODE is not 00) and the data register does not need to be
// written (when FMODE = 01 or DMODE = 00), the command sequence starts as soon as the
// address is updated with a write to QUADSPI_AR.
// In case of data transmission (FMODE = 00 and DMODE! = 00), the communication start is
// triggered by a write in the FIFO through QUADSPI_DR.

bool qspi::send(command &cmd)
{
    bool result = false;

    while (QUADSPI->SR & QUADSPI_SR_BUSY);

    uint32_t ccr = QUADSPI->CCR;
    ccr &= ~(QUADSPI_CCR_FMODE | QUADSPI_CCR_DMODE | QUADSPI_CCR_DCYC | QUADSPI_CCR_ABSIZE |
             QUADSPI_CCR_ABMODE | QUADSPI_CCR_ABSIZE | QUADSPI_CCR_ADMODE | QUADSPI_CCR_IMODE | QUADSPI_CCR_INSTRUCTION);

    ccr |= (static_cast<uint32_t>(cmd.data.mode) << QUADSPI_CCR_DMODE_Pos);
    ccr |= (cmd.dummy_cycles << QUADSPI_CCR_DCYC_Pos);
    ccr |= (value_to_size(cmd.address.value) << QUADSPI_CCR_ADSIZE_Pos) |
           (static_cast<uint32_t>(cmd.address.mode) << QUADSPI_CCR_ADMODE_Pos);
    ccr |= (cmd.instruction.value << QUADSPI_CCR_INSTRUCTION_Pos) |
           (static_cast<uint32_t>(cmd.instruction.mode) << QUADSPI_CCR_IMODE_Pos);
    ccr |= (value_to_size(cmd.alt_bytes.value) << QUADSPI_CCR_ABSIZE_Pos) |
           (static_cast<uint32_t>(cmd.alt_bytes.mode) << QUADSPI_CCR_ABMODE_Pos);
    ccr |= (static_cast<uint32_t>(cmd.mode) << QUADSPI_CCR_FMODE_Pos);

    if (cmd.mode == functional_mode::auto_polling)
    {
        // Set the 'mask', 'match', and 'polling interval' values.
        QUADSPI->PSMKR = cmd.auto_polling.mask;
        QUADSPI->PSMAR = cmd.auto_polling.match;
        QUADSPI->PIR = cmd.auto_polling.interval;
        // Set the 'auto-stop' bit to end the transaction after a match.
        QUADSPI->CR |= QUADSPI_CR_APMS;
    }

    QUADSPI->DLR = cmd.data.size - 1;
    QUADSPI->ABR = cmd.alt_bytes.value;
    QUADSPI->CR |= QUADSPI_CR_EN;
    QUADSPI->CCR = ccr;
    QUADSPI->AR = cmd.address.value;

    switch (cmd.mode)
    {
        case functional_mode::indirect_write:
            while (cmd.data.size--)
            {
                while (QUADSPI->SR & QUADSPI_SR_FTF);
                *reinterpret_cast<volatile std::byte*>(&QUADSPI->DR) = *cmd.data.value++;
            }

            result = true;
            break;
        case functional_mode::indirect_read:
            while (cmd.data.size--)
            {
                while (!(QUADSPI->SR & QUADSPI_SR_FTF));
                *cmd.data.value++ = *reinterpret_cast<volatile std::byte*>(&QUADSPI->DR);
            }

            result = true;
            break;
        case functional_mode::auto_polling:
            // Wait for a match
            while (QUADSPI->SR & QUADSPI_SR_BUSY);
            // Acknowledge the 'status match flag'
            QUADSPI->FCR |= QUADSPI_FCR_CSMF;
            break;
        case functional_mode::memory_mapped:
            // TODO: Write implementation
            break;
        default:
            break;
    }

    while (QUADSPI->SR & QUADSPI_SR_BUSY);
    QUADSPI->CR &= ~QUADSPI_CR_EN;

    // TODO: Check errors
    return result;
}

