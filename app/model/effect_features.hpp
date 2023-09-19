/*
 * effect_features.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EFFECT_FEATURES_HPP_
#define MODEL_EFFECT_FEATURES_HPP_

#include <array>
#include <variant>
#include <string_view>

namespace mfx
{

//-----------------------------------------------------------------------------
/* Available effects */
enum class effect_id { tremolo, echo, overdrive, cabinet_sim };

//-----------------------------------------------------------------------------
/* Effects attributes */
struct effect_basic_attributes
{
    const effect_id id;
    const std::string_view name;
    bool bypassed;
    int status;
};

struct tremolo_attributes
{
    struct controls
    {
        float rate; // LFO frequency in Hz, range: [1, 20]
        float depth; // Effect depth, range: [0, 0.5]
        enum class shape_type {sine, triangle} shape; // LFO shape
    } ctrl;
};

struct echo_attributes
{
    struct controls
    {
        float blur; // Blur level (1-st order LP filter cutoff), range: [0, 1.0]
        float time; // Delay time, range: [0.05s, 1.0s]
        float feedback; // Feedback, range: [0.0, 0.9]
        enum class mode_type {delay, echo} mode; // Mode of effect
    } ctrl;
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
};

struct cabinet_sim_attributes
{
    struct controls
    {
        enum class resolution {standart = 1024, high = 2048} ir_res; // IR resolution in samples
        uint8_t ir_idx; // Currently selected IR index
        static constexpr inline std::array<const char *, 3> ir_names // List of available impulses
        {
            "Marshall 1960A 4x12",
            "Orange 2x12",
            "Catharsis Fredman"
        };
    } ctrl;
};

typedef std::variant
<
    tremolo_attributes,
    echo_attributes,
    overdrive_attributes,
    cabinet_sim_attributes
> effect_specific_attributes;

}

#endif /* MODEL_EFFECT_FEATURES_HPP_ */
