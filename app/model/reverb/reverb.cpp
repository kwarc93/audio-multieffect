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

constexpr float excursion = 0.00026881f;
constexpr float decay_diffusion_1 = 0.7f;
constexpr float decay_diffusion_2 = 0.5f;
constexpr float input_diffusion_1 = 0.23f;
constexpr float input_diffusion_2 = 0.667f;
}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


reverb::reverb(float bandwidth, float damping, float decay) : effect { effect_id::reverb },
pdel {predelay_line_memory.data(), predelay_line_memory.size(), config::sampling_frequency_hz},
del1 {0.14169551f, config::sampling_frequency_hz},
del2 {0.10628003f, config::sampling_frequency_hz},
del3 {0.14962535f, config::sampling_frequency_hz},
del4 {0.12499580f, config::sampling_frequency_hz},
lpf1 {},
lpf2 {},
lpf3 {},
apf1 {0.00477135f, input_diffusion_1},
apf2 {0.00359531f, input_diffusion_1},
apf3 {0.01273479f, input_diffusion_2},
apf4 {0.00930748f, input_diffusion_2},
apf5 {0.08924431f, decay_diffusion_2},
apf6 {0.06048184f, decay_diffusion_2},
mapf1 {0.03050973f + excursion, excursion, 0.55f, decay_diffusion_1},
mapf2 {0.02257989f + excursion * 0.63f, excursion * 0.63f, 0.333f, decay_diffusion_1},
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

        /* right loop */
        float mod1 = this->mapf1.process(sample + this->del4.get() * this->attr.ctrl.decay);
        float sample1 = this->del1.get();
        this->del1.put(mod1);
        this->lpf2.process(&sample1, &sample1, 1);
        sample1 = this->apf5.process(sample1);

        /* left loop */
        float mod2 = this->mapf2.process(sample + this->del2.get() * this->attr.ctrl.decay);
        float sample2 = this->del3.get();
        this->del3.put(mod2);
        this->lpf3.process(&sample2, &sample2, 1);
        sample2 = this->apf6.process(sample2);

        this->del2.put(sample1);
        this->del4.put(sample2);

        /* Left output */
        float left_sample = this->del1.at(0.00893787f) +
                            this->del1.at(0.09992944f) -
                            this->apf5.at(0.06427875f) +
                            this->del2.at(0.06706764f) -
                            this->del3.at(0.06686603f) -
                            this->apf6.at(0.00628339f) -
                            this->del4.at(0.03581869f);

        /* Right output */
        float right_sample = this->del1.at(0.01186116f) +
                             this->del1.at(0.12187090f) -
                             this->apf5.at(0.04126205f) +
                             this->del2.at(0.08981553f) -
                             this->del3.at(0.07093176f) -
                             this->apf6.at(0.01125634f) -
                             this->del4.at(0.00406572f);

        /* L/R mix & wet/dry mix */
        const float lr_mix = 0.5f * (left_sample + right_sample);
        return 0.3f * lr_mix + 0.5f * input;
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



