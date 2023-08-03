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
/* TODO*/
public:
    hw_i2c() {};
    std::byte read(uint8_t address) { return std::byte(0); };
    void write(uint8_t address, std::byte byte, bool no_stop) {};
    std::size_t read(uint8_t address, std::byte *data, std::size_t size) { return size; };
    std::size_t write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop) { return size; };
    void read(uint8_t address, std::byte *data, std::size_t size, const read_cb_t &callback) {};
    void write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop, const write_cb_t &callback) {};
};

}

#endif /* STM32F7_I2C_HPP_ */
