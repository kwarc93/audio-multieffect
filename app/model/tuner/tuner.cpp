/*
 * tuner.cpp
 *
 *  Created on: 22 sie 2025
 *      Author: kwarc
 */

#include "tuner.hpp"

#include <limits>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr float min_freq = 55.0f;   // A1
constexpr float max_freq = 1760.0f; // A6

}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

tuner::tuner() : effect { effect_id::tuner },
decimator {},
hpf {},
envelope_follower { libs::adsp::envelope_follower::mode::root_mean_square, 0.005f, 0.2f, config::sampling_frequency_hz / decim_factor },
pitch_median {},
pitch_avg { 0.2f, 1.0f / (config::sampling_frequency_hz / config::dsp_vector_size), 0.0f },
pitch_det { min_freq, max_freq, config::sampling_frequency_hz / decim_factor },
mute { false },
envelope { 0.0f },
detected_pitch { 0.0f },
frame_counter { 0 },
attr {}
{
    const auto& def = tuner_attr::default_ctrl;

    this->set_a4_tuning(def.a4_tuning);

    /* Calculate HPF coeff */
    const float fc = 50.0f;
    this->hpf.calc_coeff(fc, config::sampling_frequency_hz / decim_factor);
}

tuner::~tuner()
{

}

void tuner::process(const dsp_input& in, dsp_output& out)
{
    /* 1. Pass through the signal to output */
    if (!this->mute)
        out = in;

    /* 2. Decimate signal for further processing */
    this->decimator.process(in.data(), this->decim_input.data());

    /* 3. Apply high-pass filter & detect envelope */
    std::transform(this->decim_input.begin(), this->decim_input.end(), this->decim_input.begin(),
    [this](auto in)
    {
        float out = this->hpf.process(in);
        this->envelope = this->envelope_follower.process(out);
        return out;
    }
    );

    /* 3. Detect pitch only for signals above threshold */
    constexpr float threshold = libs::adsp::db2lin(-45.0f);
    if (this->envelope > threshold && this->pitch_det.process(this->decim_input.data()))
    {
        /* TODO: Use EMA in the log2 domain (on semitones, not on frequency) */
        this->detected_pitch = pitch_avg.process(this->pitch_median.process(this->pitch_det.get_pitch()));
    }
    else
    {
        this->detected_pitch = pitch_avg.process(this->detected_pitch);
    }

    /* Update output every 10 frames */
    if (++this->frame_counter >= 10)
    {
        this->frame_counter = 0;

        if (std::abs(this->detected_pitch - this->attr.out.pitch) > 0.05f)
        {
            this->attr.out.pitch = this->detected_pitch;

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

            /* Fill result */
            this->attr.out.note = notes[std::clamp(note_idx, 0, 11)];
            this->attr.out.octave = std::clamp(octave, 0, 8);
            this->attr.out.cents = std::clamp((int)std::round(cents_err), -50, 50);

            /* Notify about the change */
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

