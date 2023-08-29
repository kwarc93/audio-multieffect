/*
 * echo.cpp
 *
 *  Created on: 29 sie 2023
 *      Author: kwarc
 */

#include "echo.hpp"

#include <cstring>
#include <algorithm>

#include <cmsis/stm32f7xx.h>
#include <cmsis/dsp/arm_math.h>

using namespace mfx;

/* TODO: Move universal comb filter to separate class */

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
__attribute__((section(".sdram")))
std::array<dsp_sample_t, 1 * sampling_frequency_hz> delay_line; // Maximum delay time: 1s
}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


echo::echo(float blend, float time, float feedback, mode_type mode) : effect { effect_id::echo, "echo" }
{
    this->set_mode(mode);
    this->set_feedback(feedback);
    this->set_time(time);
    this->set_blend(blend);

    /* Initialize delay line */
    delay_line.fill(0);

    this->delay_line_delay_samples = this->time * delay_line.size();
    this->delay_line_write_index = 0;

    if (this->delay_line_delay_samples == 0 || this->delay_line_delay_samples == delay_line.size())
        this->delay_line_read_index = this->delay_line_write_index;
    else
        this->delay_line_read_index = this->delay_line_write_index + delay_line.size() - this->delay_line_delay_samples;
}

echo::~echo()
{

}

void echo::process(const dsp_input_t& in, dsp_output_t& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Universal comb filter */
        dsp_sample_t in_d = delay_line[this->delay_line_read_index % delay_line.size()];
        this->delay_line_read_index++;

        dsp_sample_t in_h = input + this->feedback * in_d;

        delay_line[this->delay_line_write_index % delay_line.size()] = in_h;
        this->delay_line_write_index++;

        return this->feedforward * in_d + this->blend * in_h;
    }
    );
}

void echo::set_blend(float blend)
{
    if (this->blend == blend || mode == mode_type::delay)
        return;

    this->blend = std::clamp(blend, 0.0f, 1.0f);
}

void echo::set_time(float time)
{
    if (this->time == time)
        return;

    this->time = std::clamp(time, 0.1f, 1.0f);

    this->delay_line_delay_samples = this->time * delay_line.size();

    if (this->delay_line_delay_samples == 0 || this->delay_line_delay_samples == delay_line.size())
        this->delay_line_read_index = this->delay_line_write_index;
    else
        this->delay_line_read_index = this->delay_line_write_index + delay_line.size() - this->delay_line_delay_samples;

}

void echo::set_feedback(float feedback)
{
    if (this->feedback == feedback)
        return;

    this->feedback = std::clamp(feedback, 0.0f, 1.0f);

    if (this->mode == mode_type::delay)
    {
        this->feedforward = this->feedback;
        this->feedback = 0;
    }
}

void echo::set_mode(mode_type mode)
{
    if (this->mode == mode)
        return;

    if (mode == mode_type::delay)
    {
        this->blend = 1;
        this->feedback = 0;
    }
    else if (mode == mode_type::echo)
    {
        this->feedforward = 0;
    }

    this->mode = mode;
}

