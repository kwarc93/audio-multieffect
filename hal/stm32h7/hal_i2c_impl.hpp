/*
 * hal_i2c_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_I2C_IMPL_HPP_
#define STM32H7_HAL_I2C_IMPL_HPP_

#include <drivers/stm32h7/i2c.hpp>
#include <drivers/stm32h7/i2c_sw.hpp>

namespace hal::i2c
{
    constexpr inline bool use_software_i2c = false;

    namespace main
    {
        inline hal::interface::i2c & get_instance(void)
        {
            if constexpr (use_software_i2c)
            {
                constexpr drivers::gpio::io sda { drivers::gpio::port::portd, drivers::gpio::pin::pin13 };
                constexpr drivers::gpio::io scl { drivers::gpio::port::portd, drivers::gpio::pin::pin12 };
                static drivers::i2c_sw main_i2c { sda, scl, drivers::i2c_sw::mode::master, drivers::i2c_sw::speed::fast };
                return main_i2c;
            }
            else
            {
                static drivers::i2c main_i2c { drivers::i2c::id::i2c4, drivers::i2c::mode::master, drivers::i2c::speed::fast };
                return main_i2c;
            }
        }
    }
}


#endif /* STM32H7_HAL_I2C_IMPL_HPP_ */