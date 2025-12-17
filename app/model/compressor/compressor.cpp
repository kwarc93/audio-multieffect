/*
 * compressor.cpp
 *
 *  Created on: 16 Dec 2025
 *      Author: Roo
 */

#include "compressor.hpp"
#include "app/config.hpp"
#include <algorithm>

using namespace mfx;

compressor::compressor() : effect { effect_id::compressor },
envelope {0.0f},
attack_coeff {0.0f},
release_coeff {0.0f},
threshold_linear {0.0f},
attr {}
{
    // Initialize with default values
    this->attr.ctrl = compressor_attr::default_ctrl;
    this->attr.out.gain_reduction = 0.0f;
    
    // Calculate initial coefficients
    this->set_threshold(this->attr.ctrl.threshold);
    this->set_attack(this->attr.ctrl.attack);
    this->set_release(this->attr.ctrl.release);
}

compressor::~compressor()
{

}

void compressor::process(const dsp_input &in, dsp_output &out)
{
    const float makeup_gain_linear = db_to_linear(this->attr.ctrl.makeup_gain);
    
    for (size_t i = 0; i < in.size(); i++)
    {
        // Get input sample
        float input = in[i];
        
        // Calculate input level in dB
        float input_level_db = linear_to_db(input);
        
        // Calculate desired gain reduction
        float target_gain_reduction_db = this->calculate_gain_reduction(input_level_db);
        
        // Smooth gain reduction with attack/release
        if (target_gain_reduction_db < this->envelope)
        {
            // Attack (gain reduction increasing)
            this->envelope += this->attack_coeff * (target_gain_reduction_db - this->envelope);
        }
        else
        {
            // Release (gain reduction decreasing)
            this->envelope += this->release_coeff * (target_gain_reduction_db - this->envelope);
        }
        
        // Convert gain reduction to linear
        float gain_linear = db_to_linear(this->envelope);
        
        // Apply compression and makeup gain
        out[i] = input * gain_linear * makeup_gain_linear;
        
        // Clamp output to prevent clipping
        out[i] = std::clamp(out[i], -1.0f, 1.0f);
    }
    
    // Update output attributes
    this->attr.out.gain_reduction = this->envelope;
}

const effect_specific_attr compressor::get_specific_attributes(void) const
{
    return this->attr;
}

void compressor::set_threshold(float threshold_db)
{
    this->attr.ctrl.threshold = std::clamp(threshold_db, -60.0f, 0.0f);
    this->threshold_linear = db_to_linear(this->attr.ctrl.threshold);
}

void compressor::set_ratio(float ratio)
{
    this->attr.ctrl.ratio = std::clamp(ratio, 1.0f, 20.0f);
}

void compressor::set_attack(float attack_ms)
{
    this->attr.ctrl.attack = std::clamp(attack_ms, 0.1f, 100.0f);
    
    // Convert attack time to samples
    float attack_samples = (this->attr.ctrl.attack / 1000.0f) * config::sampling_frequency_hz;
    
    // Calculate exponential smoothing coefficient
    this->attack_coeff = 1.0f - std::exp(-1.0f / attack_samples);
}

void compressor::set_release(float release_ms)
{
    this->attr.ctrl.release = std::clamp(release_ms, 10.0f, 1000.0f);
    
    // Convert release time to samples
    float release_samples = (this->attr.ctrl.release / 1000.0f) * config::sampling_frequency_hz;
    
    // Calculate exponential smoothing coefficient
    this->release_coeff = 1.0f - std::exp(-1.0f / release_samples);
}

void compressor::set_makeup_gain(float gain_db)
{
    this->attr.ctrl.makeup_gain = std::clamp(gain_db, 0.0f, 30.0f);
}

void compressor::set_knee(float knee_db)
{
    this->attr.ctrl.knee = std::clamp(knee_db, 0.0f, 12.0f);
}

float compressor::calculate_gain_reduction(float input_db)
{
    const float threshold_db = this->attr.ctrl.threshold;
    const float ratio = this->attr.ctrl.ratio;
    const float knee_db = this->attr.ctrl.knee;
    
    float output_db;
    
    if (knee_db > 0.0f)
    {
        // Soft knee compression
        if (input_db < (threshold_db - knee_db / 2.0f))
        {
            // Below knee - no compression
            output_db = input_db;
        }
        else if (input_db > (threshold_db + knee_db / 2.0f))
        {
            // Above knee - full compression
            output_db = threshold_db + (input_db - threshold_db) / ratio;
        }
        else
        {
            // In knee region - smooth transition
            float knee_factor = (input_db - threshold_db + knee_db / 2.0f) / knee_db;
            float compressed = threshold_db + (input_db - threshold_db) / ratio;
            output_db = input_db + knee_factor * (compressed - input_db);
        }
    }
    else
    {
        // Hard knee compression
        if (input_db <= threshold_db)
        {
            output_db = input_db;
        }
        else
        {
            output_db = threshold_db + (input_db - threshold_db) / ratio;
        }
    }
    
    // Return gain reduction (negative value)
    return output_db - input_db;
}
