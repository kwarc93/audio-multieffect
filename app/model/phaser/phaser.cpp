/*
 * phaser.cpp
 *
 *  Created on: 29 gru 2024
 *      Author: kwarc
 */

#include "phaser.hpp"

#include "app/utils.hpp"

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

phaser::phaser(float rate, float depth, phaser_attr::controls::contour_mode contour) : effect { effect_id::phaser },
apf_feedback {0},
lfo { libs::adsp::oscillator::shape::sine, rate, config::sampling_frequency_hz },
attr {}
{
    this->set_rate(rate);
    this->set_depth(depth);
    this->set_contour(contour);
}

phaser::~phaser()
{

}

void phaser::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /*
         * Modulate allpass fc (1 - 3 octaves):
         * - map LFO sine range from [-1,1] to [a, b] using equation: y = 0.5 * (b - a) * (sin(x) + 1) + a
         * - a is always 1
         * - b is depth mapped from [0, 1] to [2, 8]
         */
        constexpr float apf_fc = 141;
        const float depth = 0.5f * ((2.0f + this->attr.ctrl.depth * 6.0f) - 1.0f);
        const float mod = depth * (this->lfo.generate() + 1.0f) + 1.0f;
        const float apf_coeff = this->calc_apf_coeff(apf_fc * mod, config::sampling_frequency_hz);

        /* Cascade of four all-pass filters (with feeedback) */
        float output = input;
        if (this->attr.ctrl.contour == phaser_attr::controls::contour_mode::on)
            output += this->apf_feedback;
        this->apf1.set_coeff(apf_coeff);
        output = this->apf1.process(output);
        this->apf2.set_coeff(apf_coeff);
        output = this->apf2.process(output);
        this->apf3.set_coeff(apf_coeff);
        output = this->apf3.process(output);
        this->apf4.set_coeff(apf_coeff);
        output = this->apf4.process(output);
        this->apf_feedback = output * 0.416f;

        /* Mix */
        return input * 0.707f + output * 0.707f;
    }
    );
}

const effect_specific_attr phaser::get_specific_attributes(void) const
{
    phaser_attr attributes { this->attr };
    attributes.ctrl.rate = utils::log_to_lin(this->attr.ctrl.rate);
    return attributes;
}

void phaser::set_rate(float rate)
{
    rate = utils::lin_to_log(rate);
    rate = std::clamp(rate, 0.01f, 1.0f);

    if (this->attr.ctrl.rate == rate)
        return;

    this->attr.ctrl.rate = rate;
    this->lfo.set_frequency(rate * 10);
}

void phaser::set_depth(float depth)
{
    depth = std::clamp(depth, 0.0f, 1.0f);

    if (this->attr.ctrl.depth == depth)
        return;

    this->attr.ctrl.depth = depth;
}

void phaser::set_contour(phaser_attr::controls::contour_mode contour)
{
    if (this->attr.ctrl.contour == contour)
        return;

    this->attr.ctrl.contour = contour;
}

