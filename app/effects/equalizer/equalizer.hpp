/*
 * equalizer.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_EQUALIZER_EQUALIZER_HPP_
#define EFFECTS_EQUALIZER_EQUALIZER_HPP_

#include "app/effects/effect_interface.hpp"

class equalizer : public effect
{
public:
    equalizer();
    virtual ~equalizer();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
};

#endif /* EFFECTS_EQUALIZER_EQUALIZER_HPP_ */
