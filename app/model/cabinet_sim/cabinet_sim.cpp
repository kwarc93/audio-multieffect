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
    arm_biquad_cascade_df1_init_f32
    (
        &this->iir_spk_cab_sim,
        this->iir_spk_cab_sim_biquad_stages,
        const_cast<float*>(this->iir_spk_cab_sim_coeffs.data()),
        this->iir_spk_cab_sim_state.data()
    );
}

cabinet_sim::~cabinet_sim()
{

}

void cabinet_sim::process(const dsp_input_t& in, dsp_output_t& out)
{
    /* Apply speaker cabinet emulation using 8-th order IIR filter */
    arm_biquad_cascade_df1_f32(&this->iir_spk_cab_sim, const_cast<dsp_sample_t*>(in.data()), out.data(), out.size());
}


