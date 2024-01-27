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

vocoder::vocoder(float clarity) : effect {effect_id::vocoder}, noise_generator {-1.0f, 1.0f}
{
    this->lowpass_filter.set_coeffs(this->lowpass_coeffs);
    this->highpass_filter.set_coeffs(this->highpass_coeffs);
    this->highpass_filter2.set_coeffs(this->highpass_coeffs);

    this->set_clarity(clarity);
}

vocoder::~vocoder()
{

}

void vocoder::process(const dsp_input& in, dsp_output& out)
{
    /* Vocoder algorithm based on analog 16-band 'Bode 7702 Vocoder' */

    if (this->aux_in == nullptr)
        return;

    arm_fill_f32(0, out.data(), out.size());

    /* Get envelope of 'hiss' */
    this->highpass_filter.process(this->aux_in->data(), this->hiss_hp_buf.data(), this->hiss_hp_buf.size());
    arm_mult_f32(this->hiss_hp_buf.data(), this->hiss_hp_buf.data(), this->hiss_env_buf.data(), this->hiss_env_buf.size());
    this->lowpass_filter.process(this->hiss_env_buf.data(), this->hiss_env_buf.data(), this->hiss_env_buf.size());
    for (auto &&e : this->hiss_env_buf)
    {
        arm_sqrt_f32(e, &e);
        e = std::clamp(e, 0.0f, 1.0f);
    }

    /* buzz/hiss balance (0 - 'buzz' only, 1 - 'hiss' only) */
    const float buzz_hiss_balance = this->attr.ctrl.clarity;

    /* Add noise to the carrier */
    for (unsigned i = 0; i < this->hiss_env_buf.size(); i++)
    {
        /* Generate noise & pass through HP filter */
        float noise = this->noise_generator();
        this->highpass_filter2.process(&noise, &noise, 1);

        /* Mix buzz & hiss */
        const float hiss = this->hiss_env_buf[i] * noise * buzz_hiss_balance;
        const float buzz = (1 - this->hiss_env_buf[i]) * in[i] * (1 - buzz_hiss_balance);
        this->carrier_and_noise_buf[i] = buzz + hiss;
    }

    const auto bands = this->carrier_fb.bands;
    for (unsigned band = 0; band < bands; band++)
    {
        /* Bandpass the carrier & modulator signals */
        this->carrier_fb.bandpass(band, this->carrier_and_noise_buf.data(), this->carrier_bp_buf.data(), carrier_bp_buf.size());
        this->modulator_fb.bandpass(band, this->aux_in->data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());

        /* Square the modulator signal */
        arm_mult_f32(this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());

        /* Lowpass the squared modulator signal */
        this->modulator_fb.lowpass(band, this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());

        /* Square root the modulator signal and make synthesis */
        for (unsigned i = 0; i < out.size(); i++)
        {
            arm_sqrt_f32(this->modulator_env_buf[i], &this->modulator_env_buf[i]);
            out[i] += this->carrier_bp_buf[i] * this->modulator_env_buf[i];
        }
    }

    /* Add 'hiss' to the output */
    for (unsigned i = 0; i < out.size(); i++)
        out[i] += this->hiss_env_buf[i] * this->hiss_hp_buf[i];
}

const effect_specific_attributes vocoder::get_specific_attributes(void) const
{
    return this->attr;
}

void vocoder::set_clarity(float clarity)
{
    clarity = std::clamp(clarity, 0.0f, 0.99f);

    if (this->attr.ctrl.clarity == clarity)
        return;

    this->attr.ctrl.clarity = clarity;
}

