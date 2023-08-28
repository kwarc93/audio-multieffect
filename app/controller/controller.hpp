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

#include "controller_events.hpp"

namespace mfx
{

class controller : public controller_event, public middlewares::active_object<controller_event::holder>
{
public:
    controller(std::unique_ptr<effect_processor> model, std::unique_ptr<view_interface> view);
    ~controller();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const led_evt_t &e);
    void event_handler(const button_evt_t &e);
    void event_handler(const effect_controls_evt_t &e);

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
