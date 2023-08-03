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

class i2c : public hal::interface::i2c
{
/* TODO*/
public:
    i2c() {};
    std::byte read(void) { return std::byte(0); };
    void write(std::byte byte) {};
    std::size_t read(std::byte *data, std::size_t size) { return size; };
    std::size_t write(const std::byte *data, std::size_t size) { return size; };
    void read(std::byte *data, std::size_t size, const read_cb_t &callback) {};
    void write(const std::byte *data, std::size_t size, const write_cb_t &callback) {};
};

}

#endif /* STM32F7_I2C_HPP_ */
