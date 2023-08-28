/*
 * lcd_view.hpp
 *
 *  Created on: 21 lip 2023
 *      Author: kwarc
 */

#ifndef GUI_HPP_
#define GUI_HPP_

#include <variant>

#include <hal/hal_lcd.hpp>

#include "app/view/view_interface.hpp"

#include "middlewares/active_object.hpp"

namespace mfx
{

struct lcd_view_event
{
    struct timer_evt_t
    {

    };

    struct demo_test_evt_t
    {

    };

    using holder = std::variant<timer_evt_t, demo_test_evt_t>;
};

class lcd_view : public view_interface, public lcd_view_event, public middlewares::active_object<lcd_view_event::holder>
{
public:
    lcd_view();
private:
    void update(const data_holder &data) override;
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const timer_evt_t &e);
    void event_handler(const demo_test_evt_t &e);

    hal::displays::main display;
    osTimerId_t timer;
};

}

#endif /* GUI_HPP_ */
