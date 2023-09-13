/*
 * controller.cpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#include "controller.hpp"

#include <array>

using namespace mfx;
namespace events = controller_events;

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

    static controller::event e { events::button {}, controller::event::flags::immutable };

    if (button->was_pressed())
    {
        std::get<events::button>(e.data).state = true;
        controller::instance->send(e);
    }
    else if (button->was_released())
    {
        std::get<events::button>(e.data).state = false;
        controller::instance->send(e);
    }
}

void led_timer_cb(void *arg)
{
    if (arg == nullptr)
        return;

    auto *ctrl = static_cast<controller*>(arg);

    static const controller::event e { events::led {}, controller::event::flags::immutable };
    ctrl->send(e);
}

}

//-----------------------------------------------------------------------------
/* private */

void controller::dispatch(const event& e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void controller::event_handler(const events::led &e)
{
    this->led.set(!this->led.get());

    const effect_processor::event evt {effect_processor_events::get_processing_load
    {
        [this](auto... params)
        {
            const controller::event e {events::effect_processor_load {params...}};
            this->send(e);
        }
    }};

    this->model.get()->send(evt);
}

void controller::event_handler(const events::button &e)
{
    printf("Button %s\n", e.state ? "pressed" : "released");
}

void controller::event_handler(const events::effect_processor_load &e)
{
    printf("Effect processor load: %u%%\n", e.load);
}

void controller::view_event_handler(const view_interface_events::settings_volume_changed &e)
{
    const effect_processor::event evt {effect_processor_events::volume {e.input_vol, e.output_vol}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const view_interface_events::effect_bypass_changed &e)
{
    const effect_processor::event evt {effect_processor_events::bypass {e.id, e.bypassed}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const view_interface_events::tremolo_controls_changed &e)
{
    effect_processor::event evt {effect_processor_events::effect_controls {e.ctrl}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const view_interface_events::echo_controls_changed &e)
{
    effect_processor::event evt {effect_processor_events::effect_controls {e.ctrl}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const view_interface_events::overdrive_controls_changed &e)
{
    effect_processor::event evt {effect_processor_events::effect_controls {e.ctrl}};
    this->model.get()->send(evt);
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

    /* Register event handler for view(s) */
    this->view.get()->event_handler =
    [this](const view_interface_events::holder &e)
    {
        std::visit([this](const auto &e) { this->view_event_handler(e); }, e);
    };

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
