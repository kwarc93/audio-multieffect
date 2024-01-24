/*
 * vocoder.cpp
 *
 *  Created on: 24 sty 2024
 *      Author: kwarc
 */

#include "vocoder.hpp"

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

vocoder::vocoder() : effect {effect_id::vocoder}
{

}

vocoder::~vocoder()
{

}

void vocoder::process(const dsp_input& in, dsp_output& out)
{
    if (this->aux_in == nullptr)
        return;

    arm_fill_f32(0, out.data(), out.size());

    const auto bands = this->source_fb.bands;
    for (unsigned band = 0; band < bands; band++)
    {
        /* Bandpass the source & filter signals */
        this->source_fb.bandpass(band, in.data(), this->source_bp_buf.data(), in.size());
        this->filter_fb.bandpass(band, this->aux_in->data(), this->filter_env_buf.data(), this->aux_in->size());

        /* Square the source & filter signals */
        arm_mult_f32(this->source_bp_buf.data(), this->source_bp_buf.data(), this->source_env_buf.data(), this->source_env_buf.size());
        arm_mult_f32(this->filter_env_buf.data(), this->filter_env_buf.data(), this->filter_env_buf.data(), this->filter_env_buf.size());

        /* Lowpass the squared source & filter signals */
        this->source_fb.lowpass(band, this->source_env_buf.data(), this->source_env_buf.data(), this->source_env_buf.size());
        this->filter_fb.lowpass(band, this->filter_env_buf.data(), this->filter_env_buf.data(), this->filter_env_buf.size());

        /* Square root the source & filter signals */
        constexpr float epsi = 0.00001f;
        for (unsigned i = 0; i < this->source_env_buf.size(); i++)
        {
            arm_sqrt_f32(epsi + this->source_env_buf[i], &this->source_env_buf[i]);
            arm_sqrt_f32(this->filter_env_buf[i], &this->filter_env_buf[i]);

            /* Calculate output value */
            out[i] = out[i] + this->source_bp_buf[i] * this->filter_env_buf[i] / this->source_env_buf[i];
        }
    }
}

const effect_specific_attributes vocoder::get_specific_attributes(void) const
{
    return this->attr;
}

