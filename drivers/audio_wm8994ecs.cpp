/*
 * audio_wm8994ecs.cpp
 *
 *  Created on: 11 sie 2023
 *      Author: kwarc
 */


#include "audio_wm8994ecs.hpp"

#include <cassert>

#include <drivers/stm32f7/delay.hpp>

using namespace drivers;

//-----------------------------------------------------------------------------
/* helpers */

//-----------------------------------------------------------------------------

/*
 * In W8994 codec the Audio frame contains 4 slots : TDM Mode
 * TDM format:
 * +------------------|------------------|--------------------|-------------------+
 * | CODEC_SLOT0 Left | CODEC_SLOT1 Left | CODEC_SLOT0 Right  | CODEC_SLOT1 Right |
 * +------------------------------------------------------------------------------+
 */

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
    using transfer_desc = hal::interface::i2c_device::transfer_desc;

    uint8_t tx[2] {(reg_addr >> 8) & 0xFF, (reg_addr & 0xFF)};
    uint8_t rx[2] {0};

    transfer_desc desc
    {
        this->i2c_addr,
        reinterpret_cast<std::byte*>(tx),
        sizeof(tx),
        reinterpret_cast<std::byte*>(rx),
        sizeof(rx)
    };

    this->i2c_dev.transfer(desc);
    assert(desc.stat == transfer_desc::status::ok);

    return (rx[0] << 8) | rx[1];
}

void audio_wm8994ecs::write_reg(uint16_t reg_addr, uint16_t reg_val)
{
    using transfr_desc = hal::interface::i2c_device::transfer_desc;

    uint8_t tx[4] = {(reg_addr >> 8) & 0xFF, (reg_addr & 0xFF), (reg_val >> 8) & 0xFF, (reg_val & 0xFF)};

    transfr_desc desc
    {
        this->i2c_addr,
        reinterpret_cast<std::byte*>(tx),
        sizeof(tx),
    };

    this->i2c_dev.transfer(desc);
    assert(desc.stat == transfr_desc::status::ok);

    if constexpr (verify_i2c_writes)
    {
        if (reg_addr != WM8994_SW_RESET)
            assert(this->read_reg(reg_addr) == reg_val);
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

audio_wm8994ecs::audio_wm8994ecs(hal::interface::i2c_device &dev, uint8_t addr, input in, output out) :
i2c_dev {dev}, i2c_addr {addr}, sai_drv{audio_sai::id::sai2}
{
    constexpr uint32_t audio_freq = 48000;

    constexpr uint8_t output_vol = 57; // 0dB
    constexpr uint8_t input_vol = 192; // 0dB

    if (out != output::none)
    {
        static const audio_sai::block::config sai_a_cfg
        {
            audio_sai::block::mode_type::master_tx,
            audio_sai::block::protocol_type::generic,
            audio_sai::block::data_size::_16bit,
            audio_sai::block::sync_type::none,
            audio_sai::block::frame_type::stereo,
            audio_sai::block::active_slots::slots_0_2,
            audio_sai::block::audio_freq::_48kHz,
        };

        sai_drv.block_a.configure(sai_a_cfg);
        sai_drv.block_a.enable(true);
    }

    if (in != input::none)
    {
        static const audio_sai::block::config sai_b_cfg
        {
            out != output::none ?
            audio_sai::block::mode_type::slave_rx : audio_sai::block::mode_type::master_rx,
            audio_sai::block::protocol_type::generic,
            audio_sai::block::data_size::_16bit,
            out != output::none ?
            audio_sai::block::sync_type::internal : audio_sai::block::sync_type::none,
            audio_sai::block::frame_type::stereo,
            in == input::mic2 || in == input::line2 ?
            audio_sai::block::active_slots::slots_1_3 : audio_sai::block::active_slots::slots_0_2,
            audio_sai::block::audio_freq::_48kHz,
        };

        sai_drv.block_b.configure(sai_b_cfg);
        sai_drv.block_b.enable(true);
    }

    /* Initialize wm8994 codec */
    assert(this->read_id() == WM8994_ID);
    this->reset();

    drivers::delay::ms(10);

    uint16_t power_mgnt_reg_1 = 0;

    /* wm8994 Errata Work-Arounds */
    this->write_reg(0x102, 0x0003);
    this->write_reg(0x817, 0x0000);
    this->write_reg(0x102, 0x0000);

    /* Enable VMID soft start (fast), Start-up Bias Current Enabled */
    this->write_reg(0x39, 0x006C);

    /* Enable bias generator, Enable VMID */
    if (in != input::none)
    {
        this->write_reg(0x01, 0x0013);
    }
    else
    {
        this->write_reg(0x01, 0x0003);
    }

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
            if (in == input::mic1_mic2)
            {
                /* Enable DAC1 (Left), Enable DAC1 (Right),
                 also Enable DAC2 (Left), Enable DAC2 (Right)*/
                this->write_reg(0x05, 0x0303 | 0x0C0C);

                /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path
                 Enable the AIF1 Timeslot 1 (Left) to DAC 1 (Left) mixer path */
                this->write_reg(0x601, 0x0003);

                /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path
                 Enable the AIF1 Timeslot 1 (Right) to DAC 1 (Right) mixer path */
                this->write_reg(0x602, 0x0003);

                /* Enable the AIF1 Timeslot 0 (Left) to DAC 2 (Left) mixer path
                 Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path  */
                this->write_reg(0x604, 0x0003);

                /* Enable the AIF1 Timeslot 0 (Right) to DAC 2 (Right) mixer path
                 Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
                this->write_reg(0x605, 0x0003);
            }
            else
            {
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
            }
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
        case input::mic1_mic2:
            /* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
             * Enable DMICDAT1 (Left), Enable DMICDAT1 (Right)
             * Enable Left ADC, Enable Right ADC */
            this->write_reg(0x04, 0x0F3C);

            /* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC2 Left/Right Timeslot 1 */
            this->write_reg(0x450, 0x00DB);

            /* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
            this->write_reg(0x440, 0x00DB);

            /* Disable IN1L, IN1R, Enable IN2L, IN2R, Thermal sensor & shutdown */
            this->write_reg(0x02, 0x63A0);

            /* Enable the DMIC1(Left) to AIF1 Timeslot 0 (Left) mixer path */
            this->write_reg(0x606, 0x0002);

            /* Enable the DMIC1(Right) to AIF1 Timeslot 0 (Right) mixer path */
            this->write_reg(0x607, 0x0002);

            /* Enable the DMIC2(Left) to AIF1 Timeslot 1 (Left) mixer path */
            this->write_reg(0x608, 0x0002);

            /* Enable the DMIC2(Right) to AIF1 Timeslot 1 (Right) mixer path */
            this->write_reg(0x609, 0x0002);

            /* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 signal detect */
            this->write_reg(0x700, 0x000D);
            break;
        case input::line2:
        default:
            /* Actually, no other input devices supported */
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

    if (in == input::mic1_mic2)
    {
        /* AIF1 Word Length = 16-bits, AIF1 Format = DSP mode */
        this->write_reg(0x300, 0x4018);
    }
    else
    {
        /* AIF1 Word Length = 16-bits, AIF1 Format = I2S (Default Register Value) */
        this->write_reg(0x300, 0x4010);
    }

    /* slave mode */
    this->write_reg(0x302, 0x0000);

    /* Enable the DSP processing clock for AIF1, Enable the core clock */
    this->write_reg(0x208, 0x000A);

    /* Enable AIF1 Clock, AIF1 Clock Source = MCLK1 pin */
    this->write_reg(0x200, 0x0001);

    if (out != output::none) /* Audio output selected */
    {
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

            /* Soft un-Mute the AIF1 Timeslot 0 DAC1 path L&R */
            this->write_reg(0x420, 0x0000);
        }

        /* Analog Output Configuration */

        /* Enable SPKRVOL PGA, Enable SPKMIXR, Enable SPKLVOL PGA, Enable SPKMIXL */
        this->write_reg(0x03, 0x0300);

        /* Left Speaker Mixer Volume = 0dB */
        this->write_reg(0x22, 0x0000);

        /* Speaker output mode = Class D, Right Speaker Mixer Volume = 0dB ((0x23, 0x0100) = class AB)*/
        this->write_reg(0x23, 0x0000);

        /* Unmute DAC2 (Left) to Left Speaker Mixer (SPKMIXL) path,
           Unmute DAC2 (Right) to Right Speaker Mixer (SPKMIXR) path */
        this->write_reg(0x36, 0x0300);

        /* Enable bias generator, Enable VMID, Enable SPKOUTL, Enable SPKOUTR */
        this->write_reg(0x01, 0x3003);

        /* Headphone/Speaker Enable */

        if (in == input::mic1_mic2)
        {
            /* Enable Class W, Class W Envelope Tracking = AIF1 Timeslots 0 and 1 */
            this->write_reg(0x51, 0x0205);
        }
        else
        {
            /* Enable Class W, Class W Envelope Tracking = AIF1 Timeslot 0 */
            this->write_reg(0x51, 0x0005);
        }

        /* Enable bias generator, Enable VMID, Enable HPOUT1 (Left) and Enable HPOUT1 (Right) input stages */
        /* idem for Speaker */
        power_mgnt_reg_1 |= 0x0303 | 0x3003;
        this->write_reg(0x01, power_mgnt_reg_1);

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

        /* Unmute DAC 1 (Left) */
        this->write_reg(0x610, 0x00C0);

        /* Unmute DAC 1 (Right) */
        this->write_reg(0x611, 0x00C0);

        /* Unmute the AIF1 Timeslot 0 DAC path */
        this->write_reg(0x420, 0x0010);

        /* Unmute DAC 2 (Left) */
        this->write_reg(0x612, 0x00C0);

        /* Unmute DAC 2 (Right) */
        this->write_reg(0x613, 0x00C0);

        /* Unmute the AIF1 Timeslot 1 DAC2 path */
        this->write_reg(0x422, 0x0010);

        /* Volume Control */
        this->set_volume(output_vol);
    }

    if (in != input::none) /* Audio input selected */
    {
        if ((in == input::mic1) || (in == input::mic2))
        {
            /* Enable Microphone bias 1 generator, Enable VMID */
            power_mgnt_reg_1 |= 0x0013;
            this->write_reg(0x01, power_mgnt_reg_1);

            /* ADC oversample enable */
            this->write_reg(0x620, 0x0002);

            /* AIF ADC2 HPF enable, HPF cut = voice mode 1 fc=127Hz at fs=8kHz */
            this->write_reg(0x411, 0x3800);
        }
        else if (in == input::mic1_mic2)
        {
            /* Enable Microphone bias 1 generator, Enable VMID */
            power_mgnt_reg_1 |= 0x0013;
            this->write_reg(0x01, power_mgnt_reg_1);

            /* ADC oversample enable */
            this->write_reg(0x620, 0x0002);

            /* AIF ADC1 HPF enable, HPF cut = voice mode 1 fc=127Hz at fs=8kHz */
            this->write_reg(0x410, 0x1800);

            /* AIF ADC2 HPF enable, HPF cut = voice mode 1 fc=127Hz at fs=8kHz */
            this->write_reg(0x411, 0x1800);
        }
        else if ((in == input::line1) || (in == input::line2))
        {

            /* Disable mute on IN1L, IN1L Volume = +0dB */
            this->write_reg(0x18, 0x000B);

            /* Disable mute on IN1R, IN1R Volume = +0dB */
            this->write_reg(0x1A, 0x000B);

            /* AIF ADC1 HPF enable, HPF cut = hifi mode fc=4Hz at fs=48kHz */
            this->write_reg(0x410, 0x1800);
        }

        /* Volume Control */
        uint8_t convertedvol = (input_vol >= 100) ? 239 : static_cast<uint8_t>((input_vol * 240) / 100);

        /* Left AIF1 ADC1 volume */
        this->write_reg(0x400, convertedvol | 0x100);

        /* Right AIF1 ADC1 volume */
        this->write_reg(0x401, convertedvol | 0x100);

        /* Left AIF1 ADC2 volume */
        this->write_reg(0x404, convertedvol | 0x100);

        /* Right AIF1 ADC2 volume */
        this->write_reg(0x405, convertedvol | 0x100);
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
    [this](const int16_t *data, std::size_t bytes_read)
    {
        this->capture_callback(data, bytes_read / sizeof(audio_wm8994ecs::audio_input::sample_t));
    }
    );
}

void audio_wm8994ecs::stop_capture(void)
{
    /* TODO */
}

void audio_wm8994ecs::play(const audio_output::sample_t *output, uint16_t length, const play_cb_t &cb, bool loop)
{
    this->play_callback = cb;
    this->sai_drv.loop_write(loop);
    this->sai_drv.write(output, length * sizeof(*output),
    [this](std::size_t bytes_written)
    {
        this->play_callback(bytes_written / sizeof(audio_wm8994ecs::audio_output::sample_t));
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
    /* TODO */
}

void audio_wm8994ecs::set_volume(uint8_t vol)
{
    uint8_t convertedvol = (vol > 100) ? 100 : static_cast<uint8_t>((vol * 63) / 100);

    if (convertedvol > 0x3E)
    {
        /* Unmute audio codec */

        /* Unmute the AIF1 Timeslot 0 DAC1 path L&R */
        this->write_reg(0x420, 0x0010);

        /* Unmute the AIF1 Timeslot 1 DAC2 path L&R */
        this->write_reg(0x422, 0x0010);

        /* Left Headphone Volume */
        this->write_reg(0x1C, 0x3F | 0x140);

        /* Right Headphone Volume */
        this->write_reg(0x1D, 0x3F | 0x140);

        /* Left Speaker Volume */
        this->write_reg(0x26, 0x3F | 0x140);

        /* Right Speaker Volume */
        this->write_reg(0x27, 0x3F | 0x140);
    }
    else if (vol == 0)
    {
        /* Mute audio codec */

        /* Soft Mute the AIF1 Timeslot 0 DAC1 path L&R */
        this->write_reg(0x420, 0x0200);

        /* Soft Mute the AIF1 Timeslot 1 DAC2 path L&R */
        this->write_reg(0x422, 0x0200);
    }
    else
    {
        /* Unmute audio codec */

        /* Unmute the AIF1 Timeslot 0 DAC1 path L&R */
        this->write_reg(0x420, 0x0010);

        /* Unmute the AIF1 Timeslot 1 DAC2 path L&R */
        this->write_reg(0x422, 0x0010);

        /* Left Headphone Volume */
        this->write_reg(0x1C, convertedvol | 0x140);

        /* Right Headphone Volume */
        this->write_reg(0x1D, convertedvol | 0x140);

        /* Left Speaker Volume */
        this->write_reg(0x26, convertedvol | 0x140);

        /* Right Speaker Volume */
        this->write_reg(0x27, convertedvol | 0x140);
    }
}
