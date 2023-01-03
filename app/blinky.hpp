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

namespace blinky_evt
{

struct timer_evt_t
{

};

struct button_evt_t
{
    bool pressed;
};

using blinky_evt_t = std::variant<timer_evt_t, button_evt_t>;

}

class blinky : public ao::active_object<blinky_evt::blinky_evt_t>
{
public:
    using event_t = ao::active_object<blinky_evt::blinky_evt_t>::base_evt_t;
    blinky();
private:
    void dispatch(const blinky_evt::blinky_evt_t &e) override;

    /* Event handlers */
    void event_handler(const blinky_evt::timer_evt_t &e);
    void event_handler(const blinky_evt::button_evt_t &e);

    hal::leds::debug led;
};

#endif /* BLINKY_HPP_ */
