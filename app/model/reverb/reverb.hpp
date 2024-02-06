/*
 * reverb.hpp
 *
 *  Created on: 8 lis 2023
 *      Author: kwarc
 */

#ifndef MODEL_REVERB_REVERB_HPP_
#define MODEL_REVERB_REVERB_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

namespace mfx
{

class reverb : public effect
{
public:
    reverb(float bandwidth = 0.9995f, float damping = 0.0005f, float decay = 0.5f, reverb_attr::controls::mode_type mode = reverb_attr::controls::mode_type::plate);
    virtual ~reverb();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_bandwidth(float bw);
    void set_damping(float d);
    void set_decay(float decay);
    void set_mode(reverb_attr::controls::mode_type mode);

private:
    libs::adsp::delay_line pdel, del1, del2, del3, del4;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::lowpass> lpf1, lpf2, lpf3;
    libs::adsp::unicomb apf1, apf2, apf3, apf4, apf5, apf6;
    libs::adsp::unicomb mapf1, mapf2;
    libs::adsp::oscillator lfo1, lfo2;

    float mix;

    reverb_attr attr {0};
};

}

#endif /* MODEL_REVERB_REVERB_HPP_ */
