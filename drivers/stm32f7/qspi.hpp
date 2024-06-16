/*
 * qspi.hpp
 *
 *  Created on: 16 cze 2024
 *      Author: kwarc
 */

#ifndef STM32F7_QSPI_HPP_
#define STM32F7_QSPI_HPP_

#include <cstdint>

namespace drivers
{

class qspi final
{
public:
    qspi() = delete;

    enum class io { single = 0, dual, quad };
    enum class clk_mode { mode0 = 0, mode3 = 1 };

    struct config
    {
        uint16_t size; // FLASH size [Mbit]
        uint8_t cs_ht; // FLASH CS high time: 1 - 8 [cycles]
        clk_mode mode; // FLASH functional mode
        io io_mode;    // FLASH IO mode: single/dual/quad

        uint16_t clk_div; // Clock divider: 1 - 256
        bool ddr; // Enable Double Data Rate
        bool dual; // Enable Dual-Flash mode
    };

    static void configure(const config &cfg);

    static bool write(uint8_t *data, uint32_t timeout);
    static bool read(uint8_t *data, uint32_t timeout);
//    bool auto_polling(command *cmd, auto_polling_cfg *cfg, uint32_t timeout);

private:
    static io io_mode;
};

}

#endif /* STM32F7_QSPI_HPP_ */
