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
    echo();
    virtual ~echo();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attr get_specific_attributes(void) const override;

    void set_blur(float blur);
    void set_time(float time);
    void set_feedback(float feedback);
    void set_mode(echo_attr::controls::mode_type mode);

private:
    libs::adsp::unicomb unicomb;

    echo_attr attr {0};
};

}

#endif /* MODEL_ECHO_ECHO_HPP_ */
