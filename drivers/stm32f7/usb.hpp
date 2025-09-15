/*
 * usb.hpp
 *
 *  Created on: 15 wrz 2025
 *      Author: kwarc
 */

#ifndef STM32F7_USB_HPP_
#define STM32F7_USB_HPP_


namespace drivers
{

class usb final
{
public:
    usb() = delete;

    enum class usb_type { fs, hs };

    static void init(usb_type type);
};

}

#endif /* STM32F7_USB_HPP_ */
