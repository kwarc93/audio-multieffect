/*
 * compressor.hpp
 *
 *  Created on: 16 Dec 2025
 *      Author: Roo
 */

#ifndef MODEL_COMPRESSOR_COMPRESSOR_HPP_
#define MODEL_COMPRESSOR_COMPRESSOR_HPP_

#include "app/model/effect_interface.hpp"
#include "libs/audio_dsp.hpp"
#include <cmath>

namespace mfx
{

class compressor : public effect
{
public:
    compressor();
    virtual ~compressor();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attr get_specific_attributes(void) const override;

    void set_threshold(float threshold_db);
    void set_ratio(float ratio);
    void set_attack(float attack_ms);
    void set_release(float release_ms);
    void set_makeup_gain(float gain_db);
    void set_knee(float knee_db);

private:
    // Convert dB to linear
    static float db_to_linear(float db) { return std::pow(10.0f, db / 20.0f); }
    
    // Convert linear to dB
    static float linear_to_db(float linear) { return 20.0f * std::log10(std::abs(linear) + 1e-10f); }
    
    // Calculate gain reduction based on input level
    float calculate_gain_reduction(float input_db);
    
    // Envelope follower state
    float envelope;
    
    // Time constants
    float attack_coeff;
    float release_coeff;
    
    // Threshold in linear
    float threshold_linear;
    
    compressor_attr attr;
};

}

#endif /* MODEL_COMPRESSOR_COMPRESSOR_HPP_ */
