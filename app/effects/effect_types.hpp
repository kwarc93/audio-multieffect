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

using input_t = std::vector<uint32_t>;
using output_t = std::vector<uint32_t>;

#endif /* EFFECTS_EFFECT_TYPES_HPP_ */
