/*
 * phaser.cpp
 *
 *  Created on: 29 gru 2024
 *      Author: kwarc
 */

#include "phaser.hpp"

#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

//-----------------------------------------------------------------------------
/* private */

float phaser::calc_apf_coeff(float fc, float fs)
{
    const float wc = fc / fs;
    const float k = std::tan(libs::adsp::pi * wc);
    return (k - 1) / (k + 1);
}

//-----------------------------------------------------------------------------
/* public */

phaser::phaser(float rate) : effect { effect_id::phaser },
apf_feedback {0},
lfo { libs::adsp::oscillator::shape::sine, rate, config::sampling_frequency_hz },
attr {}
{
    this->set_rate(rate);
}

phaser::~phaser()
{

}

void phaser::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Modulate allpass fc */
        constexpr float apf_fc = 141;
        const float apf_coeff = this->calc_apf_coeff(apf_fc * (1.0f + this->lfo.generate() * 0.99f), config::sampling_frequency_hz);

        /* Cascade of four all-pass filters */
        float output = input;
        this->apf1.set_coeff(apf_coeff);
        output = this->apf1.process(output);
        this->apf2.set_coeff(apf_coeff);
        output = this->apf2.process(output) + this->apf_feedback;
        this->apf3.set_coeff(apf_coeff);
        output = this->apf3.process(output);
        this->apf4.set_coeff(apf_coeff);
        output = this->apf4.process(output);
        this->apf_feedback = output * 0.41f;

        /* Mix: 50/50 */
        return input * 0.5f + output * 0.5f;
    }
    );
}

const effect_specific_attributes phaser::get_specific_attributes(void) const
{
    return this->attr;
}

void phaser::set_rate(float rate)
{
    rate = std::clamp(rate, 0.1f, 10.0f);

    if (this->attr.ctrl.rate == rate)
        return;

    this->attr.ctrl.rate = rate;
    this->lfo.set_frequency(rate);
}

