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


overdrive::overdrive(float low, float high, float gain, float mix, overdrive_attr::controls::mode_type mode) : effect { effect_id::overdrive },
attr {}
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
    /* 1. Apply 1-st order high-pass IIR filter (in-place) */
    this->iir_hp.process(in.data(), const_cast<float*>(in.data()), in.size());

    /* 2. Interpolate */
    this->intrpl.process(const_cast<float*>(in.data()), this->sample_buffer.data());

    std::transform(this->sample_buffer.begin(), this->sample_buffer.end(), this->sample_buffer.begin(),
    [this](auto input)
    {
        /* 3. Apply gain, clip & mix */
        float sample;
        if (this->attr.ctrl.mode == overdrive_attr::controls::mode_type::hard)
            sample = this->hard_clip(input * this->attr.ctrl.gain);
        else
            sample = this->soft_clip(input * this->attr.ctrl.gain);

        return this->attr.ctrl.mix * sample + (1.0f - this->attr.ctrl.mix) * input;
    }
    );

    /* 4. Decimate */
    this->decim.process(this->sample_buffer.data(), out.data());

    /* 5. Apply 2-nd order low-pass IIR filter (in-place) */
    this->iir_lp.process(out.data(), out.data(), out.size());
}

const effect_specific_attributes overdrive::get_specific_attributes(void) const
{
    return this->attr;
}

void overdrive::set_high(float high)
{
    high = std::clamp(high, 0.0f, 1.0f);

    if (this->attr.ctrl.high == high)
        return;

    /* Calculate coefficient for 2-nd order low-pass IIR (3kHz - 9kHz range) */
    const float fc = 3000 + high * 6000;

    this->iir_lp.calc_coeffs(fc, config::sampling_frequency_hz);

    this->attr.ctrl.high = high;
}

void overdrive::set_low(float low)
{
    low = std::clamp(low, 0.0f, 1.0f);

    if (this->attr.ctrl.low == low)
        return;

    /* Calculate coefficient for 2-nd order high-pass IIR (50Hz - 250Hz range) */
    const float fc = 50 + (1.0f - low) * 200;

    this->iir_hp.calc_coeffs(fc, config::sampling_frequency_hz);

    this->attr.ctrl.low = low;
}

void overdrive::set_gain(float gain)
{
    gain = std::clamp(gain, 1.0f, 200.0f);

    if (this->attr.ctrl.gain == gain)
        return;

    this->attr.ctrl.gain = gain;
}

void overdrive::set_mix(float mix)
{
    mix = std::clamp(mix, 0.0f, 1.0f);

    if (this->attr.ctrl.mix == mix)
        return;

    this->attr.ctrl.mix = mix;
}

void overdrive::set_mode(overdrive_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    this->attr.ctrl.mode = mode;
}


