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

void fmc::deinit()
{
    rcc::enable_periph_clock({rcc::bus::AHB3, RCC_AHB3ENR_FMCEN}, false);
}

bool fmc::sdram::init(const fmc::sdram::cfg &cfg)
{
    rcc::enable_periph_clock({rcc::bus::AHB3, RCC_AHB3ENR_FMCEN}, true);

    /* Program the memory device features */
    const uint8_t bank = cfg.bank == fmc::sdram::bank::bank1 ? 0 : 1;

    uint32_t tmp_reg = 0;
    tmp_reg |= (static_cast<uint32_t>(cfg.col_addr_width) & FMC_SDCR1_NC_Msk) << FMC_SDCR1_NC_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.row_addr_width) & FMC_SDCR1_NR_Msk) << FMC_SDCR1_NR_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.data_width) & FMC_SDCR1_MWID_Msk) << FMC_SDCR1_MWID_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.internal_banks) & FMC_SDCR1_NB_Msk) << FMC_SDCR1_NB_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.cas_latency) & FMC_SDCR1_CAS_Msk) << FMC_SDCR1_CAS_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.clock_period) & FMC_SDCR1_SDCLK_Msk) << FMC_SDCR1_SDCLK_Pos;
    tmp_reg |= FMC_SDCR1_RBURST;

    FMC_Bank5_6->SDCR[bank] = tmp_reg;

    /* Program the memory device timings */
    tmp_reg = 0;

    tmp_reg |= (static_cast<uint32_t>(cfg.timing.load_to_active_delay) & FMC_SDTR1_TMRD_Msk) << FMC_SDTR1_TMRD_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.timing.exit_self_refresh_time) & FMC_SDTR1_TXSR_Msk) << FMC_SDTR1_TXSR_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.timing.self_refresh_time) & FMC_SDTR1_TRAS_Msk) << FMC_SDTR1_TRAS_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.timing.row_cycle_delay) & FMC_SDTR1_TRC_Msk) << FMC_SDTR1_TRC_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.timing.write_recovery_delay) & FMC_SDTR1_TWR_Msk) << FMC_SDTR1_TWR_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.timing.row_precharge_delay) & FMC_SDTR1_TRP_Msk) << FMC_SDTR1_TRP_Pos;
    tmp_reg |= (static_cast<uint32_t>(cfg.timing.row_to_col_delay) & FMC_SDTR1_TRCD_Msk) << FMC_SDTR1_TRCD_Pos;

    FMC_Bank5_6->SDTR[bank] = tmp_reg;

    return true;
}

void fmc::sdram::send_cmd(fmc::sdram::bank bank, fmc::sdram::cmd cmd, uint32_t param)
{
    while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);

    FMC_Bank5_6->SDCMR = 0;

    switch (cmd)
    {
    case fmc::sdram::cmd::auto_refresh:
        FMC_Bank5_6->SDCMR |= (static_cast<uint32_t>(param) & FMC_SDCMR_NRFS_Msk) << FMC_SDCMR_NRFS_Pos;
        break;
    case fmc::sdram::cmd::load_mode_register:
        FMC_Bank5_6->SDCMR |= (static_cast<uint32_t>(param) & FMC_SDCMR_MRD_Msk) << FMC_SDCMR_MRD_Pos;
        break;
    default:
        break;
    }

    FMC_Bank5_6->SDCMR |= (static_cast<uint32_t>(bank) & (FMC_SDCMR_CTB1_Msk | FMC_SDCMR_CTB2_Msk)) << FMC_SDCMR_CTB2_Pos;
    FMC_Bank5_6->SDCMR |= (static_cast<uint32_t>(cmd) & FMC_SDCMR_MODE_Msk) << FMC_SDCMR_MODE_Pos;
}

void fmc::sdram::set_refresh_rate(uint16_t refresh_timer_count)
{
    while (FMC_Bank5_6->SDSR & FMC_SDSR_BUSY);

    /* It must be set at least to 41 SDRAM clock cycles (0x29) */
    if (refresh_timer_count < 41)
        refresh_timer_count = 41;

    FMC_Bank5_6->SDRTR = (refresh_timer_count & FMC_SDRTR_COUNT_Msk) << FMC_SDRTR_COUNT_Pos;
}



