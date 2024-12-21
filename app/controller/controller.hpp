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

#include <middlewares/active_object.hpp>
#include <middlewares/observer.hpp>

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/model/effect_processor.hpp"

namespace mfx
{

namespace controller_events
{

struct led_toggle
{

};

struct button_state_changed
{
    enum class state { released, pressed, hold } state;
};

struct load_preset
{

};

struct dsp_load
{
    uint8_t load_pct;
};

using incoming = std::variant
<
    led_toggle,
    button_state_changed,
    dsp_load,
    load_preset
>;

}

class controller : public middlewares::active_object<controller_events::incoming>,
                   public middlewares::observer<effect_processor_events::outgoing>,
                   public middlewares::observer<lcd_view_events::outgoing>
{
public:
    controller(std::unique_ptr<effect_processor_base> model, std::unique_ptr<lcd_view> view);
    ~controller();

private:
    void dispatch(const event &e) override;
    void update(const effect_processor_events::outgoing &e) override;
    void update(const lcd_view_events::outgoing &e) override;

    /* Event handlers */
    void event_handler(const controller_events::led_toggle &e);
    void event_handler(const controller_events::button_state_changed &e);
    void event_handler(const controller_events::dsp_load &e);
    void event_handler(const controller_events::load_preset &e);

    void view_event_handler(const lcd_view_events::splash_loaded &e);
    void view_event_handler(const lcd_view_events::next_effect_screen_request &e);
    void view_event_handler(const lcd_view_events::prev_effect_screen_request &e);
    void view_event_handler(const lcd_view_events::input_volume_changed &e);
    void view_event_handler(const lcd_view_events::output_volume_changed &e);
    void view_event_handler(const lcd_view_events::route_mic_to_aux_changed &e);
    void view_event_handler(const lcd_view_events::mute_changed &e);
    void view_event_handler(const lcd_view_events::effect_bypass_changed &e);
    void view_event_handler(const lcd_view_events::effect_controls_changed &e);
    void view_event_handler(const lcd_view_events::add_effect_request &e);
    void view_event_handler(const lcd_view_events::remove_effect_request &e);
    void view_event_handler(const lcd_view_events::move_effect_request &e);

    void model_event_handler(const effect_processor_events::dsp_load_changed &e);
    void model_event_handler(const effect_processor_events::input_volume_changed &e);
    void model_event_handler(const effect_processor_events::output_volume_changed &e);
    void model_event_handler(const effect_processor_events::effect_attributes_changed &e);

    void update_effect_attributes(effect_id id);

    int error_code;

    std::unique_ptr<effect_processor_base> model;
    std::unique_ptr<lcd_view> view;

    effect_id current_effect;
    std::vector<effect_id> active_effects;

    hal::buttons::blue_btn button;
    osTimerId_t button_timer;

    hal::leds::debug led;
    osTimerId_t led_timer;
};

}

#endif /* CONTROLLER_CONTROLLER_HPP_ */
