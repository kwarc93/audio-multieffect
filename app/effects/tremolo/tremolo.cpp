/*
 * tremolo.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "tremolo.hpp"

#include <algorithm>

#include <cmsis/stm32f7xx.h>
#include <cmsis/dsp/arm_math.h>

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr float pi = 3.14159265359f;

}

//-----------------------------------------------------------------------------
/* private */

float tremolo::lfo(void)
{
    float out = 1.0f;

    if (this->lfo_shape == shape_type::triangle)
    {
        out = this->lfo_counter / this->lfo_counter_limit;

        if (this->lfo_counter >= this->lfo_counter_limit)
            this->lfo_counter_dir = -1.0f;
        else if (this->lfo_counter <= -this->lfo_counter_limit)
            this->lfo_counter_dir = 1.0f;

        this->lfo_counter += this->lfo_counter_dir;
    }
    else if (this->lfo_shape == shape_type::sine)
    {
        out = arm_sin_f32(this->lfo_counter);

        if (this->lfo_counter >= pi)
            this->lfo_counter -= 2 * pi;

        this->lfo_counter += 2 * pi * this->lfo_freq / sampling_frequency_hz;
    }

    return out;
}

//-----------------------------------------------------------------------------
/* public */

tremolo::tremolo(float rate, float depth, shape_type shape) : effect { effect_id::tremolo, "tremolo" }
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
        return input * ((1.0f - this->depth) + this->depth * this->lfo());
    }
    );
}

void tremolo::set_depth(float depth)
{
    this->depth = std::clamp(depth, 0.0f, 1.0f);
}

void tremolo::set_rate(float rate)
{
    this->lfo_freq = std::clamp(rate, 1.0f, 20.0f);

    if (this->lfo_shape == shape_type::triangle)
    {
        /* Compute counter limit based on desired LFO frequency (cnt_limit = (fs / f_lfo) / 4) */
        this->lfo_counter_limit = 0.25f * (sampling_frequency_hz / this->lfo_freq);

        /* Update current LFO counter after calculating new limit */
        this->lfo_counter = std::clamp(this->lfo_counter, -this->lfo_counter_limit, this->lfo_counter_limit);
    }
}

void tremolo::set_shape(shape_type shape)
{
    this->lfo_shape = shape;

    /* Reset lfo */
    this->lfo_counter = 0;
    this->lfo_counter_dir = 1;
    this->lfo_counter_limit = 0.25f * (sampling_frequency_hz / this->lfo_freq);
}

