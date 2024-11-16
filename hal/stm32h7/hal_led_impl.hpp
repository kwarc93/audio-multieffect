/*
 * hal_led_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_LED_IMPL_HPP_
#define STM32H7_HAL_LED_IMPL_HPP_

#include <drivers/led_gpio.hpp>
#include <drivers/led_pwm.hpp>

namespace hal::leds
{
    class debug : public led
    {
    public:
        debug(void) : led {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::portj, drivers::gpio::pin::pin2 };
        drivers::led_gpio drv {io};
    };

    class error : public led
    {
    public:
        error(void) : led {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porti, drivers::gpio::pin::pin13 };
        drivers::led_gpio drv {io};
    };

    class backlight : public led
    {
    public:
        backlight(void) : led {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::portk, drivers::gpio::pin::pin0 };
        drivers::led_gpio drv {io};
    };
}


#endif /* STM32H7_HAL_LED_IMPL_HPP_ */
