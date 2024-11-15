/*
 * audio_wm8994ecs.cpp
 *
 *  Created on: 11 sie 2023
 *      Author: kwarc
 */


#include "audio_wm8994ecs.hpp"

#include <cassert>
#include <algorithm>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------

/*
 * In W8994 codec the audio frame contains 4 slots:
 * +-----------------------------------------------------------------------------+
 * | CODEC_SLOT0 Left | CODEC_SLOT1 Left | CODEC_SLOT0 Right | CODEC_SLOT1 Right |
 * +-----------------------------------------------------------------------------+
 */

#define WM8994_SLOTS_NUMBER           4

//-----------------------------------------------------------------------------

/* SW Reset/Chip ID */
#define WM8994_SW_RESET               (uint16_t)0x0000

/* Power Management */
#define WM8994_PWR_MANAGEMENT_1       (uint16_t)0x0001
#define WM8994_PWR_MANAGEMENT_2       (uint16_t)0x0002
#define WM8994_PWR_MANAGEMENT_3       (uint16_t)0x0003
#define WM8994_PWR_MANAGEMENT_4       (uint16_t)0x0004
#define WM8994_PWR_MANAGEMENT_5       (uint16_t)0x0005
#define WM8994_PWR_MANAGEMENT_6       (uint16_t)0x0006

/* Input mixer */
#define WM8994_INPUT_MIXER_1          (uint16_t)0x0015
/* Input volume */
#define WM8994_LEFT_LINE_IN12_VOL     (uint16_t)0x0018
#define WM8994_LEFT_LINE_IN34_VOL     (uint16_t)0x0019
#define WM8994_RIGHT_LINE_IN12_VOL    (uint16_t)0x001A
#define WM8994_RIGHT_LINE_IN34_VOL    (uint16_t)0x001B

/* L/R Output volumes */
#define WM8994_LEFT_OUTPUT_VOL        (uint16_t)0x001C
#define WM8994_RIGHT_OUTPUT_VOL       (uint16_t)0x001D
#define WM8994_LINE_OUTPUT_VOL        (uint16_t)0x001E
#define WM8994_OUTPUT2_VOL            (uint16_t)0x001F


/* L/R OPGA volumes */
#define WM8994_LEFT_OPGA_VOL          (uint16_t)0x0020
#define WM8994_RIGHT_OPGA_VOL         (uint16_t)0x0021

/* SPKMIXL/R Attenuation */
#define WM8994_SPKMIXL_ATT            (uint16_t)0x0022
#define WM8994_SPKMIXR_ATT            (uint16_t)0x0023
#define WM8994_OUTPUT_MIXER           (uint16_t)0x0024
#define WM8994_CLASS_D                (uint16_t)0x0025
/* L/R Speakers volumes */
#define WM8994_SPK_LEFT_VOL           (uint16_t)0x0026
#define WM8994_SPK_RIGHT_VOL          (uint16_t)0x0027

/* Input mixer */
#define WM8994_INPUT_MIXER_2          (uint16_t)0x0028
#define WM8994_INPUT_MIXER_3          (uint16_t)0x0029
#define WM8994_INPUT_MIXER_4          (uint16_t)0x002A
#define WM8994_INPUT_MIXER_5          (uint16_t)0x002B
#define WM8994_INPUT_MIXER_6          (uint16_t)0x002C

/* Output mixer */
#define WM8994_OUTPUT_MIXER_1         (uint16_t)0x002D
#define WM8994_OUTPUT_MIXER_2         (uint16_t)0x002E
#define WM8994_OUTPUT_MIXER_3         (uint16_t)0x002F
#define WM8994_OUTPUT_MIXER_4         (uint16_t)0x0030
#define WM8994_OUTPUT_MIXER_5         (uint16_t)0x0031
#define WM8994_OUTPUT_MIXER_6         (uint16_t)0x0032
#define WM8994_OUTPUT2_MIXER          (uint16_t)0x0033
#define WM8994_LINE_MIXER_1           (uint16_t)0x0034
#define WM8994_LINE_MIXER_2           (uint16_t)0x0035
#define WM8994_SPEAKER_MIXER          (uint16_t)0x0036
#define WM8994_ADD_CONTROL            (uint16_t)0x0037
/* Antipop */
#define WM8994_ANTIPOP1               (uint16_t)0x0038
#define WM8994_ANTIPOP2               (uint16_t)0x0039
#define WM8994_MICBIAS                (uint16_t)0x003A
#define WM8994_LDO1                   (uint16_t)0x003B
#define WM8994_LDO2                   (uint16_t)0x003C

/* Charge pump */
#define WM8994_CHARGE_PUMP1           (uint16_t)0x004C
#define WM8994_CHARGE_PUMP2           (uint16_t)0x004D

#define WM8994_CLASS_W                (uint16_t)0x0051

#define WM8994_DC_SERVO1              (uint16_t)0x0054
#define WM8994_DC_SERVO2              (uint16_t)0x0055
#define WM8994_DC_SERVO_READBACK      (uint16_t)0x0058
#define WM8994_DC_SERVO_WRITEVAL      (uint16_t)0x0059

/* Analog HP */
#define WM8994_ANALOG_HP              (uint16_t)0x0060

#define WM8994_CHIP_REVISION          (uint16_t)0x0100
#define WM8994_CONTROL_INTERFACE      (uint16_t)0x0101
#define WM8994_WRITE_SEQ_CTRL1        (uint16_t)0x0110
#define WM8994_WRITE_SEQ_CTRL2        (uint16_t)0x0111

/* WM8994 clocking */
#define WM8994_AIF1_CLOCKING1         (uint16_t)0x0200
#define WM8994_AIF1_CLOCKING2         (uint16_t)0x0201
#define WM8994_AIF2_CLOCKING1         (uint16_t)0x0204
#define WM8994_AIF2_CLOCKING2         (uint16_t)0x0205
#define WM8994_CLOCKING1              (uint16_t)0x0208
#define WM8994_CLOCKING2              (uint16_t)0x0209
#define WM8994_AIF1_RATE              (uint16_t)0x0210
#define WM8994_AIF2_RATE              (uint16_t)0x0211
#define WM8994_RATE_STATUS            (uint16_t)0x0212

/* FLL1 Control */
#define WM8994_FLL1_CONTROL1          (uint16_t)0x0220
#define WM8994_FLL1_CONTROL2          (uint16_t)0x0221
#define WM8994_FLL1_CONTROL3          (uint16_t)0x0222
#define WM8994_FLL1_CONTROL4          (uint16_t)0x0223
#define WM8994_FLL1_CONTROL5          (uint16_t)0x0224

/* FLL2 Control */
#define WM8994_FLL2_CONTROL1          (uint16_t)0x0240
#define WM8994_FLL2_CONTROL2          (uint16_t)0x0241
#define WM8994_FLL2_CONTROL3          (uint16_t)0x0242
#define WM8994_FLL2_CONTROL4          (uint16_t)0x0243
#define WM8994_FLL2_CONTROL5          (uint16_t)0x0244


/* AIF1 control */
#define WM8994_AIF1_CONTROL1          (uint16_t)0x0300
#define WM8994_AIF1_CONTROL2          (uint16_t)0x0301
#define WM8994_AIF1_MASTER_SLAVE      (uint16_t)0x0302
#define WM8994_AIF1_BCLK              (uint16_t)0x0303
#define WM8994_AIF1_ADC_LRCLK         (uint16_t)0x0304
#define WM8994_AIF1_DAC_LRCLK         (uint16_t)0x0305
#define WM8994_AIF1_DAC_DELTA         (uint16_t)0x0306
#define WM8994_AIF1_ADC_DELTA         (uint16_t)0x0307

/* AIF2 control */
#define WM8994_AIF2_CONTROL1          (uint16_t)0x0310
#define WM8994_AIF2_CONTROL2          (uint16_t)0x0311
#define WM8994_AIF2_MASTER_SLAVE      (uint16_t)0x0312
#define WM8994_AIF2_BCLK              (uint16_t)0x0313
#define WM8994_AIF2_ADC_LRCLK         (uint16_t)0x0314
#define WM8994_AIF2_DAC_LRCLK         (uint16_t)0x0315
#define WM8994_AIF2_DAC_DELTA         (uint16_t)0x0316
#define WM8994_AIF2_ADC_DELTA         (uint16_t)0x0317

/* AIF1 ADC/DAC LR volumes */
#define WM8994_AIF1_ADC1_LEFT_VOL     (uint16_t)0x0400
#define WM8994_AIF1_ADC1_RIGHT_VOL    (uint16_t)0x0401
#define WM8994_AIF1_DAC1_LEFT_VOL     (uint16_t)0x0402
#define WM8994_AIF1_DAC1_RIGHT_VOL    (uint16_t)0x0403
#define WM8994_AIF1_ADC2_LEFT_VOL     (uint16_t)0x0404
#define WM8994_AIF1_ADC2_RIGHT_VOL    (uint16_t)0x0405
#define WM8994_AIF1_DAC2_LEFT_VOL     (uint16_t)0x0406
#define WM8994_AIF1_DAC2_RIGHT_VOL    (uint16_t)0x0407

/* AIF1 ADC/DAC filters */
#define WM8994_AIF1_ADC1_FILTERS      (uint16_t)0x0410
#define WM8994_AIF1_ADC2_FILTERS      (uint16_t)0x0411
#define WM8994_AIF1_DAC1_FILTER1      (uint16_t)0x0420
#define WM8994_AIF1_DAC1_FILTER2      (uint16_t)0x0421
#define WM8994_AIF1_DAC2_FILTER1      (uint16_t)0x0422
#define WM8994_AIF1_DAC2_FILTER2      (uint16_t)0x0423

/* AIF1 DRC1 registers */
#define WM8994_AIF1_DRC1              (uint16_t)0x0440
#define WM8994_AIF1_DRC1_1            (uint16_t)0x0441
#define WM8994_AIF1_DRC1_2            (uint16_t)0x0442
#define WM8994_AIF1_DRC1_3            (uint16_t)0x0443
#define WM8994_AIF1_DRC1_4            (uint16_t)0x0444
/* AIF1 DRC2 registers */
#define WM8994_AIF1_DRC2              (uint16_t)0x0450
#define WM8994_AIF1_DRC2_1            (uint16_t)0x0451
#define WM8994_AIF1_DRC2_2            (uint16_t)0x0452
#define WM8994_AIF1_DRC2_3            (uint16_t)0x0453
#define WM8994_AIF1_DRC2_4            (uint16_t)0x0454

/* AIF1 DAC1 EQ Gains Bands */
#define WM8994_AIF1_DAC1_EQG_1        (uint16_t)0x0480
#define WM8994_AIF1_DAC1_EQG_2        (uint16_t)0x0481
#define WM8994_AIF1_DAC1_EQG_1A       (uint16_t)0x0482
#define WM8994_AIF1_DAC1_EQG_1B       (uint16_t)0x0483
#define WM8994_AIF1_DAC1_EQG_1PG      (uint16_t)0x0484
#define WM8994_AIF1_DAC1_EQG_2A       (uint16_t)0x0485
#define WM8994_AIF1_DAC1_EQG_2B       (uint16_t)0x0486
#define WM8994_AIF1_DAC1_EQG_2C       (uint16_t)0x0487
#define WM8994_AIF1_DAC1_EQG_2PG      (uint16_t)0x0488
#define WM8994_AIF1_DAC1_EQG_3A       (uint16_t)0x0489
#define WM8994_AIF1_DAC1_EQG_3B       (uint16_t)0x048A
#define WM8994_AIF1_DAC1_EQG_3C       (uint16_t)0x048B
#define WM8994_AIF1_DAC1_EQG_3PG      (uint16_t)0x048C
#define WM8994_AIF1_DAC1_EQG_4A       (uint16_t)0x048D
#define WM8994_AIF1_DAC1_EQG_4B       (uint16_t)0x048E
#define WM8994_AIF1_DAC1_EQG_4C       (uint16_t)0x048F
#define WM8994_AIF1_DAC1_EQG_4PG      (uint16_t)0x0490
#define WM8994_AIF1_DAC1_EQG_5A       (uint16_t)0x0491
#define WM8994_AIF1_DAC1_EQG_5B       (uint16_t)0x0492
#define WM8994_AIF1_DAC1_EQG_5PG      (uint16_t)0x0493

/* AIF1 DAC2 EQ Gains/bands */
#define WM8994_AIF1_DAC2_EQG_1        (uint16_t)0x04A0
#define WM8994_AIF1_DAC2_EQG_2        (uint16_t)0x04A1
#define WM8994_AIF1_DAC2_EQG_1A       (uint16_t)0x04A2
#define WM8994_AIF1_DAC2_EQG_1B       (uint16_t)0x04A3
#define WM8994_AIF1_DAC2_EQG_1PG      (uint16_t)0x04A4
#define WM8994_AIF1_DAC2_EQG_2A       (uint16_t)0x04A5
#define WM8994_AIF1_DAC2_EQG_2B       (uint16_t)0x04A6
#define WM8994_AIF1_DAC2_EQG_2C       (uint16_t)0x04A7
#define WM8994_AIF1_DAC2_EQG_2PG      (uint16_t)0x04A8
#define WM8994_AIF1_DAC2_EQG_3A       (uint16_t)0x04A9
#define WM8994_AIF1_DAC2_EQG_3B       (uint16_t)0x04AA
#define WM8994_AIF1_DAC2_EQG_3C       (uint16_t)0x04AB
#define WM8994_AIF1_DAC2_EQG_3PG      (uint16_t)0x04AC
#define WM8994_AIF1_DAC2_EQG_4A       (uint16_t)0x04AD
#define WM8994_AIF1_DAC2_EQG_4B       (uint16_t)0x04AE
#define WM8994_AIF1_DAC2_EQG_4C       (uint16_t)0x04AF
#define WM8994_AIF1_DAC2_EQG_4PG      (uint16_t)0x04B0
#define WM8994_AIF1_DAC2_EQG_5A       (uint16_t)0x04B1
#define WM8994_AIF1_DAC2_EQG_5B       (uint16_t)0x04B2
#define WM8994_AIF1_DAC2_EQG_5PG      (uint16_t)0x04B3

/* AIF2 ADC/DAC LR volumes */
#define WM8994_AIF2_ADC_LEFT_VOL      (uint16_t)0x0500
#define WM8994_AIF2_ADC_RIGHT_VOL     (uint16_t)0x0501
#define WM8994_AIF2_DAC_LEFT_VOL      (uint16_t)0x0502
#define WM8994_AIF2_DAC_RIGHT_VOL     (uint16_t)0x0503

/* AIF2 ADC/DAC filters */
#define WM8994_AIF2_ADC_FILTERS       (uint16_t)0x0510
#define WM8994_AIF2_DAC_FILTER_1      (uint16_t)0x0520
#define WM8994_AIF2_DAC_FILTER_2      (uint16_t)0x0521

/* AIF2 DRC registers */
#define WM8994_AIF2_DRC_1             (uint16_t)0x0540
#define WM8994_AIF2_DRC_2             (uint16_t)0x0541
#define WM8994_AIF2_DRC_3             (uint16_t)0x0542
#define WM8994_AIF2_DRC_4             (uint16_t)0x0543
#define WM8994_AIF2_DRC_5             (uint16_t)0x0544

/* AIF2 EQ Gains/bands */
#define WM8994_AIF2_EQG_1             (uint16_t)0x0580
#define WM8994_AIF2_EQG_2             (uint16_t)0x0581
#define WM8994_AIF2_EQG_1A            (uint16_t)0x0582
#define WM8994_AIF2_EQG_1B            (uint16_t)0x0583
#define WM8994_AIF2_EQG_1PG           (uint16_t)0x0584
#define WM8994_AIF2_EQG_2A            (uint16_t)0x0585
#define WM8994_AIF2_EQG_2B            (uint16_t)0x0586
#define WM8994_AIF2_EQG_2C            (uint16_t)0x0587
#define WM8994_AIF2_EQG_2PG           (uint16_t)0x0588
#define WM8994_AIF2_EQG_3A            (uint16_t)0x0589
#define WM8994_AIF2_EQG_3B            (uint16_t)0x058A
#define WM8994_AIF2_EQG_3C            (uint16_t)0x058B
#define WM8994_AIF2_EQG_3PG           (uint16_t)0x058C
#define WM8994_AIF2_EQG_4A            (uint16_t)0x058D
#define WM8994_AIF2_EQG_4B            (uint16_t)0x058E
#define WM8994_AIF2_EQG_4C            (uint16_t)0x058F
#define WM8994_AIF2_EQG_4PG           (uint16_t)0x0590
#define WM8994_AIF2_EQG_5A            (uint16_t)0x0591
#define WM8994_AIF2_EQG_5B            (uint16_t)0x0592
#define WM8994_AIF2_EQG_5PG           (uint16_t)0x0593

/* AIF1 DAC1 Mixer volume */
#define WM8994_DAC1_MIXER_VOL         (uint16_t)0x0600
/* AIF1 DAC1 Left Mixer Routing */
#define WM8994_AIF1_DAC1_LMR          (uint16_t)0x0601
/* AIF1 DAC1 Righ Mixer Routing */
#define WM8994_AIF1_DAC1_RMR          (uint16_t)0x0602
/* AIF1 DAC2 Mixer volume */
#define WM8994_DAC2_MIXER_VOL         (uint16_t)0x0603
/* AIF1 DAC2 Left Mixer Routing */
#define WM8994_AIF1_DAC2_LMR          (uint16_t)0x0604
/* AIF1 DAC2 Righ Mixer Routing */
#define WM8994_AIF1_DAC2_RMR          (uint16_t)0x0605
/* AIF1 ADC1 Left Mixer Routing */
#define WM8994_AIF1_ADC1_LMR          (uint16_t)0x0606
/* AIF1 ADC1 Righ Mixer Routing */
#define WM8994_AIF1_ADC1_RMR          (uint16_t)0x0607
/* AIF1 ADC2 Left Mixer Routing */
#define WM8994_AIF1_ADC2_LMR          (uint16_t)0x0608
/* AIF1 ADC2 Righ Mixer Routing */
#define WM8994_AIF1_ADC2_RMR          (uint16_t)0x0609

/* Volume control */
#define WM8994_DAC1_LEFT_VOL          (uint16_t)0x0610
#define WM8994_DAC1_RIGHT_VOL         (uint16_t)0x0611
#define WM8994_DAC2_LEFT_VOL          (uint16_t)0x0612
#define WM8994_DAC2_RIGHT_VOL         (uint16_t)0x0613
#define WM8994_DAC_SOFTMUTE           (uint16_t)0x0614

#define WM8994_OVERSAMPLING           (uint16_t)0x0620
#define WM8994_SIDETONE               (uint16_t)0x0621

/* GPIO */
#define WM8994_GPIO1                  (uint16_t)0x0700
#define WM8994_GPIO2                  (uint16_t)0x0701
#define WM8994_GPIO3                  (uint16_t)0x0702
#define WM8994_GPIO4                  (uint16_t)0x0703
#define WM8994_GPIO5                  (uint16_t)0x0704
#define WM8994_GPIO6                  (uint16_t)0x0705
#define WM8994_GPIO7                  (uint16_t)0x0706
#define WM8994_GPIO8                  (uint16_t)0x0707
#define WM8994_GPIO9                  (uint16_t)0x0708
#define WM8994_GPIO10                 (uint16_t)0x0709
#define WM8994_GPIO11                 (uint16_t)0x070A
/* Pull Contol */
#define WM8994_PULL_CONTROL_1         (uint16_t)0x0720
#define WM8994_PULL_CONTROL_2         (uint16_t)0x0721
/* WM8994 Inturrupts */
#define WM8994_INT_STATUS_1           (uint16_t)0x0730
#define WM8994_INT_STATUS_2           (uint16_t)0x0731
#define WM8994_INT_RAW_STATUS_2       (uint16_t)0x0732
#define WM8994_INT_STATUS1_MASK       (uint16_t)0x0738
#define WM8994_INT_STATUS2_MASK       (uint16_t)0x0739
#define WM8994_INT_CONTROL            (uint16_t)0x0740
#define WM8994_IRQ_DEBOUNCE           (uint16_t)0x0748

/* Write Sequencer registers from 0 to 511 */
#define WM8994_WRITE_SEQUENCER0       (uint16_t)0x3000
#define WM8994_WRITE_SEQUENCER1       (uint16_t)0x3001
#define WM8994_WRITE_SEQUENCER2       (uint16_t)0x3002
#define WM8994_WRITE_SEQUENCER3       (uint16_t)0x3003

#define WM8994_WRITE_SEQUENCER4       (uint16_t)0x3508
#define WM8994_WRITE_SEQUENCER5       (uint16_t)0x3509
#define WM8994_WRITE_SEQUENCER6       (uint16_t)0x3510
#define WM8994_WRITE_SEQUENCER7       (uint16_t)0x3511

//-----------------------------------------------------------------------------

#define  WM8994_ID                    (uint16_t)0x8994

//-----------------------------------------------------------------------------
/* private */

uint16_t audio_wm8994ecs::read_reg(uint16_t reg_addr)
{
    using transfer_desc = hal::interface::i2c_proxy::transfer_desc;

    uint8_t tx[2] {static_cast<uint8_t>((reg_addr >> 8) & 0xFF), static_cast<uint8_t>(reg_addr & 0xFF)};
    uint8_t rx[2] {0};

    transfer_desc desc
    {
        this->i2c_addr,
        reinterpret_cast<std::byte*>(tx),
        sizeof(tx),
        reinterpret_cast<std::byte*>(rx),
        sizeof(rx)
    };

    this->i2c.transfer(desc);
    assert(desc.stat == transfer_desc::status::ok);

    return (rx[0] << 8) | rx[1];
}

void audio_wm8994ecs::write_reg(uint16_t reg_addr, uint16_t reg_val)
{
    using transfr_desc = hal::interface::i2c_proxy::transfer_desc;

    uint8_t tx[4]
    {
        static_cast<uint8_t>((reg_addr >> 8) & 0xFF),
        static_cast<uint8_t>(reg_addr & 0xFF),
        static_cast<uint8_t>((reg_val >> 8) & 0xFF),
        static_cast<uint8_t>(reg_val & 0xFF)
    };

    transfr_desc desc
    {
        this->i2c_addr,
        reinterpret_cast<std::byte*>(tx),
        sizeof(tx),
    };

    if constexpr (verify_i2c_writes)
    {
        this->i2c.transfer(desc);
        if (reg_addr != WM8994_SW_RESET)
            assert(this->read_reg(reg_addr) == reg_val);
    }
    else
    {
        this->i2c.transfer(desc, {});
    }
}

uint16_t audio_wm8994ecs::read_id(void)
{
    return this->read_reg(WM8994_SW_RESET);
}

void audio_wm8994ecs::reset(void)
{
    return this->write_reg(WM8994_SW_RESET, 0);
}

//-----------------------------------------------------------------------------
/* public */

audio_wm8994ecs::audio_wm8994ecs(hal::interface::i2c_proxy &i2c, uint8_t addr, input in, output out, bool in_swap) :
i2c {i2c}, i2c_addr {addr}, sai_drv{sai_32bit::id::sai2}, in_swapped{in_swap}
{
    constexpr uint32_t audio_freq = 48000;
    constexpr uint16_t slots_1_3 = 0b1010;
    constexpr uint16_t slots_0_2 = 0b0101;

    if (out != output::none)
    {
        const sai_32bit::block::config sai_a_cfg
        {
            sai_32bit::block::mode_type::master_tx,
            sai_32bit::block::protocol_type::generic,
            sai_32bit::block::data_size::_32bit,
            sai_32bit::block::sync_type::none,
            sai_32bit::block::frame_type::stereo,
            sai_32bit::block::audio_freq::_48kHz,
            WM8994_SLOTS_NUMBER,
            slots_0_2
        };

        this->sai_drv.block_a.enable(false);
        this->sai_drv.block_a.configure(sai_a_cfg);
        this->sai_drv.block_a.enable(true);
    }

    if (in != input::none)
    {
        const sai_32bit::block::config sai_b_cfg
        {
            out != output::none ?
            sai_32bit::block::mode_type::slave_rx : sai_32bit::block::mode_type::master_rx,
            sai_32bit::block::protocol_type::generic,
            sai_32bit::block::data_size::_32bit,
            out != output::none ?
            sai_32bit::block::sync_type::internal : sai_32bit::block::sync_type::none,
            sai_32bit::block::frame_type::stereo,
            sai_32bit::block::audio_freq::_48kHz,
            WM8994_SLOTS_NUMBER,
            (in == input::mic2 || in == input::line2) ? slots_1_3 : slots_0_2,
        };

        this->sai_drv.block_b.enable(false);
        this->sai_drv.block_b.configure(sai_b_cfg);
        this->sai_drv.block_b.enable(true);
    }

    /* Initialize wm8994 codec */
    assert(this->read_id() == WM8994_ID);
    this->reset();

    delay::ms(10);

    uint16_t power_mgnt_reg_1 = 0;

    /* wm8994 Errata Work-Arounds */
    this->write_reg(0x102, 0x0003);
    this->write_reg(0x817, 0x0000);
    this->write_reg(0x102, 0x0000);

    /* Enable VMID soft start (fast), Start-up Bias Current Enabled */
    this->write_reg(0x39, 0x006C);

    /* Enable bias generator, Enable VMID */
    power_mgnt_reg_1 |= 0x0003;
    this->write_reg(0x01, power_mgnt_reg_1);

    /* Add Delay */
    delay::ms(50);

    /* Path Configurations for output */
    if (out != output::none)
    {
        switch (out)
        {
        case output::speaker:
            /* Disable DAC1 (Left), Disable DAC1 (Right),
             Enable DAC2 (Left), Enable DAC2 (Right)*/
            this->write_reg(0x05, 0x0C0C);

            /* Disable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
            this->write_reg(0x601, 0x0000);

            /* Disable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
            this->write_reg(0x602, 0x0000);

            /* Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
            this->write_reg(0x604, 0x0002);

            /* Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
            this->write_reg(0x605, 0x0002);
            break;

        case output::headphone:
            /* Enable DAC1 (Left), Enable DAC1 (Right),
             Disable DAC2 (Left), Disable DAC2 (Right)*/
            this->write_reg(0x05, 0x0303);

            /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
            this->write_reg(0x601, 0x0001);

            /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
            this->write_reg(0x602, 0x0001);

            /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
            this->write_reg(0x604, 0x0000);

            /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
            this->write_reg(0x605, 0x0000);
            break;

        case output::both:
            /* Enable DAC1 (Left), Enable DAC1 (Right),
             also Enable DAC2 (Left), Enable DAC2 (Right)*/
            this->write_reg(0x05, 0x0303 | 0x0C0C);

            /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
            this->write_reg(0x601, 0x0001);

            /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
            this->write_reg(0x602, 0x0001);

            /* Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
            this->write_reg(0x604, 0x0002);

            /* Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
            this->write_reg(0x605, 0x0002);
            break;

        case output::automatic:
        default:
            /* Disable DAC1 (Left), Disable DAC1 (Right),
             Enable DAC2 (Left), Enable DAC2 (Right)*/
            this->write_reg(0x05, 0x0303);

            /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
            this->write_reg(0x601, 0x0001);

            /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
            this->write_reg(0x602, 0x0001);

            /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
            this->write_reg(0x604, 0x0000);

            /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
            this->write_reg(0x605, 0x0000);
            break;
        }
    }

    /* Path Configurations for input */
    if (in != input::none)
    {
        switch (in)
        {
        case input::line1:
            /* IN1LN_TO_IN1L, IN1LP_TO_VMID, IN1RN_TO_IN1R, IN1RP_TO_VMID */
            this->write_reg(0x28, 0x0011);

            /* Disable mute on IN1L_TO_MIXINL, +0dB on IN1L PGA output, mute MIXOUTL_MIXINL_VOL */
            this->write_reg(0x29, 0x0020);

            /* Disable mute on IN1R_TO_MIXINL, +0dB on IN1L PGA output, mute MIXOUTR_MIXINR_VOL  */
            this->write_reg(0x2A, 0x0020);

            /* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
             * Enable Left ADC, Enable Right ADC */
            this->write_reg(0x04, 0x0303);

            /* Enable AIF1 DRC1 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
            this->write_reg(0x440, 0x00DB);

            /* Enable IN1L and IN1R, Disable IN2L and IN2R, Enable Thermal sensor & shutdown */
            this->write_reg(0x02, 0x6350);

            /* Enable the ADCL(Left) to AIF1 Timeslot 0 (Left) mixer path */
            this->write_reg(0x606, 0x0002);

            /* Enable the ADCR(Right) to AIF1 Timeslot 0 (Right) mixer path */
            this->write_reg(0x607, 0x0002);

            /* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 signal detect */
            this->write_reg(0x700, 0x000D);
            break;

        case input::line2:
            /* Not supported */
            break;

        case input::mic1:
            /* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
             * Enable DMICDAT1 (Left), Enable DMICDAT1 (Right)
             * Enable Left ADC, Enable Right ADC */
            this->write_reg(0x04, 0x030C);

            /* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
            this->write_reg(0x440, 0x00DB);

            /* Disable IN1L, IN1R, IN2L, IN2R, Enable Thermal sensor & shutdown */
            this->write_reg(0x02, 0x6350);

            /* Enable the DMIC1(Left) to AIF1 Timeslot 0 (Left) mixer path */
            this->write_reg(0x606, 0x0002);

            /* Enable the DMIC1(Right) to AIF1 Timeslot 0 (Right) mixer path */
            this->write_reg(0x607, 0x0002);

            /* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 signal detect */
            this->write_reg(0x700, 0x000D);
            break;

        case input::mic2:
            /* Enable AIF1ADC2 (Left), Enable AIF1ADC2 (Right)
             * Enable DMICDAT2 (Left), Enable DMICDAT2 (Right)
             * Enable Left ADC, Enable Right ADC */
            this->write_reg(0x04, 0x0C30);

            /* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC2 Left/Right Timeslot 1 */
            this->write_reg(0x450, 0x00DB);

            /* Disable IN1L, IN1R, IN2L, IN2R, Enable Thermal sensor & shutdown */
            this->write_reg(0x02, 0x6000);

            /* Enable the DMIC2(Left) to AIF1 Timeslot 1 (Left) mixer path */
            this->write_reg(0x608, 0x0002);

            /* Enable the DMIC2(Right) to AIF1 Timeslot 1 (Right) mixer path */
            this->write_reg(0x609, 0x0002);

            /* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC2 signal detect */
            this->write_reg(0x700, 0x000E);
            break;

        case input::line1_mic2:
            /* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
             * Enable AIF1ADC2 (Left), Enable AIF1ADC2 (Right)
             * Enable DMICDAT2 (Left), Enable DMICDAT2 (Right)
             * Enable Left ADC, Enable Right ADC */
            this->write_reg(0x04, 0x0303 | 0x0C30);

            /* IN1LN_TO_IN1L, IN1LP_TO_VMID, IN1RN_TO_IN1R, IN1RP_TO_VMID */
            this->write_reg(0x28, 0x0011);

            /* Disable mute on IN1L_TO_MIXINL, +0dB on IN1L PGA output, mute MIXOUTL_MIXINL_VOL */
            this->write_reg(0x29, 0x0020);

            /* Disable mute on IN1R_TO_MIXINL, +0dB on IN1L PGA output, mute MIXOUTR_MIXINR_VOL  */
            this->write_reg(0x2A, 0x0020);

            /* Enable AIF1 DRC1 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
            this->write_reg(0x440, 0x00DB);

            /* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC2 Left/Right Timeslot 1 */
            this->write_reg(0x450, 0x00DB);

            /* Enable IN1L and IN1R, Disable IN2L and IN2R, Enable Thermal sensor & shutdown */
            this->write_reg(0x02, 0x6350);

            /* Enable the ADCL(Left) to AIF1 Timeslot 0 (Left) mixer path */
            this->write_reg(0x606, 0x0002);

            /* Enable the ADCR(Right) to AIF1 Timeslot 0 (Right) mixer path */
            this->write_reg(0x607, 0x0002);

            /* Enable the DMIC2(Left) to AIF1 Timeslot 1 (Left) mixer path */
            this->write_reg(0x608, 0x0002);

            /* Enable the DMIC2(Right) to AIF1 Timeslot 1 (Right) mixer path */
            this->write_reg(0x609, 0x0002);

            /* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 & DRC2 signal detect */
            this->write_reg(0x700, 0x000D | 0x000E);
            break;

        default:
            break;
        }
    }

    /*  Clock Configurations */
    switch (audio_freq)
    {
    case 8000:
        /* AIF1 Sample Rate = 8 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0003);
        break;

    case 16000:
        /* AIF1 Sample Rate = 16 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0033);
        break;

    case 32000:
        /* AIF1 Sample Rate = 32 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0063);
        break;

    case 48000:
        /* AIF1 Sample Rate = 48 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0083);
        break;

    case 96000:
        /* AIF1 Sample Rate = 96 (KHz), ratio=256 */
        this->write_reg(0x210, 0x00A3);
        break;

    case 11025:
        /* AIF1 Sample Rate = 11.025 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0013);
        break;

    case 22050:
        /* AIF1 Sample Rate = 22.050 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0043);
        break;

    case 44100:
        /* AIF1 Sample Rate = 44.1 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0073);
        break;

    default:
        /* AIF1 Sample Rate = 48 (KHz), ratio=256 */
        this->write_reg(0x210, 0x0083);
        break;
    }

    /* AIF1 Word Length = 32-bits, AIF1 Format = I2S */
    (this->in_swapped) ? this->write_reg(0x300, 0x8070) : this->write_reg(0x300, 0x4070);

    /* slave mode */
    this->write_reg(0x302, 0x0000);

    /* Enable the DSP processing clock for AIF1, Enable the core clock */
    this->write_reg(0x208, 0x000A);

    /* Enable AIF1 Clock, AIF1 Clock Source = MCLK1 pin */
    this->write_reg(0x200, 0x0001);

    /* Analog Output Configuration */
    if (out != output::none)
    {
        /* ADC & DAC oversample enable */
        this->write_reg(WM8994_OVERSAMPLING, 0x0003);

        if (out == output::headphone)
        {
            /* Select DAC1 (Left) to Left Headphone Output PGA (HPOUT1LVOL) path */
            this->write_reg(0x2D, 0x0100);

            /* Select DAC1 (Right) to Right Headphone Output PGA (HPOUT1RVOL) path */
            this->write_reg(0x2E, 0x0100);

            /* Startup sequence for Headphone */
            static bool cold_startup = true;
            if (cold_startup)
            {
                this->write_reg(0x110, 0x8100);

                cold_startup = false;
                /* Add Delay */
                delay::ms(300);
            }
            else /* Headphone Warm Start-Up */
            {
                this->write_reg(0x110, 0x8108);
                /* Add Delay */
                delay::ms(50);
            }

            /* Enable HPOUT1 (Left) and Enable HPOUT1 (Right) input stages */
            power_mgnt_reg_1 |= 0x0300;
            this->write_reg(0x01, power_mgnt_reg_1);
        }

        if (out == output::speaker)
        {
            /* Enable SPKRVOL PGA, Enable SPKMIXR, Enable SPKLVOL PGA, Enable SPKMIXL */
            this->write_reg(0x03, 0x0300);

            /* Left Speaker Mixer Volume = 0dB */
            this->write_reg(0x22, 0x0000);

            /* Speaker output mode = Class D, Right Speaker Mixer Volume = 0dB ((0x23, 0x0100) = class AB)*/
            this->write_reg(0x23, 0x0000);

            /* Unmute DAC2 (Left) to Left Speaker Mixer (SPKMIXL) path,
               Unmute DAC2 (Right) to Right Speaker Mixer (SPKMIXR) path */
            this->write_reg(0x36, 0x0300);

            /* Enable SPKOUTL, Enable SPKOUTR */
            power_mgnt_reg_1 |= 0x3000;
            this->write_reg(0x01, power_mgnt_reg_1);
        }

        /* Headphone/Speaker Enable */

        /* Enable Class W, Class W Envelope Tracking = AIF1 Timeslot 0 */
        this->write_reg(0x51, 0x0005);

        /* Enable HPOUT1 (Left) and HPOUT1 (Right) intermediate stages */
        this->write_reg(0x60, 0x0022);

        /* Enable Charge Pump */
        this->write_reg(0x4C, 0x9F25);

        /* Add Delay */
        delay::ms(15);

        /* Select DAC1 (Left) to Left Headphone Output PGA (HPOUT1LVOL) path */
        this->write_reg(0x2D, 0x0001);

        /* Select DAC1 (Right) to Right Headphone Output PGA (HPOUT1RVOL) path */
        this->write_reg(0x2E, 0x0001);

        /* Enable Left Output Mixer (MIXOUTL), Enable Right Output Mixer (MIXOUTR) */
        /* idem for SPKOUTL and SPKOUTR */
        this->write_reg(0x03, 0x0030 | 0x0300);

        /* Enable DC Servo and trigger start-up mode on left and right channels */
        this->write_reg(0x54, 0x0033);

        /* Add Delay */
        delay::ms(256);

        /* Enable HPOUT1 (Left) and HPOUT1 (Right) intermediate and output stages. Remove clamps */
        this->write_reg(0x60, 0x00EE);

        /* Unmutes */

        /* Mute ramp: fs/32 */
        this->write_reg(0x0614, 0x0003);

        /* Unmute DAC 1 (Left) */
        this->write_reg(0x610, 0x00C0);

        /* Unmute DAC 1 (Right) */
        this->write_reg(0x611, 0x00C0);

        /* Unmute the AIF1 Timeslot 0 DAC path with unmute ramp */
        this->write_reg(0x420, 0x0010);

        /* Unmute DAC 2 (Left) */
        this->write_reg(0x612, 0x00C0);

        /* Unmute DAC 2 (Right) */
        this->write_reg(0x613, 0x00C0);

        /* Unmute the AIF1 Timeslot 1 DAC2 path with unmute ramp */
        this->write_reg(0x422, 0x0010);

        /* Set Volume to 0dB */
        this->set_output_volume(0x39);
    }

    if (in != input::none) /* Audio input selected */
    {
        if ((in == input::mic1) || (in == input::mic2) || (in == input::line1_mic2))
        {
            /* Enable Microphone bias 1 generator */
            power_mgnt_reg_1 |= 0x0010;
            this->write_reg(0x01, power_mgnt_reg_1);

            /* AIF ADC2 HPF enable, HPF cut = hifi mode fc=4Hz at fs=48kHz */
            this->write_reg(0x411, 0x1800);
        }
        if ((in == input::line1) || (in == input::line2) || (in == input::line1_mic2))
        {
            /* Disable mute on IN1L, IN1L Volume = +0dB */
            this->write_reg(0x18, 0x000B);

            /* Disable mute on IN1R, IN1R Volume = +0dB */
            this->write_reg(0x1A, 0x000B);

            /* AIF ADC1 HPF enable, HPF cut = hifi mode fc=4Hz at fs=48kHz */
            this->write_reg(0x410, 0x1800);
        }

        /* Set Volume to 0dB */
        this->set_input_volume(0x0B, 0);
        this->set_input_volume(0x0B, 1);
    }
}

audio_wm8994ecs::~audio_wm8994ecs()
{

}

void audio_wm8994ecs::capture(audio_input::sample_t *input, uint16_t length, const capture_cb_t &cb, bool loop)
{
    this->capture_callback = cb;
    this->sai_drv.loop_read(loop);
    this->sai_drv.read(input, length * sizeof(*input),
    [this](const audio_input::sample_t *data, std::size_t bytes_read)
    {
        this->capture_callback(data, bytes_read / sizeof(*input));
    }
    );
}

void audio_wm8994ecs::stop_capture(void)
{
    this->sai_drv.block_b.enable(false);
}

void audio_wm8994ecs::set_input_volume(uint8_t vol, uint8_t ch)
{
    /* Analog volume of PGA (-16.5dB to 30dB) */
    constexpr uint8_t vol_m16_5db = 0;
    constexpr uint8_t vol_30db = 0x1F;
    vol = std::clamp(vol, vol_m16_5db, vol_30db);

    /* Digital volume of ADC2 OUT (-6.0dB to 17.25dB) */
    const uint16_t dvol = 0xB0 + 2 * vol;

    if (this->in_swapped)
        ch = !ch;

    switch (ch)
    {
        case 0: // Left
            this->write_reg(WM8994_LEFT_LINE_IN12_VOL, vol | 0x0140);
            this->write_reg(WM8994_AIF1_ADC2_LEFT_VOL, dvol | 0x100);
            break;
        case 1: // Right
            this->write_reg(WM8994_RIGHT_LINE_IN12_VOL, vol | 0x0140);
            this->write_reg(WM8994_AIF1_ADC2_RIGHT_VOL, dvol | 0x100);
            break;
        default:
            break;
    }
}

void audio_wm8994ecs::set_input_channels(frame_slots left_ch, frame_slots right_ch)
{
    const bool out_enabled = this->sai_drv.block_a.is_enabled();
    const uint16_t left_slot = static_cast<uint16_t>(left_ch);
    const uint16_t right_slot = static_cast<uint16_t>(right_ch);

    const sai_32bit::block::config sai_b_cfg
    {
        out_enabled ?
        sai_32bit::block::mode_type::slave_rx : sai_32bit::block::mode_type::master_rx,
        sai_32bit::block::protocol_type::generic,
        sai_32bit::block::data_size::_32bit,
        out_enabled ?
        sai_32bit::block::sync_type::internal : sai_32bit::block::sync_type::none,
        sai_32bit::block::frame_type::stereo,
        sai_32bit::block::audio_freq::_48kHz,
        WM8994_SLOTS_NUMBER,
        static_cast<uint16_t>((1 << left_slot) | (1 << right_slot)),
    };

    this->mute(true);
    this->sai_drv.block_b.enable(false);
    this->sai_drv.block_b.configure(sai_b_cfg);
    this->sai_drv.block_b.enable(true);
    this->mute(false);
}

void audio_wm8994ecs::play(const audio_output::sample_t *output, uint16_t length, const play_cb_t &cb, bool loop)
{
    this->play_callback = cb;
    this->sai_drv.loop_write(loop);
    this->sai_drv.write(output, length * sizeof(*output),
    [this](std::size_t bytes_written)
    {
        this->play_callback(bytes_written / sizeof(*output));
    }
    );
}

void audio_wm8994ecs::pause(void)
{
    /* TODO */
}

void audio_wm8994ecs::resume(void)
{
    /* TODO */
}

void audio_wm8994ecs::stop(void)
{
    this->sai_drv.block_a.enable(false);

    /* Speaker & Headphone fast shutdown */
    this->write_reg(WM8994_WRITE_SEQ_CTRL1, 0x8122);
}

void audio_wm8994ecs::mute(bool value)
{
    uint16_t regval = 0;

    if (value)
        regval = 0x0200;
    else
        regval = 0x00C0;

    this->write_reg(WM8994_DAC1_LEFT_VOL, regval);
    this->write_reg(WM8994_DAC1_RIGHT_VOL, regval);
    this->write_reg(WM8994_DAC2_LEFT_VOL, regval);
    this->write_reg(WM8994_DAC2_RIGHT_VOL, regval);
}

void audio_wm8994ecs::set_output_volume(uint8_t vol)
{
    /* Analog volume of PGA (-57dB to 6dB) */

    constexpr uint8_t vol_m57db = 0;
    constexpr uint8_t vol_6db = 0x3F;
    vol = std::clamp(vol, vol_m57db, vol_6db);

    this->write_reg(WM8994_LEFT_OUTPUT_VOL, vol | 0x00C0);
    this->write_reg(WM8994_RIGHT_OUTPUT_VOL, vol | 0x01C0);
    this->write_reg(WM8994_SPK_LEFT_VOL, vol | 0x00C0);
    this->write_reg(WM8994_SPK_RIGHT_VOL, vol | 0x01C0);
}

