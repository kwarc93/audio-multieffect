/*
 * console_view.cpp
 *
 *  Created on: 28 sie 2023
 *      Author: kwarc
 */

#include "console_view.hpp"

#include <cmath>
#include <cstdio>

#include <app/utils.hpp>

#include <hal_usart.hpp>

using namespace mfx;
namespace events = console_view_events;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------
/* private */

void console_view::character_received_callback(const std::byte *data, std::size_t bytes_read)
{
    /* WARNING: This is called from interrupt */

    if (bytes_read == 0)
        return;

    const bool queue_was_empty = this->char_queue.empty();

    while (bytes_read--)
        this->char_queue.push(static_cast<char>(*(data++)));

    if (queue_was_empty)
    {
        static const console_view::event e { events::char_queue_not_empty{}, true };
        this->send(e);
    }
}

void console_view::dispatch(const event &e)
{
    std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
}

void console_view::event_handler(const events::char_queue_not_empty &e)
{
    /* TODO: Add command line parser */
}

//-----------------------------------------------------------------------------
/* public */

console_view::console_view() : actor("console_view", configTASK_PRIO_NORMAL, 1024), stdio_serial { hal::usart::stdio::get_instance() }
{
    /* Start listening for character */
    this->stdio_serial.listen(true);
    this->stdio_serial.read(reinterpret_cast<std::byte*>(&this->received_char),
                            1,
                            [this](auto... params)
                            {
                                 this->character_received_callback(params...);
                            });
};

console_view::~console_view()
{

}


