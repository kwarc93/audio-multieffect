/*
 * controller.cpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#include "controller.hpp"

#include <array>
#include <algorithm>

#include <hal_system.hpp>

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

    static uint32_t press_time_ms;
    static controller::event e { events::button_state_changed {}, controller::event::flags::immutable };

    if (button->was_pressed())
    {
        std::get<events::button_state_changed>(e.data).state = events::button_state_changed::state::pressed;
        controller::instance->send(e);
        press_time_ms = 0;
    }
    else if (button->was_released())
    {
        std::get<events::button_state_changed>(e.data).state = events::button_state_changed::state::released;
        controller::instance->send(e);
        press_time_ms = 0;
    }

    if (button->is_pressed() && press_time_ms < 2000)
    {
        press_time_ms += 20;
        if (press_time_ms >= 2000)
        {
            std::get<events::button_state_changed>(e.data).state = events::button_state_changed::state::hold;
            controller::instance->send(e);
        }
    }
}

void led_timer_cb(void *arg)
{
    if (arg == nullptr)
        return;

    auto *ctrl = static_cast<controller*>(arg);

    static const controller::event e { events::led_toggle {}, controller::event::flags::immutable };
    ctrl->send(e);
}

}

//-----------------------------------------------------------------------------
/* private */

void controller::dispatch(const event& e)
{
    std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
}

void controller::update(const effect_processor_events::outgoing &e)
{
    /* WARNING: This method could have been called from another thread */
    /* FIXME: Forward this event dispatching to the controller thread (to avoid race conditions) */

    std::visit([this](auto &&e) { this->model_event_handler(e); }, e);
}

void controller::update(const lcd_view_events::outgoing &e)
{
    /* WARNING: This method could have been called from another thread */
    /* FIXME: Forward this event dispatching to the controller thread (to avoid race conditions) */

    std::visit([this](auto &&e) { this->view_event_handler(e); }, e);
}

void controller::event_handler(const events::led_toggle &e)
{
    this->led.set(!this->led.get());

    const effect_processor::event evt {effect_processor_events::get_dsp_load {}};
    this->model->send(evt);
}

void controller::event_handler(const events::button_state_changed &e)
{
    if (e.state == events::button_state_changed::state::hold)
    {
        this->view->send({lcd_view_events::shutdown {}});
        this->model->send({effect_processor_events::shutdown {}});
        this->led.set(false);

        printf("System shutdown\r\n");
        osDelay(100);
        hal::system::stop();
        hal::system::reset();
    }
}

void controller::event_handler(const events::dsp_load &e)
{
    this->view->send({lcd_view_events::update_dsp_load {e.load_pct}});
}

void controller::event_handler(const events::load_preset &e)
{
    /* TODO: Load last active preset ('pedal board') of effects */

    /* Example of simple preset (order is important!) */
    static constexpr std::array<effect_id, 4> preset =
    {{
        effect_id::overdrive,
        effect_id::tremolo,
        effect_id::echo,
        effect_id::cabinet_sim,
    }};

    for (auto &&p : preset)
    {
        this->active_effects.push_back(p);
        this->model->send({effect_processor_events::add_effect {p}});
    }

    /* Show screen of the first effect in preset */
    this->current_effect = this->active_effects.at(0);
    this->view->send({lcd_view_events::show_next_effect_screen {this->current_effect}});
    this->update_effect_attributes(this->current_effect);
}

void controller::view_event_handler(const lcd_view_events::splash_loaded &e)
{
    this->view->send({lcd_view_events::show_blank_screen {}});
}

void controller::view_event_handler(const lcd_view_events::next_effect_screen_request &e)
{
    const auto it = std::find(this->active_effects.begin(), this->active_effects.end(), this->current_effect);
    if (it >= (this->active_effects.end() - 1))
        return;

    this->current_effect = *std::next(it);
    this->view->send({lcd_view_events::show_next_effect_screen {this->current_effect}});
    this->update_effect_attributes(this->current_effect);
}

void controller::view_event_handler(const lcd_view_events::prev_effect_screen_request &e)
{
    const auto it = std::find(this->active_effects.begin(), this->active_effects.end(), this->current_effect);
    if (it == this->active_effects.begin())
        return;

    this->current_effect = *std::prev(it);
    this->view->send({lcd_view_events::show_prev_effect_screen {this->current_effect}});
    this->update_effect_attributes(this->current_effect);
}

void controller::view_event_handler(const lcd_view_events::input_volume_changed &e)
{
    const effect_processor::event evt {effect_processor_events::set_input_volume {e.main_input_vol, e.aux_input_vol}};
    this->model->send(evt);
}

void controller::view_event_handler(const lcd_view_events::output_volume_changed &e)
{
    const effect_processor::event evt {effect_processor_events::set_output_volume {e.output_vol}};
    this->model->send(evt);
}

void controller::view_event_handler(const lcd_view_events::route_mic_to_aux_changed &e)
{
    const effect_processor::event evt {effect_processor_events::route_mic_to_aux {e.value}};
    this->model->send(evt);
}

void controller::view_event_handler(const lcd_view_events::mute_changed &e)
{
    const effect_processor::event evt {effect_processor_events::set_mute {e.value}};
    this->model->send(evt);
}

void controller::view_event_handler(const lcd_view_events::effect_bypass_changed &e)
{
    const effect_processor::event evt {effect_processor_events::bypass_effect {e.id, e.bypassed}};
    this->model->send(evt);
}

void controller::view_event_handler(const lcd_view_events::effect_controls_changed &e)
{
    effect_processor::event evt {effect_processor_events::set_effect_controls {e.ctrl}};
    this->model->send(evt);
}

void controller::view_event_handler(const lcd_view_events::add_effect_request &e)
{
    this->current_effect = e.id;
    this->active_effects.push_back(e.id);
    this->model->send({effect_processor_events::add_effect {e.id}});
    this->view->send({lcd_view_events::show_next_effect_screen {this->current_effect}});
    this->update_effect_attributes(this->current_effect);
}

void controller::view_event_handler(const lcd_view_events::remove_effect_request &e)
{
    if (e.id == this->current_effect)
    {
        /* Select new current effect */
        auto it = std::find(this->active_effects.begin(), this->active_effects.end(), e.id);

        if (it == this->active_effects.begin())
            it = std::next(it);
        else
            it = std::prev(it);

        if (this->active_effects.size() == 1)
        {
            this->view->send({lcd_view_events::show_blank_screen {}});
        }
        else
        {
            this->current_effect = *it;
            this->view->send({lcd_view_events::show_prev_effect_screen {this->current_effect}});
            this->update_effect_attributes(this->current_effect);
        }
    }

    this->active_effects.erase(std::remove(this->active_effects.begin(), this->active_effects.end(), e.id), this->active_effects.end());
    this->model->send({effect_processor_events::remove_effect {e.id}});
}

void controller::view_event_handler(const lcd_view_events::move_effect_request &e)
{
    auto it = std::find(this->active_effects.begin(), this->active_effects.end(), e.id);

    if (it == this->active_effects.begin() && e.step < 0)
        return;

    if (it == (this->active_effects.end() - 1) && e.step > 0)
        return;

    /* Only moves by +1/-1 are supported */
    std::swap(*it, *std::next(it, std::clamp(e.step, -1L, 1L)));

    this->model->send({effect_processor_events::move_effect {e.id, e.step}});
}

void controller::model_event_handler(const effect_processor_events::dsp_load_changed &e)
{
    const controller::event evt {events::dsp_load {e.load_pct}};
    this->send(evt);
}

void controller::model_event_handler(const effect_processor_events::input_volume_changed &e)
{
    /* TODO */
}

void controller::model_event_handler(const effect_processor_events::output_volume_changed &e)
{
    /* TODO */
}

void controller::model_event_handler(const effect_processor_events::effect_attributes_changed &e)
{
    lcd_view::event evt {lcd_view_events::set_effect_attributes {e.basic, e.specific}};
    this->view->send(evt);
}

void controller::update_effect_attributes(effect_id id)
{
    const effect_processor::event e {effect_processor_events::get_effect_attributes {id}};
    this->model->send(e);
}

//-----------------------------------------------------------------------------
/* public */

controller::controller(std::unique_ptr<effect_processor_base> model, std::unique_ptr<lcd_view> view) :
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
    this->model->attach(this);

    /* Start observing view(s) */
    this->view->attach(this);

    /* Show splash screen */
    this->view->send({lcd_view_events::show_splash_screen {}});
}

controller::~controller()
{

}
