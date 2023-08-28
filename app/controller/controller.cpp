/*
 * controller.cpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#include "controller.hpp"

using namespace mfx;

//-----------------------------------------------------------------------------
/* private */

namespace
{

void button_timer_cb(void *arg)
{
    if (arg == nullptr)
        return;

    hal::button *button = static_cast<hal::button*>(arg);

    button->debounce();

    static controller::event e { controller::button_evt_t {}, controller::event::flags::immutable };

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

void led_timer_cb(void *arg)
{
    if (arg == nullptr)
        return;

    auto *ctrl = static_cast<controller*>(arg);

    static const controller::event e { controller::led_evt_t {}, controller::event::flags::immutable };
    ctrl->send(e);
}

}

//-----------------------------------------------------------------------------
/* public */

controller::controller(effect_processor *model, std::vector<view_interface*> &views) : active_object("controller", osPriorityNormal, 2048),
error_code{0},
model {model},
views {views}
{
    /* Create timer for button debouncing */
    this->button_timer = osTimerNew(button_timer_cb, osTimerPeriodic, &this->button, NULL);
    assert(this->button_timer != nullptr);
    osTimerStart(this->button_timer, 20);

    /* Create timer for LED blink */
    this->led_timer = osTimerNew(led_timer_cb, osTimerPeriodic, this, NULL);
    assert(this->button_timer != nullptr);
    osTimerStart(this->button_timer, 500);
}

controller::~controller()
{

}

void controller::dispatch(const event& e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void controller::event_handler(const led_evt_t &e)
{
    this->led.set(!this->led.get());
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
        if constexpr (std::is_same_v<T, mfx::equalizer::controls>)
        {
            /* TODO */
        }
        else if constexpr (std::is_same_v<T, mfx::noise_gate::controls>)
        {
            /* TODO */
        }
        else if constexpr (std::is_same_v<T, mfx::tremolo::controls>)
        {
            /* TODO */
        }
    }, e.controls);
}
