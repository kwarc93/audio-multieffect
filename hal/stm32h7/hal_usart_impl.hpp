/*
 * hal_usart_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_USART_IMPL_HPP_
#define STM32H7_HAL_USART_IMPL_HPP_

#include <drivers/stm32h7/usart.hpp>

namespace hal::usart
{
   namespace stdio
   {
        inline interface::serial & get_instance(void)
        {
            static drivers::usart usart3 { drivers::usart::id::usart3, 115200 };
            return usart3;
        }
   }
}

#endif /* STM32H7_HAL_USART_IMPL_HPP_ */
