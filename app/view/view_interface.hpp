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

#include <app/model/effect_features.hpp>

namespace mfx
{

namespace view_interface_events
{
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

    void set_event_handler(const event_handler_t &h) { this->send_event = h; };
protected:
    event_handler_t send_event;
};

}

#endif /* VIEW_VIEW_INTERFACE_HPP_ */
