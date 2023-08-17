/*
 * audio_wm8994ecs.hpp
 *
 *  Created on: 11 sie 2023
 *      Author: kwarc
 */

#ifndef AUDIO_WM8994ECS_HPP_
#define AUDIO_WM8994ECS_HPP_

#include <hal/hal_interface.hpp>

#include <drivers/stm32f7/sai.hpp>

namespace drivers
{

class audio_wm8994ecs : public hal::interface::audio_input<int16_t>, public hal::interface::audio_output<int16_t>
{
public:
    static constexpr uint8_t i2c_address = 0b00011010;
    static constexpr bool verify_i2c_writes = false;

    enum class input { none, mic1, mic2, mic1_mic2, line1, line2 };
    enum class output { none, speaker, headphone, both, automatic };

    audio_wm8994ecs(hal::interface::i2c_device &dev, uint8_t addr, input in, output out);
    ~audio_wm8994ecs();

    void capture(audio_input::sample_t *input, uint16_t length, const capture_cb_t &cb, bool loop) override;
    void end(void) override;

    void play(const audio_output::sample_t *output, uint16_t length, const play_cb_t &cb, bool loop) override;
    void pause(void) override;
    void resume(void) override;
    void stop(void) override;
    void set_volume(uint8_t vol) override;
private:
    hal::interface::i2c_device &i2c_dev;
    const uint8_t i2c_addr;
    typedef sai<int16_t> audio_sai;
    audio_sai sai_drv;

    capture_cb_t capture_callback;
    play_cb_t play_callback;

    uint16_t read_reg(uint16_t reg_addr);
    void write_reg(uint16_t reg_addr, uint16_t reg_val);

    uint16_t read_id(void);
    void reset(void);
};

}

#endif /* AUDIO_WM8994ECS_HPP_ */
