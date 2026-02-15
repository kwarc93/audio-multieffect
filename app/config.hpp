/*
 * config.hpp
 *
 *  Created on: 16 wrz 2023
 *      Author: kwarc
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstdint>

namespace mfx::config
{

/* Buffer size of audio samples (directly affects in/out latency) */
constexpr inline uint16_t dsp_vector_size {128};

/* Sampling frequency of audio signals */
constexpr inline uint32_t sampling_frequency_hz {48000 + CFG_FS_CALIB};

/* String containing HW-related info */
#ifdef DUAL_CORE_APP
constexpr inline const char * hw_info_txt {"Board: STM32H745I-DISCO DKH745IO$AT2\nCPU1: ARM Cortex-M7 400MHz\nCPU2: ARM Cortex-M4 200MHz\nRAM: 8MB\nStorage: 64MB\nAudio: 24bit/48kHz\nDisplay: 480x272 RGB565"};
#else
constexpr inline const char * hw_info_txt {"Board: STM32F746G-DISCO MB1191B\nCPU: ARM Cortex-M7 200MHz\nRAM: 8MB\nStorage: 16MB\nAudio: 24bit/48kHz\nDisplay: 480x272 RGB565"};
#endif

}

#endif /* CONFIG_HPP_ */
