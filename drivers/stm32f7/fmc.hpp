/*
 * fmc.hpp
 *
 *  Created on: 16 lip 2023
 *      Author: kwarc
 */

#ifndef STM32F7_FMC_HPP_
#define STM32F7_FMC_HPP_

#include <cstdint>

namespace drivers
{

class fmc final
{
public:
    fmc() = delete;

//-----------------------------------------------------------------------------
/* SDRAM */

    class sdram final
    {
    public:
        sdram() = delete;

        /* TODO: This driver supports either bank0 or bank1 but not both simultaneously */
        enum class bank
        {
            bank1 = 0b10, bank2 = 0b01
        };

        enum class clock_period
        {
            disabled = 0b00, hclk_x2 = 0b10, hclk_x3 = 0b11
        };

        enum class cas_latency
        {
            _1_cycle = 0b01, _2_cycles = 0b10, _3_cycles = 0b11
        };

        enum class internal_banks
        {
            two, four
        };

        enum class data_width
        {
            _8bit, _16bit, _32bit
        };

        enum class row_addr_width
        {
            _11bit, _12bit, _13bit
        };

        enum class col_addr_width
        {
            _8bit, _9bit, _10bit, _11bit
        };

        enum class cmd
        {
            normal, clock_cfg_enable, precharge_all, auto_refresh, load_mode_register, self_refresh, power_down
        };

        struct config
        {
            sdram::bank bank;
            sdram::col_addr_width col_addr_width;
            sdram::row_addr_width row_addr_width;
            sdram::data_width data_width;
            sdram::internal_banks internal_banks;
            sdram::cas_latency cas_latency;
            sdram::clock_period clock_period;

            struct timing
            {
                /* All parameters are expressed in memory clock_period cycles */
                uint8_t load_to_active_delay;
                uint8_t exit_self_refresh_time;
                uint8_t self_refresh_time;
                uint8_t row_cycle_delay;
                uint8_t write_recovery_delay;
                uint8_t row_precharge_delay;
                uint8_t row_to_col_delay;
            } timing;

        };

        static bool configure(const config &cfg);
        static void send_cmd(bank bank, cmd cmd, uint32_t param);
        static void set_refresh_rate(uint16_t refresh_timer_count);
    };

//-----------------------------------------------------------------------------
/* SRAM/PSRAM */

    class sram final
    {
    public:
        sram() = delete;

        /* TODO */
    };
};

}


#endif /* STM32F7_FMC_HPP_ */
