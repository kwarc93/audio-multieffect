/*
 * noise_gate.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_NOISE_GATE_NOISE_GATE_HPP_
#define EFFECTS_NOISE_GATE_NOISE_GATE_HPP_

#include "app/effects/effect_interface.hpp"

class noise_gate : public effect
{
public:
    struct controls
    {
        float threshold;
        float hold;
        float release;
        float attack;
    };

    struct state
    {
        int error_code;
    };

    noise_gate();
    virtual ~noise_gate();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
};

#endif /* EFFECTS_NOISE_GATE_NOISE_GATE_HPP_ */
