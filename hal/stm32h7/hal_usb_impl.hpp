/*
 * hal_usb_impl.hpp
 *
 *  Created on: 15 wrz 2025
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_USB_IMPL_HPP_
#define STM32H7_HAL_USB_IMPL_HPP_

#include <drivers/stm32h7/usb.hpp>

namespace hal::usbd /* USB Device */
{
    inline void init_fs(void)
    {
        drivers::usb::init(drivers::usb::usb_type::fs);
    }

    inline void init_hs(void)
    {
        drivers::usb::init(drivers::usb::usb_type::hs);
    }
}

#endif /* STM32H7_HAL_USB_IMPL_HPP_ */
