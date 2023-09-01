/*
 * cabinet_sim.hpp
 *
 *  Created on: 1 wrz 2023
 *      Author: kwarc
 */

#ifndef MODEL_CABINET_SIM_CABINET_SIM_HPP_
#define MODEL_CABINET_SIM_CABINET_SIM_HPP_

#include "app/model/effect_interface.hpp"

#include <array>

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

    cabinet_sim();
    virtual ~cabinet_sim();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
private:
    /* Emulation of speaker cabinet Blackheart BH112 1×12″ driven by a Little Giant 5 amp.
     * Cascade of 4 biquad IIR filters is used to emulate sound of speaker.
     *
     * Copyright Z Squared DSP Pty Limited
     * Permission is granted for non-commercial use of these filters
     */
    arm_biquad_casd_df1_inst_f32 iir_spk_cab_sim;
    constexpr static unsigned iir_spk_cab_sim_biquad_stages = 4;
    std::array<float, 4 * iir_spk_cab_sim_biquad_stages> iir_spk_cab_sim_state;
    constexpr static inline std::array<float, 5 * iir_spk_cab_sim_biquad_stages> iir_spk_cab_sim_coeffs
    {{
        // bx_0, bx_1, bx_2, -ax_1, -ax_2, where x - number of biquad stage
        0.998427797774257, -1.996855595548515, 0.998427797774258, 1.996729031901556, -0.996982159195472,
        1.71381752013609, -3.59123502602204, 1.89042101128582, 1.946518614625237, -0.959522120025104,
        2.44712046192491, -5.07920063666641, 2.71162250478877, 1.847874331025749, -0.92741666107301,
        0.0744394809810769, 0.1488789619621539, 0.0744394809810770, 1.433046457023383, -0.730804380947690,
    }};
};

}

#endif /* MODEL_CABINET_SIM_CABINET_SIM_HPP_ */
