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


cabinet_sim::cabinet_sim(const ir_t &ir) : effect { effect_id::cabinet_sim, "cabinet_sim" },
ir{ir}
{
    arm_fill_f32(0, this->input_buffer.data(), this->input_buffer.size());

    /* Precompute FFT of IR */
    arm_fill_f32(0, this->ir_fft.data(), this->ir.size());
    arm_copy_f32(const_cast<float*>(this->ir.data()), this->ir_fft.data(), this->fft_size);
    arm_rfft_fast_init_f32(&this->fft, this->fft_size);
    arm_rfft_fast_f32(&this->fft, this->ir_fft.data(), this->ir_fft.data() + this->fft_size, 0);
}

cabinet_sim::~cabinet_sim()
{

}

void cabinet_sim::process(const dsp_input_t& in, dsp_output_t& out)
{
    const uint32_t block_size = in.size();
    const uint32_t move_size = this->input_buffer.size() - block_size;

    /* Sliding window of input blocks */
    arm_copy_f32(&this->input_buffer[block_size], &this->input_buffer[0], move_size);
    arm_copy_f32(const_cast<dsp_sample_t*>(in.data()), &this->input_buffer[move_size], block_size);

    /* FFT of sliding window */
    arm_copy_f32(this->input_buffer.data(), this->input_buffer_fft.data(), this->input_buffer.size());
    arm_rfft_fast_init_f32(&this->fft, this->fft_size);
    arm_rfft_fast_f32(&this->fft, this->input_buffer_fft.data(), this->input_buffer_fft.data() + this->fft_size, 0);

    /* Multiplication (convolution) in frequency domain */
    arm_cmplx_mult_cmplx_f32(this->ir_fft.data() + this->fft_size, this->input_buffer_fft.data() + this->fft_size, this->convolution.data(), this->fft_size / 2);

    /* Inverse FFT */
    arm_rfft_fast_init_f32(&this->fft, this->fft_size);
    arm_rfft_fast_f32(&this->fft, this->convolution.data(), this->convolution.data() + this->fft_size, 1);

    /* Copy output */
    arm_copy_f32(this->convolution.data() + this->convolution.size() - block_size, out.data(), block_size);
}


