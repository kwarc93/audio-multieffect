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

tremolo::tremolo(float rate, float depth, tremolo_attributes::controls::shape_type shape) : effect { effect_id::tremolo, "tremolo" },
lfo { libs::adsp::oscillator::shape::sine, config::sampling_frequency_hz }, attributes {}
{
    this->set_shape(shape);
    this->set_depth(depth);
    this->set_rate(rate);
}

tremolo::~tremolo()
{

}

void tremolo::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Modulate output signal: y[n] = x[n] * ((1 - d) + d * m[n]) */
        return input * ((1.0f - this->attributes.ctrl.depth) + this->attributes.ctrl.depth * this->lfo.generate());
    }
    );
}

const effect_specific_attributes tremolo::get_specific_attributes(void) const
{
    return this->attributes;
}

void tremolo::set_depth(float depth)
{
    if (this->attributes.ctrl.depth == depth)
        return;

    this->attributes.ctrl.depth = std::clamp(depth, 0.0f, 0.5f);
}

void tremolo::set_rate(float rate)
{
    if (this->attributes.ctrl.rate == rate)
        return;

    this->attributes.ctrl.rate = std::clamp(rate, 1.0f, 20.0f);
    this->lfo.set_frequency(this->attributes.ctrl.rate);
}

void tremolo::set_shape(tremolo_attributes::controls::shape_type shape)
{
    if (this->attributes.ctrl.shape == shape)
        return;

    this->attributes.ctrl.shape = shape;

    switch (this->attributes.ctrl.shape)
    {
    case tremolo_attributes::controls::shape_type::sine:
        this->lfo.set_shape(libs::adsp::oscillator::shape::sine);
        break;
    case tremolo_attributes::controls::shape_type::triangle:
        this->lfo.set_shape(libs::adsp::oscillator::shape::triangle);
        break;
    default:
        break;
    }
}

