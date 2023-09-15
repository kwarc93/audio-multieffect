/*
 * tremolo.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_TREMOLO_TREMOLO_HPP_
#define MODEL_TREMOLO_TREMOLO_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

namespace mfx
{

class tremolo : public effect
{
public:
    tremolo(float rate = 8, float depth = 0.3f, tremolo_attributes::controls::shape_type shape = tremolo_attributes::controls::shape_type::triangle);
    virtual ~tremolo();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_depth(float depth);
    void set_rate(float rate);
    void set_shape(tremolo_attributes::controls::shape_type shape);
private:
    /* Low frequency oscillator */
    libs::adsp::oscillator lfo;

    tremolo_attributes attributes;
};

}

#endif /* MODEL_TREMOLO_TREMOLO_HPP_ */
