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
        out = sign * (3 - ((2 - 3 * in_abs) * (2 - 3 * in_abs))) / 3;

    if (in_abs > 2 * th)
        out = sign;

    return out;
}

dsp_sample_t overdrive::soft_clip(dsp_sample_t in)
{
    return sgn(in) * (1 - std::exp(-std::abs(in)));
}

void overdrive::iir_biquad_lp_calc_coeffs(float fs, float fc)
{
    const float wc = fc / fs;
    const float k = std::tan(pi * wc);
    const float q = 1.0f / std::sqrt(2.0f);

    const float k2q = k * k * q;
    const float denum = 1.0f / (k2q + k + q);

    /* 2-nd order */
    this->iir_lp_coeffs[0] = k2q * denum; // b0
    this->iir_lp_coeffs[1] = 2 * this->iir_lp_coeffs[0]; // b1
    this->iir_lp_coeffs[2] = this->iir_lp_coeffs[0]; // b2
    this->iir_lp_coeffs[3] = -2 * q * (k * k - 1) * denum; // -a1
    this->iir_lp_coeffs[4] = -(k2q - k + q) * denum; // -a2
}

void overdrive::iir_biquad_hp_calc_coeffs(float fs, float fc)
{
    const float wc = fc / fs;
    const float k = std::tan(pi * wc);
    const float q = 1.0f / std::sqrt(2.0f);

    const float k2q = k * k * q;
    const float denum = 1.0f / (k2q + k + q);

    /* 2-nd order */
    this->iir_hp_coeffs[0] = q * denum; // b0
    this->iir_hp_coeffs[1] = -2 * this->iir_hp_coeffs[0]; // b1
    this->iir_hp_coeffs[2] = this->iir_hp_coeffs[0]; // b2
    this->iir_hp_coeffs[3] = -2 * q * (k * k - 1) * denum; // -a1
    this->iir_hp_coeffs[4] = -(k2q - k + q) * denum; // -a2
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

    arm_fir_init_f32
    (
        &this->fir,
        this->fir_coeffs.size(),
        const_cast<float*>(this->fir_coeffs.data()),
        this->fir_state.data(),
        this->fir_block_size
    );

    arm_biquad_cascade_df1_init_f32
    (
        &this->iir_lp,
        this->iir_lp_biquad_stages,
        this->iir_lp_coeffs.data(),
        this->iir_lp_state.data()
    );

    arm_biquad_cascade_df1_init_f32
    (
        &this->iir_hp,
        this->iir_hp_biquad_stages,
        this->iir_hp_coeffs.data(),
        this->iir_hp_state.data()
    );
}

overdrive::~overdrive()
{

}

void overdrive::process(const dsp_input_t& in, dsp_output_t& out)
{
    auto &ccin = const_cast<dsp_input_t&>(in);

    /* 1. Low-pass filter for anti-aliasing. Filter whole block using FIR filter. */
    arm_fir_f32(&this->fir, ccin.data(), out.data(), out.size());

    /* 2. Apply 1-st order high-pass IIR filter (in-place) */
    arm_biquad_cascade_df1_f32(&this->iir_hp, out.data(), out.data(), out.size());

    std::transform(ccin.begin(), ccin.end(), out.begin(), out.begin(),
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
    arm_biquad_cascade_df1_f32(&this->iir_lp, out.data(), out.data(), out.size());
}

void overdrive::set_high(float high)
{
    if (this->high == high)
        return;

    /* Calculate coefficient for 2-nd order low-pass IIR (2kHz - 6kHz range) */
    const float fc = 2000 + high * 4000;

    iir_biquad_lp_calc_coeffs(sampling_frequency_hz, fc);

    this->high = high;
}

void overdrive::set_low(float low)
{
    if (this->low == low)
        return;

    /* Calculate coefficient for 2-nd order high-pass IIR (50Hz - 250Hz range) */
    const float fc = 50 + (1.0f - low) * 200;

    iir_biquad_hp_calc_coeffs(sampling_frequency_hz, fc);

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


