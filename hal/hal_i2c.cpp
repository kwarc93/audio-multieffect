/*
 * hal_i2c.cpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#include "hal_i2c.hpp"

#include <drivers/stm32f7/i2c.hpp>
#include <drivers/stm32f7/i2c_sw.hpp>

using namespace hal;

#define HAL_I2C_USE_I2C_SW    (0)

interface::i2c & i2c::main::get_instance(void)
{
#if HAL_I2C_USE_I2C_SW
    constexpr drivers::gpio::io sda { drivers::gpio::port::porth, drivers::gpio::pin::pin8 };
    constexpr drivers::gpio::io scl { drivers::gpio::port::porth, drivers::gpio::pin::pin7 };
    static drivers::i2c_sw main_i2c { sda, scl, drivers::i2c_sw::mode::master, drivers::i2c_sw::speed::fast };
#else
    static drivers::i2c main_i2c { drivers::i2c::id::i2c3, drivers::i2c::mode::master, drivers::i2c::speed::fast };
#endif
    return main_i2c;
}

