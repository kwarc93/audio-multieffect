/*
 * flash.hpp
 *
 *  Created on: 8 gru 2020
 *      Author: kwarc
 */

#ifndef STM32F7_FLASH_HPP_
#define STM32F7_FLASH_HPP_

#include <cstdint>

namespace drivers
{

//--------------------------------------------------------------------------------

class flash final
{
public:
    flash() = delete;

    static void set_wait_states(uint32_t sysclk);
};

//--------------------------------------------------------------------------------

}

#endif /* STM32F7_FLASH_HPP_ */
