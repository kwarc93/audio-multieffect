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

namespace lcd_view_events
{
    struct timer
    {

    };

    using holder = std::variant<timer>;
}

class lcd_view : public view_interface, public middlewares::active_object<lcd_view_events::holder>
{
public:
    lcd_view();
private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const lcd_view_events::timer &e);

    hal::displays::main display;
    osTimerId_t timer;
};

}

#endif /* GUI_HPP_ */
