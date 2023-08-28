/*
 * noise_gate.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "noise_gate.hpp"

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

noise_gate::noise_gate() : effect { effect_id::noise_gate, "noise_gate" }
{

}

noise_gate::~noise_gate()
{

}

void noise_gate::process(const dsp_input_t& in, dsp_output_t& out)
{
//    printf("Effect '%s' (id:%u) processing data\n", this->name.data(), static_cast<unsigned>(this->id));
    out = in;
}


