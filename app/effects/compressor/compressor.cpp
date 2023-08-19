/*
 * compressor.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "compressor.hpp"

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */

compressor::compressor() : effect { effect_id::compressor, "compressor" }
{

}

compressor::~compressor()
{

}

void compressor::process(const dsp_input_t& in, dsp_output_t& out)
{
//    printf("Effect '%s' (id:%u) processing data\n", this->name.data(), static_cast<unsigned>(this->id));
    out = in;
}


