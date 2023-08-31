/*
 * hal_audio.cpp
 *
 *  Created on: 19 sie 2023
 *      Author: kwarc
 */

#include "hal_audio.hpp"

using namespace hal;

//-----------------------------------------------------------------------------
/* helpers */

template class audio<drivers::audio_wm8994ecs::audio_input::sample_t, drivers::audio_wm8994ecs::audio_output::sample_t>;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

template <typename T1, typename T2>
audio<T1, T2>::audio(hal::interface::audio_input<T1> *in_interface, hal::interface::audio_output<T2> *out_interface) :
input_drv {in_interface}, output_drv {out_interface}
{

}

template <typename T1, typename T2>
audio<T1, T2>::~audio()
{

}

template <typename T1, typename T2>
void audio<T1, T2>::capture(T1 *input, uint16_t length, const typename hal::interface::audio_input<T1>::capture_cb_t &cb, bool loop)
{
    if (this->input_drv)
        this->input_drv->capture(input, length, cb, loop);
}

template <typename T1, typename T2>
void audio<T1, T2>::stop_capture(void)
{
    if (this->input_drv)
        this->input_drv->stop_capture();
}

template <typename T1, typename T2>
void audio<T1, T2>::set_input_volume(uint8_t vol)
{
    if (this->input_drv)
        this->input_drv->set_input_volume(vol);
}


template <typename T1, typename T2>
void audio<T1, T2>::play(const T2 *output, uint16_t length, const typename hal::interface::audio_output<T2>::play_cb_t &cb, bool loop)
{
    if (this->output_drv)
        this->output_drv->play(output, length, cb, loop);
}

template <typename T1, typename T2>
void audio<T1, T2>::pause(void)
{
    if (this->output_drv)
        this->output_drv->pause();
}

template <typename T1, typename T2>
void audio<T1, T2>::resume(void)
{
    if (this->output_drv)
        this->output_drv->resume();
}

template <typename T1, typename T2>
void audio<T1, T2>::stop(void)
{
    if (this->output_drv)
        this->output_drv->stop();
}

template <typename T1, typename T2>
void audio<T1, T2>::set_output_volume(uint8_t vol)
{
    if (this->output_drv)
        this->output_drv->set_output_volume(vol);
}

