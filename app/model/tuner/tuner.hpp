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

    constexpr static unsigned decim_factor {4};
private:
    libs::adsp::envelope_follower envelope;
    libs::adsp::median_filter pitch_median;
    libs::adsp::averaging_filter pitch_avg;
    libs::adsp::pitch_detector<config::dsp_vector_size / decim_factor, 256> pitch_det;
    float detected_pitch;
    unsigned frame_counter;

    tuner_attr attr {0};
};

}

#endif /* MODEL_TUNER_TUNER_HPP_ */
