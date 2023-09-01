/*
 * controller.cpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#include "controller.hpp"

#include <array>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

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
/* private */

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
    std::visit([this](auto &&controls)
    {
        /* TODO: Try to avoid duplicating & forwarding events to model */
        const effect_processor::event evt {effect_processor::effect_controls_evt_t {controls}};
        this->model->send(evt);

    }, e.controls);
}

//-----------------------------------------------------------------------------
/* public */

controller::controller(std::unique_ptr<effect_processor> model, std::unique_ptr<view_interface> view) :
active_object("controller", osPriorityNormal, 2048),
error_code{0},
model {std::move(model)},
view {std::move(view)}
{
    /* Create timer for button debouncing */
    this->button_timer = osTimerNew(button_timer_cb, osTimerPeriodic, &this->button, NULL);
    assert(this->button_timer != nullptr);
    osTimerStart(this->button_timer, 20);

    /* Create timer for LED blink */
    this->led_timer = osTimerNew(led_timer_cb, osTimerPeriodic, this, NULL);
    assert(this->led_timer != nullptr);
    osTimerStart(this->led_timer, 500);

    /* Add some effects (ordering affects sound) */
    static const std::array<effect_processor::event, 4> model_events =
    {{
        { effect_processor::add_effect_evt_t {effect_id::overdrive} },
        { effect_processor::add_effect_evt_t {effect_id::tremolo} },
        { effect_processor::add_effect_evt_t {effect_id::echo} },
        { effect_processor::add_effect_evt_t {effect_id::cabinet_sim} },
    }};

    for (const auto &e : model_events)
        this->model->send(e);
}

controller::~controller()
{

}
