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
std::array<float, 1 * config::sampling_frequency_hz + 1> delay_line_memory; // Maximum delay time: 1s
}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


echo::echo(float blur, float time, float feedback, echo_attr::controls::mode_type mode) : effect { effect_id::echo },
delay_line{delay_line_memory.data(), delay_line_memory.size(), config::sampling_frequency_hz}, attr {}
{
    this->set_mode(mode);
    this->set_blur(blur);
    this->set_time(time);
    this->set_feedback(feedback);
}

echo::~echo()
{

}

void echo::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Universal comb filter with LP in feedback */
        const float in_d = this->delay_line.get();
        float in_df = in_d;
        if (this->attr.ctrl.blur > 0) this->iir_lp.process(&in_d, &in_df, 1);
        const float in_h = input + this->feedback * in_df;
        this->delay_line.put(in_h);
        return this->feedforward * in_df + this->blend * in_h;
    }
    );
}

const effect_specific_attributes echo::get_specific_attributes(void) const
{
    return this->attr;
}

void echo::set_blur(float blur)
{
    blur = std::clamp(blur, 0.0f, 1.0f);

    if (this->attr.ctrl.blur == blur || this->attr.ctrl.mode == echo_attr::controls::mode_type::delay)
        return;

    this->attr.ctrl.blur = blur;

    /* Calculate coefficient for 1-st order low-pass IIR (2kHz - 8kHz range) */
    const float fc = 2000 + (1 - blur) * 6000;

    this->iir_lp.calc_coeffs(fc, config::sampling_frequency_hz, true);

}

void echo::set_time(float time)
{
    time = std::clamp(time, 0.05f, 1.0f);

    if (this->attr.ctrl.time == time)
        return;

    this->attr.ctrl.time = time;

    this->delay_line.set_delay(this->attr.ctrl.time);
}

void echo::set_feedback(float feedback)
{
    feedback = std::clamp(feedback, 0.0f, 0.9f);

    if (this->attr.ctrl.feedback == feedback)
        return;

    this->attr.ctrl.feedback = feedback;

    if (this->attr.ctrl.mode == echo_attr::controls::mode_type::delay)
    {
        this->feedforward = this->attr.ctrl.feedback;
    }
    else if (this->attr.ctrl.mode == echo_attr::controls::mode_type::echo)
    {
        this->feedback = this->attr.ctrl.feedback;
        this->blend = std::sqrt(1 - this->attr.ctrl.feedback * this->attr.ctrl.feedback); // L2 normalization
    }
}

void echo::set_mode(echo_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    if (mode == echo_attr::controls::mode_type::delay)
    {
        this->feedback = 0;
        this->feedforward = this->attr.ctrl.feedback;
        this->blend = 1;
    }
    else if (mode == echo_attr::controls::mode_type::echo)
    {
        this->feedback = this->attr.ctrl.feedback;
        this->feedforward = 0;
        this->blend = std::sqrt(1 - this->attr.ctrl.feedback * this->attr.ctrl.feedback); // L2 normalization
    }

    this->attr.ctrl.mode = mode;
}

