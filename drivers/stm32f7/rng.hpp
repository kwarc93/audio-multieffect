/*
 * rng.hpp
 *
 *  Created on: 12 mar 2024
 *      Author: kwarc
 */

#ifndef STM32F7_RNG_HPP_
#define STM32F7_RNG_HPP_

#include <cstdint>

namespace drivers
{

//--------------------------------------------------------------------------------

class rng final
{
public:
    rng() = delete;

    static void enable(bool state);
    static uint32_t get(void);
};

//--------------------------------------------------------------------------------

}


#endif /* STM32F7_RNG_HPP_ */
