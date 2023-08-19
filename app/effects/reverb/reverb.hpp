/*
 * reverb.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_REVERB_REVERB_HPP_
#define EFFECTS_REVERB_REVERB_HPP_

#include "app/effects/effect_interface.hpp"

class reverb : public effect
{
public:
    reverb();
    virtual ~reverb();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
};

#endif /* EFFECTS_REVERB_REVERB_HPP_ */
