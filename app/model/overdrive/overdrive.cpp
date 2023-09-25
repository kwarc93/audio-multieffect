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

float overdrive::hard_clip(float in)
{
    float out = in;

    /* Threshold for symmetrical soft clipping by Schetzen Formula */
    constexpr float th = 1.0f/3.0f;

    const float in_abs = std::abs(in);
    const float sign = libs::adsp::sgn(in);

    if (in_abs < th)
        out = 2 * in;
    else
        out = sign * (3 - ((2 - 3 * in_abs) * (2 - 3 * in_abs))) / 3;

    if (in_abs > 2 * th)
        out = sign;

    return out;
}

float overdrive::soft_clip(float in)
{
    return libs::adsp::sgn(in) * (1 - std::exp(-std::abs(in)));
}

//-----------------------------------------------------------------------------
/* public */


overdrive::overdrive(float low, float high, float gain, float mix, overdrive_attributes::controls::mode_type mode) : effect { effect_id::overdrive, "overdrive" },
attributes {}
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

void overdrive::process(const dsp_input& in, dsp_output& out)
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
        if (this->attributes.ctrl.mode == overdrive_attributes::controls::mode_type::hard)
            sample = this->hard_clip(sample * this->attributes.ctrl.gain);
        else
            sample = this->soft_clip(sample * this->attributes.ctrl.gain);

        return this->attributes.ctrl.mix * sample + (1.0f - this->attributes.ctrl.mix) * input;
    }
    );

    /* 4. Apply 2-nd order low-pass IIR filter (in-place) */
    this->iir_lp.process(out.data(), out.data(), out.size());
}

const effect_specific_attributes overdrive::get_specific_attributes(void) const
{
    return this->attributes;
}

void overdrive::set_high(float high)
{
    high = std::clamp(high, 0.0f, 1.0f);

    if (this->attributes.ctrl.high == high)
        return;

    /* Calculate coefficient for 2-nd order low-pass IIR (3kHz - 9kHz range) */
    const float fc = 3000 + high * 6000;

    this->iir_lp.calc_coeffs(fc, config::sampling_frequency_hz);

    this->attributes.ctrl.high = high;
}

void overdrive::set_low(float low)
{
    low = std::clamp(low, 0.0f, 1.0f);

    if (this->attributes.ctrl.low == low)
        return;

    /* Calculate coefficient for 2-nd order high-pass IIR (50Hz - 250Hz range) */
    const float fc = 50 + (1.0f - low) * 200;

    this->iir_hp.calc_coeffs(fc, config::sampling_frequency_hz);

    this->attributes.ctrl.low = low;
}

void overdrive::set_gain(float gain)
{
    gain = std::clamp(gain, 0.0f, 100.0f);

    if (this->attributes.ctrl.gain == gain)
        return;

    this->attributes.ctrl.gain = gain;
}

void overdrive::set_mix(float mix)
{
    mix = std::clamp(mix, 0.0f, 1.0f);

    if (this->attributes.ctrl.mix == mix)
        return;

    this->attributes.ctrl.mix = mix;
}

void overdrive::set_mode(overdrive_attributes::controls::mode_type mode)
{
    if (this->attributes.ctrl.mode == mode)
        return;

    this->attributes.ctrl.mode = mode;
}


