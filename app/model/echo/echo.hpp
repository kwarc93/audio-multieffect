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
    enum class mode_type {delay, echo};

    struct controls
    {
        float blend; // Blend level, range: [0, 1.0]
        float time; // Delay time, range: [0.1s, 1.0s]
        float feedback; // Feedback, range: [0.0, 1.0]
        mode_type mode; // Mode of effect, allowed values: echo, delay
    };

    struct state
    {
        int error_code;
    };

    echo(float blend = 0.5f, float time = 0.3f, float feedback = 0.5f, mode_type mode = mode_type::echo);
    virtual ~echo();

    void process(const dsp_input_t &in, dsp_output_t &out) override;

    void set_blend(float blend);
    void set_time(float time);
    void set_feedback(float feedback);
    void set_mode(mode_type mode);

private:
    float blend;
    float time;
    float feedback;
    float feedforward;
    mode_type mode;

    libs::adsp::delay_line delay_line;
};

}

#endif /* MODEL_ECHO_ECHO_HPP_ */
