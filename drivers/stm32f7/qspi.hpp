/*
 * qspi.hpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#ifndef STM32F7_QSPI_HPP_
#define STM32F7_QSPI_HPP_

#include <cstdint>
#include <cstddef>

namespace drivers
{

class qspi final
{
public:
    qspi() = delete;

    enum class io_mode { none = 0, single, dual, quad };
    enum class clk_mode { mode0 = 0, mode3 = 1 };
    enum class functional_mode { indirect_write = 0, indirect_read, auto_polling, memory_mapped };

    struct config
    {
        uint16_t size; // FLASH size [Mbit]
        uint8_t cs_ht; // FLASH CS high time: 1 - 8 [cycles]
        clk_mode mode; // FLASH clock mode
        uint16_t clk_div; // Clock divider: 1 - 256
        bool ddr; // Enable Double Data Rate
        bool dual; // Enable Dual-Flash mode
    };

    static void configure(const config &cfg);

    struct command
    {
        functional_mode mode;

        struct
        {
            io_mode mode;
            uint8_t value;
            bool once;
        }
        instruction;

        struct
        {
            io_mode mode;
            uint32_t value;
            uint8_t bits;
        }
        address;

        struct
        {
            io_mode mode;
            uint32_t value;
            uint8_t bits;
        }
        alt_bytes;

        uint8_t dummy_cycles;

        struct
        {
            io_mode mode;
            std::byte *value;
            size_t size;
        }
        data;

        struct
        {
            uint32_t mask;
            uint32_t match;
            uint16_t interval;
        }
        auto_polling;
    };

    static bool send(command &cmd);

};

}

#endif /* STM32F7_QSPI_HPP_ */
