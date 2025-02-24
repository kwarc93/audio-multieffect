/*
 * hal_button_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_BUTTON_IMPL_HPP_
#define STM32F7_HAL_BUTTON_IMPL_HPP_

#include <drivers/button_gpio.hpp>

namespace hal::buttons
{
    class blue_btn : public button
    {
    public:
        blue_btn(void) : button {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porti, drivers::gpio::pin::pin11 };
        drivers::button_gpio drv {io};
    };
}

#endif /* STM32F7_HAL_BUTTON_IMPL_HPP_ */
