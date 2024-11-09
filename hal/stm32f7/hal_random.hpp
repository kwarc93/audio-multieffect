/*
 * hal_random.hpp
 *
 *  Created on: 12 mar 2024
 *      Author: kwarc
 */

#ifndef HAL_RANDOM_HPP_
#define HAL_RANDOM_HPP_

#include <cstdint>

namespace hal::random
{
    void enable(bool state);
    uint32_t get(void);
}

#endif /* HAL_RANDOM_HPP_ */
