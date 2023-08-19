/*
 * effect_types.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_EFFECT_TYPES_HPP_
#define EFFECTS_EFFECT_TYPES_HPP_

#include <array>

enum class effect_id { equalizer, reverb, compressor };

template <typename T1, typename T2>
struct audio_buffer
{
    static constexpr uint16_t samples = 256;

    using raw_sample_t = T1;
    using dsp_sample_t = T2;
    using raw_buf_t = std::array<T1, samples>; // One buffer acts as two for double-buffering
    using dsp_buf_t = std::array<T2, samples/2>;

    volatile uint16_t raw_idx;
    alignas(32) raw_buf_t raw; // Alignment at 32-byte boundary needed for DMA & CPU D-Cache
    dsp_buf_t dsp;
};

typedef audio_buffer<int16_t, float>::dsp_buf_t dsp_input_t;
typedef audio_buffer<int16_t, float>::dsp_buf_t dsp_output_t;

typedef audio_buffer<int16_t, float> input_buffer_t;
typedef audio_buffer<int16_t, float> output_buffer_t;

#endif /* EFFECTS_EFFECT_TYPES_HPP_ */
