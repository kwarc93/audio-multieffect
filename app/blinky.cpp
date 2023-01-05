/*
 * blinky.cpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#include "blinky.hpp"


//-----------------------------------------------------------------------------
/* public */

blinky::blinky() : active_object("blinky", osPriorityNormal, 512)
{
    this->led.set(false);
};

//-----------------------------------------------------------------------------
/* private */

void blinky::dispatch(const event &e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void blinky::event_handler(const timer_evt_t &e)
{
    this->led.set(!led.get());
}

void blinky::event_handler(const button_evt_t &e)
{
    this->led.set(!led.get());
}
