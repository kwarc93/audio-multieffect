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
    chorus,
    reverb,
    overdrive,
    cabinet_sim,

    _count // Indicates total number of effects
};

constexpr inline std::array<const char*, static_cast<uint8_t>(effect_id::_count)> effect_name
{{
    "Tremolo",
    "Echo",
    "Chorus",
    "Reverb",
    "Overdrive",
    "Cabinet simulator"
}};

//-----------------------------------------------------------------------------
/* Base effect attributes */

struct effect_attr
{
    const effect_id id; // Effect unique ID, see above
    const char *name; // Effect unique name, see above
    bool bypassed; // Bypass status: true - effect inactive, false - effect active
    int status; // Status/error code
};

//-----------------------------------------------------------------------------
/* Effect specific attributes */

struct tremolo_attr
{
    struct controls
    {
        float rate; // LFO frequency in Hz, range: [1, 20]
        float depth; // Effect depth, range: [0, 0.5]
        enum class shape_type {sine, square} shape; // LFO shape
    } ctrl;
};

struct echo_attr
{
    struct controls
    {
        float blur; // Blur level (1-st order LP filter cutoff), range: [0, 1.0]
        float time; // Delay time, range: [0.05, 1.0]
        float feedback; // Feedback, range: [0, 0.9]
        enum class mode_type {delay, echo} mode; // Mode of effect
    } ctrl;
};

struct chorus_attr
{
    struct controls
    {
        float depth; // Depth level, range: [0, 1.0]
        float rate; // Modulation rate, range: [0, 1.0]
        float tone; // Tone, range: [0, 1.0]
        float mix; // Wet/dry mix, range: [0, 1.0]
        enum class mode_type {white, deep} mode; // Mode of effect
    } ctrl;
};

struct reverb_attr
{
    struct controls
    {
        float bandwidth; // Input LPF, range: [0, 1.0]
        float damping; // Tank LPFs rate, range: [0, 1.0]
        float decay; // Reverb time, range: [0, 0.99]
        enum class mode_type {plate, mod} mode; // Mode of effect
    } ctrl;
};

struct overdrive_attr
{
    struct controls
    {
        float low; // High pass filter cutoff, range [0, 1.0]
        float gain; // Gain, range: [1, 200]
        float high; // Low pass filter cutoff, range [0, 1.0]
        float mix; // Wet/dry mix, range: [0, 1.0]
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
    chorus_attr,
    reverb_attr,
    overdrive_attr,
    cabinet_sim_attr
> effect_specific_attributes;

}

#endif /* MODEL_EFFECT_FEATURES_HPP_ */
