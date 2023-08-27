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
/* private */

//-----------------------------------------------------------------------------
/* public */

tremolo::tremolo(float rate, float depth, shape_type shape) : effect { effect_id::tremolo, "tremolo" }
{
    this->lfo_counter = 0;
    this->lfo_dir = 1;

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
        /* Modulate output signal: y[n] = x[n] * ((1 - d) + d * lfo[n]) */
        float output = input * ((1.0f - this->depth) + this->depth * (this->lfo_counter / this->lfo_counter_limit));

        if (this->lfo_shape == shape_type::triangle)
        {
            if (this->lfo_counter >= this->lfo_counter_limit)
            {
                this->lfo_dir = -1.0f;
            }
            else if (this->lfo_counter <= -this->lfo_counter_limit)
            {
                this->lfo_dir = 1.0f;
            }

            this->lfo_counter += this->lfo_dir;
        }
        else if (this->lfo_shape == shape_type::sine)
        {
            /* TODO */
        }

        return output;
    }
    );
}

void tremolo::set_depth(float depth)
{
    this->depth = std::clamp(depth, 0.0f, 1.0f);
}

void tremolo::set_rate(float rate)
{
    float lfo_frequency = std::clamp(rate, 1.0f, 20.0f);

    /* Compute counter limit based on desired LFO frequency (cnt_limit = (fs / f_lfo) / 4) */
    this->lfo_counter_limit = 0.25f * (sampling_frequency_hz / lfo_frequency);

    /* Update current LFO counter after calculating new limit */
    this->lfo_counter = std::clamp(this->lfo_counter, -this->lfo_counter_limit, this->lfo_counter_limit);
}


void tremolo::set_shape(shape_type shape)
{
    this->lfo_shape = shape;
}




