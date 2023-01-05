/*
 * data_types.hpp
 *
 *  Created on: 6 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_COMPRESSOR_DATA_TYPES_HPP_
#define EFFECTS_COMPRESSOR_DATA_TYPES_HPP_

namespace compressor
{

struct controls_t
{
    float threshold;
    float ratio;
    float attack;
    float release;
};

struct state_t
{
    int error_code;
};

}



#endif /* EFFECTS_COMPRESSOR_DATA_TYPES_HPP_ */
