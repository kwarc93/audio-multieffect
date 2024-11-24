/*
 * touch_ft5336.cpp
 *
 *  Created on: 1 sie 2023
 *      Author: kwarc
 */


#include "touch_ft5336.hpp"

#include <cassert>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

/* Current mode register of the FT5336 (R/W) */
#define FT5336_DEV_MODE_REG         0x00U

/* Gesture ID register */
#define FT5336_GEST_ID_REG          0x01U

/* Touch Data Status register : gives number of active touch points (0..2) */
#define FT5336_TD_STAT_REG          0x02U
#define FT5336_TD_STAT_MASK         0x0FU
#define FT5336_TD_STAT_SHIFT        0x00U

/* P1 X, Y coordinates, weight and misc registers */
#define FT5336_P1_XH_REG            0x03U
#define FT5336_P1_XL_REG            0x04U
#define FT5336_P1_YH_REG            0x05U
#define FT5336_P1_YL_REG            0x06U
#define FT5336_P1_WEIGHT_REG        0x07U
#define FT5336_P1_MISC_REG          0x08U

/* P2 X, Y coordinates, weight and misc registers */
#define FT5336_P2_XH_REG            0x09U
#define FT5336_P2_XL_REG            0x0AU
#define FT5336_P2_YH_REG            0x0BU
#define FT5336_P2_YL_REG            0x0CU
#define FT5336_P2_WEIGHT_REG        0x0DU
#define FT5336_P2_MISC_REG          0x0EU

/* P3 X, Y coordinates, weight and misc registers */
#define FT5336_P3_XH_REG            0x0FU
#define FT5336_P3_XL_REG            0x10U
#define FT5336_P3_YH_REG            0x11U
#define FT5336_P3_YL_REG            0x12U
#define FT5336_P3_WEIGHT_REG        0x13U
#define FT5336_P3_MISC_REG          0x14U

/* P4 X, Y coordinates, weight and misc registers */
#define FT5336_P4_XH_REG            0x15U
#define FT5336_P4_XL_REG            0x16U
#define FT5336_P4_YH_REG            0x17U
#define FT5336_P4_YL_REG            0x18U
#define FT5336_P4_WEIGHT_REG        0x19U
#define FT5336_P4_MISC_REG          0x1AU

/* P5 X, Y coordinates, weight and misc registers */
#define FT5336_P5_XH_REG            0x1BU
#define FT5336_P5_XL_REG            0x1CU
#define FT5336_P5_YH_REG            0x1DU
#define FT5336_P5_YL_REG            0x1EU
#define FT5336_P5_WEIGHT_REG        0x1FU
#define FT5336_P5_MISC_REG          0x20U

/* P6 X, Y coordinates, weight and misc registers */
#define FT5336_P6_XH_REG            0x21U
#define FT5336_P6_XL_REG            0x22U
#define FT5336_P6_YH_REG            0x23U
#define FT5336_P6_YL_REG            0x24U
#define FT5336_P6_WEIGHT_REG        0x25U
#define FT5336_P6_MISC_REG          0x26U

/* P7 X, Y coordinates, weight and misc registers */
#define FT5336_P7_XH_REG            0x27U
#define FT5336_P7_XL_REG            0x28U
#define FT5336_P7_YH_REG            0x29U
#define FT5336_P7_YL_REG            0x2AU
#define FT5336_P7_WEIGHT_REG        0x2BU
#define FT5336_P7_MISC_REG          0x2CU

/* P8 X, Y coordinates, weight and misc registers */
#define FT5336_P8_XH_REG            0x2DU
#define FT5336_P8_XL_REG            0x2EU
#define FT5336_P8_YH_REG            0x2FU
#define FT5336_P8_YL_REG            0x30U
#define FT5336_P8_WEIGHT_REG        0x31U
#define FT5336_P8_MISC_REG          0x32U

/* P9 X, Y coordinates, weight and misc registers */
#define FT5336_P9_XH_REG            0x33U
#define FT5336_P9_XL_REG            0x34U
#define FT5336_P9_YH_REG            0x35U
#define FT5336_P9_YL_REG            0x36U
#define FT5336_P9_WEIGHT_REG        0x37U
#define FT5336_P9_MISC_REG          0x38U

/* P10 X, Y coordinates, weight and misc registers */
#define FT5336_P10_XH_REG           0x39U
#define FT5336_P10_XL_REG           0x3AU
#define FT5336_P10_YH_REG           0x3BU
#define FT5336_P10_YL_REG           0x3CU
#define FT5336_P10_WEIGHT_REG       0x3DU
#define FT5336_P10_MISC_REG         0x3EU

/* Values Pn_XL and Pn_YL related */
#define FT5336_TOUCH_POS_MSB_MASK   0x0FU
#define FT5336_TOUCH_POS_MSB_SHIFT  0x00U

#define FT5336_TOUCH_POS_LSB_MASK   0xFFU
#define FT5336_TOUCH_POS_LSB_SHIFT  0x00U

/* Threshold for touch detection */
#define FT5336_TH_GROUP_REG         0x80

/* Filter function coefficients */
#define FT5336_TH_DIFF_REG          0x85

/* Control register */
#define FT5336_CTRL_REG             0x86

/* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT5336_TIMEENTERMONITOR_REG 0x87

/* Report rate in Active mode */
#define FT5336_PERIODACTIVE_REG     0x88

/* Report rate in Monitor mode */
#define FT5336_PERIODMONITOR_REG    0x89

/* The value of the minimum allowed angle while Rotating gesture mode */
#define FT5336_RADIAN_VALUE_REG     0x91

/* Maximum offset while Moving Left and Moving Right gesture */
#define FT5336_OFFSET_LR_REG        0x92

/* Maximum offset while Moving Up and Moving Down gesture */
#define FT5336_OFFSET_UD_REG        0x93

/* Minimum distance while Moving Left and Moving Right gesture */
#define FT5336_DISTANCE_LR_REG      0x94

/* Minimum distance while Moving Up and Moving Down gesture */
#define FT5336_DISTANCE_UD_REG      0x95

/* Maximum distance while Zoom In and Zoom Out gesture */
#define FT5336_DISTANCE_ZOOM_REG    0x96

/* High 8-bit of LIB Version info */
#define FT5336_LIB_VER_H_REG        0xA1

/* Low 8-bit of LIB Version info */
#define FT5336_LIB_VER_L_REG        0xA2

/* Chip Selecting */
#define FT5336_CIPHER_REG           0xA3

/* Interrupt mode register (used when in interrupt mode) */
#define FT5336_GMODE_REG            0xA4

/* Current power mode the FT5336 system is in (R) */
#define FT5336_PWR_MODE_REG         0xA5

/* FT5336 firmware version */
#define FT5336_FIRMID_REG           0xA6

/* FT5336 Chip identification register */
#define FT5336_CHIP_ID_REG          0xA8

/* Release code version */
#define FT5336_RELEASE_CODE_ID_REG  0xAF

/* Current operating mode the FT5336 system is in (R) */
#define FT5336_STATE_REG            0xBC

//-----------------------------------------------------------------------------

/* Max detectable simultaneous touches */
#define FT5336_MAX_NB_TOUCH              5U

/* Touch FT5336 IDs */
#define FT5336_ID                        0x51U

/* Possible values of FT5336_DEV_MODE_REG */
#define FT5336_DEV_MODE_WORKING          0x00U
#define FT5336_DEV_MODE_FACTORY          0x04U

/* Possible values of FT5336_GEST_ID_REG */
#define FT5336_GEST_ID_NO_GESTURE        0x00U
#define FT5336_GEST_ID_MOVE_UP           0x10U
#define FT5336_GEST_ID_MOVE_RIGHT        0x14U
#define FT5336_GEST_ID_MOVE_DOWN         0x18U
#define FT5336_GEST_ID_MOVE_LEFT         0x1CU
#define FT5336_GEST_ID_ZOOM_IN           0x48U
#define FT5336_GEST_ID_ZOOM_OUT          0x49U

/* Values Pn_XH and Pn_YH related */
#define FT5336_TOUCH_EVT_FLAG_PRESS_DOWN 0x00U
#define FT5336_TOUCH_EVT_FLAG_LIFT_UP    0x01U
#define FT5336_TOUCH_EVT_FLAG_CONTACT    0x02U
#define FT5336_TOUCH_EVT_FLAG_NO_EVENT   0x03U

/* Possible values of FT5336_GMODE_REG */
#define FT5336_G_MODE_INTERRUPT_POLLING  0x00U
#define FT5336_G_MODE_INTERRUPT_TRIGGER  0x01U

//-----------------------------------------------------------------------------
/* private */

uint8_t touch_ft5336::read_reg(uint8_t reg_addr)
{
    using transfer_desc = hal::interface::i2c_proxy::transfer_desc;

    uint8_t reg_val {0};

    transfer_desc desc
    {
        this->i2c_addr,
        reinterpret_cast<const std::byte*>(&reg_addr),
        sizeof(reg_addr),
        reinterpret_cast<std::byte*>(&reg_val),
        sizeof(reg_val)
    };

    this->i2c.transfer(desc);
    assert(desc.stat == transfer_desc::status::ok);

    return reg_val;
}

void touch_ft5336::write_reg(uint8_t reg_addr, uint8_t reg_val)
{
    using transfr_desc = hal::interface::i2c_proxy::transfer_desc;

    std::array<uint8_t, 2> tx {{ reg_addr, reg_val }};

    transfr_desc desc
    {
        this->i2c_addr,
        reinterpret_cast<const std::byte*>(tx.data()),
        sizeof(tx),
    };

    this->i2c.transfer(desc);
    assert(desc.stat == transfr_desc::status::ok);
}

void touch_ft5336::read_reg_burst(uint8_t reg_addr, std::byte *reg_val, std::size_t size)
{
    using transfer_desc = hal::interface::i2c_proxy::transfer_desc;

    transfer_desc desc
    {
        this->i2c_addr,
        reinterpret_cast<const std::byte*>(&reg_addr),
        sizeof(reg_addr),
        reg_val,
        size
    };

    this->i2c.transfer(desc);
    assert(desc.stat == transfer_desc::status::ok);
}

uint8_t touch_ft5336::read_id(void)
{
    return this->read_reg(FT5336_CHIP_ID_REG);
}

uint8_t touch_ft5336::detect_touch(void)
{
    /* TODO: Add support for multi-touch */

    uint8_t touch_points = 0;

    switch (this->td_mode)
    {
    case touch_detect_mode::reg_poll:
        touch_points = this->read_reg(FT5336_TD_STAT_REG) & FT5336_TD_STAT_MASK;
        break;
    case touch_detect_mode::int_poll:
        touch_points = !this->get_int_status();
        break;
    case touch_detect_mode::int_trigg:
        if (this->int_detected)
        {
            this->int_detected = false;
            touch_points = 1;
        }
        break;
    default:
        break;
    }

    /* If invalid number of touch detected, set it to zero */
    return touch_points > FT5336_MAX_NB_TOUCH ? 0 : touch_points;
}

bool touch_ft5336::get_int_status(void)
{
    return gpio::read(this->int_io);
}

void touch_ft5336::get_xy(uint16_t &x, uint16_t &y)
{
    /* TODO: Add support for multi-touch */

    struct xy
    {
        uint8_t xh, xl, yh, yl;
    };

    xy regs {0, 0, 0, 0};
    this->read_reg_burst(FT5336_P1_XH_REG, reinterpret_cast<std::byte*>(&regs), sizeof(regs));
    x = (regs.xh & FT5336_TOUCH_POS_MSB_MASK) << 8 | (regs.xl & FT5336_TOUCH_POS_LSB_MASK);
    y = (regs.yh & FT5336_TOUCH_POS_MSB_MASK) << 8 | (regs.yl & FT5336_TOUCH_POS_LSB_MASK);
}

//-----------------------------------------------------------------------------
/* public */

touch_ft5336::touch_ft5336(hal::interface::i2c_proxy &i2c, uint8_t addr, const gpio::io &int_io, touch_ft5336::orientation ori) :
i2c {i2c}, i2c_addr {addr}, int_io {int_io}, orient {ori}, td_mode {touch_detect_mode::reg_poll}
{
    /* Wait at least 200ms after power up before accessing registers
       Trsi timing (Time of starting to report point after resetting) from FT5336GQQ datasheet */
    delay::ms(200);
    uint8_t id = this->read_id();
    assert(id == FT5336_ID);

    /* Check INT pin mode: poll or trigger */
    const uint8_t int_mode = /*this->read_reg(FT5336_GMODE_REG)*/3;

    switch (int_mode)
    {
        case 0:
            this->td_mode = touch_detect_mode::int_poll;
            gpio::configure(this->int_io, gpio::mode::input, gpio::af::af0, gpio::pupd::pu);
            break;
        case 1:
            this->td_mode = touch_detect_mode::int_trigg;
            gpio::configure(this->int_io, gpio::mode::input, gpio::af::af0, gpio::pupd::pu);
            exti::configure(true,
                            static_cast<exti::line>(this->int_io.pin),
                            static_cast<exti::port>(this->int_io.port),
                            exti::mode::interrupt,
                            exti::edge::falling,
                            [this](){ this->int_detected = true; });
            break;
        default:
            break;
    }
}

touch_ft5336::~touch_ft5336()
{

}

bool touch_ft5336::get_touch(int16_t &x, int16_t &y)
{
    if (this->detect_touch() == 0)
        return false;

    uint16_t raw_x, raw_y;
    this->get_xy(raw_x, raw_y);

    switch (this->orient)
    {
    case orientation::normal:
    default:
        x = raw_x;
        y = raw_y;
        break;
    case orientation::mirror_x:
        x = 480 - raw_x;
        break;
    case orientation::mirror_y:
        y = 272 - raw_y;
        break;
    case orientation::mirror_xy:
        y = raw_x;
        x = raw_y;
        break;
    }

    return true;
}
