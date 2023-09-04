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
//    arm_fir_init_f32
//    (
//        &this->fir,
//        this->fir_coeffs.size(),
//        const_cast<float*>(this->fir_coeffs.data()),
//        this->fir_state.data(),
//        this->fir_block_size
//    );

    this->input_buffer.fill(0);

    /* Precompute FFT of IR */
    arm_fill_f32(0.0f, this->ir_fft.data(), this->ir.size());
    arm_copy_f32(const_cast<float*>(this->ir.data()), this->ir_fft.data(), this->fft_size);
    arm_cfft_radix4_init_f32(&this->fft, this->fft_size, 0, 1);
    arm_cfft_radix4_f32(&this->fft, this->ir_fft.data());
}

cabinet_sim::~cabinet_sim()
{

}

void cabinet_sim::process(const dsp_input_t& in, dsp_output_t& out)
{
//    /* Apply speaker cabinet emulation using FIR filter */
//    arm_fir_f32(&this->fir, const_cast<dsp_sample_t*>(in.data()), out.data(), out.size());

    const uint32_t block_size = in.size();
    const uint32_t move_size = this->input_buffer.size() - block_size;

    /* Sliding window */
    arm_copy_f32(&this->input_buffer[block_size], &this->input_buffer[0], move_size);
    arm_copy_f32(const_cast<dsp_sample_t*>(in.data()), &this->input_buffer[move_size], block_size);

    /* FFT of sliding window */
    arm_fill_f32(0.0f, this->input_buffer_fft.data(), this->input_buffer_fft.size());
    arm_copy_f32(this->input_buffer.data(), this->input_buffer_fft.data(), this->input_buffer.size());
    arm_cfft_radix4_init_f32(&this->fft, this->fft_size, 0, 1);
    arm_cfft_radix4_f32(&this->fft, this->input_buffer_fft.data());

    /* Multiplication (convolution) in frequency domain */
    arm_cmplx_mult_cmplx_f32(this->ir_fft.data(), this->input_buffer_fft.data(), this->convolution.data(), this->fft_size);

    /* Inverse FFT */
    arm_cfft_radix4_init_f32(&this->fft, this->fft_size, 1, 1);
    arm_cfft_radix4_f32(&this->fft, this->convolution.data());

    arm_copy_f32(&this->convolution[this->convolution.size() - block_size], out.data(), block_size);
}


