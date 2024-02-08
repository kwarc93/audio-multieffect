/*
 * vocoder.cpp
 *
 *  Created on: 24 sty 2024
 *      Author: kwarc
 */

#include "vocoder.hpp"

#include <algorithm>
#include <cassert>

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

vocoder::vocoder(float clarity, float tone, unsigned channels, vocoder_attr::controls::mode_type mode) : effect {effect_id::vocoder}
{
//    this->highpass_filter.set_coeffs(this->highpass_coeffs);

    arm_rfft_fast_init_f32(&this->fft, this->window_size);
    arm_rfft_fast_init_f32(&this->fft_conv, this->window_size / 2);

    /* Clear signal buffers */
    arm_fill_f32(0, this->carrier_input.data(), this->carrier_input.size());
    arm_fill_f32(0, this->modulator_input.data(), this->modulator_input.size());
    arm_fill_f32(0, this->output.data(), this->output.size());

    this->set_mode(mode);
    this->hold(false);
    this->set_tone(tone);
    this->set_clarity(clarity);
    this->set_channels(channels);
}

vocoder::~vocoder()
{

}

void vocoder::process(const dsp_input& in, dsp_output& out)
{
    if (this->aux_in == nullptr)
        return;

    if (this->attr.ctrl.mode == vocoder_attr::controls::mode_type::vintage)
    {
        arm_fill_f32(0, out.data(), out.size());
        this->hp.process(this->aux_in->data(), const_cast<float*>(this->aux_in->data()), this->aux_in->size());

        const auto bands = this->bands;
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
                this->modulator_hold[band] = this->modulator_env_buf[0];
            }

            const float epsi = (1 - this->attr.ctrl.clarity);
            for (unsigned i = 0; i < out.size(); i++)
            {
                if (this->attr.ctrl.hold)
                    this->modulator_env_buf[i] = this->modulator_hold[band];

                float env;
                arm_sqrt_f32(this->modulator_env_buf[i] / (this->carrier_env_buf[i] + epsi), &env);
                out[i] += this->carrier_bp_buf[i] * env;
            }
        }
    }
    else
    {
        constexpr unsigned block_size = config::dsp_vector_size;
        constexpr unsigned move_size = this->window_size - block_size;

        float *cenv_in = this->carrier_env.data();
        float *cenv_out = this->carrier_env.data() + this->window_size;
        float *menv_in = this->modulator_env.data();
        float *menv_out = this->modulator_env.data() + this->window_size;

        /* 1. ANALYSIS */

        /* Sliding window of input blocks */
        arm_copy_f32(this->carrier_input.data() + block_size, this->carrier_input.data(), move_size);
        arm_copy_f32(const_cast<float*>(in.data()), this->carrier_input.data() + move_size, block_size);

        if (!this->attr.ctrl.hold)
        {
            arm_copy_f32(this->modulator_input.data() + block_size, this->modulator_input.data(), move_size);
            this->hp.process(this->aux_in->data(), this->modulator_input.data() + move_size, block_size);
        }

        /* Windowing */
        arm_mult_f32(this->carrier_input.data(), const_cast<float*>(this->window_hann.data()), cenv_in, this->window_size);
        arm_mult_f32(this->modulator_input.data(), const_cast<float*>(this->window_hann.data()), menv_in, this->window_size);

        /* FFT of sliding window */
        arm_rfft_fast_f32(&this->fft, cenv_in, cenv_out, 0); cenv_out[1] = 0; // Clear Nyquist bin
        arm_rfft_fast_f32(&this->fft, menv_in, menv_out, 0); menv_out[1] = 0; // Clear Nyquist bin
        std::swap(cenv_in, cenv_out);
        std::swap(menv_in, menv_out);

        /* Save carrier spectrum */
        arm_copy_f32(cenv_in, this->carrier_stfft.data(), this->window_size);

        /* Squared FFT magnitude */
        arm_cmplx_mag_squared_f32(cenv_in, cenv_out, this->window_size / 2);
        arm_cmplx_mag_squared_f32(menv_in, menv_out, this->window_size / 2);
        std::swap(cenv_in, cenv_out);
        std::swap(menv_in, menv_out);

        /* Envelope smoothing (circular convolution using FFT, half the window size) */
        float *ch_fft = this->channel_fft.data() + this->window_size / 2;
        const unsigned nob = this->window_size / this->attr.ctrl.bands / 2;
        const unsigned offset = this->window_size / 2 - nob;

        arm_rfft_fast_f32(&this->fft_conv, cenv_in, cenv_out, 0); cenv_out[1] = 0; // Clear Nyquist bin
        std::swap(cenv_in, cenv_out);
        arm_cmplx_mult_cmplx_f32(cenv_in, ch_fft, cenv_out, this->window_size / 4);
        std::swap(cenv_in, cenv_out);
        arm_rfft_fast_f32(&this->fft_conv, cenv_in, cenv_out, 1);
        arm_fill_f32(cenv_out[offset], cenv_out + offset, nob);
        std::swap(cenv_in, cenv_out);

        arm_rfft_fast_f32(&this->fft_conv, menv_in, menv_out, 0); menv_out[1] = 0; // Clear Nyquist bin
        std::swap(menv_in, menv_out);
        arm_cmplx_mult_cmplx_f32(menv_in, ch_fft, menv_out, this->window_size / 4);
        std::swap(menv_in, menv_out);
        arm_rfft_fast_f32(&this->fft_conv, menv_in, menv_out, 1);
        arm_fill_f32(menv_out[offset], menv_out + offset, nob);
        std::swap(menv_in, menv_out);

        /* 2. TRANSFORMATION */
        const float epsi = (0.1f - 0.1f * this->attr.ctrl.clarity);
        for (unsigned i = 0; i < (this->window_size / 2); i++)
            arm_sqrt_f32(std::abs(menv_in[i]) / (std::abs(cenv_in[i]) + epsi), &cenv_out[i]);
        std::swap(cenv_in, cenv_out);

        arm_cmplx_mult_real_f32(this->carrier_stfft.data(), cenv_in, cenv_out, this->window_size / 2);
        std::swap(cenv_in, cenv_out);

        /* 3. SYNTHESIS */

        /* Final inverse FFT & optional windowing  */
        arm_rfft_fast_f32(&this->fft, cenv_in, cenv_out, 1);
        std::swap(cenv_in, cenv_out);

        /* Overlap add */
        arm_copy_f32(this->output.data() + block_size, this->output.data(), move_size); arm_fill_f32(0, this->output.data() + move_size, block_size);
        arm_add_f32(this->output.data(), cenv_in, this->output.data(), this->window_size);

        /* Copy result to output */
        arm_copy_f32(this->output.data(), out.data(), block_size);
    }
}

const effect_specific_attributes vocoder::get_specific_attributes(void) const
{
    return this->attr;
}

void vocoder::set_mode(vocoder_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    if (mode == vocoder_attr::controls::mode_type::vintage)
    {
        this->attr.bands_list.resize(1);
        this->attr.bands_list.at(0) = this->bands;
    }
    else
    {
        this->attr.bands_list.resize(this->bands_variants);
        for (unsigned i = 0; i < this->bands_variants; i++)
            this->attr.bands_list.at(i) = (1U << (3U + i));

        this->attr.ctrl.bands = 0;
    }

    this->attr.ctrl.mode = mode;
}

void vocoder::set_clarity(float clarity)
{
    clarity = std::clamp(clarity, 0.0f, 0.9999f);

    if (this->attr.ctrl.clarity == clarity)
        return;

    this->attr.ctrl.clarity = clarity;
}

void vocoder::set_tone(float tone)
{
    tone = std::clamp(tone, 0.0f, 1.0f);

    if (this->attr.ctrl.tone == tone)
        return;

    /* Calculate coefficient for 2nd order high-pass IIR (50Hz - 950Hz range) */
    const float fc = 50 + tone * 900;
    this->hp.calc_coeffs(fc, config::sampling_frequency_hz);

    this->attr.ctrl.tone = tone;
}

void vocoder::set_channels(unsigned ch_num)
{
    ch_num = std::clamp(ch_num, 8U, 256U);

    if (this->attr.ctrl.bands == ch_num)
        return;

    if (this->attr.ctrl.mode == vocoder_attr::controls::mode_type::vintage)
    {
        /* Number of bands is fixed */
        this->attr.ctrl.bands = this->bands;
    }
    else
    {
        /* Ceil to next power of 2 */
        this->attr.ctrl.bands = 1U << static_cast<unsigned>(std::floor(std::log2(static_cast<float>(ch_num - 1))) + 1);

        /*
         * Hanning window generation for bandpass filtering in frequency domain. Channel bandwidth is: nch * fs / window_size
         * It's equivalent to MATLAB's syntax:
         *
         * h = hanning(nob, 'periodic')
         * fftshift([zeros((window_size/2-nob)/2,1); h/sum(h); zeros((window_size/2-nob)/2,1)])
         */

        arm_fill_f32(0, this->channel_fft.data(), this->window_size);
        const unsigned nob = this->window_size / this->attr.ctrl.bands;
        assert(nob % 2 == 0);

        const float k = 2.0f / nob;
        const unsigned half_nob = nob / 2;
        for (unsigned i = 0; i < nob; i++)
        {
            float w = libs::adsp::pi * (i + 1) * k;
            w = (0.5f * (1.0f - std::cos(w))) * k;
            if (i < half_nob)
                this->channel_fft[half_nob - 1 - i] = w;
            else
                this->channel_fft[this->window_size / 2 + half_nob - 1 - i] = w;
        }

        arm_rfft_fast_f32(&this->fft_conv, this->channel_fft.data(), this->channel_fft.data() + this->window_size / 2, 0);
        this->channel_fft[this->window_size / 2 + 1] = 0; // Clear Nyquist bin
    }
}

void vocoder::hold(bool state)
{
    if (this->attr.ctrl.hold == state)
        return;

    this->attr.ctrl.hold = state;
}
