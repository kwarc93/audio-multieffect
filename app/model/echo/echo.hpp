/*
 * echo.hpp
 *
 *  Created on: 29 sie 2023
 *      Author: kwarc
 */

#ifndef MODEL_ECHO_ECHO_HPP_
#define MODEL_ECHO_ECHO_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

namespace mfx
{

class echo : public effect
{
public:
    echo(float blur = 0.5f, float time = 0.3f, float feedback = 0.6f, echo_attributes::controls::mode_type mode = echo_attributes::controls::mode_type::echo);
    virtual ~echo();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_blur(float blur);
    void set_time(float time);
    void set_feedback(float feedback);
    void set_mode(echo_attributes::controls::mode_type mode);

private:
    float blend;
    float feedforward;

    libs::adsp::iir_lowpass iir_lp;
    libs::adsp::delay_line<libs::adsp::delay_line_intrpl::none> delay_line;

    echo_attributes attributes;
};

}

#endif /* MODEL_ECHO_ECHO_HPP_ */
