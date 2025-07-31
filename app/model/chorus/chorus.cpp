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

constexpr float delay_line1_tap = 0.01f;
constexpr uint32_t delay_line1_tap_samples = delay_line1_tap * config::sampling_frequency_hz;
__attribute__((section(".sdram")))
std::array<float, static_cast<unsigned>(2 * delay_line1_tap_samples)> delay_line1_memory;

constexpr float delay_line2_tap = 0.025f;
constexpr uint32_t delay_line2_tap_samples = delay_line2_tap * config::sampling_frequency_hz;
__attribute__((section(".sdram")))
std::array<float, static_cast<unsigned>(2 * delay_line2_tap_samples)> delay_line2_memory;

}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


chorus::chorus() : effect { effect_id::chorus },
lfo1 { libs::adsp::oscillator::shape::sine, 0.2f, config::sampling_frequency_hz },
lfo2 { libs::adsp::oscillator::shape::cosine, 0.2f, config::sampling_frequency_hz },
unicomb1 { 0.7f, -0.7f, 1, delay_line1_memory.data(), delay_line1_memory.size(), config::sampling_frequency_hz},
unicomb2 { 0, 0, 1, delay_line2_memory.data(), delay_line2_memory.size(), config::sampling_frequency_hz},
attr {}
{
    const auto& def = chorus_attr::default_ctrl;

    this->set_depth(def.depth);
    this->set_rate(def.rate);
    this->set_tone(def.tone);
    this->set_mix(def.mix);
    this->set_mode(def.mode);
}

chorus::~chorus()
{

}

void chorus::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        const float depth = 0.0001f + this->attr.ctrl.depth * 0.0015f;

        if (this->attr.ctrl.mode == chorus_attr::controls::mode_type::white)
        {
            this->unicomb1.set_delay(delay_line1_tap + this->lfo1.generate() * depth);
            const float output = this->unicomb1.process<false, true, delay_line1_tap_samples>(input);
            return this->attr.ctrl.mix * output + (1 - this->attr.ctrl.mix) * input;
        }
        else
        {
            this->unicomb1.set_delay(delay_line1_tap + this->lfo1.generate() * depth);
            this->unicomb2.set_delay(delay_line2_tap + this->lfo2.generate() * depth);
            const float out1 = this->unicomb1.process<false, true, 0>(input);
            const float out2 = this->unicomb2.process<false, true, 0>(input);
            const float output = 0.7f * (out1 + out2);
            return this->attr.ctrl.mix * output + (1 - this->attr.ctrl.mix) * input;
        }
    }
    );
}

const effect_specific_attr chorus::get_specific_attributes(void) const
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

    this->lfo1.set_frequency(rate);
    this->lfo2.set_frequency(0.333f * rate);
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

    if (mode == chorus_attr::controls::mode_type::white)
    {
        this->unicomb1.set_blend(0.7f);
        this->unicomb1.set_feedback(-0.7f);
    }
    else if (mode == chorus_attr::controls::mode_type::deep)
    {
        this->unicomb1.set_blend(0);
        this->unicomb1.set_feedback(0);
    }
}



