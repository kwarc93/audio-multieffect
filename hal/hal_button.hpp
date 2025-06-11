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
            this->state = false;
            this->debounce_cnt = 0;
        }

        virtual ~button()
        {

        }

        virtual bool debounce(uint8_t count)
        {
            const bool pressed = this->interface->is_pressed();

            if (pressed != this->state)
            {
                if (++this->debounce_cnt >= count) this->state = pressed;
            }
            else
            {
                this->debounce_cnt = 0;
            }

            return this->state;
        }

        bool is_pressed(void)
        {
            return this->interface->is_pressed();
        }

    protected:
        hal::interface::button *interface;
    private:
        bool state;
        uint8_t debounce_cnt;
    };
}

#include <hal_button_impl.hpp>

#endif /* HAL_BUTTON_HPP_ */
