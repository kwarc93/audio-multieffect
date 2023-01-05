/*
 * data_types.hpp
 *
 *  Created on: 6 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_REVERB_DATA_TYPES_HPP_
#define EFFECTS_REVERB_DATA_TYPES_HPP_

namespace reverb
{

struct controls_t
{
    float decay_time;
    float pre_delay;
    float size;
    float dmping;
};

struct state_t
{
    int error_code;
};

}


#endif /* EFFECTS_REVERB_DATA_TYPES_HPP_ */
