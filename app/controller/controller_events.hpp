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

#include "app/effects/equalizer/data_types.hpp"
#include "app/effects/compressor/data_types.hpp"
#include "app/effects/reverb/data_types.hpp"

struct controller_event
{
    struct button_evt_t
    {
        bool state;
    };

    struct effect_controls_evt_t
    {
        std::variant<equalizer::controls_t, reverb::controls_t, compressor::controls_t> controls;
    };

    using holder = std::variant<button_evt_t, effect_controls_evt_t>;
};

#endif /* CONTROLLER_CONTROLLER_EVENTS_HPP_ */
