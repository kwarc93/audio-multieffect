/*
 * vocoder.cpp
 *
 *  Created on: 24 sty 2024
 *      Author: kwarc
 */

#include "vocoder.hpp"

#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

//-----------------------------------------------------------------------------
/* private */


//-----------------------------------------------------------------------------
/* public */

vocoder::vocoder(float clarity, bool hold) : effect {effect_id::vocoder}
{
#ifndef VOCODER_ALG_FFT
    this->highpass_filter.set_coeffs(this->highpass_coeffs);
#else
    arm_rfft_fast_init_f32(&this->fft, this->fft_size);
    arm_fill_f32(0, this->ir_fft.data(), this->ir_fft.size());
    arm_fill_f32(0, this->carrier_input.data(), this->carrier_input.size());
    arm_fill_f32(0, this->modulator_input.data(), this->modulator_input.size());
    arm_fill_f32(0, this->output.data(), this->output.size());

    /* Precompute FFT of IR */
//    constexpr uint32_t offset = (this->fft_size - this->fft_size / this->bands) / 2;
    arm_rfft_fast_init_f32(&this->fft, this->fft_size / 2);
    arm_fill_f32(0, this->ir_fft.data(), this->ir_fft.size());
    arm_copy_f32(const_cast<float*>(this->band_hann.data()), this->ir_fft.data(), this->band_hann.size());
    arm_rfft_fast_f32(&this->fft, this->ir_fft.data(), this->ir_fft.data() + this->fft_size / 2, 0); // fftshift?

    arm_rfft_fast_init_f32(&this->fft, this->fft_size);
#endif

    this->set_clarity(clarity);
    this->hold(hold);
}

vocoder::~vocoder()
{

}

void vocoder::process(const dsp_input& in, dsp_output& out)
{
    if (this->aux_in == nullptr)
        return;
#ifndef VOCODER_ALG_FFT
    /* Add highpassed modulator to the output */
    this->highpass_filter.process(this->aux_in->data(), out.data(), out.size());
    for (auto &&s : out) s *= this->attr.ctrl.clarity;

    const auto bands = this->carrier_fb.bands;
    for (unsigned band = 0; band < bands; band++)
    {
        this->carrier_fb.bandpass(band, in.data(), this->carrier_bp_buf.data(), in.size());
        arm_mult_f32(this->carrier_bp_buf.data(), this->carrier_bp_buf.data(), this->carrier_env_buf.data(), this->carrier_env_buf.size());
        this->carrier_fb.lowpass(band, this->carrier_env_buf.data(), this->carrier_env_buf.data(), this->carrier_env_buf.size());

        if (!this->attr.ctrl.hold)
        {
            this->modulator_fb.bandpass(band, this->aux_in->data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());
            arm_mult_f32(this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());
            this->modulator_fb.lowpass(band, this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());
            arm_sqrt_f32(this->modulator_env_buf[0], &this->modulator_hold[band]);
        }

        for (unsigned i = 0; i < out.size(); i++)
        {
            arm_sqrt_f32((1 - this->attr.ctrl.clarity) + this->carrier_env_buf[i], &this->carrier_env_buf[i]);
            if (!this->attr.ctrl.hold)
                arm_sqrt_f32(this->modulator_env_buf[i], &this->modulator_env_buf[i]);
            else
                this->modulator_env_buf[i] = this->modulator_hold[band];

            out[i] += this->carrier_bp_buf[i] * this->modulator_env_buf[i] / this->carrier_env_buf[i];
        }
    }
#else
    constexpr uint32_t block_size = config::dsp_vector_size;

    const uint32_t move_size = this->carrier_input.size() - block_size;

    /* Sliding window of input blocks */
    arm_copy_f32(this->carrier_input.data() + block_size, this->carrier_input.data(), move_size);
    arm_copy_f32(const_cast<float*>(in.data()), this->carrier_input.data() + move_size, block_size);

    arm_copy_f32(this->modulator_input.data() + block_size, this->modulator_input.data(), move_size);
    arm_copy_f32(const_cast<float*>(this->aux_in->data()), this->modulator_input.data() + move_size, block_size);

    /* Windowing */
    arm_mult_f32(this->carrier_input.data(), const_cast<float*>(this->window_hann.data()), this->carrier_fft.data(), this->fft_size);
    arm_mult_f32(this->modulator_input.data(), const_cast<float*>(this->window_hann.data()), this->modulator_fft.data(), this->fft_size);

    /* FFT of sliding window */
    arm_rfft_fast_f32(&this->fft, this->carrier_fft.data(), this->carrier_fft.data() + this->fft_size, 0);
    arm_rfft_fast_f32(&this->fft, this->modulator_fft.data(), this->modulator_fft.data() + this->fft_size, 0);

    /* Save carrier spectrum */
    arm_copy_f32(this->carrier_fft.data() + this->fft_size, this->carrier_spectrum.data(), this->fft_size);

    /* Squared FFT */
    arm_cmplx_mag_squared_f32(this->carrier_fft.data() + this->fft_size, this->carrier_fft.data(), this->fft_size / 2);
    arm_cmplx_mag_squared_f32(this->modulator_fft.data() + this->fft_size, this->modulator_fft.data(), this->fft_size / 2);

    /* Switch to 512 FFT */
    arm_rfft_fast_init_f32(&this->fft, this->fft_size / 2);

    /* Envelope calculation (circular convolution) */
    arm_rfft_fast_f32(&this->fft, this->carrier_fft.data(), this->carrier_fft.data() + this->fft_size, 0);
    arm_cmplx_mult_cmplx_f32(this->carrier_fft.data() + this->fft_size, this->ir_fft.data() + this->fft_size, this->carrier_fft.data(), this->fft_size / 4);
    arm_rfft_fast_f32(&this->fft, this->carrier_fft.data(), this->carrier_fft.data() + this->fft_size, 1);

    arm_rfft_fast_f32(&this->fft, this->modulator_fft.data(), this->modulator_fft.data() + this->fft_size, 0);
    arm_cmplx_mult_cmplx_f32(this->modulator_fft.data() + this->fft_size, this->ir_fft.data() + this->fft_size, this->modulator_fft.data(), this->fft_size / 4);
    arm_rfft_fast_f32(&this->fft, this->modulator_fft.data(), this->modulator_fft.data() + this->fft_size, 1);

    for (unsigned i = this->fft_size; i < (this->fft_size + this->fft_size / 2); i++)
    {
        arm_sqrt_f32(0.00001f + this->carrier_fft[i], &this->carrier_fft[i]);
        arm_sqrt_f32(this->modulator_fft[i], &this->modulator_fft[i]);
        this->carrier_fft[i] = 1.0f / this->carrier_fft[i];
    }

    /* Synthesis */
    arm_cmplx_mult_real_f32(this->carrier_spectrum.data(), this->modulator_fft.data() + this->fft_size, this->modulator_fft.data(), this->fft_size / 2);
    arm_cmplx_mult_real_f32(this->modulator_fft.data(), this->carrier_fft.data() + this->fft_size, this->carrier_fft.data(), this->fft_size / 2);

    /* Switch back to 1024 FFT */
    arm_rfft_fast_init_f32(&this->fft, this->fft_size);

    /* Final inverse FFT & windowing  */
    arm_rfft_fast_f32(&this->fft, this->carrier_fft.data(), this->carrier_fft.data() + this->fft_size, 1);
    arm_mult_f32(this->carrier_fft.data() + this->fft_size, const_cast<float*>(this->window_hann.data()), this->carrier_fft.data(), this->fft_size);

    /* Overlap add */
    arm_copy_f32(this->output.data() + block_size, this->output.data(), move_size); arm_fill_f32(0, this->output.data() + move_size, block_size);
    arm_add_f32(this->output.data(), this->carrier_fft.data(), this->output.data(), this->fft_size);

    /* Copy result to output */
    arm_copy_f32(this->output.data(), out.data(), block_size);

#endif
}

const effect_specific_attributes vocoder::get_specific_attributes(void) const
{
    return this->attr;
}

void vocoder::set_clarity(float clarity)
{
    clarity = std::clamp(clarity, 0.0f, 0.999f);

    if (this->attr.ctrl.clarity == clarity)
        return;

    this->attr.ctrl.clarity = clarity;
}

void vocoder::hold(bool state)
{
    if (this->attr.ctrl.hold == state)
        return;

    this->attr.ctrl.hold = state;
}

