/*
 * hal_random_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32H7_HAL_RANDOM_IMPL_HPP_
#define STM32H7_HAL_RANDOM_IMPL_HPP_

#include <cstdint>

#include <drivers/stm32h7/rng.hpp>

namespace hal::random
{
    inline void enable(bool state)
    {
        drivers::rng::enable(state);
    }

    inline uint32_t get(void)
    {
        return drivers::rng::get();
    }
}

#endif /* STM32H7_HAL_RANDOM_IMPL_HPP_ */
