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

template <typename T>
int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

}

//-----------------------------------------------------------------------------
/* private */

dsp_sample_t overdrive::hard_clip(dsp_sample_t in)
{
    dsp_sample_t out = in;

    /* Threshold for symmetrical soft clipping by Schetzen Formula */
    constexpr dsp_sample_t th = 1.0f/3.0f;

    const dsp_sample_t in_abs = std::abs(in);
    const dsp_sample_t sign = sgn(in);

    if (in_abs < th)
        out = 2 * in;
    else
        out = sign * (3 - ((2 - 3 * in * sign) * (2 - 3 * in * sign))) / 3;

    if (in_abs > 2 * th)
        out = sign;

    return out;
}

dsp_sample_t overdrive::soft_clip(dsp_sample_t in)
{
    return sgn(in) * (1 - std::exp(-std::abs(in)));
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

    arm_fir_init_f32(
        &this->fir,
        this->fir_coeffs.size(),
        const_cast<float32_t*>(this->fir_coeffs.data()),
        this->fir_state.data(),
        this->fir_block_size
    );
}

overdrive::~overdrive()
{

}

void overdrive::process(const dsp_input_t& in, dsp_output_t& out)
{
    /* 1. Low-pass filter for anti-aliasing. Filter whole block using FIR in-place. */
    float32_t *ptr = const_cast<float32_t*>(in.data());
    arm_fir_f32(&this->fir, ptr, ptr, this->fir_block_size);

    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        dsp_sample_t output;

        /* 2. Apply 1-st order high-pass IIR filter */
        dsp_sample_t xh_new = input - this->hp_c * this->hp_h;
        dsp_sample_t ap_y = this->hp_c * xh_new + this->hp_h;
        this->hp_h = xh_new;
        output = 0.5f * (input - ap_y);

        /* 3. Apply gain, clip & mix */
        if (this->mode == mode_type::hard)
            output = this->hard_clip(output * this->gain);
        else
            output = this->soft_clip(output * this->gain);

        output = this->mix * output + (1.0f - this->mix) * input;

        /* 4. Apply 1-st order high-shelf IIR filter */
        xh_new = output - this->hs_cc * this->hs_h;
        ap_y = this->hs_cc * xh_new + this->hs_h;
        this->hs_h = xh_new;
        output = 0.5f * this->hs_h0 * (output - ap_y) + output;

        return output;
    }
    );
}

void overdrive::set_high(float high)
{
    if (this->high == high)
        return;

    /* Calculate coefficient for 1-st order high-shelf IIR (4kHz - 10kHz range) */
    const float fc = 4000 + high * 6000;
    const float wc = 2 * fc / sampling_frequency_hz;

    const float g = -18;
    const float v0 = std::pow(10, g / 20);
    this->hs_h0 = v0 - 1.0f;

    /* Calculate coef for cut (g < 0) */
    this->hs_cc = (v0 * std::tan(pi * wc / 2) - 1) / (v0 * std::tan(pi * wc / 2) + 1);

    this->high = high;
}

void overdrive::set_low(float low)
{
    if (this->low == low)
        return;

    /* Calculate coefficient for 1-st order high-pass IIR (50Hz - 250Hz range) */
    const float fc = 50 + (1.0f - low) * 200;
    const float wc = 2 * fc / sampling_frequency_hz;

    this->hp_c = (std::tan(pi * wc / 2) - 1) / (std::tan(pi * wc / 2) + 1);

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

    /* TODO */

    this->mix = mix;
}

void overdrive::set_mode(mode_type mode)
{
    if (this->mode == mode)
        return;

    this->mode = mode;
}


