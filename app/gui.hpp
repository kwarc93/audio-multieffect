/*
 * gui.hpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#ifndef GUI_HPP_
#define GUI_HPP_

#include <variant>

#include <hal/hal_lcd.hpp>

#include "middlewares/active_object.hpp"

struct gui_event
{
    struct timer_evt_t
    {

    };

    struct demo_test_evt_t
    {

    };

    using holder = std::variant<timer_evt_t, demo_test_evt_t>;
};

class gui : public gui_event, public active_object<gui_event::holder>
{
public:
    gui();
private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const timer_evt_t &e);
    void event_handler(const demo_test_evt_t &e);

    hal::lcd_tft lcd;
    osTimerId_t timer;
};


#endif /* GUI_HPP_ */
