/*
 * cabinet_sim.hpp
 *
 *  Created on: 1 wrz 2023
 *      Author: kwarc
 */

#ifndef MODEL_CABINET_SIM_CABINET_SIM_HPP_
#define MODEL_CABINET_SIM_CABINET_SIM_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

#include "impulse_responses.hpp"

namespace mfx
{

class cabinet_sim : public effect
{
public:
    enum class resolution {standart = 1024, high = 2048};

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
    constexpr static resolution ir_res {resolution::standart};
    constexpr static uint32_t ir_size {static_cast<uint32_t>(ir_res) - dsp_vector_size};

    libs::adsp::fast_convolution<dsp_vector_size, ir_size> fast_conv;
};

}

#endif /* MODEL_CABINET_SIM_CABINET_SIM_HPP_ */
