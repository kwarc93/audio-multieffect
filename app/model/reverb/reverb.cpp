/*
 * reverb.cpp
 *
 *  Created on: 8 lis 2023
 *      Author: kwarc
 */

#include "reverb.hpp"

#include <algorithm>
#include <array>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
__attribute__((section(".sdram")))
std::array<float, 1 * config::sampling_frequency_hz + 1> predelay_line_memory; // Maximum delay time: 1s
}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


reverb::reverb(float bandwidth, float damping, float decay) : effect { effect_id::reverb },
pdel {predelay_line_memory.data(), predelay_line_memory.size(), config::sampling_frequency_hz},
del1 {0.149625348610598f, config::sampling_frequency_hz},
del2 {0.124995799872316f, config::sampling_frequency_hz},
del3 {0.141695507543429f, config::sampling_frequency_hz},
del4 {0.105372803333221f, config::sampling_frequency_hz},
lpf1 {},
lpf2 {},
lpf3 {},
apf1 {0.00477134504888949f, 0.75f},
apf2 {0.00359530929740264f, 0.75f},
apf3 {0.012734787137529f, 0.625f},
apf4 {0.0093074829474816f, 0.625f},
apf5 {0.0604818386478949f, 0.5f},
apf6 {0.089244313027116f, 0.5f},
mapf1 {0.0225798864285474f + 0.000268808171768422f, 0.000268808171768422f, 0.7f},
mapf2 {0.0305097274957159f + 0.000268808171768422f, 0.000268808171768422f, 0.7f},
attr {}
{
    this->set_bandwidth(bandwidth);
    this->set_damping(damping);
    this->set_decay(decay);
    this->set_mode(reverb_attr::controls::mode_type::plate);

    this->pdel.set_delay(0.001f);
}

reverb::~reverb()
{

}

void reverb::process(const dsp_input& in, dsp_output& out)
{
    /* Dattorro's reverb */
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Input diffusers */
        float sample = input;/*this->pdel.get();
        this->pdel.put(input);*/
        this->lpf1.process(&sample, &sample, 1);
        sample = this->apf4.process(this->apf3.process(this->apf2.process(this->apf1.process(sample))));

        /* 8-figure "tank" */
        float mod1 = this->mapf1.process(sample + this->del4.get() * this->attr.ctrl.decay);
        float sample1 = this->del1.get();
        this->del1.put(mod1);
        this->lpf2.process(&sample1, &sample1, 1);
        sample1 = this->apf5.process(sample1);

        float mod2 = this->mapf2.process(sample + this->del2.get() * this->attr.ctrl.decay);
        float sample2 = this->del3.get();
        this->del3.put(mod2);
        this->lpf3.process(&sample2, &sample2, 1);
        sample2 = this->apf6.process(sample2);

        this->del2.put(sample1);
        this->del4.put(sample2);

        /* Left output */
        float left_sample = this->del1.at(0.00893787171130002f) +
                            this->del1.at(0.0999294378549108f) -
                            this->apf5.at(0.0642787540741239f) +
                            this->del2.at(0.0670676388562212f) -
                            this->del3.at(0.0668660327273949f) -
                            this->apf6.at(0.00628339101508686f) -
                            this->del4.at(0.0358186888881422f);

        /* Right output */
        float right_sample = this->del1.at(0.0118611605792816f) +
                             this->del1.at(0.121870904875508f) -
                             this->apf5.at(0.0412620543664527f) +
                             this->del2.at(0.0898155303921239f) -
                             this->del3.at(0.0709317563253923f) -
                             this->apf6.at(0.0112563421928027f) -
                             this->del4.at(0.00406572359799738f);

        return 0.5f * left_sample + 0.5f * right_sample;
    }
    );
}

const effect_specific_attributes reverb::get_specific_attributes(void) const
{
    return this->attr;
}

void reverb::set_bandwidth(float bandwidth)
{
    bandwidth = std::clamp(bandwidth, 0.0f, 0.999f);

    if (this->attr.ctrl.bandwidth == bandwidth)
        return;

    this->attr.ctrl.bandwidth = bandwidth;

    /* Calculate coefficient for 1-st order low-pass IIR (0kHz - 24kHz range) */
    const float fc = bandwidth * config::sampling_frequency_hz * 0.5f;
    this->lpf1.calc_coeffs(fc, config::sampling_frequency_hz, true);
}

void reverb::set_damping(float damping)
{
    damping = std::clamp(damping, 0.0f, 1.0f);

    if (this->attr.ctrl.damping == damping)
        return;

    this->attr.ctrl.damping = damping;

    /* Calculate coefficient for 1-st order low-pass IIR (0kHz - 24kHz range) */
    const float fc = (1 - damping) * config::sampling_frequency_hz * 0.5f;
    this->lpf2.calc_coeffs(fc, config::sampling_frequency_hz, true);
    this->lpf3.calc_coeffs(fc, config::sampling_frequency_hz, true);
}

void reverb::set_decay(float decay)
{
    decay = std::clamp(decay, 0.0f, 0.999f);

    if (this->attr.ctrl.decay == decay)
        return;

    this->attr.ctrl.decay = decay;
}

void reverb::set_mode(reverb_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    this->attr.ctrl.mode = mode;
}



