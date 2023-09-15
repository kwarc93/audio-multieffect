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
    cabinet_sim(const ir_t &ir = ir_1960_G12M25_SM57_Cap45_0_5in);
    virtual ~cabinet_sim();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

private:
    constexpr static cabinet_sim_attributes::controls::resolution ir_res {cabinet_sim_attributes::controls::resolution::standart};
    constexpr static uint32_t ir_size {static_cast<uint32_t>(ir_res) - dsp_vector_size};

    /* FFT based convolution */
    libs::adsp::fast_convolution<dsp_vector_size, ir_size> fast_conv;

    cabinet_sim_attributes attributes;
};

}

#endif /* MODEL_CABINET_SIM_CABINET_SIM_HPP_ */
