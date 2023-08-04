/*
 * led_gpio.cpp
 *
 *  Created on: 2 lis 2020
 *      Author: kwarc
 */

#include "led_gpio.hpp"

using namespace drivers;

led_gpio::led_gpio(const gpio::io &io)
{
    this->io = io;

    gpio::configure(this->io);
}

void led_gpio::set(uint8_t brightness)
{
    gpio::write(this->io, brightness);
}

uint8_t led_gpio::get(void)
{
    return gpio::read(this->io);
}
