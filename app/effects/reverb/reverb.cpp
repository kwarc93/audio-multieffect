/*
 * reverb.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "app/effects/reverb/reverb.hpp"

//-----------------------------------------------------------------------------
/* public */

reverb::reverb() : effect { effect_id::reverb, "reverb" }
{

}

reverb::~reverb()
{

}

void reverb::process(const input_t& in, output_t& out)
{
    printf("Effect '%s' (id:%u) processing data\n", this->name.data(), static_cast<unsigned>(this->id));
}

//-----------------------------------------------------------------------------
/* private */


