/*
 * tremolo.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "tremolo.hpp"

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

tremolo::tremolo() : effect { effect_id::tremolo, "tremolo" }
{

}

tremolo::~tremolo()
{

}

void tremolo::process(const dsp_input_t& in, dsp_output_t& out)
{
//    printf("Effect '%s' (id:%u) processing data\n", this->name.data(), static_cast<unsigned>(this->id));
    out = in;
}


