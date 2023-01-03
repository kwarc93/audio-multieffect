/*
 * blinky.hpp
 *
 *  Created on: 2 sty 2023
 *      Author: kwarc
 */

#ifndef BLINKY_HPP_
#define BLINKY_HPP_

#include <variant>
#include <functional>

#include <hal/hal_led.hpp>

#include "middlewares/active_object.hpp"

struct blinky_event
{
    struct timer_evt_t
    {

    };

    struct button_evt_t
    {
        bool pressed;
    };

    using holder = std::variant<timer_evt_t, button_evt_t>;
};

class blinky : public blinky_event, public ao::active_object<blinky_event::holder>
{
public:
    blinky();
private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const timer_evt_t &e);
    void event_handler(const button_evt_t &e);

    hal::leds::debug led;
};

#endif /* BLINKY_HPP_ */
