/*
 * hal_i2c.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef HAL_I2C_HPP_
#define HAL_I2C_HPP_

#include <hal_interface.hpp>

namespace hal::i2c
{
    constexpr bool use_software_i2c = false;

    namespace main
    {
        hal::interface::i2c & get_instance(void);
    }
}

#endif /* HAL_I2C_HPP_ */
