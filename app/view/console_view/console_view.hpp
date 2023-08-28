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
#include <libs/spsc_queue.hpp>

#include <hal/hal_interface.hpp>

#include "app/view/view_interface.hpp"

namespace mfx
{

struct console_view_event
{
    struct char_queue_not_empty_evt_t
    {

    };

    using holder = std::variant<char_queue_not_empty_evt_t>;
};

class console_view : public view_interface, public console_view_event, public middlewares::active_object<console_view_event::holder>
{
public:
    console_view();
    ~console_view();

    void update(const data_holder &data) override;
private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const char_queue_not_empty_evt_t &e);

    void character_received_callback(const std::byte *data, std::size_t bytes_read);

    char received_char;
    spsc_queue<char, 16> char_queue;
    hal::interface::serial &stdio_serial;
};

}


#endif /* VIEW_CONSOLE_VIEW_HPP_ */
