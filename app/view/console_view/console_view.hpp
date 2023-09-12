/*
 * console_view.hpp
 *
 *  Created on: 28 sie 2023
 *      Author: kwarc
 */

#ifndef VIEW_CONSOLE_VIEW_HPP_
#define VIEW_CONSOLE_VIEW_HPP_


#include <cstdint>
#include <variant>

#include <middlewares/active_object.hpp>
#include <libs/fast_queue.hpp>

#include <hal/hal_interface.hpp>

#include "app/view/view_interface.hpp"

namespace mfx
{

namespace console_view_events
{
    struct char_queue_not_empty
    {

    };

    using holder = std::variant<char_queue_not_empty>;
}

class console_view : public view_interface, public middlewares::active_object<console_view_events::holder>
{
public:
    console_view();
    ~console_view();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const console_view_events::char_queue_not_empty &e);

    void character_received_callback(const std::byte *data, std::size_t bytes_read);

    char received_char;
    libs::fast_queue<char, 16> char_queue;
    hal::interface::serial &stdio_serial;
};

}


#endif /* VIEW_CONSOLE_VIEW_HPP_ */
