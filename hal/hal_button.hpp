/*
 * hal_button.hpp
 *
 *  Created on: 5 paź 2021
 *      Author: kwarc
 */

#ifndef HAL_BUTTON_HPP_
#define HAL_BUTTON_HPP_

#include <hal_interface.hpp>

namespace hal
{
    class button
    {
    public:
        button(hal::interface::button *interface) : interface {interface}
        {
            this->released = this->pressed = false;
            this->debounce_state = 0;
        }

        virtual ~button()
        {

        }

        virtual void debounce(void)
        {
            // This works like FIFO of button states, shifts actual button state bit from LSB to MSB
            //          _      ____
            // MSB < __| |____|    |_______ < LSB
            //                  ^      ^
            //                  |      |
            //        pressed --+      +-- released

            constexpr uint32_t ignore_mask = 0xFFFFFFE0;
            /* Release debounce time: 3 <bits> * <loop period> */
            constexpr uint32_t release_mask = 0xFFFFFFF8;
            /* Press debounce time: 2 <bit> * <loop period> */
            constexpr uint32_t press_mask = 0xFFFFFFE3;

            this->debounce_state = (this->debounce_state << 1) | this->interface->is_pressed() | ignore_mask;

            if (this->debounce_state == release_mask)
                this->released = true;
            else if (this->debounce_state == press_mask)
                this->pressed = true;
        }

        bool is_pressed(void)
        {
            return this->interface->is_pressed();
        }

        bool was_pressed(void)
        {
            bool state = this->released;
            this->released = false;
            return state;
        }

        bool was_released(void)
        {
            bool state = this->pressed;
            this->pressed = false;
            return state;
        }
    protected:
        hal::interface::button *interface;
    private:
        bool pressed, released;
        uint32_t debounce_state;
    };
}

#include <hal_button_impl.hpp>

#endif /* HAL_BUTTON_HPP_ */
