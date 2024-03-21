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
public:
    struct i2c_hw;

    enum class id { i2c3 };
    enum class mode { master, slave };
    enum class speed { standard, fast };

    i2c(id id, mode mode, speed speed);
    ~i2c();
    void reset(void);
    std::byte read(void) override;
    void write(std::byte byte) override;
    std::size_t read(std::byte *data, std::size_t size) override;
    std::size_t write(const std::byte *data, std::size_t size) override;
    void read(std::byte *data, std::size_t size, const read_cb_t &callback) override;
    void write(const std::byte *data, std::size_t size, const write_cb_t &callback) override;
private:
    const i2c_hw &hw;
    mode operating_mode;
    speed bus_speed;
};

}

#endif /* STM32F7_I2C_HPP_ */
