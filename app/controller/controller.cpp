/*
 * controller.cpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#include "controller.hpp"

//-----------------------------------------------------------------------------
/* private */

namespace
{

void button_timer_cb(void *arg)
{
    hal::button *button = static_cast<hal::button*>(arg);

    button->debounce();

    static controller::event e { controller::button_evt_t {}, controller::event::flags::static_storage };

    if (button->was_pressed())
    {
        std::get<controller::button_evt_t>(e.data).state = true;
        controller::instance->send(e);
    }
    else if (button->was_released())
    {
        std::get<controller::button_evt_t>(e.data).state = false;
        controller::instance->send(e);
    }
}

}

//-----------------------------------------------------------------------------
/* public */

controller::controller() : active_object("controller", osPriorityNormal, 2048)
{
    /* Create timer for button debouncing */
    this->button_timer = osTimerNew(button_timer_cb, osTimerPeriodic, &this->button, NULL);
    assert(this->button_timer != nullptr);
    osTimerStart(this->button_timer, 20);
}

controller::~controller()
{

}

void controller::dispatch(const event& e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void controller::event_handler(const button_evt_t &e)
{
    printf("Button %s\n", e.state ? "pressed" : "released");
}

void controller::event_handler(const effect_controls_evt_t &e)
{
    std::visit([](auto &&controls)
    {
        using T = std::decay_t<decltype(controls)>;
        if constexpr (std::is_same_v<T, equalizer::controls_t>)
        {
            /* TODO */
        }
        else if constexpr (std::is_same_v<T, reverb::controls_t>)
        {
            /* TODO */
        }
        else if constexpr (std::is_same_v<T, compressor::controls_t>)
        {
            /* TODO */
        }
    }, e.controls);
}
