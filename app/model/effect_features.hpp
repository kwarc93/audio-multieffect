/*
 * effect_features.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EFFECT_FEATURES_HPP_
#define MODEL_EFFECT_FEATURES_HPP_

#include <vector>
#include <variant>

namespace mfx
{

//-----------------------------------------------------------------------------
/* Constants */
constexpr inline uint16_t dsp_vector_size {128};
constexpr inline uint32_t sampling_frequency_hz {48000};

//-----------------------------------------------------------------------------
/* Type definitions */
typedef std::vector<float> dsp_input_t;
typedef std::vector<float> dsp_output_t;

//-----------------------------------------------------------------------------
/* Available effects */
enum class effect_id { tremolo, echo, overdrive, cabinet_sim };

//-----------------------------------------------------------------------------
/* Effects attributes */
struct tremolo_attributes
{
    struct controls
    {
        float rate; // LFO frequency in Hz, range: [1, 20]
        float depth; // Effect depth, range: [0, 0.5]
        enum class shape_type {sine, triangle} shape; // LFO shape
    } ctrl;

    struct state
    {
        int error_code;
    } stat;
};

struct echo_attributes
{
    struct controls
    {
        float blur; // Blur level (1-st order LP filter cutoff), range: [0, 1.0]
        float time; // Delay time, range: [0.01s, 1.0s]
        float feedback; // Feedback, range: [0.0, 0.9]
        enum class mode_type {delay, echo} mode; // Mode of effect
    } ctrl;

    struct state
    {
        int error_code;
    } stat;
};

struct overdrive_attributes
{
    struct controls
    {
        float low; // High pass filter cutoff, range [0, 1]
        float gain; // Gain, range: [1, 100]
        float high; // Low pass filter cutoff, range [0, 1]
        float mix; // Wet/dry mix, range: [0, 1]
        enum class mode_type {soft, hard} mode; // Clip mode
    } ctrl;

    struct state
    {
        int error_code;
    } stat;
};

struct cabinet_sim_attributes
{
    struct controls
    {
        enum class resolution {standart = 1024, high = 2048} res; // IR resolution in samples
    } ctrl;

    struct state
    {
        int error_code;
    } stat;
};

//-----------------------------------------------------------------------------
/* Effect controls & state */

typedef std::variant
<
    tremolo_attributes,
    echo_attributes,
    overdrive_attributes,
    cabinet_sim_attributes
>
effect_attributes;

}

#endif /* MODEL_EFFECT_FEATURES_HPP_ */
