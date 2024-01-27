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
    if (this->aux_in == nullptr)
        return;

    /* Add highpassed modulator to the output */
    this->highpass_filter.process(this->aux_in->data(), out.data(), out.size());
    for (auto &&s : out) s *= this->attr.ctrl.clarity;

    const auto bands = this->carrier_fb.bands;
    for (unsigned band = 0; band < bands; band++)
    {
        this->carrier_fb.bandpass(band, in.data(), this->carrier_bp_buf.data(), in.size());
        this->modulator_fb.bandpass(band, this->aux_in->data(), this->modulator_env_buf.data(), this->aux_in->size());

        arm_mult_f32(this->carrier_bp_buf.data(), this->carrier_bp_buf.data(), this->carrier_env_buf.data(), this->carrier_env_buf.size());
        arm_mult_f32(this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());

        this->carrier_fb.lowpass(band, this->carrier_env_buf.data(), this->carrier_env_buf.data(), this->carrier_env_buf.size());
        this->modulator_fb.lowpass(band, this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());

        for (unsigned i = 0; i < out.size(); i++)
        {
            arm_sqrt_f32((1 - this->attr.ctrl.clarity) + this->carrier_env_buf[i], &this->carrier_env_buf[i]);
            arm_sqrt_f32(this->modulator_env_buf[i], &this->modulator_env_buf[i]);

            out[i] += this->carrier_bp_buf[i] * this->modulator_env_buf[i] / this->carrier_env_buf[i];
        }
    }
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

