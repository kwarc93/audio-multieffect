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

template <typename T>
struct audio_buffer
{
    static constexpr uint16_t samples = 256;
    alignas(32) std::array<T, samples> raw_buf;
    volatile uint16_t current_sample_index;
};

#endif /* EFFECTS_AUDIO_BUFFER_HPP_ */
