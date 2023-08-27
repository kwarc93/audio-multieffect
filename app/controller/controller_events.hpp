/*
 * controller_events.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef CONTROLLER_CONTROLLER_EVENTS_HPP_
#define CONTROLLER_CONTROLLER_EVENTS_HPP_

#include <variant>

#include "app/effects/effect_types.hpp"

#include "app/effects/equalizer/equalizer.hpp"
#include "app/effects/noise_gate/noise_gate.hpp"
#include "app/effects/tremolo/tremolo.hpp"

struct controller_event
{
    struct button_evt_t
    {
        bool state;
    };

    struct effect_controls_evt_t
    {
        std::variant<equalizer::controls, noise_gate::controls, tremolo::controls> controls;
    };

    using holder = std::variant<button_evt_t, effect_controls_evt_t>;
};

#endif /* CONTROLLER_CONTROLLER_EVENTS_HPP_ */
