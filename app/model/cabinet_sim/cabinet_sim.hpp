/*
 * cabinet_sim.hpp
 *
 *  Created on: 1 wrz 2023
 *      Author: kwarc
 */

#ifndef MODEL_CABINET_SIM_CABINET_SIM_HPP_
#define MODEL_CABINET_SIM_CABINET_SIM_HPP_

#include "app/model/effect_interface.hpp"

#include "impulse_responses.hpp"

#include <array>
#include <cmath>

#include <cmsis/stm32f7xx.h>
#include <cmsis/dsp/arm_math.h>

namespace mfx
{

class cabinet_sim : public effect
{
public:
    struct controls
    {

    };

    struct state
    {
        int error_code;
    };

    cabinet_sim(const ir_t &ir = ir_1960_G12M25_SM57_Cap45_0_5in);
    virtual ~cabinet_sim();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
private:

    constexpr static uint32_t ir_size {1024};
    const ir_t &ir;

    /* Ceil to next power of 2 */
    constexpr static uint32_t fft_size {std::max((uint32_t)dsp_vector_size, 1UL << static_cast<uint32_t>(std::floor(std::log2(ir_size - 1)) + 1))};

    arm_rfft_fast_instance_f32 fft;
    std::array<float, 2 * fft_size> ir_fft;
    std::array<float, 2 * fft_size> input_buffer_fft;
    std::array<float, 2 * fft_size> convolution;
    std::array<float, fft_size> input_buffer;
};

}

#endif /* MODEL_CABINET_SIM_CABINET_SIM_HPP_ */
