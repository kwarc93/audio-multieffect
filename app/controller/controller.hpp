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

#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>

#include <middlewares/active_object.hpp>
#include <middlewares/observer.hpp>

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/model/effect_processor.hpp"

namespace mfx
{

namespace controller_events
{

struct led
{

};

struct button
{
    bool state;
};

struct load_preset
{

};

struct effect_processor_load
{
    uint8_t load;
};

using incoming = std::variant
<
    button,
    led,
    effect_processor_load,
    load_preset
>;

}

class controller : public middlewares::active_object<controller_events::incoming>,
                   public middlewares::observer<effect_processor_events::outgoing>,
                   public middlewares::observer<lcd_view_events::outgoing>
{
public:
    controller(std::unique_ptr<effect_processor> model, std::unique_ptr<lcd_view> view);
    ~controller();

private:
    void dispatch(const event &e) override;
    void update(const effect_processor_events::outgoing &e) override;
    void update(const lcd_view_events::outgoing &e) override;

    /* Event handlers */
    void event_handler(const controller_events::led &e);
    void event_handler(const controller_events::button &e);
    void event_handler(const controller_events::effect_processor_load &e);
    void event_handler(const controller_events::load_preset &e);

    void view_event_handler(const lcd_view_events::splash_loaded &e);
    void view_event_handler(const lcd_view_events::next_effect_screen_request &e);
    void view_event_handler(const lcd_view_events::prev_effect_screen_request &e);
    void view_event_handler(const lcd_view_events::settings_volume_changed &e);
    void view_event_handler(const lcd_view_events::effect_bypass_changed &e);
    void view_event_handler(const lcd_view_events::tremolo_controls_changed &e);
    void view_event_handler(const lcd_view_events::echo_controls_changed &e);
    void view_event_handler(const lcd_view_events::overdrive_controls_changed &e);

    void model_event_handler(const effect_processor_events::bypass &e);
    void model_event_handler(const effect_processor_events::volume &e);
    void model_event_handler(const effect_processor_events::effect_attr &e);

    void effect_attr_handler(const tremolo_attributes &attr);
    void effect_attr_handler(const echo_attributes &attr);
    void effect_attr_handler(const overdrive_attributes &attr);
    void effect_attr_handler(const cabinet_sim_attributes &attr);

    int error_code;

    std::unique_ptr<effect_processor> model;
    std::unique_ptr<lcd_view> view;

    std::vector<effect_id> active_effects;
    std::vector<effect_id>::iterator current_effect;

    hal::buttons::blue_btn button;
    osTimerId_t button_timer;

    hal::leds::debug led;
    osTimerId_t led_timer;
};

}

#endif /* CONTROLLER_CONTROLLER_HPP_ */
