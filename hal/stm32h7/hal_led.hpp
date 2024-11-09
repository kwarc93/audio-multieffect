/*
 * hal_led.hpp
 *
 *  Created on: 25 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_LED_HPP_
#define HAL_LED_HPP_

#include <array>

#include <hal_interface.hpp>

#include <drivers/led_gpio.hpp>
#include <drivers/led_pwm.hpp>

namespace hal
{

//-----------------------------------------------------------------------------

    class led
    {
    public:
        led(hal::interface::led *interface);
        virtual ~led();
        void set(uint8_t brightness);
        uint8_t get(void);
        void set(bool state);
    protected:
        hal::interface::led *interface;
    private:
        uint8_t brightness;
    };

//-----------------------------------------------------------------------------

namespace leds
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

//-----------------------------------------------------------------------------

}

#endif /* HAL_LED_HPP_ */
