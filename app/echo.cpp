/*
 * echo.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "echo.hpp"

#include <cstdio>

//-----------------------------------------------------------------------------
/* public */

echo::echo() : active_object("echo", osPriorityNormal, 1024), stdio_serial { hal::usart::stdio::get_instance() }
{
    /* Start listening for character */
    this->stdio_serial.read_async(reinterpret_cast<std::byte*>(&this->received_char), 1,
                                 [this](const std::byte *data, std::size_t bytes_read)
                                 {
                                        static const echo::event e {echo_event::char_received_evt_t{}, echo::event::flags::static_storage};
                                        this->send(e);
                                 }, true);
};

//-----------------------------------------------------------------------------
/* private */

void echo::dispatch(const event& e)
{
    std::visit([this](const auto &e) { return this->event_handler(e); }, e.data);
}

void echo::event_handler(const char_received_evt_t& e)
{
    putchar(this->received_char);
}

void echo::event_handler(const button_evt_t& e)
{
    printf("Button pressed\n");
}
