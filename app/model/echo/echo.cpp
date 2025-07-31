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


echo::echo() : effect { effect_id::echo },
unicomb { 0, 0, 0, delay_line_memory.data(), delay_line_memory.size(), config::sampling_frequency_hz },
attr {}
{
    const auto& def = echo_attr::default_ctrl;

    this->set_mode(def.mode);
    this->set_blur(def.blur);
    this->set_time(def.time);
    this->set_feedback(def.feedback);
}

echo::~echo()
{

}

void echo::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        return this->unicomb.process<true>(input);
    }
    );
}

const effect_specific_attr echo::get_specific_attributes(void) const
{
    return this->attr;
}

void echo::set_blur(float blur)
{
    blur = std::clamp(blur, 0.0f, 1.0f);

    if (this->attr.ctrl.blur == blur)
        return;

    this->attr.ctrl.blur = blur;

    const float fc = 2000 + (1 - blur) * 6000;
    this->unicomb.set_lowpass(fc);
}

void echo::set_time(float time)
{
    time = std::clamp(time, 0.05f, 1.0f);

    if (this->attr.ctrl.time == time)
        return;

    this->attr.ctrl.time = time;

    this->unicomb.set_delay(this->attr.ctrl.time);
}

void echo::set_feedback(float feedback)
{
    feedback = std::clamp(feedback, 0.0f, 0.9f);

    if (this->attr.ctrl.feedback == feedback)
        return;

    this->attr.ctrl.feedback = feedback;

    if (this->attr.ctrl.mode == echo_attr::controls::mode_type::delay)
    {
        this->unicomb.set_feedforward(this->attr.ctrl.feedback);
    }
    else if (this->attr.ctrl.mode == echo_attr::controls::mode_type::echo)
    {
        this->unicomb.set_feedback(this->attr.ctrl.feedback);
        this->unicomb.normalize();
    }
}

void echo::set_mode(echo_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    this->attr.ctrl.mode = mode;

    if (mode == echo_attr::controls::mode_type::delay)
    {
        this->unicomb.set_feedback(0);
        this->unicomb.set_feedforward(this->attr.ctrl.feedback);
        this->unicomb.set_blend(1);
    }
    else if (mode == echo_attr::controls::mode_type::echo)
    {
        this->unicomb.set_feedback(this->attr.ctrl.feedback);
        this->unicomb.set_feedforward(0);
        this->unicomb.normalize();
    }
}

