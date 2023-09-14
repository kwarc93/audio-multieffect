/*
 * tremolo.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "tremolo.hpp"

#include <algorithm>

#include <libs/audio_dsp.hpp>

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

tremolo::tremolo(float rate, float depth, controls::shape_type shape) : effect { effect_id::tremolo, "tremolo" },
lfo { libs::adsp::oscillator::shape::sine, sampling_frequency_hz }
{
    this->set_shape(shape);
    this->set_depth(depth);
    this->set_rate(rate);
}

tremolo::~tremolo()
{

}

void tremolo::process(const dsp_input_t& in, dsp_output_t& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Modulate output signal: y[n] = x[n] * ((1 - d) + d * m[n]) */
        return input * ((1.0f - this->ctrl.depth) + this->ctrl.depth * this->lfo.generate());
    }
    );
}

effect_attributes tremolo::get_attributes(void) const
{
    return tremolo_attributes {this->ctrl, this->stat};
}

void tremolo::set_depth(float depth)
{
    if (this->ctrl.depth == depth)
        return;

    this->ctrl.depth = std::clamp(depth, 0.0f, 0.5f);
}

void tremolo::set_rate(float rate)
{
    if (this->ctrl.rate == rate)
        return;

    this->ctrl.rate = std::clamp(rate, 1.0f, 20.0f);
    this->lfo.set_frequency(this->ctrl.rate);
}

void tremolo::set_shape(controls::shape_type shape)
{
    if (this->ctrl.shape == shape)
        return;

    this->ctrl.shape = shape;

    switch (this->ctrl.shape)
    {
    case controls::shape_type::sine:
        this->lfo.set_shape(libs::adsp::oscillator::shape::sine);
        break;
    case controls::shape_type::triangle:
        this->lfo.set_shape(libs::adsp::oscillator::shape::triangle);
        break;
    default:
        break;
    }
}

