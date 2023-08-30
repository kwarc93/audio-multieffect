/*
 * overdrive.cpp
 *
 *  Created on: 30 sie 2023
 *      Author: kwarc
 */

#include "overdrive.hpp"

#include <array>
#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

//-----------------------------------------------------------------------------
/* private */

//-----------------------------------------------------------------------------
/* public */


overdrive::overdrive(float high, float low, float gain, float mix, mode_type mode) : effect { effect_id::overdrive, "overdrive" }
{
    this->set_mode(mode);
    this->set_gain(gain);
    this->set_low(low);
    this->set_high(high);

    arm_fir_init_f32(
        &this->fir,
        this->fir_coeffs.size(),
        const_cast<float32_t*>(this->fir_coeffs.data()),
        this->fir_state.data(), this->fir_block_size
    );
    /* TODO */
}

overdrive::~overdrive()
{

}

void overdrive::process(const dsp_input_t& in, dsp_output_t& out)
{
    std::transform(in.begin(), in.end(), out.begin(),
    [this](auto input)
    {
        /* TODO */
        return input;
    }
    );
}

void overdrive::set_high(float high)
{
    if (this->high == high)
        return;

    /* TODO */
}

void overdrive::set_low(float low)
{
    if (this->low == low)
        return;

    /* TODO */
}

void overdrive::set_gain(float gain)
{
    if (this->gain == gain)
        return;

    /* TODO */
}

void overdrive::set_mix(float mix)
{
    if (this->mix == mix)
        return;

    /* TODO */
}

void overdrive::set_mode(mode_type mode)
{
    if (this->mode == mode)
        return;

    /* TODO */

    this->mode = mode;
}


