/*
 * tuner.hpp
 *
 *  Created on: 22 sie 2025
 *      Author: kwarc
 */

#ifndef MODEL_TUNER_TUNER_HPP_
#define MODEL_TUNER_TUNER_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

#include <q/pitch/pitch_detector.hpp>

namespace mfx
{

class tuner : public effect
{
public:
    tuner();
    virtual ~tuner();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attr get_specific_attributes(void) const override;

    void set_a4_tuning(unsigned frequency);
private:
    libs::adsp::averaging_filter pitch_avg;
    cycfi::q::pitch_detector pitch_det;
    float detected_pitch;
    unsigned frame_counter;

    tuner_attr attr {0};
};

}

#endif /* MODEL_TUNER_TUNER_HPP_ */
