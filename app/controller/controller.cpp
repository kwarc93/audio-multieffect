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

void controller::update(const effect_processor_events::outgoing &e)
{
    /* WARINING: This method could have been called from another thread */

    std::visit([this](const auto &e) { this->model_event_handler(e); }, e);
}

void controller::update(const lcd_view_events::outgoing &e)
{
    /* WARINING: This method could have been called from another thread */

    std::visit([this](const auto &e) { this->view_event_handler(e); }, e);
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

void controller::event_handler(const events::load_preset &e)
{
    /* TODO: Load last active preset ('pedal board') of effects */

    /* Example of simple preset (order is important!) */
    static const std::array<effect_id, 4> preset =
    {{
        effect_id::overdrive,
        effect_id::tremolo,
        effect_id::echo,
        effect_id::cabinet_sim,
    }};

    for (const auto &p : preset)
    {
        this->view.get()->send({lcd_view_events::add_effect_screen {p}});
        this->model.get()->send({effect_processor_events::add_effect {p}});
    }

    this->view.get()->send({lcd_view_events::show_effect_screen {preset[0]}});
}

void controller::view_event_handler(const lcd_view_events::splash_loaded &e)
{
    this->send({events::load_preset{}});
}

void controller::view_event_handler(const lcd_view_events::settings_volume_changed &e)
{
    const effect_processor::event evt {effect_processor_events::volume {e.input_vol, e.output_vol}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const lcd_view_events::effect_bypass_changed &e)
{
    const effect_processor::event evt {effect_processor_events::bypass {e.id, e.bypassed}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const lcd_view_events::tremolo_controls_changed &e)
{
    effect_processor::event evt {effect_processor_events::effect_controls {e.ctrl}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const lcd_view_events::echo_controls_changed &e)
{
    effect_processor::event evt {effect_processor_events::effect_controls {e.ctrl}};
    this->model.get()->send(evt);
}

void controller::view_event_handler(const lcd_view_events::overdrive_controls_changed &e)
{
    effect_processor::event evt {effect_processor_events::effect_controls {e.ctrl}};
    this->model.get()->send(evt);
}

void controller::model_event_handler(const effect_processor_events::bypass &e)
{

}

void controller::model_event_handler(const effect_processor_events::volume &e)
{

}

void controller::model_event_handler(const effect_processor_events::effect_attr &e)
{
    std::visit([this](const auto &attr) { this->effect_attr_handler(attr); }, e);
}


void controller::effect_attr_handler(const tremolo_attributes &attr)
{

}

void controller::effect_attr_handler(const echo_attributes &attr)
{

}

void controller::effect_attr_handler(const overdrive_attributes &attr)
{

}

void controller::effect_attr_handler(const cabinet_sim_attributes &attr)
{

}

//-----------------------------------------------------------------------------
/* public */

controller::controller(std::unique_ptr<effect_processor> model, std::unique_ptr<lcd_view> view) :
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

    /* Start observing model */
    this->model.get()->attach(this);

    /* Start observing view(s) */
    this->view.get()->attach(this);

    /* Show splash screen */
    this->view.get()->send({lcd_view_events::show_splash_screen {}});
}

controller::~controller()
{

}
