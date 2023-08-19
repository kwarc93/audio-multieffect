/*
 * audio_buffer.hpp
 *
 *  Created on: 19 sie 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_AUDIO_BUFFER_HPP_
#define EFFECTS_AUDIO_BUFFER_HPP_

#include <cstdint>
#include <array>

template <typename T1, typename T2>
struct audio_buffer
{
    using raw_sample_t = T1;
    using dsp_sample_t = T2;

    static constexpr uint16_t samples = 256;
    alignas(32) std::array<T1, samples> raw;
    volatile uint16_t raw_idx;
    std::array<T2, samples/2> dsp;
};

#endif /* EFFECTS_AUDIO_BUFFER_HPP_ */
