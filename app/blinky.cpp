/*
 * blinky.cpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#include "blinky.hpp"

#include <cassert>

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

void blinky_timer_callback(void *arg)
{
    blinky *blinky_ao = static_cast<blinky*>(arg);

    static const blinky::event e { blinky::timer_evt_t {}, blinky::event::flags::static_storage };
    blinky_ao->send(e);
}

}

//-----------------------------------------------------------------------------
/* public */

blinky::blinky() : active_object("blinky", osPriorityNormal, 512)
{
    this->led.set(false);

    this->timer = osTimerNew(blinky_timer_callback, osTimerPeriodic, this, NULL);
    assert(this->timer != nullptr);
    osTimerStart(this->timer, 500);
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
