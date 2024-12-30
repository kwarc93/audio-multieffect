/*
 * phaser.hpp
 *
 *  Created on: 29 gru 2024
 *      Author: kwarc
 */

#ifndef MODEL_PHASER_PHASER_HPP_
#define MODEL_PHASER_PHASER_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

namespace mfx
{

class phaser : public effect
{
public:
    phaser(float rate = 1);
    virtual ~phaser();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_rate(float rate);
private:
    float calc_apf_coeff(float fc, float fs);

    float apf_feedback;
    libs::adsp::oscillator lfo;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::allpass> apf1;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::allpass> apf2;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::allpass> apf3;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::allpass> apf4;

    phaser_attr attr {0};
};

}

#endif /* MODEL_PHASER_PHASER_HPP_ */
