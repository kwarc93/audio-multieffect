/*
 * overdrive.cpp
 *
 *  Created on: 30 sie 2023
 *      Author: kwarc
 */

#include "overdrive.hpp"

#include <cmath>
#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

//-----------------------------------------------------------------------------
/* private */

dsp_sample_t overdrive::hard_clip(dsp_sample_t in)
{
    dsp_sample_t out = in;

    /* Threshold for symmetrical soft clipping by Schetzen Formula */
    constexpr dsp_sample_t th = 1.0f/3.0f;

    const dsp_sample_t in_abs = std::abs(in);
    const dsp_sample_t sign = libs::adsp::sgn(in);

    if (in_abs < th)
        out = 2 * in;
    else
        out = sign * (3 - ((2 - 3 * in_abs) * (2 - 3 * in_abs))) / 3;

    if (in_abs > 2 * th)
        out = sign;

    return out;
}

dsp_sample_t overdrive::soft_clip(dsp_sample_t in)
{
    return libs::adsp::sgn(in) * (1 - std::exp(-std::abs(in)));
}

//-----------------------------------------------------------------------------
/* public */


overdrive::overdrive(float low, float high, float gain, float mix, mode_type mode) : effect { effect_id::overdrive, "overdrive" }
{
    this->set_mode(mode);
    this->set_low(low);
    this->set_high(high);
    this->set_gain(gain);
    this->set_mix(mix);
}

overdrive::~overdrive()
{

}

void overdrive::process(const dsp_input_t& in, dsp_output_t& out)
{
    /* 1. Low-pass filter for anti-aliasing. Filter whole block using FIR filter. */
    this->fir_lp.process(in.data(), out.data());

    /* 2. Apply 1-st order high-pass IIR filter (in-place) */
    this->iir_hp.process(out.data(), out.data(), out.size());

    std::transform(in.begin(), in.end(), out.begin(), out.begin(),
    [this](auto input, auto output)
    {
        auto sample = output;

        /* 3. Apply gain, clip & mix */
        if (this->mode == mode_type::hard)
            sample = this->hard_clip(sample * this->gain);
        else
            sample = this->soft_clip(sample * this->gain);

        return this->mix * sample + (1.0f - this->mix) * input;
    }
    );

    /* 4. Apply 2-nd order low-pass IIR filter (in-place) */
    this->iir_lp.process(out.data(), out.data(), out.size());
}

void overdrive::set_high(float high)
{
    if (this->high == high)
        return;

    /* Calculate coefficient for 2-nd order low-pass IIR (3kHz - 9kHz range) */
    const float fc = 3000 + high * 6000;

    this->iir_lp.calc_coeffs(fc, sampling_frequency_hz);

    this->high = high;
}

void overdrive::set_low(float low)
{
    if (this->low == low)
        return;

    /* Calculate coefficient for 2-nd order high-pass IIR (50Hz - 250Hz range) */
    const float fc = 50 + (1.0f - low) * 200;

    this->iir_hp.calc_coeffs(fc, sampling_frequency_hz);

    this->low = low;
}

void overdrive::set_gain(float gain)
{
    if (this->gain == gain)
        return;

    this->gain = gain;
}

void overdrive::set_mix(float mix)
{
    if (this->mix == mix)
        return;

    this->mix = mix;
}

void overdrive::set_mode(mode_type mode)
{
    if (this->mode == mode)
        return;

    this->mode = mode;
}


