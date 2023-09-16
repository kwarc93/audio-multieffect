/*
 * config.hpp
 *
 *  Created on: 16 wrz 2023
 *      Author: kwarc
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstdint>
#include <vector>

namespace mfx::config
{

/* Buffer size of audio samples (directly affects in/out latency */
constexpr inline uint16_t dsp_vector_size {128};

/* Sampling frequency of audio signals */
constexpr inline uint32_t sampling_frequency_hz {48000};

}

#endif /* CONFIG_HPP_ */
