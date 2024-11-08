/*
 * hal_audio.hpp
 *
 *  Created on: 19 sie 2023
 *      Author: kwarc
 */

#ifndef HAL_AUDIO_HPP_
#define HAL_AUDIO_HPP_

#include <hal/hal_interface.hpp>

#include <drivers/audio_wm8994ecs.hpp>

namespace hal
{

//-----------------------------------------------------------------------------

    template <typename T1, typename T2>
    class audio
    {
    public:
        using input_sample_t = T1;
        using output_sample_t = T2;

        template <uint16_t N>
        using input_buffer_t = hal::interface::audio_buffer<input_sample_t, N, 2, 24>;

        template <uint16_t N>
        using output_buffer_t = hal::interface::audio_buffer<output_sample_t, N, 2, 24>;

        audio(hal::interface::audio_input<T1> *in_interface, hal::interface::audio_output<T2> *out_interface);
        virtual ~audio();

        /* Input related methods */
        void capture(T1 *input, uint16_t length, const typename hal::interface::audio_input<T1>::capture_cb_t &cb, bool loop);
        void stop_capture(void);
        void set_input_volume(uint8_t vol, uint8_t ch);

        /* Output related methods */
        void play(const T2 *output, uint16_t length, const typename hal::interface::audio_output<T2>::play_cb_t &cb, bool loop);
        void pause(void);
        void resume(void);
        void stop(void);
        void mute(bool value);
        void set_output_volume(uint8_t vol);

    protected:
        hal::interface::audio_input<T1> *input_drv;
        hal::interface::audio_output<T2> *output_drv;
    };

//-----------------------------------------------------------------------------

namespace audio_devices
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
        audio_drv{i2c, drivers::audio_wm8994ecs::i2c_address, drivers::audio_wm8994ecs::input::line1_mic2, drivers::audio_wm8994ecs::output::headphone} {}
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

//--------------------------------------------------------------------------

}

#endif /* HAL_AUDIO_HPP_ */
