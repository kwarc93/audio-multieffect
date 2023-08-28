/*
 * controller.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef CONTROLLER_CONTROLLER_HPP_
#define CONTROLLER_CONTROLLER_HPP_

#include <middlewares/active_object.hpp>

#include <memory>

#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>

#include "controller_events.hpp"

namespace mfx
{

class controller : public controller_event, public middlewares::active_object<controller_event::holder>
{
public:
    controller();
    ~controller();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const led_evt_t &e);
    void event_handler(const button_evt_t &e);
    void event_handler(const effect_controls_evt_t &e);

    hal::buttons::blue_btn button;
    osTimerId_t button_timer;

    hal::leds::debug led;
    osTimerId_t led_timer;
};

}

#endif /* CONTROLLER_CONTROLLER_HPP_ */
