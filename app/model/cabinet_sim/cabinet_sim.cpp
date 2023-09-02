/*
 * cabinet_sim.cpp
 *
 *  Created on: 1 wrz 2023
 *      Author: kwarc
 */

#include "cabinet_sim.hpp"

using namespace mfx;

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


cabinet_sim::cabinet_sim() : effect { effect_id::cabinet_sim, "cabinet_sim" }
{
    arm_fir_init_f32
    (
        &this->fir,
        this->fir_coeffs.size(),
        const_cast<float*>(this->fir_coeffs.data()),
        this->fir_state.data(),
        this->fir_block_size
    );
}

cabinet_sim::~cabinet_sim()
{

}

void cabinet_sim::process(const dsp_input_t& in, dsp_output_t& out)
{
    /* Apply speaker cabinet emulation using FIR filter */
    arm_fir_f32(&this->fir, const_cast<dsp_sample_t*>(in.data()), out.data(), out.size());
}


