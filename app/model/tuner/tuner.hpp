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
    libs::adsp::decimator<decim_factor, config::dsp_vector_size> decimator;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::highpass> hpf;
    libs::adsp::envelope_follower envf;
    libs::adsp::median_filter median;
    libs::adsp::averaging_filter ema;
    libs::adsp::pitch_detector<config::dsp_vector_size / decim_factor, 256> pitch_detector;

    bool mute;
    float envelope;
    float detected_pitch;
    unsigned frame_counter;
    std::array<float, config::dsp_vector_size / decim_factor> decim_input;

    tuner_attr attr {0};
};

}

#endif /* MODEL_TUNER_TUNER_HPP_ */
