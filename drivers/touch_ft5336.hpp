/*
 * touch_ft5336.hpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */

#ifndef TOUCH_FT5336_HPP_
#define TOUCH_FT5336_HPP_

#include <hal/hal_interface.hpp>

namespace drivers
{

class touch_ft5336 : public hal::interface::touch_panel
{
public:
    static constexpr uint8_t i2c_address = 0b00111000;

    enum class orientation { normal, mirror_x, mirror_y, mirror_xy };

    touch_ft5336(hal::interface::i2c_device &dev, uint8_t addr, orientation ori);
    ~touch_ft5336();

    bool get_touch(int16_t &x, int16_t &y);
private:
    hal::interface::i2c_device &i2c_dev;
    const uint8_t i2c_addr;

    orientation orient;

    uint8_t read_reg(uint8_t reg_addr);
    void read_reg_burst(uint8_t reg_addr, std::byte *reg_val, std::size_t size);
    void write_reg(uint8_t reg_addr, uint8_t reg_val);

    uint8_t read_id(void);
    uint8_t detect_touch(void);
    void get_xy(uint16_t &x, uint16_t &y);
};

}

#endif /* TOUCH_FT5336_HPP_ */
