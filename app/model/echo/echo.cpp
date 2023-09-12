/*
 * echo.cpp
 *
 *  Created on: 29 sie 2023
 *      Author: kwarc
 */

#include "echo.hpp"

#include <array>
#include <algorithm>

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


echo::echo(float blur, float time, float feedback, controls::mode_type mode) : effect { effect_id::echo, "echo" },
delay_line{delay_line_memory.data(), delay_line_memory.size(), sampling_frequency_hz}
{
    this->set_mode(mode);
    this->set_blur(blur);
    this->set_time(time);
    this->set_feedback(feedback);
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
        if (this->ctrl.blur > 0) this->iir_lp.process(&in_d, &in_df, 1);
        const dsp_sample_t in_h = input + this->ctrl.feedback * in_df;
        this->delay_line.put(in_h);
        return this->feedforward * in_df + this->blend * in_h;
    }
    );
}

void echo::set_blur(float blur)
{
    blur = std::clamp(blur, 0.0f, 1.0f);

    if (this->ctrl.blur == blur || this->ctrl.mode == controls::mode_type::delay)
        return;

    this->ctrl.blur = blur;

    /* Calculate coefficient for 1-st order low-pass IIR (2kHz - 8kHz range) */
    const float fc = 2000 + (1 - blur) * 6000;

    this->iir_lp.calc_coeffs(fc, sampling_frequency_hz, true);

}

void echo::set_time(float time)
{
    time = std::clamp(time, 0.05f, 1.0f);

    if (this->ctrl.time == time)
        return;

    this->ctrl.time = time;

    this->delay_line.set_delay(this->ctrl.time);
}

void echo::set_feedback(float feedback)
{
    feedback = std::clamp(feedback, 0.0f, 0.9f);

    if (this->ctrl.feedback == feedback)
        return;

    this->ctrl.feedback = feedback;

    if (this->ctrl.mode == controls::mode_type::delay)
    {
        this->feedforward = this->ctrl.feedback;
        this->ctrl.feedback = 0;
    }
    else if (this->ctrl.mode == controls::mode_type::echo)
    {
        this->blend = std::sqrt(1 - this->ctrl.feedback * this->ctrl.feedback); // L2 normalization
    }
}

void echo::set_mode(controls::mode_type mode)
{
    if (this->ctrl.mode == mode)
        return;

    if (mode == controls::mode_type::delay)
    {
        this->blend = 1;
        this->ctrl.feedback = 0;
    }
    else if (mode == controls::mode_type::echo)
    {
        this->blend = std::sqrt(1 - this->ctrl.feedback * this->ctrl.feedback); // L2 normalization
        this->feedforward = 0;
    }

    this->ctrl.mode = mode;
}

