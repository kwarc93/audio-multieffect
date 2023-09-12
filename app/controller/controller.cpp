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

    static controller::event e { controller_events::button {}, controller::event::flags::immutable };

    if (button->was_pressed())
    {
        std::get<controller_events::button>(e.data).state = true;
        controller::instance->send(e);
    }
    else if (button->was_released())
    {
        std::get<controller_events::button>(e.data).state = false;
        controller::instance->send(e);
    }
}

void led_timer_cb(void *arg)
{
    if (arg == nullptr)
        return;

    auto *ctrl = static_cast<controller*>(arg);

    static const controller::event e { controller_events::led {}, controller::event::flags::immutable };
    ctrl->send(e);
}

}

//-----------------------------------------------------------------------------
/* private */

void controller::dispatch(const event& e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void controller::event_handler(const controller_events::led &e)
{
    this->led.set(!this->led.get());
    printf("Effect processor load: %u%%\n", this->model.get()->get_processing_load());
}

void controller::event_handler(const controller_events::button &e)
{
    printf("Button %s\n", e.state ? "pressed" : "released");
}

void controller::event_handler(const controller_events::effect_controls &e)
{
    std::visit([this](auto &&controls)
    {
        /* TODO: Try to avoid duplicating & forwarding events to model */
        const effect_processor::event evt {effect_processor_events::effect_controls {controls}};
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

    /* Register event handler for view */
    this->view.get()->set_event_handler(
    [this](const view_interface_events::holder &e)
    {
        /* TODO */
    }
    );

    /* Add some effects (order is important!) */
    static const std::array<effect_processor::event, 4> model_events =
    {{
        { effect_processor_events::add_effect {effect_id::overdrive} },
        { effect_processor_events::add_effect {effect_id::tremolo} },
        { effect_processor_events::add_effect {effect_id::echo} },
        { effect_processor_events::add_effect {effect_id::cabinet_sim} },
    }};

    for (const auto &e : model_events)
        this->model->send(e);
}

controller::~controller()
{

}
