/*
 * data_types.hpp
 *
 *  Created on: 5 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_EQUALIZER_DATA_TYPES_HPP_
#define EFFECTS_EQUALIZER_DATA_TYPES_HPP_

namespace equalizer
{

/* 3-band equalizer */
struct controls_t
{
    float low;
    float mid;
    float hi;
};

struct state_t
{
    int error_code;
};

}

#endif /* EFFECTS_EQUALIZER_DATA_TYPES_HPP_ */
