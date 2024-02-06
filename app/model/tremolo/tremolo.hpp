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
    tremolo(float rate = 8, float depth = 0.3f, tremolo_attr::controls::shape_type shape = tremolo_attr::controls::shape_type::sine);
    virtual ~tremolo();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_depth(float depth);
    void set_rate(float rate);
    void set_shape(tremolo_attr::controls::shape_type shape);
private:
    libs::adsp::oscillator lfo;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::lowpass> lpf;

    tremolo_attr attr {0};
};

}

#endif /* MODEL_TREMOLO_TREMOLO_HPP_ */
