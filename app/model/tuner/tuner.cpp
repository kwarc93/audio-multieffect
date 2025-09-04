/*
 * tuner.cpp
 *
 *  Created on: 22 sie 2025
 *      Author: kwarc
 */

#include "tuner.hpp"

#include <limits>

using namespace mfx;

/*
 * TODO:
 *
 * 1. Consider proper signal decimation (with anti-alias FIR LPF)
 * 2. Use high-pass filter before pitch detector (1st order IIR, 50Hz)
 * 3. Add envelope follower to trigger the pitch detector above certain level
 * 4. Limit the frequencies (min - max) in the pitch detector
 * 5. Use EMA in the log2 domain (on semitones, not on frequency)
 *
 */

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr float min_freq = 55.0f;   // A1
constexpr float max_freq = 1760.0f; // A6

constexpr float lin2db(float x)
{
    return x > 0 ? 20.0f * std::log10(x) : -144.5f;
}

std::array<float, config::dsp_vector_size / tuner::decim_factor> decim_buf;

}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

tuner::tuner() : effect { effect_id::tuner },
envelope { libs::adsp::envelope_follower::mode::root_mean_square, 0.005f, 0.2f, config::sampling_frequency_hz },
pitch_median {},
pitch_avg { 0.2f, 1.0f / (config::sampling_frequency_hz / config::dsp_vector_size), 0.0f },
pitch_det { min_freq, max_freq, config::sampling_frequency_hz / decim_factor },
detected_pitch { 0.0f },
frame_counter { 0 },
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
    for (unsigned i = 0; i < in.size() / decim_factor; i++)
    {
        /* Do not alter the signal, just pass it through */
        const auto idx = i * decim_factor;
        out[idx + 0] = in[idx + 0];
        out[idx + 1] = in[idx + 1];
        out[idx + 2] = in[idx + 2];
        out[idx + 3] = in[idx + 3];

        /* Decimate signal to temporary buffer */
        decim_buf[i] = in[idx];
    }

    this->detected_pitch = pitch_avg.process(this->pitch_det.process(decim_buf.data()) ? this->pitch_median.process(this->pitch_det.get_pitch()) : this->detected_pitch);

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

