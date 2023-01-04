/*
 * equalizer_effect.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */


#include "equalizer_effect.hpp"

//-----------------------------------------------------------------------------
/* public */

equalizer::equalizer() : effect {effect_id::equalizer, "equalizer"}
{

}

equalizer::~equalizer()
{

}

void equalizer::process(const input_t& in, output_t& out)
{
    printf("Effect '%s'(%u) processing data\n", this->name.data(), static_cast<unsigned>(this->id));
}

//-----------------------------------------------------------------------------
/* private */

