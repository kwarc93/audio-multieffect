/*
 * chorus.cpp
 *
 *  Created on: 30 wrz 2023
 *      Author: kwarc
 */

#include "chorus.hpp"

#include <algorithm>
#include <random>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr float delay_center_tap = 0.05;
__attribute__((section(".sdram")))
std::array<float, static_cast<unsigned>(2 * delay_center_tap * config::sampling_frequency_hz)> delay_line_memory; // Maximum delay depth: 60ms

float generate_random(void)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(-1.0f, 1.0f);
    return dis(gen);
}


}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


chorus::chorus(float depth, float rate, float tone, float mix) : effect { effect_id::chorus },
lfo { libs::adsp::oscillator::shape::sine, config::sampling_frequency_hz },
delay_line{delay_line_memory.data(), delay_line_memory.size(), config::sampling_frequency_hz}, attr {}
{
    this->set_depth(depth);
    this->set_rate(rate);
    this->set_tone(tone);
    this->set_mix(mix);

    this->lfo.set_shape(libs::adsp::oscillator::shape::triangle);
    this->bypass(false); // Remove
}

chorus::~chorus()
{

}

void chorus::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Modulate delay line with LFO */
        this->delay_line.set_delay(delay_center_tap + this->lfo.generate() * this->attr.ctrl.depth);

        /* Apply Dattorro's approximation of allpass filter with a fixed center tap */
        constexpr float c = 1 / std::sqrt(2);
        const float in_h = input - c * this->delay_line.at(delay_center_tap);
        this->delay_line.put(in_h);
        return this->delay_line.get() + c * in_h;
    }
    );
}

const effect_specific_attributes chorus::get_specific_attributes(void) const
{
    return this->attr;
}

void chorus::set_depth(float depth)
{
//    depth = std::clamp(depth, 0.001f, 0.03f);

    if (this->attr.ctrl.depth == depth)
        return;

    this->attr.ctrl.depth = depth;
}

void chorus::set_rate(float rate)
{
//    rate = std::clamp(rate, 0.05f, 0.3f);

    if (this->attr.ctrl.rate == rate)
        return;

    this->attr.ctrl.rate = rate;

    this->lfo.set_frequency(rate);
}

void chorus::set_tone(float tone)
{
//    tone = std::clamp(tone, 0.0f, 1.0f);

    if (this->attr.ctrl.tone == tone)
        return;

    this->attr.ctrl.tone = tone;
}

void chorus::set_mix(float mix)
{
//    mix = std::clamp(mix, 0.0f, 1.0f);

    if (this->attr.ctrl.mix == mix)
        return;

    this->attr.ctrl.mix = mix;
}



