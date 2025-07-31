/*
 * overdrive.hpp
 *
 *  Created on: 30 sie 2023
 *      Author: kwarc
 */

#ifndef MODEL_OVERDRIVE_OVERDRIVE_HPP_
#define MODEL_OVERDRIVE_OVERDRIVE_HPP_

#include "app/model/effect_interface.hpp"

#include <array>

#include <libs/audio_dsp.hpp>

namespace mfx
{

class overdrive : public effect
{
public:
    overdrive();
    virtual ~overdrive();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attr get_specific_attributes(void) const override;

    void set_high(float high);
    void set_gain(float gain);
    void set_low(float low);
    void set_mix(float mix);
    void set_mode(overdrive_attr::controls::mode_type mode);

private:
    float soft_clip(float in);
    float hard_clip(float in);

    /* Interpolator & decimator for oversampling & decimation*/
    constexpr static uint8_t oversampling_factor = 2;
    libs::adsp::interpolator<oversampling_factor, config::dsp_vector_size> intrpl;
    libs::adsp::decimator<oversampling_factor, oversampling_factor * config::dsp_vector_size> decim;
    std::array<float, oversampling_factor * config::dsp_vector_size> sample_buffer;

    /* Tunable high-pass 2nd order IIR filter */
    libs::adsp::iir_highpass iir_hp;

    /* Tunable low-pass 2-nd order IIR filter */
    libs::adsp::iir_lowpass iir_lp;

    overdrive_attr attr {0};
};

}


#endif /* MODEL_OVERDRIVE_OVERDRIVE_HPP_ */
