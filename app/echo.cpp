/*
 * echo.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "echo.hpp"

#include <cstdio>

//-----------------------------------------------------------------------------
/* private */

void echo::dispatch(const event& e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void echo::event_handler(const char_queue_not_empty_evt_t& e)
{
    while (!this->char_queue.empty())
    {
        char c;
        if (this->char_queue.pop(c))
        {
            putchar(c);
            fflush(stdout);
        }
    }
}

void echo::event_handler(const button_evt_t& e)
{
    printf("Button pressed\n");
}

void echo::character_received_callback(const std::byte *data, std::size_t bytes_read)
{
    /* WARNING: This is called from interrupt */
    if (bytes_read == 0)
        return;

    const bool queue_was_empty = this->char_queue.empty();

    while (bytes_read--)
        this->char_queue.push(static_cast<char>(*(data++)));

    if (queue_was_empty)
    {
        static const echo::event e { echo_event::char_queue_not_empty_evt_t{}, echo::event::flags::immutable };
        this->send(e, 0);
    }
}

//-----------------------------------------------------------------------------
/* public */

echo::echo() : active_object("echo", osPriorityNormal, 1024), stdio_serial { hal::usart::stdio::get_instance() }
{
    /* Start listening for character */
    this->stdio_serial.listen(true);
    this->stdio_serial.read(reinterpret_cast<std::byte*>(&this->received_char),
                            1,
                            [this](const std::byte *data, std::size_t bytes_read)
                            {
                                 this->character_received_callback(data, bytes_read);
                            });
};

