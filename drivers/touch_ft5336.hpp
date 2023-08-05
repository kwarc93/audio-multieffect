/*
 * touch_ft5336.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef TOUCH_FT5336_HPP_
#define TOUCH_FT5336_HPP_

#include <bitset>

#include <hal/hal_interface.hpp>

namespace drivers
{

class touch_ft5336 : public hal::interface::touch_panel
{
public:
    static constexpr uint8_t default_i2c_address = 0b00111000;

    enum class orient { normal, swap_x, swap_y, swap_xy };

    touch_ft5336(hal::interface::i2c_device &dev, uint8_t addr);
    ~touch_ft5336();

    void configure(uint16_t x_size, uint16_t y_size, orient orientation);
    bool get_touch(int16_t &x, int16_t &y);
private:
    hal::interface::i2c_device &device;

    const uint8_t address;
    uint16_t x_size;
    uint16_t y_size;
    orient orientation;

    uint8_t read_reg(uint8_t reg_addr);
    void write_reg(uint8_t reg_addr, uint8_t reg_val);

    uint8_t read_id(void);
};

}

#endif /* TOUCH_FT5336_HPP_ */
