/*
 * hal_sdram_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_SDRAM_IMPL_HPP_
#define STM32F7_HAL_SDRAM_IMPL_HPP_

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/fmc.hpp>
#include <drivers/stm32f7/rcc.hpp>
#include <drivers/stm32f7/delay.hpp>
#include <drivers/stm32f7/gpio.hpp>

#include <cstddef>
#include <array>

//-----------------------------------------------------------------------------

/* SDRAM MODE REG definitions */
#define SDRAM_MODE_REG_Msk                        (0b1111111111111u)
#define SDRAM_MODE_REG_BURST_LEN_Pos              (0u)
#define SDRAM_MODE_REG_BURST_LEN_Msk              (0b111u << SDRAM_MODE_REG_BURST_LEN_Pos)
#define SDRAM_MODE_REG_BURST_TYPE_Pos             (3u)
#define SDRAM_MODE_REG_BURST_TYPE_Msk             (0b1u << SDRAM_MODE_REG_BURST_TYPE_Pos)
#define SDRAM_MODE_REG_CAS_LATENCY_Pos            (4u)
#define SDRAM_MODE_REG_CAS_LATENCY_Msk            (0b111u << SDRAM_MODE_REG_CAS_LATENCY_Pos)
#define SDRAM_MODE_REG_OP_MODE_Pos                (7u)
#define SDRAM_MODE_REG_OP_MODE_Msk                (0b11u << SDRAM_MODE_REG_OP_MODE_Pos)
#define SDRAM_MODE_REG_WRITE_BURST_MODE_Pos       (9u)
#define SDRAM_MODE_REG_WRITE_BURST_MODE_Msk       (0b1u << SDRAM_MODE_REG_WRITE_BURST_MODE_Pos)

/* SDRAM target setup definitions */
#define SDRAM_START_ADDR                          (0x60000000u)
#define SDRAM_SIZE                                (8u * 1024u * 1024u)
#define SDRAM_SDCLK_HZ                            (100000000u)
#define SDRAM_SDCLK_NS                            (1000000000u / SDRAM_SDCLK_HZ)
#define NS_TO_SDCLK_CYCLES(_ns)                   ((_ns + SDRAM_SDCLK_NS - 1) / SDRAM_SDCLK_NS)

//-----------------------------------------------------------------------------

namespace hal::sdram
{
    inline void init(void)
    {
        static constexpr std::array<const drivers::gpio::io, 38> gpios =
        {{
            /* Address lines */
            {drivers::gpio::port::portf, drivers::gpio::pin::pin0},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin1},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin2},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin3},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin4},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin5},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin12},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin13},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin14},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin15},
            {drivers::gpio::port::portg, drivers::gpio::pin::pin0},
            {drivers::gpio::port::portg, drivers::gpio::pin::pin1},

            /* Data lines */
            {drivers::gpio::port::portd, drivers::gpio::pin::pin14},
            {drivers::gpio::port::portd, drivers::gpio::pin::pin15},
            {drivers::gpio::port::portd, drivers::gpio::pin::pin0},
            {drivers::gpio::port::portd, drivers::gpio::pin::pin1},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin7},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin8},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin9},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin10},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin11},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin12},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin13},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin14},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin15},
            {drivers::gpio::port::portd, drivers::gpio::pin::pin8},
            {drivers::gpio::port::portd, drivers::gpio::pin::pin9},
            {drivers::gpio::port::portd, drivers::gpio::pin::pin10},

            /* Control lines */
            {drivers::gpio::port::porte, drivers::gpio::pin::pin0},
            {drivers::gpio::port::porte, drivers::gpio::pin::pin1},

            {drivers::gpio::port::portg, drivers::gpio::pin::pin4},
            {drivers::gpio::port::portg, drivers::gpio::pin::pin5},

            {drivers::gpio::port::portc, drivers::gpio::pin::pin3},
            {drivers::gpio::port::portg, drivers::gpio::pin::pin8},
            {drivers::gpio::port::portg, drivers::gpio::pin::pin15},
            {drivers::gpio::port::porth, drivers::gpio::pin::pin3},
            {drivers::gpio::port::porth, drivers::gpio::pin::pin5},
            {drivers::gpio::port::portf, drivers::gpio::pin::pin11},
        }};

        /* Initialize GPIOs */
        for (const auto &pin : gpios)
            drivers::gpio::configure(pin, drivers::gpio::mode::af, drivers::gpio::af::af12);

        /* Remap the SDRAM to a different address (0x60000000)
         *
         * NOTE: The area 0xC0000000-0xDFFFFFFF (32MB, FMC bank 5 & 6) is specified as Device Memory Type.
         * According to the ARMv7-M Architecture Reference Manual chapter B3.1 (table B3-1),
         * all accesses to Device Memory Types must be naturally aligned.
         * If they are not, a hard fault will execute no matter if the bit UNALIGN_TRP (bit 3) in the CCR register is enabled or not.*/
        drivers::rcc::enable_periph_clock({drivers::rcc::bus::APB2, RCC_APB2ENR_SYSCFGEN}, true);
        SYSCFG->MEMRMP |= SYSCFG_MEMRMP_SWP_FMC_0;

        /* Configuration for HCLK = 100MHz */
        static constexpr drivers::fmc::sdram::config cfg
        {
            drivers::fmc::sdram::bank::bank1,
            drivers::fmc::sdram::col_addr_width::_8bit,
            drivers::fmc::sdram::row_addr_width::_12bit,
            drivers::fmc::sdram::data_width::_16bit,
            drivers::fmc::sdram::internal_banks::four,
            drivers::fmc::sdram::cas_latency::_2_cycles,
            drivers::fmc::sdram::clock_period::hclk_x2,

            {
                2,                      // tMRD
                NS_TO_SDCLK_CYCLES(67), // tXSR
                NS_TO_SDCLK_CYCLES(42), // tRAS
                NS_TO_SDCLK_CYCLES(60), // tRC
                3,                      // tDPL (note: tDPL >= tRAS – tRCD)
                NS_TO_SDCLK_CYCLES(18), // tRP
                NS_TO_SDCLK_CYCLES(18)  // tRCD
            }
        };

        drivers::fmc::sdram::configure(cfg);
        drivers::fmc::sdram::send_cmd(cfg.bank, drivers::fmc::sdram::cmd::clock_cfg_enable, 0);
        drivers::delay::us(100);
        drivers::fmc::sdram::send_cmd(cfg.bank, drivers::fmc::sdram::cmd::precharge_all, 0);
        drivers::fmc::sdram::send_cmd(cfg.bank, drivers::fmc::sdram::cmd::auto_refresh, 2);
        const uint32_t mode_register = static_cast<uint32_t>(cfg.cas_latency) << SDRAM_MODE_REG_CAS_LATENCY_Pos;
        drivers::fmc::sdram::send_cmd(cfg.bank, drivers::fmc::sdram::cmd::load_mode_register, mode_register);

        /* count = SDRAM refresh period (us) / number of SDRAM rows * SDCLK (MHz) - 20 */
        constexpr uint16_t count = 64000.f / 4096.f * (SDRAM_SDCLK_HZ / 1000000.0f) - 20.f;
        drivers::fmc::sdram::set_refresh_rate(count);
    }

    inline void *start_addr(void)
    {
        return reinterpret_cast<void*>(SDRAM_START_ADDR);
    }

    inline std::size_t size(void)
    {
        return SDRAM_SIZE;
    }
}

#endif /* STM32F7_HAL_SDRAM_IMPL_HPP_ */
