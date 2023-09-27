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

namespace mfx
{

//-----------------------------------------------------------------------------
/* Available effects & their names */
enum class effect_id : uint8_t
{
    tremolo,
    echo,
    overdrive,
    cabinet_sim,

    _count // Indicates total number of effects
};

constexpr inline std::array<const char*, static_cast<uint8_t>(effect_id::_count)> effect_name
{{
    "Tremolo",
    "Echo",
    "Overdrive",
    "Cabinet simulator"
}};

//-----------------------------------------------------------------------------
/* Base effect attributes */

struct effect_attr
{
    const effect_id id;
    const char *name;
    bool bypassed;
    int status;
};

//-----------------------------------------------------------------------------
/* Effect specific attributes */

struct tremolo_attr
{
    struct controls
    {
        float rate; // LFO frequency in Hz, range: [1, 20]
        float depth; // Effect depth, range: [0, 0.5]
        enum class shape_type {sine, triangle} shape; // LFO shape
    } ctrl;
};

struct echo_attr
{
    struct controls
    {
        float blur; // Blur level (1-st order LP filter cutoff), range: [0, 1.0]
        float time; // Delay time, range: [0.05s, 1.0s]
        float feedback; // Feedback, range: [0.0, 0.9]
        enum class mode_type {delay, echo} mode; // Mode of effect
    } ctrl;
};

struct overdrive_attr
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

struct cabinet_sim_attr
{
    struct controls
    {
        enum class resolution {standart = 1024, high = 2048} ir_res; // IR resolution in samples
        uint8_t ir_idx; // Currently selected IR index
        std::array<const char *, 3> ir_names; // List of available impulses
    } ctrl;
};

typedef std::variant
<
    tremolo_attr,
    echo_attr,
    overdrive_attr,
    cabinet_sim_attr
> effect_specific_attributes;

}

#endif /* MODEL_EFFECT_FEATURES_HPP_ */
