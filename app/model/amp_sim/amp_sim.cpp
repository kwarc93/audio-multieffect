/*
 * amp_sim.cpp
 *
 *  Created on: 5 sty 2025
 *      Author: kwarc
 */


#include "amp_sim.hpp"

#include <cmath>
#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


amp_sim::amp_sim(float input, float drive, float compression, amp_sim_attr::controls::mode_type mode) : effect { effect_id::amplifier_sim },
attr {}
{
    this->amp.reset(config::sampling_frequency_hz);

    this->set_mode(mode);
    this->set_input(input);
    this->set_drive(drive);
    this->set_compression(compression);
}

amp_sim::~amp_sim()
{

}

void amp_sim::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        return this->amp.processAudioSample(input);
    }
    );
}

const effect_specific_attributes amp_sim::get_specific_attributes(void) const
{
    return this->attr;
}

void amp_sim::set_input(float input)
{
    input = std::clamp(input, 0.0f, 1.0f);

    if (this->attr.ctrl.input == input)
        return;

    this->attr.ctrl.input = input;

    this->amp_params.volume1_010 = input * 10;
    this->amp.setParameters(this->amp_params);
}

void amp_sim::set_drive(float drive)
{
    drive = std::clamp(drive, 0.0f, 1.0f);

    if (this->attr.ctrl.drive == drive)
        return;

    this->attr.ctrl.drive = drive;

    this->amp_params.volume2_010 = drive * 10;
    this->amp.setParameters(this->amp_params);
}

void amp_sim::set_compression(float compression)
{
    compression = std::clamp(compression, 0.0f, 1.0f);

    if (this->attr.ctrl.compression == compression)
        return;

    this->attr.ctrl.compression = compression;

    this->amp_params.tubeCompression_010 = compression * 10;
    this->amp.setParameters(this->amp_params);
}

void amp_sim::set_mode(amp_sim_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    this->attr.ctrl.mode = mode;

    if (mode == amp_sim_attr::controls::mode_type::higain)
    {
        this->amp_params.toneStackParameters.LFToneControl_010 = 5;
        this->amp_params.toneStackParameters.MFToneControl_010 = 5;
        this->amp_params.toneStackParameters.HFToneControl_010 = 5;

        this->amp_params.masterVolume_010 = 8;
        this->amp_params.ampGainStyle = ampGainStructure::medium;
    }
    else
    {
        this->amp_params.toneStackParameters.LFToneControl_010 = 5;
        this->amp_params.toneStackParameters.MFToneControl_010 = 5;
        this->amp_params.toneStackParameters.HFToneControl_010 = 5;

        this->amp_params.masterVolume_010 = 8;
        this->amp_params.ampGainStyle = ampGainStructure::low;
    }

    this->amp.setParameters(this->amp_params);
}

