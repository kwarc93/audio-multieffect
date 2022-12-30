/*
 * hal.hpp
 *
 *  Created on: 20 paź 2020
 *      Author: kwarc
 */

#ifndef HAL_SYSTEM_HPP_
#define HAL_SYSTEM_HPP_

#include <cmsis/stm32f7xx.h>

namespace hal::system
{
    static constexpr uint32_t hsi_clock = 16000000;
    static constexpr uint32_t hse_clock = 25000000;
    static constexpr uint32_t system_clock = 50000000;
    static constexpr uint32_t systick_freq = 1000;

    void init(void);
}


#endif /* HAL_SYSTEM_HPP_ */
