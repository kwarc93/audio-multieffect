/*
 * hal_usart_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_USART_IMPL_HPP_
#define STM32F7_HAL_USART_IMPL_HPP_

#include <drivers/stm32f7/usart.hpp>

namespace hal::usart
{
   namespace stdio
   {
        inline interface::serial & get_instance(void)
        {
            static drivers::usart usart1 { drivers::usart::id::usart1, 115200 };
            return usart1;
        }
   }
}

#endif /* STM32F7_HAL_USART_IMPL_HPP_ */
