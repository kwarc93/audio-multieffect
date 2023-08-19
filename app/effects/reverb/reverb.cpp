/*
 * reverb.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "reverb.hpp"

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

reverb::reverb() : effect { effect_id::reverb, "reverb" }
{

}

reverb::~reverb()
{

}

void reverb::process(const dsp_input_t& in, dsp_output_t& out)
{
//    printf("Effect '%s' (id:%u) processing data\n", this->name.data(), static_cast<unsigned>(this->id));
    out = in;
}


