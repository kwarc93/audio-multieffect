/*
 * reverb.cpp
 *
 *  Created on: 8 lis 2023
 *      Author: kwarc
 */

#include "reverb.hpp"

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


reverb::reverb(float bandwidth, float damping, float decay) : effect { effect_id::reverb },
attr {}
{
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
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* TODO */
        return input;
    }
    );
}

const effect_specific_attributes reverb::get_specific_attributes(void) const
{
    return this->attr;
}

void reverb::set_bandwidth(float bandwidth)
{
    bandwidth = std::clamp(bandwidth, 0.0f, 1.0f);

    if (this->attr.ctrl.bandwidth == bandwidth)
        return;

    this->attr.ctrl.bandwidth = bandwidth;
}

void reverb::set_damping(float damping)
{
    damping = std::clamp(damping, 0.0f, 1.0f);

    if (this->attr.ctrl.damping == damping)
        return;

    this->attr.ctrl.damping = damping;
}

void reverb::set_decay(float decay)
{
    decay = std::clamp(decay, 0.0f, 0.99f);

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



