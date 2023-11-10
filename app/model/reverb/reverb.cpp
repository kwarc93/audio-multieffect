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

/* Delay lines sizes, delay taps & other constants (according to J. Dattorro's reverb) */
constexpr float pdel_len = 1.0f;

constexpr float del1_len = 0.14169551f;
constexpr float del2_len = 0.10628003f;
constexpr float del3_len = 0.14962535f;
constexpr float del4_len = 0.12499580f;

constexpr float apf1_del_len = 0.00477135f;
constexpr float apf2_del_len = 0.00359531f;
constexpr float apf3_del_len = 0.01273479f;
constexpr float apf4_del_len = 0.00930748f;
constexpr float apf5_del_len = 0.08924431f;
constexpr float apf6_del_len = 0.06048184f;

constexpr float mapf_excursion = 0.00053762f;
constexpr float mapf1_del_len = 0.03050973f;
constexpr float mapf2_del_len = 0.02257989f;

constexpr float decay_diffusion_1 = 0.7f;
constexpr float decay_diffusion_2 = 0.5f;
constexpr float input_diffusion_1 = 0.75f;
constexpr float input_diffusion_2 = 0.625f;

constexpr float lr_out_scale = 0.6f;
constexpr float left_out_del1_tap1 = 0.00893787f;
constexpr float left_out_del1_tap2 = 0.09992944f;
constexpr float left_out_apf5_tap = 0.06427875f;
constexpr float left_out_del2_tap = 0.06706764f;
constexpr float left_out_del3_tap = 0.06686603f;
constexpr float left_out_apf6_tap = 0.00628339f;
constexpr float left_out_del4_tap = 0.03581869f;

constexpr float right_out_del3_tap1 = 0.01186116f;
constexpr float right_out_del3_tap2 = 0.12187090f;
constexpr float right_out_apf6_tap = 0.04126205f;
constexpr float right_out_del4_tap = 0.08981553f;
constexpr float right_out_del1_tap = 0.07093176f;
constexpr float right_out_apf5_tap = 0.01125634f;
constexpr float right_out_del2_tap = 0.00406572f;

/* Large delay lines are placed in SDRAM to save internal RAM */
__attribute__((section(".sdram")))
std::array<float, static_cast<uint32_t>(pdel_len * config::sampling_frequency_hz + 1)> pdel_line_memory;

__attribute__((section(".sdram")))
std::array<float, static_cast<uint32_t>(del1_len * config::sampling_frequency_hz + 1)> del1_line_memory;

__attribute__((section(".sdram")))
std::array<float, static_cast<uint32_t>(del2_len * config::sampling_frequency_hz + 1)> del2_line_memory;

__attribute__((section(".sdram")))
std::array<float, static_cast<uint32_t>(del3_len * config::sampling_frequency_hz + 1)> del3_line_memory;

__attribute__((section(".sdram")))
std::array<float, static_cast<uint32_t>(del4_len * config::sampling_frequency_hz + 1)> del4_line_memory;

}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


reverb::reverb(float bandwidth, float damping, float decay) : effect { effect_id::reverb },
pdel { pdel_line_memory.data(), pdel_line_memory.size(), config::sampling_frequency_hz },
del1 { del1_line_memory.data(), del1_line_memory.size(), config::sampling_frequency_hz },
del2 { del2_line_memory.data(), del2_line_memory.size(), config::sampling_frequency_hz },
del3 { del3_line_memory.data(), del3_line_memory.size(), config::sampling_frequency_hz },
del4 { del4_line_memory.data(), del4_line_memory.size(), config::sampling_frequency_hz },
apf1 { apf1_del_len, input_diffusion_1 },
apf2 { apf2_del_len, input_diffusion_1 },
apf3 { apf3_del_len, input_diffusion_2 },
apf4 { apf4_del_len, input_diffusion_2 },
apf5 { apf5_del_len, decay_diffusion_2 },
apf6 { apf6_del_len, decay_diffusion_2 },
mapf1 { mapf1_del_len, mapf_excursion, 0.55f, decay_diffusion_1 },
mapf2 { mapf2_del_len, mapf_excursion, 0.333f, decay_diffusion_1 },
attr {}
{
    this->pdel.set_delay(0);
    this->del1.set_delay(del1_len);
    this->del2.set_delay(del2_len);
    this->del3.set_delay(del3_len);
    this->del4.set_delay(del4_len);

    this->set_bandwidth(bandwidth);
    this->set_damping(damping);
    this->set_decay(decay);
    this->set_mode(reverb_attr::controls::mode_type::plate);
}

reverb::~reverb()
{

}

void reverb::process(const dsp_input& in, dsp_output& out)
{
    /* J. Dattorro's reverb implementation */
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
        float rl_sample = this->del1.get();
        this->del1.put(this->mapf1.process(sample + this->del4.get() * this->attr.ctrl.decay));
        rl_sample = this->apf5.process(this->lpf2.process(rl_sample) * this->attr.ctrl.decay);

        /* left loop */
        float ll_sample = this->del3.get();
        this->del3.put(this->mapf2.process(sample + this->del2.get() * this->attr.ctrl.decay));
        ll_sample = this->apf6.process(this->lpf3.process(ll_sample) * this->attr.ctrl.decay);

        this->del2.put(rl_sample);
        this->del4.put(ll_sample);

        /* Output taps */
        const float left_out = this->del1.at(left_out_del1_tap1) +
                               this->del1.at(left_out_del1_tap2) -
                               this->apf5.at(left_out_apf5_tap) +
                               this->del2.at(left_out_del2_tap) -
                               this->del3.at(left_out_del3_tap) -
                               this->apf6.at(left_out_apf6_tap) -
                               this->del4.at(left_out_del4_tap);

        const float right_out = this->del3.at(right_out_del3_tap1) +
                                this->del3.at(right_out_del3_tap2) -
                                this->apf6.at(right_out_apf6_tap) +
                                this->del4.at(right_out_del4_tap) -
                                this->del1.at(right_out_del1_tap) -
                                this->apf5.at(right_out_apf5_tap) -
                                this->del2.at(right_out_del2_tap);

        constexpr float wet_dry_mix = 0.5f;
        return wet_dry_mix * lr_out_scale * 0.5f * (left_out + right_out) + (1 - wet_dry_mix) * input;
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
    damping = std::clamp(damping, 0.0f, 0.9999f);

    if (this->attr.ctrl.damping == damping)
        return;

    this->attr.ctrl.damping = damping;

    const float d = (1 - damping) * config::sampling_frequency_hz * 0.5f;
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



