/*
 * fmc.cpp
 *
 *  Created on: 16 lip 2023
 *      Author: kwarc
 */

#include "fmc.hpp"

#include <cmsis/stm32h7xx.h>
#include <drivers/stm32h7/rcc.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */
#define FMC_SDSR_BUSY (1 << 5)

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

void fmc::enable(bool state)
{
    if (state)
    {
        rcc::enable_periph_clock({rcc::bus::AHB3, RCC_AHB3ENR_FMCEN}, true);
        FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;
    }
    else
    {
        FMC_Bank1_R->BTCR[0] &= ~FMC_BCR1_FMCEN;
        rcc::enable_periph_clock({rcc::bus::AHB3, RCC_AHB3ENR_FMCEN}, false);
    }
}

void fmc::remap_bank(remap_type remap)
{
    FMC_Bank1_R->BTCR[0] |= static_cast<uint8_t>(remap) << FMC_BCR1_BMAP_Pos;
}

bool fmc::sdram::configure(const fmc::sdram::config &cfg)
{
    /* Disable FMC */
    FMC_Bank1_R->BTCR[0] &= ~FMC_BCR1_FMCEN;

    /* Program the memory device features */
    uint32_t tmp_reg = 0;
    tmp_reg |= static_cast<uint32_t>(cfg.col_addr_width) << FMC_SDCRx_NC_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.row_addr_width) << FMC_SDCRx_NR_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.data_width) << FMC_SDCRx_MWID_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.internal_banks) << FMC_SDCRx_NB_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.cas_latency) << FMC_SDCRx_CAS_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.clock_period) << FMC_SDCRx_SDCLK_Pos;
    tmp_reg |= FMC_SDCRx_RBURST;

    FMC_Bank5_6_R->SDCR[0] = tmp_reg;
    FMC_Bank5_6_R->SDCR[1] = tmp_reg;

    /* Program the memory device timings */
    tmp_reg = 0;

    tmp_reg |= static_cast<uint32_t>(cfg.timing.load_to_active_delay - 1) << FMC_SDTRx_TMRD_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.exit_self_refresh_time - 1) << FMC_SDTRx_TXSR_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.self_refresh_time - 1) << FMC_SDTRx_TRAS_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.row_cycle_delay - 1) << FMC_SDTRx_TRC_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.write_recovery_delay - 1) << FMC_SDTRx_TWR_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.row_precharge_delay - 1) << FMC_SDTRx_TRP_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.row_to_col_delay - 1) << FMC_SDTRx_TRCD_Pos;

    FMC_Bank5_6_R->SDTR[0] = tmp_reg;
    FMC_Bank5_6_R->SDTR[1] = tmp_reg;

    /* Enable FMC */
    FMC_Bank1_R->BTCR[0] |= FMC_BCR1_FMCEN;

    return true;
}

void fmc::sdram::send_cmd(fmc::sdram::bank bank, fmc::sdram::cmd cmd, uint32_t param)
{
    while (FMC_Bank5_6_R->SDSR & FMC_SDSR_BUSY);

    uint32_t tmp_reg = 0;

    switch (cmd)
    {
    case fmc::sdram::cmd::auto_refresh:
        tmp_reg |= static_cast<uint32_t>(param) << FMC_SDCMR_NRFS_Pos;
        break;
    case fmc::sdram::cmd::load_mode_register:
        tmp_reg |= static_cast<uint32_t>(param) << FMC_SDCMR_MRD_Pos;
        break;
    default:
        break;
    }

    tmp_reg |= static_cast<uint32_t>(bank) << FMC_SDCMR_CTB2_Pos;
    tmp_reg |= static_cast<uint32_t>(cmd) << FMC_SDCMR_MODE_Pos;

    FMC_Bank5_6_R->SDCMR = tmp_reg;
}

void fmc::sdram::set_refresh_rate(uint16_t refresh_timer_count)
{
    while (FMC_Bank5_6_R->SDSR & FMC_SDSR_BUSY);

    /* It must be set at least to 41 SDRAM clock cycles...*/
    if (refresh_timer_count < 0x29)
        refresh_timer_count = 0x29;

    /* ... and not exceed 13 bit max value */
    if (refresh_timer_count > 0x1FFF)
        refresh_timer_count = 0x1FFF;

    FMC_Bank5_6_R->SDRTR |= (refresh_timer_count) << FMC_SDRTR_COUNT_Pos;
}

