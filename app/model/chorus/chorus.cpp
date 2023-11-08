/*
 * chorus.cpp
 *
 *  Created on: 30 wrz 2023
 *      Author: kwarc
 */

#include "chorus.hpp"

#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr float delay_line_center_tap = 0.01;
__attribute__((section(".sdram")))
std::array<float, static_cast<unsigned>(2 * delay_line_center_tap * config::sampling_frequency_hz)> delay_line_memory;

constexpr float delay_line2_center_tap = 0.025;
__attribute__((section(".sdram")))
std::array<float, static_cast<unsigned>(2 * delay_line2_center_tap * config::sampling_frequency_hz)> delay_line2_memory;

}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


chorus::chorus(float depth, float rate, float tone, float mix) : effect { effect_id::chorus },
lfo { libs::adsp::oscillator::shape::triangle, config::sampling_frequency_hz },
lfo2 { libs::adsp::oscillator::shape::sine, config::sampling_frequency_hz },
delay_line {delay_line_memory.data(), delay_line_memory.size(), config::sampling_frequency_hz},
delay_line2 {delay_line2_memory.data(), delay_line2_memory.size(), config::sampling_frequency_hz},
attr {}
{
    this->set_depth(depth);
    this->set_rate(rate);
    this->set_tone(tone);
    this->set_mix(mix);
    this->set_mode(chorus_attr::controls::mode_type::white);
}

chorus::~chorus()
{

}

void chorus::process(const dsp_input& in, dsp_output& out)
{
    constexpr float c = 1 / std::sqrt(2);

    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        const float depth = 0.0001f + this->attr.ctrl.depth * 0.0015f;

        if (this->attr.ctrl.mode == chorus_attr::controls::mode_type::white)
        {
            this->delay_line.set_delay(delay_line_center_tap + this->lfo.generate() * depth);
            const float delayed = this->delay_line.get();
            const float in_h = input - c * this->delay_line.at(delay_line_center_tap);
            this->delay_line.put(in_h);

            return this->attr.ctrl.mix * (delayed + c * in_h) + (1.0f - this->attr.ctrl.mix) * input;
        }
        else
        {
            this->delay_line.set_delay(delay_line_center_tap + this->lfo.generate() * depth);
            this->delay_line2.set_delay(delay_line2_center_tap - this->lfo2.generate() * depth);
            const float delayed_mix = this->delay_line.get() * c + this->delay_line2.get() * c;
            this->delay_line.put(input);
            this->delay_line2.put(input);

            return this->attr.ctrl.mix * delayed_mix + (1.0f - this->attr.ctrl.mix) * input;
        }
    }
    );
}

const effect_specific_attributes chorus::get_specific_attributes(void) const
{
    return this->attr;
}

void chorus::set_depth(float depth)
{
    depth = std::clamp(depth, 0.0f, 1.0f);

    if (this->attr.ctrl.depth == depth)
        return;

    this->attr.ctrl.depth = depth;
}

void chorus::set_rate(float rate)
{
    rate = std::clamp(rate, 0.0f, 1.0f);

    if (this->attr.ctrl.rate == rate)
        return;

    this->attr.ctrl.rate = rate;

    rate = 0.05f + rate * 3.95f;

    this->lfo.set_frequency(rate);
    this->lfo2.set_frequency(rate * 0.5f);
}

void chorus::set_tone(float tone)
{
    tone = std::clamp(tone, 0.0f, 1.0f);

    if (this->attr.ctrl.tone == tone)
        return;

    this->attr.ctrl.tone = tone;
}

void chorus::set_mix(float mix)
{
    mix = std::clamp(mix, 0.0f, 1.0f);

    if (this->attr.ctrl.mix == mix)
        return;

    this->attr.ctrl.mix = mix;
}

void chorus::set_mode(chorus_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    this->attr.ctrl.mode = mode;
}



