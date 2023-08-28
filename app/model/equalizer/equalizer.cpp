/*
 * equalizer.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */


#include "equalizer.hpp"

using namespace mfx;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


equalizer::equalizer() : effect { effect_id::equalizer, "equalizer" }
{

}

equalizer::~equalizer()
{

}

void equalizer::process(const dsp_input_t& in, dsp_output_t& out)
{
//    printf("Effect '%s'(id:%u) processing data\n", this->name.data(), static_cast<unsigned>(this->id));
    out = in;
}

