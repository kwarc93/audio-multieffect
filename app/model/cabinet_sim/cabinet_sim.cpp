/*
 * cabinet_sim.cpp
 *
 *  Created on: 1 wrz 2023
 *      Author: kwarc
 */

#include "cabinet_sim.hpp"

#include <cstring>

using namespace mfx;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


cabinet_sim::cabinet_sim(const ir_t &ir) : effect { effect_id::cabinet_sim, "cabinet_sim" }
{
    this->fast_conv.set_ir(ir.data());
}

cabinet_sim::~cabinet_sim()
{

}

void cabinet_sim::process(const dsp_input_t& in, dsp_output_t& out)
{
    this->fast_conv.process(in.data(), out.data());
}

const effect_specific_attributes cabinet_sim::get_specific_attributes(void) const
{
    return this->attributes;
}

