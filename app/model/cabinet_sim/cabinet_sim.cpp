/*
 * cabinet_sim.cpp
 *
 *  Created on: 1 wrz 2023
 *      Author: kwarc
 */

#include "cabinet_sim.hpp"

#include <cstring>
#include <array>

using namespace mfx;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


cabinet_sim::cabinet_sim(const ir_t &ir) : effect { effect_id::cabinet_sim, "cabinet_sim" }
{
    this->fast_conv.set_ir(ir.data());
    this->attributes.ctrl.ir_idx = 0;
}

cabinet_sim::~cabinet_sim()
{

}

void cabinet_sim::process(const dsp_input& in, dsp_output& out)
{
    this->fast_conv.process(in.data(), out.data());
}

const effect_specific_attributes cabinet_sim::get_specific_attributes(void) const
{
    return this->attributes;
}

void cabinet_sim::set_ir(const ir_t &ir)
{
    this->fast_conv.set_ir(ir.data());
}

void cabinet_sim::set_ir(uint8_t idx)
{
    static constexpr std::array<const ir_t*, 3> ir_map
    {
        &ir_1960_G12M25_SM57_Cap45_0_5in,
        &ir_orange2x12,
        &ir_cf_1on_pres8
    };

    this->fast_conv.set_ir(ir_map.at(idx)->data());
    this->attributes.ctrl.ir_idx = idx;
}

