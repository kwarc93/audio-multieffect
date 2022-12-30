/*
 * hal_usart.cpp
 *
 *  Created on: 24 paź 2020
 *      Author: kwarc
 */

#include "hal_usart.hpp"

#include <drivers/stm32f7/usart.hpp>

using namespace hal;

interface::serial & usart::debug::get_instance(void)
{
    static drivers::usart usart1 { drivers::usart::id::usart1, 115200 };
    return usart1;
}
