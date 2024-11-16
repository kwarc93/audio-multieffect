/*
 * hal_audio.hpp
 *
 *  Created on: 19 sie 2023
 *      Author: kwarc
 */

#ifndef HAL_AUDIO_HPP_
#define HAL_AUDIO_HPP_

#include <hal_interface.hpp>

namespace hal
{
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

        audio(hal::interface::audio_input<T1> *in_interface, hal::interface::audio_output<T2> *out_interface) :
        input_drv {in_interface}, output_drv {out_interface}
        {

        }

        virtual ~audio()
        {

        }

        /* Input related methods */
        void capture(T1 *input, uint16_t length, const typename hal::interface::audio_input<T1>::capture_cb_t &cb, bool loop)
        {
            if (this->input_drv)
                this->input_drv->capture(input, length, cb, loop);
        }

        void stop_capture(void)
        {
            if (this->input_drv)
                this->input_drv->stop_capture();
        }

        void set_input_volume(uint8_t vol, uint8_t ch)
        {
            if (this->input_drv)
                this->input_drv->set_input_volume(vol, ch);
        }

        /* Output related methods */
        void play(const T2 *output, uint16_t length, const typename hal::interface::audio_output<T2>::play_cb_t &cb, bool loop)
        {
            if (this->output_drv)
                this->output_drv->play(output, length, cb, loop);
        }

        void pause(void)
        {
            if (this->output_drv)
                this->output_drv->pause();
        }

        void resume(void)
        {
            if (this->output_drv)
                this->output_drv->resume();
        }

        void stop(void)
        {
            if (this->output_drv)
                this->output_drv->stop();
        }

        void mute(bool value)
        {
            if (this->output_drv)
                this->output_drv->mute(value);
        }

        void set_output_volume(uint8_t vol)
        {
            if (this->output_drv)
                this->output_drv->set_output_volume(vol);
        }


    protected:
        hal::interface::audio_input<T1> *input_drv;
        hal::interface::audio_output<T2> *output_drv;
    };
}

#include <hal_audio_impl.hpp>

#endif /* HAL_AUDIO_HPP_ */
