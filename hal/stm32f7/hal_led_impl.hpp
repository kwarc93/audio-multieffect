/*
 * hal_led_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_LED_IMPL_HPP_
#define STM32F7_HAL_LED_IMPL_HPP_

#include <drivers/led_gpio.hpp>
#include <drivers/led_pwm.hpp>

namespace hal::leds
{
    class debug : public led
    {
    public:
        debug(void) : led {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porti, drivers::gpio::pin::pin1 };
        drivers::led_gpio drv {io};
    };

    class backlight : public led
    {
    public:
        backlight(void) : led {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::portk, drivers::gpio::pin::pin3 };
        drivers::led_gpio drv {io};
    };
}


#endif /* STM32F7_HAL_LED_IMPL_HPP_ */
