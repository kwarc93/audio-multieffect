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

/* "Etheral" preset */
//constexpr float pr_bandwidth = 0.99f;
//constexpr float pr_damping = 0.3f;
//constexpr float pr_decay = 0.86f;
//constexpr float pr_excursion = 0.00026881f;
//constexpr float pr_decay_diffusion_1 = 0.7f;
//constexpr float pr_decay_diffusion_2 = 0.5f;
//constexpr float pr_input_diffusion_1 = 0.23f;
//constexpr float pr_input_diffusion_2 = 0.667f;
//constexpr float pr_dry = 0.5f;
//constexpr float pr_wet = 0.3f;

/* "Plate" preset */
constexpr float pr_bandwidth = 0.9995f;
constexpr float pr_damping = 0.0005f;
constexpr float pr_decay = 0.5f;
constexpr float pr_excursion = 0.00053762f;
constexpr float pr_decay_diffusion_1 = 0.7f;
constexpr float pr_decay_diffusion_2 = 0.5f;
constexpr float pr_input_diffusion_1 = 0.75f;
constexpr float pr_input_diffusion_2 = 0.625f;
constexpr float pr_dry = 0.5f;
constexpr float pr_wet = 0.5f;

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
apf1 {0.00477135f, pr_input_diffusion_1},
apf2 {0.00359531f, pr_input_diffusion_1},
apf3 {0.01273479f, pr_input_diffusion_2},
apf4 {0.00930748f, pr_input_diffusion_2},
apf5 {0.08924431f, pr_decay_diffusion_2},
apf6 {0.06048184f, pr_decay_diffusion_2},
mapf1 {0.03050973f, pr_excursion, 0.55f, pr_decay_diffusion_1},
mapf2 {0.02257989f, pr_excursion, 0.333f, pr_decay_diffusion_1},
attr {}
{
    this->set_bandwidth(pr_bandwidth);
    this->set_damping(pr_damping);
    this->set_decay(pr_decay);
    this->set_mode(reverb_attr::controls::mode_type::plate);

    this->pdel.set_delay(0);
    this->del1.set_delay(0.14169551f);
    this->del2.set_delay(0.10628003f);
    this->del3.set_delay(0.14962535f);
    this->del4.set_delay(0.12499580f);
}

reverb::~reverb()
{

}

void reverb::process(const dsp_input& in, dsp_output& out)
{
    /* Dattorro's reverb implementation */
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* Input diffusers */
        float sample = this->pdel.get();
        this->pdel.put(input);
        sample = this->lpf1.process(sample);
        sample = this->apf4.process(this->apf3.process(this->apf2.process(this->apf1.process(sample))));

        /* 8-figure "tank" */

        /* right loop */
        float mod1 = this->mapf1.process(sample + this->del4.get() * this->attr.ctrl.decay);
        float sample1 = this->del1.get();
        this->del1.put(mod1);
        sample1 = this->apf5.process(this->lpf2.process(sample1) * this->attr.ctrl.decay);

        /* left loop */
        float mod2 = this->mapf2.process(sample + this->del2.get() * this->attr.ctrl.decay);
        float sample2 = this->del3.get();
        this->del3.put(mod2);
        sample2 = this->apf6.process(this->lpf3.process(sample2) * this->attr.ctrl.decay);

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
        float right_sample = this->del3.at(0.01186116f) +
                             this->del3.at(0.12187090f) -
                             this->apf6.at(0.04126205f) +
                             this->del4.at(0.08981553f) -
                             this->del1.at(0.07093176f) -
                             this->apf5.at(0.01125634f) -
                             this->del2.at(0.00406572f);

        return pr_wet * 0.6f * 0.5f * (left_sample + right_sample) + pr_dry * input;
    }
    );
}

const effect_specific_attributes reverb::get_specific_attributes(void) const
{
    return this->attr;
}

void reverb::set_bandwidth(float bandwidth)
{
    bandwidth = std::clamp(bandwidth, 0.0f, 0.9999f);

    if (this->attr.ctrl.bandwidth == bandwidth)
        return;

    this->attr.ctrl.bandwidth = bandwidth;

    this->lpf1.calc_coeff(bandwidth * config::sampling_frequency_hz * 0.5f, config::sampling_frequency_hz);
}

void reverb::set_damping(float damping)
{
    damping = 1 - std::clamp(damping, 0.0f, 0.9999f);

    if (this->attr.ctrl.damping == damping)
        return;

    this->attr.ctrl.damping = damping;

    const float d = damping * config::sampling_frequency_hz * 0.5f;
    this->lpf2.calc_coeff(d, config::sampling_frequency_hz);
    this->lpf3.calc_coeff(d, config::sampling_frequency_hz);
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



