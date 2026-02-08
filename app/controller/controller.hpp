/*
 * controller.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef CONTROLLER_CONTROLLER_HPP_
#define CONTROLLER_CONTROLLER_HPP_

#include <memory>
#include <vector>
#include <variant>

#include <hal_led.hpp>
#include <hal_button.hpp>

#include <middlewares/actor.hpp>

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/model/effect_processor.hpp"
#include "app/modules/settings.hpp"
#include "app/modules/presets.hpp"

namespace mfx
{

namespace controller_events
{

struct initialize
{

};

struct led_toggle
{

};

struct button_debounce
{

};

struct button_state_changed
{
    enum class state { released, pressed, hold } state;
};

struct save_settings
{

};

struct shutdown
{

};

struct restart
{

};

using incoming = std::variant
<
    initialize,
    led_toggle,
    button_debounce,
    button_state_changed,
    save_settings,
    shutdown,
    restart,

    effect_processor_events::outgoing,
    lcd_view_events::outgoing
>;

}

class controller : public middlewares::actor<controller_events::incoming>
{
public:
    controller(std::unique_ptr<effect_processor_base> model,
               std::unique_ptr<lcd_view> view,
               std::unique_ptr<settings_manager> settings,
               std::unique_ptr<presets_manager> presets);
    ~controller();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const controller_events::initialize &e);
    void event_handler(const controller_events::led_toggle &e);
    void event_handler(const controller_events::button_debounce &e);
    void event_handler(const controller_events::button_state_changed &e);
    void event_handler(const controller_events::save_settings &e);
    void event_handler(const controller_events::shutdown &e);
    void event_handler(const controller_events::restart &e);
    void event_handler(const lcd_view_events::outgoing &e);
    void event_handler(const effect_processor_events::outgoing &e);

    void view_event_handler(const lcd_view_events::factory_reset &e);
    void view_event_handler(const lcd_view_events::splash_loaded &e);
    void view_event_handler(const lcd_view_events::load_preset &e);
    void view_event_handler(const lcd_view_events::save_preset &e);
    void view_event_handler(const lcd_view_events::rename_preset &e);
    void view_event_handler(const lcd_view_events::remove_preset &e);
    void view_event_handler(const lcd_view_events::next_effect_screen_request &e);
    void view_event_handler(const lcd_view_events::prev_effect_screen_request &e);
    void view_event_handler(const lcd_view_events::theme_changed &e);
    void view_event_handler(const lcd_view_events::lcd_brightness_changed &e);
    void view_event_handler(const lcd_view_events::input_volume_changed &e);
    void view_event_handler(const lcd_view_events::output_volume_changed &e);
    void view_event_handler(const lcd_view_events::route_mic_to_aux_changed &e);
    void view_event_handler(const lcd_view_events::mute_changed &e);
    void view_event_handler(const lcd_view_events::usb_audio_if_changed &e);
    void view_event_handler(const lcd_view_events::usb_direct_mon_changed &e);
    void view_event_handler(const lcd_view_events::effect_bypass_changed &e);
    void view_event_handler(const lcd_view_events::effect_controls_changed &e);
    void view_event_handler(const lcd_view_events::add_effect_request &e);
    void view_event_handler(const lcd_view_events::remove_effect_request &e);
    void view_event_handler(const lcd_view_events::move_effect_request &e);

    void model_event_handler(const effect_processor_events::dsp_load_changed &e);
    void model_event_handler(const effect_processor_events::mute_changed &e);
    void model_event_handler(const effect_processor_events::input_volume_changed &e);
    void model_event_handler(const effect_processor_events::output_volume_changed &e);
    void model_event_handler(const effect_processor_events::effect_attributes_changed &e);
    void model_event_handler(const effect_processor_events::effect_attributes_enumerated &e);

    void update_effect_attributes(effect_id id);

    int error_code;

    std::unique_ptr<effect_processor_base> model;
    std::unique_ptr<lcd_view> view;
    std::unique_ptr<settings_manager> settings;
    std::unique_ptr<presets_manager> presets;

    effect_id current_effect;
    std::vector<effect_id> active_effects;

    hal::buttons::blue_btn button;
    hal::leds::debug led;
};

}

#endif /* CONTROLLER_CONTROLLER_HPP_ */
