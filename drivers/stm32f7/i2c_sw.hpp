/*
 * i2c_sw.hpp
 *
 *  Created on: 3 sie 2023
 *      Author: kwarc
 */

#ifndef STM32F7_I2C_SW_HPP_
#define STM32F7_I2C_SW_HPP_

#include <hal/hal_interface.hpp>

#include <cstdint>

namespace drivers
{

class i2c_sw : public hal::interface::i2c
{
public:
    i2c_sw();
    std::byte read(uint8_t address);
    void write(uint8_t address, std::byte byte, bool no_stop);
    std::size_t read(uint8_t address, std::byte *data, std::size_t size);
    std::size_t write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop);
    void read(uint8_t address, std::byte *data, std::size_t size, const read_cb_t &callback);
    void write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop, const write_cb_t &callback);
};

}

#endif /* STM32F7_I2C_SW_HPP_ */
