/*
 * hal_delay_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_DELAY_IMPL_HPP_
#define STM32F7_HAL_DELAY_IMPL_HPP_

#include <drivers/stm32f7/delay.hpp>

namespace hal::delay
{
    inline void delay_ms(unsigned int ms)
    {
        drivers::delay::ms(ms);
    }

    inline void delay_us(unsigned int us)
    {
        drivers::delay::us(us);
    }
}

#endif /* STM32F7_HAL_DELAY_IMPL_HPP_ */
