/*
 * lcd_view.hpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#ifndef GUI_HPP_
#define GUI_HPP_

#include <variant>

#include <hal/hal_lcd.hpp>

#include <middlewares/active_object.hpp>
#include <middlewares/observer.hpp>

#include "app/model/effect_features.hpp"

namespace mfx
{

namespace lcd_view_events
{

struct timer
{

};

struct show_splash_screen
{

};

struct splash_loaded
{

};

struct show_blank_screen
{

};

struct show_next_effect_screen
{
    effect_id id;
};

struct show_prev_effect_screen
{
    effect_id id;
};

struct next_effect_screen_request
{

};

struct prev_effect_screen_request
{

};

struct input_volume_changed
{
    uint8_t main_input_vol;
    uint8_t aux_input_vol;
};

struct output_volume_changed
{
    uint8_t output_vol;
};

struct mute_changed
{
    bool value;
};

struct route_mic_to_aux_changed
{
    bool value;
};

struct effect_bypass_changed
{
    effect_id id;
    bool bypassed;
};

struct effect_controls_changed
{
    std::variant
    <
        tremolo_attr::controls,
        echo_attr::controls,
        chorus_attr::controls,
        reverb_attr::controls,
        overdrive_attr::controls,
        cabinet_sim_attr::controls,
        vocoder_attr::controls
    >
    ctrl;
};

struct add_effect_request
{
    effect_id id;
    effect_id curr_id;
};

struct remove_effect_request
{
    effect_id id;
};

struct move_effect_request
{
    effect_id id;
    int32_t step;
};

struct set_effect_attributes
{
    effect_attr basic;
    effect_specific_attributes specific;
};

struct update_processor_load
{
    uint8_t value;
};

using outgoing = std::variant
<
    splash_loaded,
    next_effect_screen_request,
    prev_effect_screen_request,
    input_volume_changed,
    output_volume_changed,
    mute_changed,
    route_mic_to_aux_changed,
    effect_bypass_changed,
    effect_controls_changed,
    add_effect_request,
    remove_effect_request,
    move_effect_request
>;

using incoming = std::variant
<
    timer,
    show_splash_screen,
    show_blank_screen,
    show_next_effect_screen,
    show_prev_effect_screen,
    set_effect_attributes,
    update_processor_load
>;

}

class lcd_view : public middlewares::active_object<lcd_view_events::incoming>,
                 public middlewares::subject<lcd_view_events::outgoing>
{
public:
    lcd_view();
private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const lcd_view_events::timer &e);
    void event_handler(const lcd_view_events::show_splash_screen &e);
    void event_handler(const lcd_view_events::show_blank_screen &e);
    void event_handler(const lcd_view_events::show_next_effect_screen &e);
    void event_handler(const lcd_view_events::show_prev_effect_screen &e);
    void event_handler(const lcd_view_events::set_effect_attributes &e);
    void event_handler(const lcd_view_events::update_processor_load &e);

    void set_effect_attr(const effect_attr &basic, const tremolo_attr &specific);
    void set_effect_attr(const effect_attr &basic, const echo_attr &specific);
    void set_effect_attr(const effect_attr &basic, const chorus_attr &specific);
    void set_effect_attr(const effect_attr &basic, const reverb_attr &specific);
    void set_effect_attr(const effect_attr &basic, const overdrive_attr &specific);
    void set_effect_attr(const effect_attr &basic, const cabinet_sim_attr &specific);
    void set_effect_attr(const effect_attr &basic, const vocoder_attr &specific);

    void change_effect_screen(effect_id id, int dir);

    hal::displays::main display;
    osTimerId_t timer;
};

}

#endif /* GUI_HPP_ */
