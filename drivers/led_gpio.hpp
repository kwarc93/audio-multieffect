/*
 * led_gpio.hpp
 *
 *  Created on: 2 lis 2020
 *      Author: kwarc
 */

#ifndef LED_GPIO_HPP_
#define LED_GPIO_HPP_

#include <hal/hal_interface.hpp>

#include <drivers/stm32f7/gpio.hpp>

namespace drivers
{
    class led_gpio : public hal::interface::led
    {
    public:
        led_gpio(const drivers::gpio::io &io);
        void set(uint8_t brightness) override;
        uint8_t get(void) override;

    private:
        drivers::gpio::io io;
    };
}

#endif /* LED_GPIO_HPP_ */
