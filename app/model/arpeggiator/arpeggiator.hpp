/*
 * arpeggiator.hpp
 *
 *  Created on: 16 Dec 2025
 *      Author: Roo
 */

#ifndef MODEL_ARPEGGIATOR_ARPEGGIATOR_HPP_
#define MODEL_ARPEGGIATOR_ARPEGGIATOR_HPP_

#include "app/model/effect_interface.hpp"
#include <array>
#include <cmath>

namespace mfx
{

class arpeggiator : public effect
{
public:
    arpeggiator();
    virtual ~arpeggiator();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attr get_specific_attributes(void) const override;

    void set_rate(float bpm);
    void set_pattern(arpeggiator_attr::controls::pattern_type pattern);
    void set_mix(float mix);
    void set_steps(uint8_t steps);

private:
    // Arpeggiator patterns (semitone offsets)
    static constexpr std::array<uint8_t, 16> pattern_up = {0, 4, 7, 12, 7, 4, 0, 0, 0, 4, 7, 12, 7, 4, 0, 0};
    static constexpr std::array<uint8_t, 16> pattern_down = {12, 7, 4, 0, 4, 7, 12, 12, 12, 7, 4, 0, 4, 7, 12, 12};
    static constexpr std::array<uint8_t, 16> pattern_updown = {0, 4, 7, 12, 12, 7, 4, 0, 0, 4, 7, 12, 12, 7, 4, 0};
    static constexpr std::array<uint8_t, 16> pattern_random = {0, 7, 4, 12, 0, 4, 12, 7, 4, 0, 7, 12, 4, 7, 0, 12};
    static constexpr std::array<uint8_t, 16> pattern_octave = {0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 12, 0, 12};
    
    // Pitch shift ratios for semitones (equal temperament)
    static constexpr std::array<float, 13> semitone_ratios = {
        1.0f, 1.059463094359f, 1.122462048309f, 1.189207115003f,
        1.259921049895f, 1.334839854170f, 1.414213562373f, 1.498307076877f,
        1.587401051968f, 1.681792830507f, 1.781797436281f, 1.887748625363f,
        2.0f
    };
    
    // Get current pattern
    const std::array<uint8_t, 16>& get_current_pattern() const;
    
    // Calculate pitch shift for current step
    float get_pitch_shift() const;
    
    // Step tracking
    uint32_t sample_counter;
    uint32_t samples_per_step;
    uint8_t current_step;
    
    // Simple delay line for pitch shifting
    static constexpr size_t delay_buffer_size = 4096;
    std::array<float, delay_buffer_size> delay_buffer;
    size_t write_pos;
    
    arpeggiator_attr attr;
};

}

#endif /* MODEL_ARPEGGIATOR_ARPEGGIATOR_HPP_ */
