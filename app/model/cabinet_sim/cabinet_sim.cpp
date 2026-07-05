/*
 * cabinet_sim.cpp
 *
 *  Created on: 1 wrz 2023
 *      Author: kwarc
 */

#include "cabinet_sim.hpp"

#include "impulse_responses.hpp"

#include <cstring>
#include <utility>
#include <array>

using namespace mfx;

//-----------------------------------------------------------------------------
/* private */

namespace
{

constexpr std::array<std::pair<const char*, const ir_t*>, 4> ir_map
{{
    { "Marshall 1960A 4x12", &ir_1960_G12M25_SM57_Cap45_0_5in },
    { "Orange 2x12", &ir_orange2x12 },
    { "Catharsis Fredman", &ir_cf_1on_pres8 },
    { "2002 Mesa Boogie 4x12", &ir_2002_Mesa_Boogie_4x12 },
}};

}

static_assert(ir_map.size() == cabinet_sim_attr{}.ir_names.size());

//-----------------------------------------------------------------------------
/* public */


cabinet_sim::cabinet_sim() : effect { effect_id::cabinet_sim },
attr {}
{
    this->attr.ctrl.ir_idx = cabinet_sim_attr::default_ctrl.ir_idx;
    this->fast_conv.set_ir(ir_map.at(this->attr.ctrl.ir_idx).second->data());

    for (unsigned i = 0; i < ir_map.size(); i++)
        this->attr.ir_names.at(i) = (ir_map.at(i).first);

}

cabinet_sim::~cabinet_sim()
{

}

void cabinet_sim::process(const dsp_input& in, dsp_output& out)
{
    this->fast_conv.process(in.data(), out.data());
}

const effect_specific_attr cabinet_sim::get_specific_attributes(void) const
{
    return this->attr;
}

void cabinet_sim::set_ir(uint8_t idx)
{
    if (idx >= this->attr.ir_names.size() || this->attr.ctrl.ir_idx == idx)
        return;

    this->attr.ctrl.ir_idx = idx;
    this->fast_conv.set_ir(ir_map.at(idx).second->data());
}

