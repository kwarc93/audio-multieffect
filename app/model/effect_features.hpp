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
    tuner,
    tremolo,
    echo,
    chorus,
    reverb,
    overdrive,
    cabinet_sim,
    vocoder,
    phaser,
    amplifier_sim,

    _count // Indicates total number of effects
};

constexpr inline std::array<const char*, static_cast<uint8_t>(effect_id::_count)> effect_name
{{
    "Tuner",
    "Tremolo",
    "Echo",
    "Chorus",
    "Reverb",
    "Overdrive",
    "Cabinet simulator",
    "Vocoder",
    "Phaser",
    "Amplifier simulator"
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

struct tuner_attr
{
    struct controls
    {
        bool mute; // Mute tuning mode: true - enabled, false - disabled
        unsigned a4_tuning; // Reference frequency for A4 in Hz, range: [410, 480]
        //enum class input_source {jack, mic} input; // Input source
    } ctrl;

    static constexpr controls default_ctrl
    {
        false, // mute
        440 // a4_tuning
    };

    struct outputs
    {
        float pitch; // Detected pitch in Hz
        char note; // Detected note (uppercase means sharp: A -> A#)
        uint8_t octave; // Detected octave, range: [0, 8]
        int8_t cents; // Cents deviation from the detected note, range: [-50, 50]
    } out;
};

struct tremolo_attr
{
    struct controls
    {
        float rate; // LFO frequency in Hz, range: [1, 20]
        float depth; // Effect depth, range: [0, 0.5]
        enum class shape_type {sine, square} shape; // LFO shape
    } ctrl;

    static constexpr controls default_ctrl
    {
        8.0f, // rate
        0.3f, // depth
        controls::shape_type::sine // shape
    };
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

    static constexpr controls default_ctrl
    {
        0.5f, // blur
        0.3f, // time
        0.6f, // feedback
        controls::mode_type::echo // mode
    };
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

    static constexpr controls default_ctrl
    {
        0.4f, // depth
        0.3f, // rate
        0.5f, // tone
        0.5f, // mix
        controls::mode_type::white // mode
    };
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

    static constexpr controls default_ctrl
    {
        0.9995f, // bandwidth
        0.0005f, // damping
        0.5f, // decay
        controls::mode_type::plate // mode
    };
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

    static constexpr controls default_ctrl
    {
        0.5f, // low
        80.0f, // gain
        0.5f, // high
        0.5f, // mix
        controls::mode_type::soft // mode
    };
};

struct cabinet_sim_attr
{
    struct controls
    {
        uint8_t ir_idx; // Currently selected IR index
        enum class resolution {standart = 1024, high = 2048} ir_res; // IR resolution in samples
    } ctrl;

    static constexpr controls default_ctrl
    {
        0, // ir_idx
        controls::resolution::standart // ir_res
    };

    std::array<const char *, 3> ir_names; // List of available impulses
};

struct vocoder_attr
{
    struct controls
    {
        unsigned bands; // Number of bandpass channels, range: 12 for vintage, 8-256 for modern
        float clarity; // Speech clarity, range [0, 1.0]
        float tone; // Tone (HP filter cutoff), range: [0, 1.0]
        bool hold; // Holds modulator envelope, true/false
        enum class mode_type {vintage, modern} mode; // Vocoder type, IIR bandpass filters or FFT
    } ctrl;

    static constexpr controls default_ctrl
    {
        64, // bands
        0.8f, // clarity
        0.2f, // tone
        false, // hold
        controls::mode_type::modern // mode
    };

    std::array<unsigned, 8> bands_list; // List of available bands
};

struct phaser_attr
{
    struct controls
    {
        float rate; // LFO frequency in Hz/10, range: [0.01, 1]
        float depth; // LFO modulation depth, range: [0, 1]
        enum class contour_mode {off, on} contour; // Contour enabled/disabled
    } ctrl;

    static constexpr controls default_ctrl
    {
        0.25f, // rate
        0.7f, // depth
        controls::contour_mode::on // contour
    };
};

struct amp_sim_attr
{
    struct controls
    {
        float input; // Input volume, range: [0, 1]
        float drive; // Preamp drive, range: [0, 1]
        float compression; // Tube compression, range: [0, 1]
        float bass; // Tone stack: bass, range [0, 1]
        float mids; // Tone stack: mids, range [0, 1]
        float treb; // Tone stack: treble, range [0, 1]
        enum class mode_type {logain, higain} mode; // Amp overall gain: low/high
    } ctrl;

    static constexpr controls default_ctrl
    {
        0.8f, // input volume
        0.8f, // preamp drive
        0.7f, // compression
        0.5f, // bass
        0.5f, // mids
        0.5f, // treble
        controls::mode_type::higain // mode
    };
};

typedef std::variant
<
    tuner_attr,
    tremolo_attr,
    echo_attr,
    chorus_attr,
    reverb_attr,
    overdrive_attr,
    cabinet_sim_attr,
    vocoder_attr,
    phaser_attr,
    amp_sim_attr
> effect_specific_attr;

typedef std::variant
<
    tuner_attr::controls,
    tremolo_attr::controls,
    echo_attr::controls,
    chorus_attr::controls,
    reverb_attr::controls,
    overdrive_attr::controls,
    cabinet_sim_attr::controls,
    vocoder_attr::controls,
    phaser_attr::controls,
    amp_sim_attr::controls
> effect_controls;

}

#endif /* MODEL_EFFECT_FEATURES_HPP_ */
