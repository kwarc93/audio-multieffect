/*
 * vocoder.hpp
 *
 *  Created on: 24 sty 2024
 *      Author: kwarc
 */

#ifndef MODEL_VOCODER_VOCODER_HPP_
#define MODEL_VOCODER_VOCODER_HPP_

#include "app/model/effect_interface.hpp"

#include <array>
#include <memory>

#include <libs/audio_dsp.hpp>

namespace mfx
{

class vocoder : public effect
{
public:
    vocoder (float clarity = 0.99f,float tone = 0.05f, unsigned bands = 64, vocoder_attr::controls::mode_type mode = vocoder_attr::controls::mode_type::modern);
    virtual ~vocoder ();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_mode(vocoder_attr::controls::mode_type mode);
    void set_clarity(float clarity);
    void set_tone(float tone);
    void set_bands(unsigned bands);
    void hold(bool state);

private:
    libs::adsp::iir_highpass hp;

    class vintage_vocoder;
    class modern_vocoder;

    std::unique_ptr<vintage_vocoder> vintage;
    std::unique_ptr<modern_vocoder> modern;

    vocoder_attr attr {0};
};

}

#endif /* MODEL_VOCODER_VOCODER_HPP_ */
