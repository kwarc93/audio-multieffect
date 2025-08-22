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
            this->attr.out.pitch = this->pitch_det.get_frequency();

            /* Calculate note and cents deviation */
            constexpr char notes[12] =
            {
                'c', 'C', 'd', 'D', 'e', 'f',
                'F', 'g', 'G', 'a', 'A', 'b'
            };

            // Step 1: compute octave relative to C0 (16.35 Hz)
            float c0 = 16.35; // frequency of C0
            float n = 12.0 * std::log2(this->attr.out.pitch / c0);
            int nearest = static_cast<int>(std::round(n));

            int noteIndex = nearest % 12;
            if (noteIndex < 0) noteIndex += 12;

            int octave = nearest / 12; // C0 = octave 0
            if (nearest < 0 && nearest % 12 != 0) octave -= 1;

            // Step 2: calculate the frequency of the nearest note
            float nearestFreq = c0 * std::pow(2.0, nearest / 12.0);

            // Step 3: calculate cents error
            float centsError = 1200.0 * std::log2(this->attr.out.pitch / nearestFreq);

            // Fill result
            this->attr.out.note = notes[noteIndex];
            this->attr.out.octave = std::clamp(octave, 0, 8);
            this->attr.out.cents = (int8_t)std::clamp((int)std::round(centsError), -50, 50);
        }

        /* Do not alter the signal, just pass it through */
        return input;
    }
    );

    // Update output every 10 frames (~26ms)
    if (++frame_counter >= 10)
    {
        frame_counter = 0;

        if (this->callback)
            this->callback(this);
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

