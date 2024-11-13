/*
 * fmc.cpp
 *
 *  Created on: 16 lip 2023
 *      Author: kwarc
 */

#include "fmc.hpp"

#include <cmsis/stm32f7xx.h>
#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

bool fmc::sdram::configure(const fmc::sdram::config &cfg)
{
    rcc::enable_periph_clock({rcc::bus::AHB3, RCC_AHB3ENR_FMCEN}, true);

    /* Program the memory device features */
    uint32_t tmp_reg = 0;
    tmp_reg |= static_cast<uint32_t>(cfg.col_addr_width) << FMC_SDCR1_NC_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.row_addr_width) << FMC_SDCR1_NR_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.data_width) << FMC_SDCR1_MWID_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.internal_banks) << FMC_SDCR1_NB_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.cas_latency) << FMC_SDCR1_CAS_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.clock_period) << FMC_SDCR1_SDCLK_Pos;
    tmp_reg |= FMC_SDCR1_RBURST;

    FMC_Bank5_6->SDCR[0] = tmp_reg;
    FMC_Bank5_6->SDCR[1] = tmp_reg;

    /* Program the memory device timings */
    tmp_reg = 0;

    tmp_reg |= static_cast<uint32_t>(cfg.timing.load_to_active_delay) << FMC_SDTR1_TMRD_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.exit_self_refresh_time) << FMC_SDTR1_TXSR_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.self_refresh_time) << FMC_SDTR1_TRAS_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.row_cycle_delay) << FMC_SDTR1_TRC_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.write_recovery_delay) << FMC_SDTR1_TWR_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.row_precharge_delay) << FMC_SDTR1_TRP_Pos;
    tmp_reg |= static_cast<uint32_t>(cfg.timing.row_to_col_delay) << FMC_SDTR1_TRCD_Pos;

    FMC_Bank5_6->SDTR[0] = tmp_reg;
    FMC_Bank5_6->SDTR[1] = tmp_reg;

    return true;
}

void fmc::sdram::send_cmd(fmc::sdram::bank bank, fmc::sdram::cmd cmd, uint32_t param)
{
    while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);

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

    FMC_Bank5_6->SDCMR = tmp_reg;
}

void fmc::sdram::set_refresh_rate(uint16_t refresh_timer_count)
{
    while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);

    /* It must be set at least to 41 SDRAM clock cycles...*/
    if (refresh_timer_count < 0x29)
        refresh_timer_count = 0x29;

    /* ... and not exceed 13 bit max value */
    if (refresh_timer_count > 0x3ff)
        refresh_timer_count = 0x3ff;

    FMC_Bank5_6->SDRTR = (refresh_timer_count) << FMC_SDRTR_COUNT_Pos;
}



