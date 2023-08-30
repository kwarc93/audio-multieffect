/*
 * overdrive.hpp
 *
 *  Created on: 30 sie 2023
 *      Author: kwarc
 */

#ifndef MODEL_OVERDRIVE_OVERDRIVE_HPP_
#define MODEL_OVERDRIVE_OVERDRIVE_HPP_

#include "app/model/effect_interface.hpp"

namespace mfx
{

class overdrive : public effect
{
public:
    enum class mode_type {distortion, overdrive};

    struct controls
    {
        float high;
        float low;
        float gain;
        float mix;
        mode_type mode;
    };

    struct state
    {
        int error_code;
    };

    overdrive(float high = 0.5f, float low = 0.5f, float gain = 0.5f, float mix = 0.5f, mode_type mode = mode_type::overdrive);
    virtual ~overdrive();

    void process(const dsp_input_t &in, dsp_output_t &out) override;

    void set_high(float high);
    void set_low(float low);
    void set_gain(float gain);
    void set_mix(float mix);
    void set_mode(mode_type mode);

private:
    float high;
    float low;
    float gain;
    float mix;

    mode_type mode;
};

}


#endif /* MODEL_OVERDRIVE_OVERDRIVE_HPP_ */
