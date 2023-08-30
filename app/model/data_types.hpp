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

static constexpr uint32_t sampling_frequency_hz = 48000;

enum class effect_id { equalizer, noise_gate, tremolo, echo, overdrive };

typedef float dsp_sample_t;
typedef std::vector<dsp_sample_t> dsp_input_t;
typedef std::vector<dsp_sample_t> dsp_output_t;

}

#endif /* MODEL_DATA_TYPES_HPP_ */
