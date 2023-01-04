/*
 * echo.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef ECHO_HPP_
#define ECHO_HPP_

#include <variant>

#include <hal/hal_usart.hpp>

#include "middlewares/active_object.hpp"

struct echo_event
{
    struct char_received_evt_t
    {

    };

    struct button_evt_t
    {

    };

    using holder = std::variant<char_received_evt_t, button_evt_t>;
};

class echo : public echo_event, public ao::active_object<echo_event::holder>
{
public:
    echo();
private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const char_received_evt_t &e);
    void event_handler(const button_evt_t &e);

    hal::interface::serial &stdio_serial;
    char received_char;
};


#endif /* ECHO_HPP_ */
