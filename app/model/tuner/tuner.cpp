/*
 * tuner.cpp
 *
 *  Created on: 22 sie 2025
 *      Author: kwarc
 */

#include "tuner.hpp"

#include <algorithm>

#include <q/support/literals.hpp>
#include <q/synth/sin_osc.hpp>

using namespace mfx;
namespace q = cycfi::q;
using namespace q::literals;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
unsigned frame_counter = 0;
float prev_pitch = 0.0f;

/* Exponential moving average */

class averaging_filter
{
public:
    averaging_filter(float time_constant, float time_delta, float inital_value = 0)
    {
        this->time_constant = time_constant;
        this->alpha = 1 - expf(-time_delta / time_constant);
        this->output = inital_value;
    }
    virtual ~averaging_filter() = default;
    float process(float input)
    {
        return output = (1 - this->alpha) * output + this->alpha * input;
    }
    float time_constant;
    float alpha;
    float output;
};

averaging_filter pitch_avg {0.0001f, 1.0f / config::sampling_frequency_hz, 440.0f};
}

//-----------------------------------------------------------------------------
/* private */


//-----------------------------------------------------------------------------
/* public */

tuner::tuner() : effect { effect_id::tuner },
pitch_det { 60.0_Hz, 2100.0_Hz, config::sampling_frequency_hz, -45.0_dB },
attr {}
{
    const auto& def = tuner_attr::default_ctrl;

    this->set_a4_tuning(def.a4_tuning);
}

tuner::~tuner()
{

}

void tuner::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        if (this->pitch_det(input))
        {
            this->attr.out.pitch = pitch_avg.process(this->pitch_det.get_frequency());

            /* Calculate note, octave and cents deviation */
            constexpr char notes[12] =
            {
                'a', 'A', 'b', 'b', 'C', 'd', 'D', 'e', 'f', 'F', 'g', 'G'
            };

            int note_number = std::round(12 * std::log2(this->attr.out.pitch / this->attr.ctrl.a4_tuning) + 49);
            int note_idx = (note_number - 1) % 12;
            int octave = (note_number + 8) / 12;
            float nearest_freq = this->attr.ctrl.a4_tuning * std::pow(2.0f, (note_number - 49) / 12.0f);
            float cents_err = 1200.0f * std::log2(this->attr.out.pitch / nearest_freq);

            // Fill result
            this->attr.out.note = notes[note_idx];
            this->attr.out.octave = std::clamp(octave, 0, 8);
            this->attr.out.cents = std::clamp((int)std::round(cents_err), -50, 50);
        }

        /* Do not alter the signal, just pass it through */
        return input;
    }
    );

    // Update output every 10 frames (~26ms)
    if (++frame_counter >= 10)
    {
        frame_counter = 0;

        if (std::abs(prev_pitch - this->attr.out.pitch) > 0.1f)
        {
            prev_pitch = this->attr.out.pitch;
            if (this->callback) this->callback(this);
        }
    }
}

const effect_specific_attr tuner::get_specific_attributes(void) const
{
    return this->attr;
}

void tuner::set_a4_tuning(unsigned frequency)
{
    frequency = std::clamp(frequency, 410u, 480u);

    if (this->attr.ctrl.a4_tuning == frequency)
        return;

    this->attr.ctrl.a4_tuning = frequency;
}

