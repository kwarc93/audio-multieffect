/*
 * hal_sdram.hpp
 *
 *  Created on: 17 lip 2023
 *      Author: kwarc
 */

#ifndef HAL_SDRAM_HPP_
#define HAL_SDRAM_HPP_

#include <cstddef>

namespace hal::sdram
{
    void init(void);
    void *start_addr(void);
    std::size_t size(void);
}

#endif /* HAL_SDRAM_HPP_ */
