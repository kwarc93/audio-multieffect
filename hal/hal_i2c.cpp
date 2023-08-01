/*
 * hal_i2c.cpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#include "hal_i2c.hpp"

#include <drivers/stm32f7/i2c.hpp>

using namespace hal;

interface::i2c & i2c::main::get_instance(void)
{
    static drivers::hw_i2c main_i2c {};
    return main_i2c;
}

