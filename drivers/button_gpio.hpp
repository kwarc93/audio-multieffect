/*
 * button_gpio.hpp
 *
 *  Created on: 5 paź 2021
 *      Author: kwarc
 */

#ifndef BUTTON_GPIO_HPP_
#define BUTTON_GPIO_HPP_

#include <hal_interface.hpp>

#include <drivers/stm32.hpp>

namespace drivers
{
    class button_gpio : public hal::interface::button
    {
    public:
        button_gpio(const drivers::gpio::io &io, bool inverted = false);
        bool is_pressed(void);
    private:
        drivers::gpio::io io;
        const bool inverted;
    };
}

#endif /* BUTTON_GPIO_HPP_ */
