/*
 * hal_audio_impl.hpp
 *
 *  Created on: 16 lis 2024
 *      Author: kwarc
 */

#ifndef STM32F7_HAL_AUDIO_IMPL_HPP_
#define STM32F7_HAL_AUDIO_IMPL_HPP_

#include <drivers/audio_wm8994ecs.hpp>

namespace hal::audio_devices
{
    /*
     * Audio codec WM8994 configuration: stereo 24bit 48kHz
     * NOTE: On STM32F746G-DISCO board, the digital microphone's left & right channels are swapped.
     */
    class codec : public audio<drivers::audio_wm8994ecs::audio_input::sample_t, drivers::audio_wm8994ecs::audio_output::sample_t>
    {
    public:
        codec(hal::interface::i2c_proxy &i2c) :
        audio{&audio_drv, &audio_drv},
        audio_drv{i2c, drivers::audio_wm8994ecs::i2c_address, drivers::audio_wm8994ecs::input::line1_mic2, drivers::audio_wm8994ecs::output::headphone}
        {

        }

        void route_onboard_mic_to_aux(bool enabled)
        {
            const auto left_ch = drivers::audio_wm8994ecs::frame_slots::slot0_left;
            const auto right_ch = enabled ? drivers::audio_wm8994ecs::frame_slots::slot1_right :
                                  drivers::audio_wm8994ecs::frame_slots::slot0_right;

            audio_drv.set_input_channels(left_ch, right_ch);
        }
    private:
        drivers::audio_wm8994ecs audio_drv;
    };
}

#endif /* STM32F7_HAL_AUDIO_IMPL_HPP_ */
