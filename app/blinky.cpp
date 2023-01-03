/*
 * blinky.cpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#include "blinky.hpp"

blinky::blinky() : active_object("blinky", osPriorityNormal, 512)
{
    this->led.set(false);
};

void blinky::dispatch(const blinky_evt::blinky_evt_t &e)
{
    std::visit([this](const auto &e) { return this->event_handler(e); }, e);
}

void blinky::event_handler(const blinky_evt::timer_evt_t &e)
{
    this->led.set(!led.get());
}

void blinky::event_handler(const blinky_evt::button_evt_t &e)
{
    this->led.set(e.pressed);
}
