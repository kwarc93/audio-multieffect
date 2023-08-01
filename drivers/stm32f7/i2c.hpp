/*
 * i2c.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef STM32F7_I2C_HPP_
#define STM32F7_I2C_HPP_

#include <hal/hal_interface.hpp>

#include <cstdint>

namespace drivers
{

class hw_i2c : public hal::interface::i2c
{

};

}

#endif /* STM32F7_I2C_HPP_ */
