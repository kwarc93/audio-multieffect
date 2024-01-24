/*
 * tremolo.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "tremolo.hpp"

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

tremolo::tremolo(float rate, float depth, tremolo_attr::controls::shape_type shape) : effect { effect_id::tremolo },
lfo { libs::adsp::oscillator::shape::sine, rate, config::sampling_frequency_hz },
attr {}
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
        float mod = this->lfo.generate();
        if (this->attr.ctrl.shape == tremolo_attr::controls::shape_type::square)
            mod = this->lpf.process(mod);
        return input * ((1.0f - this->attr.ctrl.depth) + this->attr.ctrl.depth * mod);
    }
    );
}

const effect_specific_attributes tremolo::get_specific_attributes(void) const
{
    return this->attr;
}

void tremolo::set_depth(float depth)
{
    depth = std::clamp(depth, 0.0f, 0.5f);

    if (this->attr.ctrl.depth == depth)
        return;

    this->attr.ctrl.depth = depth;
}

void tremolo::set_rate(float rate)
{
    rate = std::clamp(rate, 1.0f, 20.0f);

    if (this->attr.ctrl.rate == rate)
        return;

    this->attr.ctrl.rate = rate;
    this->lfo.set_frequency(rate);
    this->lpf.calc_coeff(3 * rate, config::sampling_frequency_hz);
}

void tremolo::set_shape(tremolo_attr::controls::shape_type shape)
{
    if (this->attr.ctrl.shape == shape)
        return;

    this->attr.ctrl.shape = shape;

    switch (this->attr.ctrl.shape)
    {
    case tremolo_attr::controls::shape_type::sine:
        this->lfo.set_shape(libs::adsp::oscillator::shape::sine);
        break;
    case tremolo_attr::controls::shape_type::square:
        this->lfo.set_shape(libs::adsp::oscillator::shape::square);
        break;
    default:
        break;
    }
}

