/*
 * hal_sdram.cpp
 *
 *  Created on: 17 lip 2023
 *      Author: kwarc
 */

#include "hal_sdram.hpp"

#include <drivers/stm32f7/fmc.hpp>
#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/delay.hpp>

using namespace hal;

//-----------------------------------------------------------------------------
/* SDRAM MODE REG definitions */

#define SDRAM_MODE_REG_Msk                        (0b1111111111111u)
#define SDRAM_MODE_REG_BURST_LEN_Pos              (0u)
#define SDRAM_MODE_REG_BURST_LEN_Msk              (0b111u)
#define SDRAM_MODE_REG_BURST_TYPE_Pos             (3u)
#define SDRAM_MODE_REG_BURST_TYPE_Msk             (0b1u)
#define SDRAM_MODE_REG_BURST_CAS_LATENCY_Pos      (4u)
#define SDRAM_MODE_REG_BURST_CAS_LATENCY_Msk      (0b111u)
#define SDRAM_MODE_REG_BURST_OP_MODE_Pos          (7u)
#define SDRAM_MODE_REG_BURST_OP_MODE_Msk          (0b11u)
#define SDRAM_MODE_REG_WRITE_BURST_MODE_Pos       (9u)
#define SDRAM_MODE_REG_WRITE_BURST_MODE_Msk       (0b1u)

//-----------------------------------------------------------------------------
/* SDRAM target setup definitions */

#define SDRAM_START_ADDR                          (0x6000000u)
#define SDRAM_SIZE                                (16u * 1024u * 1024u)
#define SDRAM_SDCLK_HZ                            (50000000u)
#define SDRAM_SDCLK_NS                            (1000000000u / SDRAM_SDCLK_HZ)
#define NS_TO_SDRAM_CLK_CYCLES(_ns)               ((_ns + SDRAM_SDCLK_NS - 1) / SDRAM_SDCLK_NS)

void sdram::init(void)
{
    /* Configuration for HCLK = 100MHz */

    static const drivers::fmc::sdram::cfg config
    {
        drivers::fmc::sdram::bank::bank1,
        drivers::fmc::sdram::col_addr_width::_8bit,
        drivers::fmc::sdram::row_addr_width::_12bit,
        drivers::fmc::sdram::data_width::_16bit,
        drivers::fmc::sdram::internal_banks::four,
        drivers::fmc::sdram::cas_latency::_1_cycle,
        drivers::fmc::sdram::clock_period::hclk_x2,

        {
            2,                          // tMRD
            NS_TO_SDRAM_CLK_CYCLES(67), // tXSR
            NS_TO_SDRAM_CLK_CYCLES(42), // tRAS
            NS_TO_SDRAM_CLK_CYCLES(60), // tRC
            3,                          // tDPL (note: tDPL >= tRAS – tRCD)
            NS_TO_SDRAM_CLK_CYCLES(18), // tRP
            NS_TO_SDRAM_CLK_CYCLES(18)  // tRCD
        }
    };

    drivers::fmc::sdram::init(config);
    drivers::fmc::sdram::send_cmd(config.bank, drivers::fmc::sdram::cmd::clock_cfg_enable, 0);
    drivers::delay::us(100);
    drivers::fmc::sdram::send_cmd(config.bank, drivers::fmc::sdram::cmd::precharge_all, 0);
    drivers::fmc::sdram::send_cmd(config.bank, drivers::fmc::sdram::cmd::auto_refresh, 2);

    const uint32_t mode_register = static_cast<uint32_t>(config.cas_latency) << SDRAM_MODE_REG_BURST_CAS_LATENCY_Pos
                           | 1 << SDRAM_MODE_REG_WRITE_BURST_MODE_Pos;

    drivers::fmc::sdram::send_cmd(config.bank, drivers::fmc::sdram::cmd::load_mode_register, mode_register);

    /* count = SDRAM refresh period (us) / number of SDRAM rows * SDCLK (MHz) - 20 */
    constexpr uint16_t count = 64000.f / 4096.f * 50.f - 20.f;
    drivers::fmc::sdram::set_refresh_rate(count);


    /* Remap the SDRAM to a different address (0x60000000)
     *
     * NOTE: The area 0xC0000000-0xDFFFFFFF (32MB, FMC bank 5 & 6) is specified as Device Memory Type.
     * According to the ARMv7-M Architecture Reference Manual chapter B3.1 (table B3-1),
     * all accesses to Device Memory Types must be naturally aligned.
     * If they are not, a hard fault will execute no matter if the bit UNALIGN_TRP (bit 3) in the CCR register is enabled or not.*/
    drivers::rcc::enable_periph_clock({drivers::rcc::bus::APB2, RCC_APB2ENR_SYSCFGEN}, true);
    SYSCFG->MEMRMP |= SYSCFG_MEMRMP_SWP_FMC_0;
}

void *sdram::start_addr(void)
{
    return reinterpret_cast<void*>(SDRAM_START_ADDR);
}

std::size_t sdram::size(void)
{
    return SDRAM_SIZE;
}
