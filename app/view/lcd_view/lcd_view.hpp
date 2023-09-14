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

struct show_effect_screen
{
    effect_id id;
};

struct add_effect_screen
{
    effect_id id;
};

struct settings_volume_changed
{
    uint8_t input_vol;
    uint8_t output_vol;
};

struct effect_bypass_changed
{
    effect_id id;
    bool bypassed;
};

struct tremolo_controls_changed
{
    tremolo_attributes::controls ctrl;
};

struct echo_controls_changed
{
    echo_attributes::controls ctrl;
};

struct overdrive_controls_changed
{
    overdrive_attributes::controls ctrl;
};

using outgoing = std::variant
<
    splash_loaded,
    settings_volume_changed,
    effect_bypass_changed,
    tremolo_controls_changed,
    echo_controls_changed,
    overdrive_controls_changed
>;

using incoming = std::variant
<
    timer,
    show_splash_screen,
    show_effect_screen,
    add_effect_screen
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
    void event_handler(const lcd_view_events::show_effect_screen &e);
    void event_handler(const lcd_view_events::add_effect_screen &e);

    hal::displays::main display;
    osTimerId_t timer;

    std::vector<effect_id> effects_screens;
};

}

#endif /* GUI_HPP_ */
