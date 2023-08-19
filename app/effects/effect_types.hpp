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

static constexpr uint16_t input_samples = 256;
static constexpr uint16_t output_samples = input_samples;

using input_t = std::array<float, input_samples/2>;
using output_t = std::array<float, output_samples/2>;

#endif /* EFFECTS_EFFECT_TYPES_HPP_ */
