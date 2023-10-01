/*
 * chorus.hpp
 *
 *  Created on: 30 wrz 2023
 *      Author: kwarc
 */

#ifndef MODEL_CHORUS_CHORUS_HPP_
#define MODEL_CHORUS_CHORUS_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

namespace mfx
{

class chorus : public effect
{
public:
    chorus(float depth = 0.4f, float rate = 0.3f, float tone = 0.5f, float mix = 0.5f);
    virtual ~chorus();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_depth(float depth);
    void set_rate(float rate);
    void set_tone(float tone);
    void set_mix(float mix);
    void set_mode(chorus_attr::controls::mode_type mode);

private:
    libs::adsp::oscillator lfo;
    libs::adsp::oscillator lfo2;
    libs::adsp::delay_line<libs::adsp::delay_line_intrpl::allpass> delay_line;
    libs::adsp::delay_line<libs::adsp::delay_line_intrpl::allpass> delay_line2;

    chorus_attr attr;
};

}



#endif /* MODEL_CHORUS_CHORUS_HPP_ */
