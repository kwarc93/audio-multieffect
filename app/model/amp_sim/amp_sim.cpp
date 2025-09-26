/*
 * amp_sim.cpp
 *
 *  Created on: 5 sty 2025
 *      Author: kwarc
 */


#include "amp_sim.hpp"

#include <cmath>
#include <algorithm>

#include <hal/hal_system.hpp>

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


amp_sim::amp_sim() : effect { effect_id::amplifier_sim },
attr {}
{
    this->amp.reset(config::sampling_frequency_hz);

    /* For performance reasons, use one triode preamp (instead of four) on slower systems */
    this->amp_params.singleTriodePreamp = (hal::system::system_clock <= 200000000);

    const auto& def = amp_sim_attr::default_ctrl;

    this->set_mode(def.mode);
    this->set_input(def.input);
    this->set_drive(def.drive);
    this->set_compression(def.compression);
    this->set_tone_stack(def.bass, def.mids, def.treb);
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

const effect_specific_attr amp_sim::get_specific_attributes(void) const
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

void amp_sim::set_tone_stack(float bass, float mids, float treb)
{
    bass = std::clamp(bass, 0.0f, 1.0f);
    mids = std::clamp(mids, 0.0f, 1.0f);
    treb = std::clamp(treb, 0.0f, 1.0f);

    if (this->attr.ctrl.bass == bass && this->attr.ctrl.mids == mids && this->attr.ctrl.treb == treb)
        return;

    this->attr.ctrl.bass = bass;
    this->attr.ctrl.mids = mids;
    this->attr.ctrl.treb = treb;

    this->amp_params.toneStackParameters.LFToneControl_010 = bass * 10;
    this->amp_params.toneStackParameters.MFToneControl_010 = mids * 10;
    this->amp_params.toneStackParameters.HFToneControl_010 = treb * 10;

    this->amp.setParameters(this->amp_params);
}

void amp_sim::set_mode(amp_sim_attr::controls::mode_type mode)
{
    if (this->attr.ctrl.mode == mode)
        return;

    this->attr.ctrl.mode = mode;

    this->amp_params.inputHPF_010 = 2;

    if (mode == amp_sim_attr::controls::mode_type::higain)
    {
        this->amp_params.masterVolume_010 = 8;
        this->amp_params.ampGainStyle = ampGainStructure::high;
    }
    else
    {
        this->amp_params.masterVolume_010 = 8;
        this->amp_params.ampGainStyle = ampGainStructure::low;
    }

    this->amp.setParameters(this->amp_params);
}

