/*
 * hal_button.cpp
 *
 *  Created on: 5 paź 2021
 *      Author: kwarc
 */

#include "hal_button.hpp"

using namespace hal;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

button::button(hal::interface::button *interface) : interface {interface}
{
    this->pressed = false;
    this->debounce_state = 0;
}

void button::debounce(void)
{
    /* This works like FIFO of button states, shifts actual button state to MSB */
    this->debounce_state = (this->debounce_state << 1) | this->interface->is_pressed() | 0xFFFFFE00;
    if (this->debounce_state == 0xFFFFFF00)
        this->pressed = true;
}

bool button::is_pressed(void)
{
    return this->interface->is_pressed();
}

bool button::was_pressed(void)
{
    if (this->pressed)
    {
        this->pressed = false;
        return true;
    }
    else
    {
        return false;
    }
}

