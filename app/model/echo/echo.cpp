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


echo::echo(float decay, float time, float feedback, mode_type mode) : effect { effect_id::echo, "echo" },
delay_line{delay_line_memory.data(), delay_line_memory.size(), sampling_frequency_hz}
{
    this->set_feedback(feedback);
    this->set_mode(mode);
    this->set_decay(decay);
    this->set_time(time);
}

echo::~echo()
{

}

void echo::process(const dsp_input_t& in, dsp_output_t& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Universal comb filter with LP in feedback */
        const dsp_sample_t in_d = this->delay_line.get();
        dsp_sample_t in_df = in_d;
        if (this->decay > 0) this->iir_lp.process(&in_d, &in_df, 1);
        const dsp_sample_t in_h = input + this->feedback * in_df;
        this->delay_line.put(in_h);
        return this->feedforward * in_df + this->blend * in_h;
    }
    );
}

void echo::set_decay(float decay)
{
    decay = std::clamp(decay, 0.0f, 1.0f);

    if (this->decay == decay || mode == mode_type::delay)
        return;

    this->decay = decay;

    /* Calculate coefficient for 1-st order low-pass IIR (2kHz - 8kHz range) */
    const float fc = 2000 + (1 - decay) * 6000;

    this->iir_lp.calc_coeffs(fc, sampling_frequency_hz, true);

}

void echo::set_time(float time)
{
    time = std::clamp(time, 0.1f, 1.0f);

    if (this->time == time)
        return;

    this->time = time;

    this->delay_line.set_delay(this->time);
}

void echo::set_feedback(float feedback)
{
    feedback = std::clamp(feedback, 0.0f, 1.0f);

    if (this->feedback == feedback)
        return;

    this->feedback = feedback;

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
        this->blend = std::sqrt(1 - this->feedback * this->feedback); // L2 normalization
        this->feedforward = 0;
    }

    this->mode = mode;
}

