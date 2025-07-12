/*
 * controller.cpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#include "controller.hpp"

#include "middlewares/filesystem.hpp"

#include <array>
#include <algorithm>

#include <hal_system.hpp>

using namespace mfx;
namespace events = controller_events;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

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
    /* Forward this event dispatching to the controller thread (to avoid race conditions) */
    const controller::event evt {e};
    this->send(evt);
}

void controller::update(const lcd_view_events::outgoing &e)
{
    /* WARNING: This method could have been called from another thread */
    /* Forward this event dispatching to the controller thread (to avoid race conditions) */
    const controller::event evt {e};
    this->send(evt);
}

void controller::event_handler(const controller_events::initialize &e)
{
    /* Load & dump settings */
    if (!this->settings->load())
    {
        printf("Failed to load settings, using defaults\r\n");
        this->settings->restore_defaults();
    }

    printf("Settings:\r\n%s\r\n", settings->dump().data());

    /* Start observing model */
    this->model->attach(this);

    /* Start observing view(s) */
    this->view->attach(this);

    /* Configure view & model */
    this->model->send({effect_processor_events::configuration
                      {
                          this->settings->get_main_input_volume(),
                          this->settings->get_aux_input_volume(),
                          this->settings->get_output_volume(),
                          this->settings->get_output_muted(),
                          this->settings->get_mic_routed_to_aux(),
                      }});

    this->view->send({lcd_view_events::configuration
                     {
                         this->settings->get_dark_mode(),
                         this->settings->get_display_brightness(),
                         this->settings->get_main_input_volume(),
                         this->settings->get_aux_input_volume(),
                         this->settings->get_output_volume(),
                         this->settings->get_output_muted(),
                         this->settings->get_mic_routed_to_aux(),
                     }});

    this->settings->set_boot_counter(this->settings->get_boot_counter() + 1);

    /* Schedule periodic events */
    this->schedule({events::button_debounce {}}, 20, true);
    this->schedule({events::led_toggle {}}, 500, true);
    this->schedule({events::save_settings {}}, 1000, true);

    /* Start view & model */
    this->view->send({lcd_view_events::show_splash_screen {}});
    this->model->send({effect_processor_events::start_audio {}});
}

void controller::event_handler(const events::led_toggle &e)
{
    this->led.set(!this->led.get());

    const effect_processor::event evt {effect_processor_events::get_dsp_load {}};
    this->model->send(evt);
}

void controller::event_handler(const controller_events::button_debounce &e)
{
    static uint32_t press_time_ms = 0;
    static bool press_state = false;

    bool new_press_state = this->button.debounce(2); // debounce time: 2 * event period

    if (new_press_state != press_state)
    {
        press_time_ms = 0;
        press_state = new_press_state;
        auto btn_state = press_state ? events::button_state_changed::state::pressed : events::button_state_changed::state::released;
        this->send({events::button_state_changed {btn_state}});
    }

    constexpr uint32_t btn_hold_time_ms = 2000;
    if (press_state && press_time_ms < btn_hold_time_ms)
    {
        press_time_ms += 20;
        if (press_time_ms >= btn_hold_time_ms)
        {
            this->send({events::button_state_changed {events::button_state_changed::state::hold}});
        }
    }
}

void controller::event_handler(const events::button_state_changed &e)
{
    if (e.state == events::button_state_changed::state::pressed)
    {
        printf("Button pressed\r\n");
    }
    else if (e.state == events::button_state_changed::state::released)
    {
        printf("Button released\r\n");
    }
    else if (e.state == events::button_state_changed::state::hold)
    {
        printf("System shutdown\r\n");

        this->settings->save();
        this->view->send({lcd_view_events::shutdown {}});
        this->model->send({effect_processor_events::shutdown {}});

        osDelay(100);
        hal::system::stop();
        hal::system::reset();
    }
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

void controller::event_handler(const controller_events::save_settings &e)
{
    if (!this->settings->save())
    {
        printf("Failed to save settings\r\n");
    }
}

void controller::event_handler(const lcd_view_events::outgoing &e)
{
    std::visit([this](auto &&e) { this->view_event_handler(e); }, e);
}

void controller::event_handler(const effect_processor_events::outgoing &e)
{
    std::visit([this](auto &&e) { this->model_event_handler(e); }, e);
}

void controller::view_event_handler(const lcd_view_events::factory_reset &e)
{
    /* Do necessary things and restart */
    middlewares::filesystem::format();

    printf("System restart\r\n");

    this->view->send({lcd_view_events::shutdown {}});
    this->model->send({effect_processor_events::shutdown {}});

    osDelay(100);
    hal::system::reset();
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

void controller::view_event_handler(const lcd_view_events::theme_changed &e)
{
    this->settings->set_dark_mode(e.dark);
}

void controller::view_event_handler(const lcd_view_events::lcd_brightness_changed &e)
{
    this->settings->set_display_brightness(e.value);
}

void controller::view_event_handler(const lcd_view_events::input_volume_changed &e)
{
    const effect_processor::event evt {effect_processor_events::set_input_volume {e.main_input_vol, e.aux_input_vol}};
    this->model->send(evt);

    this->settings->set_main_input_volume(e.main_input_vol);
    this->settings->set_aux_input_volume(e.aux_input_vol);
}

void controller::view_event_handler(const lcd_view_events::output_volume_changed &e)
{
    const effect_processor::event evt {effect_processor_events::set_output_volume {e.output_vol}};
    this->model->send(evt);

    this->settings->set_output_volume(e.output_vol);
}

void controller::view_event_handler(const lcd_view_events::route_mic_to_aux_changed &e)
{
    const effect_processor::event evt {effect_processor_events::route_mic_to_aux {e.value}};
    this->model->send(evt);

    this->settings->set_mic_routed_to_aux(e.value);
}

void controller::view_event_handler(const lcd_view_events::mute_changed &e)
{
    const effect_processor::event evt {effect_processor_events::set_mute {e.value}};
    this->model->send(evt);

    this->settings->set_output_muted(e.value);
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
    this->view->send({lcd_view_events::update_dsp_load {e.load_pct}});
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

controller::controller(std::unique_ptr<effect_processor_base> model, std::unique_ptr<lcd_view> view, std::unique_ptr<settings_manager> settings) :
active_object("controller", osPriorityNormal, 2048),
error_code{0},
model {std::move(model)},
view {std::move(view)},
settings {std::move(settings)}
{
    this->send({events::initialize {}});
}

controller::~controller()
{

}
