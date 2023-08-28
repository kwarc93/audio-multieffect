/*
 * controller_events.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef CONTROLLER_CONTROLLER_EVENTS_HPP_
#define CONTROLLER_CONTROLLER_EVENTS_HPP_

#include <variant>

#include "app/model/data_types.hpp"

#include "app/model/equalizer/equalizer.hpp"
#include "app/model/noise_gate/noise_gate.hpp"
#include "app/model/tremolo/tremolo.hpp"

namespace mfx
{

struct controller_event
{
    struct led_evt_t
    {

    };

    struct button_evt_t
    {
        bool state;
    };

    struct effect_controls_evt_t
    {
        std::variant<equalizer::controls, noise_gate::controls, tremolo::controls> controls;
    };

    using holder = std::variant<button_evt_t, led_evt_t, effect_controls_evt_t>;
};

}

#endif /* CONTROLLER_CONTROLLER_EVENTS_HPP_ */
