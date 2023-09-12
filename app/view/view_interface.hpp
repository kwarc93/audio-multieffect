/*
 * view_interface.hpp
 *
 *  Created on: 28 sie 2023
 *      Author: kwarc
 */

#ifndef VIEW_VIEW_INTERFACE_HPP_
#define VIEW_VIEW_INTERFACE_HPP_

#include <functional>
#include <variant>

#include "app/model/effect_features.hpp"

namespace mfx
{

/* Events sent by view */
namespace view_interface_events
{
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

    using holder = std::variant
    <
        settings_volume_changed,
        effect_bypass_changed,
        tremolo_controls_changed,
        echo_controls_changed,
        overdrive_controls_changed
    >;
}

class view_interface
{
public:
    typedef std::function<void(const view_interface_events::holder &e)> event_handler_t;

    virtual ~view_interface() {};

    event_handler_t event_handler;
};

}

#endif /* VIEW_VIEW_INTERFACE_HPP_ */
