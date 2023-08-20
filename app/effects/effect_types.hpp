/*
 * effect_types.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_EFFECT_TYPES_HPP_
#define EFFECTS_EFFECT_TYPES_HPP_

#include <vector>

enum class effect_id { equalizer, reverb, compressor };

typedef std::vector<float> dsp_input_t;
typedef std::vector<float> dsp_output_t;

#endif /* EFFECTS_EFFECT_TYPES_HPP_ */
