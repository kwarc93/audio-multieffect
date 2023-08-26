/*
 * tremolo.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_TREMOLO_TREMOLO_HPP_
#define EFFECTS_TREMOLO_TREMOLO_HPP_

#include "app/effects/effect_interface.hpp"

class tremolo : public effect
{
public:
    struct controls_t
    {
        float rate;
        float depth;
        enum class lfo_shape {triangle, sine} shape;
    };

    struct state_t
    {
        int error_code;
    };

    tremolo();
    virtual ~tremolo();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
};

#endif /* EFFECTS_TREMOLO_TREMOLO_HPP_ */
