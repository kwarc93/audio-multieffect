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

#include <vector>

namespace drivers
{

class audio_wm8994ecs : public hal::interface::audio_input<int16_t>, public hal::interface::audio_output<int16_t>
{
public:

    audio_wm8994ecs();
    ~audio_wm8994ecs();

    void capture(std::vector<audio_input::sample_t> &input, const capture_cb_t &cb) override;
    void end(void) override;

    void play(const std::vector<audio_output::sample_t> &output, const play_cb_t &cb) override;
    void pause(void) override;
    void resume(void) override;
    void stop(void) override;
private:
    using audio_sai = sai<int16_t>;
    audio_sai sai_drv;
};

}

#endif /* AUDIO_WM8994ECS_HPP_ */
