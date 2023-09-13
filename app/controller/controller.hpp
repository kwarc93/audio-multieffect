/*
 * controller.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef CONTROLLER_CONTROLLER_HPP_
#define CONTROLLER_CONTROLLER_HPP_

#include <memory>

#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>

#include <middlewares/active_object.hpp>

#include "app/view/view_interface.hpp"
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

    struct effect_processor_load
    {
        uint8_t load;
    };

    using holder = std::variant<button, led, effect_processor_load>;
}

class controller : public middlewares::active_object<controller_events::holder>
{
public:
    controller(std::unique_ptr<effect_processor> model, std::unique_ptr<view_interface> view);
    ~controller();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const controller_events::led &e);
    void event_handler(const controller_events::button &e);
    void event_handler(const controller_events::effect_processor_load &e);

    void view_event_handler(const view_interface_events::settings_volume_changed &e);
    void view_event_handler(const view_interface_events::effect_bypass_changed &e);
    void view_event_handler(const view_interface_events::tremolo_controls_changed &e);
    void view_event_handler(const view_interface_events::echo_controls_changed &e);
    void view_event_handler(const view_interface_events::overdrive_controls_changed &e);


    int error_code;

    std::unique_ptr<effect_processor> model;
    std::unique_ptr<view_interface> view;

    hal::buttons::blue_btn button;
    osTimerId_t button_timer;

    hal::leds::debug led;
    osTimerId_t led_timer;
};

}

#endif /* CONTROLLER_CONTROLLER_HPP_ */
