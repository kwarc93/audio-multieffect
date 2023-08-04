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

class touch_ft5336
{
public:
    /* Max detectable simultaneous touches */
    static constexpr uint8_t FT5336_MAX_NB_TOUCH              = 5;

    /* Touch FT5336 IDs */
    static constexpr uint8_t FT5336_ID                        = 0x51;

    /* Possible values of FT5336_DEV_MODE_REG */
    static constexpr uint8_t FT5336_DEV_MODE_WORKING          = 0x00;
    static constexpr uint8_t FT5336_DEV_MODE_FACTORY          = 0x04;

    /* Possible values of FT5336_GEST_ID_REG */
    static constexpr uint8_t FT5336_GEST_ID_NO_GESTURE        = 0x00;
    static constexpr uint8_t FT5336_GEST_ID_MOVE_UP           = 0x10;
    static constexpr uint8_t FT5336_GEST_ID_MOVE_RIGHT        = 0x14;
    static constexpr uint8_t FT5336_GEST_ID_MOVE_DOWN         = 0x18;
    static constexpr uint8_t FT5336_GEST_ID_MOVE_LEFT         = 0x1C;
    static constexpr uint8_t FT5336_GEST_ID_ZOOM_IN           = 0x48;
    static constexpr uint8_t FT5336_GEST_ID_ZOOM_OUT          = 0x49;

    /* Values Pn_XH and Pn_YH related */
    static constexpr uint8_t FT5336_TOUCH_EVT_FLAG_PRESS_DOWN = 0x00;
    static constexpr uint8_t FT5336_TOUCH_EVT_FLAG_LIFT_UP    = 0x01;
    static constexpr uint8_t FT5336_TOUCH_EVT_FLAG_CONTACT    = 0x02;
    static constexpr uint8_t FT5336_TOUCH_EVT_FLAG_NO_EVENT   = 0x03;

    /* Possible values of FT5336_GMODE_REG */
    static constexpr uint8_t FT5336_G_MODE_INTERRUPT_POLLING  = 0x00;
    static constexpr uint8_t FT5336_G_MODE_INTERRUPT_TRIGGER  = 0x01;

    touch_ft5336(hal::interface::i2c_device &dev, uint8_t addr = 0b00111000);
    ~touch_ft5336();
    uint8_t read_id(void);
private:
    hal::interface::i2c_device &device;
    uint8_t address;

    uint8_t read_reg(uint8_t reg_addr);
    void write_reg(uint8_t reg_addr, uint8_t reg_val);
};

}

#endif /* TOUCH_FT5336_HPP_ */
