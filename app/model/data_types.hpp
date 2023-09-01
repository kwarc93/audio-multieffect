/*
 * data_types.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EFFECT_TYPES_HPP_
#define MODEL_EFFECT_TYPES_HPP_

#include <vector>

namespace mfx
{

constexpr inline uint16_t dsp_vector_size {128};
constexpr inline uint32_t sampling_frequency_hz {48000};
constexpr inline float pi {3.14159265359f};

enum class effect_id { equalizer, noise_gate, tremolo, echo, overdrive, cabinet_sim };

typedef float dsp_sample_t;
typedef std::vector<dsp_sample_t> dsp_input_t;
typedef std::vector<dsp_sample_t> dsp_output_t;

}

#endif /* MODEL_DATA_TYPES_HPP_ */
