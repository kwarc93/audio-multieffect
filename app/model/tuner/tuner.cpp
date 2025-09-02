/*
 * tuner.cpp
 *
 *  Created on: 22 sie 2025
 *      Author: kwarc
 */

#include "tuner.hpp"

#include <algorithm>

#include <q/support/literals.hpp>
#include <q/fx/envelope.hpp>
#include <q/fx/lowpass.hpp>
#include <q/fx/biquad.hpp>
#include <q/fx/dynamic.hpp>
#include <q/fx/clip.hpp>

using namespace mfx;
using namespace cycfi::q::literals;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr float onset_threshold = lin_float(-28_dB);
constexpr float release_threshold = lin_float(-60_dB);
float threshold = onset_threshold;
constexpr float sps = config::sampling_frequency_hz / tuner::decim_factor;

cycfi::q::peak_envelope_follower  env{ 30_ms, sps };
cycfi::q::one_pole_lowpass        lp{ 1318.510_Hz, sps };
cycfi::q::one_pole_lowpass        lp2{ 82.40689_Hz, sps };

constexpr float            slope = 1.0f/4;
constexpr float            makeup_gain = 4;
cycfi::q::compressor       comp{ -18_dB, slope };
cycfi::q::clip             clip;

std::array<float, config::dsp_vector_size / tuner::decim_factor> decim_buf;

}

//-----------------------------------------------------------------------------
/* private */

void tuner::process1(const dsp_input &in, dsp_output &out)
{
    for (unsigned i = 0; i < in.size(); i++)
    {
        float input = in[i];

        /* Do not alter the signal, just pass it through */
        out[i] = input;

        /* Decimate signal to temporary buffer */
        if (i % tuner::decim_factor == 0)
        {
            decim_buf[i / tuner::decim_factor] = input;
        }
    }

    this->detected_pitch = pitch_avg.process(this->pitch_det1.process(decim_buf.data()) ? this->pitch_median.process(this->pitch_det1.get_pitch()) : this->detected_pitch);
}

void tuner::process2(const dsp_input &in, dsp_output &out)
{
    for (unsigned i = 0; i < in.size(); i++)
    {
        float input = in[i];

        /* Update pitch detector every x samples*/
        if (i % tuner::decim_factor == 0)
        {
            auto s = input;

            // Bandpass filter
            s = lp(s);
            s -= lp2(s);

            // Envelope
            auto e = env(std::abs(s));
            auto e_db = cycfi::q::lin_to_db(e);

            if (e > threshold)
            {
               // Compressor + makeup-gain + hard clip
               auto gain = cycfi::q::lin_float(comp(e_db)) * makeup_gain;
               s = clip(s * gain);
               threshold = release_threshold;
            }
            else
            {
               s = 0.0f;
               threshold = onset_threshold;
            }

            this->detected_pitch = pitch_avg.process(this->pitch_det2(s) ? this->pitch_det2.get_frequency() : this->detected_pitch);
        }

        /* Do not alter the signal, just pass it through */
        out[i] = input;
    }
}

//-----------------------------------------------------------------------------
/* public */

tuner::tuner() : effect { effect_id::tuner },
pitch_median {},
pitch_avg { 0.2f, 1.0f / (config::sampling_frequency_hz / config::dsp_vector_size), 0.0f },
//pitch_avg { tuner::decim_factor * 0.0002f, 1.0f / (config::sampling_frequency_hz / tuner::decim_factor), 0.0f },
pitch_det1 { config::sampling_frequency_hz / tuner::decim_factor },
pitch_det2 { /* E2 */ 82.40689_Hz, /* E6 */ 1318.510_Hz, config::sampling_frequency_hz / tuner::decim_factor, -45.0_dB },
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
    this->process1(in, out);
//    this->process2(in, out);

    /* Update output every 10 frames */
    if (++this->frame_counter >= 10)
    {
        this->frame_counter = 0;

        if (std::abs(this->detected_pitch - this->attr.out.pitch) > 0.1f)
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

