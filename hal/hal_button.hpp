/*
 * hal_button.hpp
 *
 *  Created on: 5 paź 2021
 *      Author: kwarc
 */

#ifndef HAL_BUTTON_HPP_
#define HAL_BUTTON_HPP_

#include <hal/hal_interface.hpp>

#include <drivers/button_gpio.hpp>

namespace hal
{

//-----------------------------------------------------------------------------

    class button
    {
    public:
        button(hal::interface::button *interface);
        virtual ~button() {};
        virtual void debounce(void);
        bool is_pressed(void);
        bool was_pressed(void);
        bool was_released(void);
    protected:
        hal::interface::button *interface;
    private:
        bool pressed, released;
        uint32_t debounce_state;
    };

//-----------------------------------------------------------------------------

namespace buttons
{
    class blue_btn : public button
    {
    public:
        blue_btn(void) : button {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porti, drivers::gpio::pin::pin11 };
        drivers::button_gpio drv {io};
    };
}

//---------------------------------------------------------------------------

}

#endif /* HAL_BUTTON_HPP_ */
