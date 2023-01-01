/*
 * hal_usart.hpp
 *
 *  Created on: 24 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_USART_HPP_
#define HAL_USART_HPP_

#include <hal/hal_interface.hpp>

namespace hal::usart
{
    namespace stdio
    {
        hal::interface::serial & get_instance(void);
    }
}


#endif /* HAL_USART_HPP_ */
