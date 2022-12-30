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

//---------------------------------------------------------------------------

    class button
    {
    public:
        button(hal::interface::button *interface);
        virtual ~button() {};
        virtual void debounce(void);
        bool is_pressed(void);
        bool was_pressed(void);
    protected:
        hal::interface::button *interface;
    private:
        bool pressed;
        uint32_t debounce_state;
    };

//-----------------------------------------------------------------------------

namespace buttons
{
    class center_btn : public button
    {
    public:
        center_btn(void) : button {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porta, drivers::gpio::pin::pin0 };
        drivers::button_gpio drv {io};
    };

    class up_btn : public button
    {
    public:
        up_btn(void) : button {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porta, drivers::gpio::pin::pin3 };
        drivers::button_gpio drv {io};
    };

    class down_btn : public button
    {
    public:
        down_btn(void) : button {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porta, drivers::gpio::pin::pin5 };
        drivers::button_gpio drv {io};
    };

    class left_btn : public button
    {
    public:
        left_btn(void) : button {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porta, drivers::gpio::pin::pin1 };
        drivers::button_gpio drv {io};
    };

    class right_btn : public button
    {
    public:
        right_btn(void) : button {&drv} {}
    private:
        const drivers::gpio::io io = { drivers::gpio::port::porta, drivers::gpio::pin::pin2 };
        drivers::button_gpio drv {io};
    };
}

//---------------------------------------------------------------------------

}

#endif /* HAL_BUTTON_HPP_ */
