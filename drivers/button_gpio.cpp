/*
 * button_gpio.cpp
 *
 *  Created on: 5 paź 2021
 *      Author: kwarc
 */

#include "button_gpio.hpp"

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */


//-----------------------------------------------------------------------------
/* public */

button_gpio::button_gpio(gpio::io io, bool inverted) : io {io}, inverted {inverted}
{
    gpio::configure(this->io, gpio::mode::input, gpio::af::af0, gpio::pupd::pd);
}

bool button_gpio::is_pressed()
{
    return this->inverted ? !gpio::read(this->io) : gpio::read(this->io);
}

