/*
 * equalizer.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EQUALIZER_EQUALIZER_HPP_
#define MODEL_EQUALIZER_EQUALIZER_HPP_

#include "app/model/effect_interface.hpp"

namespace mfx
{

class equalizer : public effect
{
public:
    /* 3-band parametric equalizer */
    struct controls
    {
        struct section_params
        {
            float gain, q_factor, freq;
        };

        section_params low;
        section_params mid;
        section_params hi;
    };

    struct state
    {
        int error_code;
    };

    equalizer();
    virtual ~equalizer();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
};

}

#endif /* MODEL_EQUALIZER_EQUALIZER_HPP_ */