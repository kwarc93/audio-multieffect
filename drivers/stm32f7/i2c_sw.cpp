/*
 * i2c_sw.cpp
 *
 *  Created on: 3 sie 2023
 *      Author: kwarc
 */

#include "i2c_sw.hpp"

using namespace drivers;

i2c_sw::i2c_sw()
{
}

std::byte i2c_sw::read(uint8_t address)
{
    return std::byte(0);
}

void i2c_sw::write(uint8_t address, std::byte byte, bool no_stop)
{
}

std::size_t i2c_sw::read(uint8_t address, std::byte *data, std::size_t size)
{
    return size;
}

std::size_t i2c_sw::write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop)
{
    return size;
}

void i2c_sw::read(uint8_t address, std::byte *data, std::size_t size, const read_cb_t &callback)
{
}

void i2c_sw::write(uint8_t address, const std::byte *data, std::size_t size, bool no_stop, const write_cb_t &callback)
{
}




