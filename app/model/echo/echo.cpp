/*
 * echo.cpp
 *
 *  Created on: 29 sie 2023
 *      Author: kwarc
 */

#include "echo.hpp"

#include <array>
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
std::array<dsp_sample_t, 1 * sampling_frequency_hz> delay_line_memory; // Maximum delay time: 1s
}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


echo::echo(float blend, float time, float feedback, mode_type mode) : effect { effect_id::echo, "echo" },
delay_line{delay_line_memory.data(), delay_line_memory.size(), sampling_frequency_hz}
{
    this->set_mode(mode);
    this->set_feedback(feedback);
    this->set_time(time);
    this->set_blend(blend);
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
        dsp_sample_t in_d = this->delay_line.get();
        dsp_sample_t in_h = input + this->feedback * in_d;
        this->delay_line.put(in_h);
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
    this->delay_line.set_delay(this->time);
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

