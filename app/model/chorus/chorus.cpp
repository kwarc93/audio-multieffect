/*
 * chorus.cpp
 *
 *  Created on: 30 wrz 2023
 *      Author: kwarc
 */

#include "chorus.hpp"

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


chorus::chorus(float depth, float rate, float tone, float mix) : effect { effect_id::chorus },
attr {}
{
    this->set_depth(depth);
    this->set_rate(rate);
    this->set_tone(tone);
    this->set_mix(mix);
}

chorus::~chorus()
{

}

void chorus::process(const dsp_input& in, dsp_output& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* TODO */
        return input;
    }
    );
}

const effect_specific_attributes chorus::get_specific_attributes(void) const
{
    return this->attr;
}

void chorus::set_depth(float depth)
{

}

void chorus::set_rate(float rate)
{

}

void chorus::set_tone(float tone)
{

}

void chorus::set_mix(float mix)
{

}



