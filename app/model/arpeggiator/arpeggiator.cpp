/*
 * arpeggiator.cpp
 *
 *  Created on: 16 Dec 2025
 *      Author: Roo
 */

#include "arpeggiator.hpp"
#include "app/config.hpp"
#include <algorithm>

using namespace mfx;

arpeggiator::arpeggiator() : effect { effect_id::arpeggiator },
sample_counter {0},
samples_per_step {0},
current_step {0},
delay_buffer {},
write_pos {0},
attr {}
{
    // Initialize with default values
    this->attr.ctrl = arpeggiator_attr::default_ctrl;
    
    // Calculate samples per step based on BPM
    this->set_rate(this->attr.ctrl.rate);
    
    // Initialize delay buffer
    std::fill(this->delay_buffer.begin(), this->delay_buffer.end(), 0.0f);
}

arpeggiator::~arpeggiator()
{

}

void arpeggiator::process(const dsp_input &in, dsp_output &out)
{
    const float mix = this->attr.ctrl.mix;
    const float pitch_shift = this->get_pitch_shift();
    
    for (size_t i = 0; i < in.size(); i++)
    {
        // Write input to delay buffer
        this->delay_buffer[this->write_pos] = in[i];
        
        // Calculate read position with pitch shift
        // Simple pitch shifting using variable delay
        float read_pos_float = this->write_pos - (pitch_shift * 100.0f);
        if (read_pos_float < 0)
            read_pos_float += delay_buffer_size;
        
        size_t read_pos = static_cast<size_t>(read_pos_float) % delay_buffer_size;
        
        // Linear interpolation for smoother pitch shifting
        size_t read_pos_next = (read_pos + 1) % delay_buffer_size;
        float frac = read_pos_float - std::floor(read_pos_float);
        float shifted_sample = this->delay_buffer[read_pos] * (1.0f - frac) + 
                              this->delay_buffer[read_pos_next] * frac;
        
        // Mix dry and wet signals
        out[i] = in[i] * (1.0f - mix) + shifted_sample * mix;
        
        // Clamp output
        out[i] = std::clamp(out[i], -1.0f, 1.0f);
        
        // Update write position
        this->write_pos = (this->write_pos + 1) % delay_buffer_size;
        
        // Update step counter
        this->sample_counter++;
        if (this->sample_counter >= this->samples_per_step)
        {
            this->sample_counter = 0;
            this->current_step = (this->current_step + 1) % this->attr.ctrl.steps;
        }
    }
}

const effect_specific_attr arpeggiator::get_specific_attributes(void) const
{
    return this->attr;
}

void arpeggiator::set_rate(float bpm)
{
    this->attr.ctrl.rate = std::clamp(bpm, 60.0f, 240.0f);
    
    // Calculate samples per step (16th note)
    float beat_duration = 60.0f / this->attr.ctrl.rate; // Duration of one beat in seconds
    float step_duration = beat_duration / 4.0f; // 16th note duration
    this->samples_per_step = static_cast<uint32_t>(step_duration * config::sampling_frequency_hz);
}

void arpeggiator::set_pattern(arpeggiator_attr::controls::pattern_type pattern)
{
    this->attr.ctrl.pattern = pattern;
    this->current_step = 0; // Reset step when pattern changes
}

void arpeggiator::set_mix(float mix)
{
    this->attr.ctrl.mix = std::clamp(mix, 0.0f, 1.0f);
}

void arpeggiator::set_steps(uint8_t steps)
{
    this->attr.ctrl.steps = std::clamp(steps, static_cast<uint8_t>(4), static_cast<uint8_t>(16));
    
    // Reset step if it's now out of range
    if (this->current_step >= this->attr.ctrl.steps)
        this->current_step = 0;
}

const std::array<uint8_t, 16>& arpeggiator::get_current_pattern() const
{
    switch (this->attr.ctrl.pattern)
    {
        case arpeggiator_attr::controls::pattern_type::up:
            return pattern_up;
        case arpeggiator_attr::controls::pattern_type::down:
            return pattern_down;
        case arpeggiator_attr::controls::pattern_type::updown:
            return pattern_updown;
        case arpeggiator_attr::controls::pattern_type::random:
            return pattern_random;
        case arpeggiator_attr::controls::pattern_type::octave:
            return pattern_octave;
        default:
            return pattern_up;
    }
}

float arpeggiator::get_pitch_shift() const
{
    const auto& pattern = this->get_current_pattern();
    uint8_t semitone = pattern[this->current_step];
    
    // Clamp semitone to valid range
    if (semitone > 12)
        semitone = 12;
    
    return semitone_ratios[semitone];
}
