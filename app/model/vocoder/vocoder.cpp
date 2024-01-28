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

vocoder::vocoder(float clarity, bool hold) : effect {effect_id::vocoder}
{
#if 0
    this->highpass_filter.set_coeffs(this->highpass_coeffs);
#endif

    this->set_clarity(clarity);
    this->hold(hold);

//    constexpr uint8_t lpc_ord = 4;
//    std::array<float, 6> x {{1, 2, 3, 4, 5, 6}};
//    std::array<float, lpc_ord + 1> a;
//    float g;
//    this->lpc.process(x.data(), x.size(), lpc_ord, a.data(), &g);
}

vocoder::~vocoder()
{

}

void vocoder::process(const dsp_input& in, dsp_output& out)
{
    if (this->aux_in == nullptr)
        return;

#if 0 /* Channel vocoder */
    /* Add highpassed modulator to the output */
    this->highpass_filter.process(this->aux_in->data(), out.data(), out.size());
    for (auto &&s : out) s *= this->attr.ctrl.clarity;

    const auto bands = this->carrier_fb.bands;
    for (unsigned band = 0; band < bands; band++)
    {
        this->carrier_fb.bandpass(band, in.data(), this->carrier_bp_buf.data(), in.size());
        arm_mult_f32(this->carrier_bp_buf.data(), this->carrier_bp_buf.data(), this->carrier_env_buf.data(), this->carrier_env_buf.size());
        this->carrier_fb.lowpass(band, this->carrier_env_buf.data(), this->carrier_env_buf.data(), this->carrier_env_buf.size());

        if (!this->attr.ctrl.hold)
        {
            this->modulator_fb.bandpass(band, this->aux_in->data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());
            arm_mult_f32(this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());
            this->modulator_fb.lowpass(band, this->modulator_env_buf.data(), this->modulator_env_buf.data(), this->modulator_env_buf.size());
            arm_sqrt_f32(this->modulator_env_buf[0], &this->modulator_hold[band]);
        }

        for (unsigned i = 0; i < out.size(); i++)
        {
            arm_sqrt_f32((1 - this->attr.ctrl.clarity) + this->carrier_env_buf[i], &this->carrier_env_buf[i]);
            if (!this->attr.ctrl.hold)
                arm_sqrt_f32(this->modulator_env_buf[i], &this->modulator_env_buf[i]);
            else
                this->modulator_env_buf[i] = this->modulator_hold[band];

            out[i] += this->carrier_bp_buf[i] * this->modulator_env_buf[i] / this->carrier_env_buf[i];
        }
    }
#else /* LPC based vocoder */

    float *envelope = const_cast<float*>(this->aux_in->data());
    float *carrier = const_cast<float*>(in.data());
    constexpr uint32_t samples = config::dsp_vector_size;

    /* Window the signals */
    arm_mult_f32(envelope, const_cast<float*>(this->hanning.data()), envelope, samples);
    arm_mult_f32(carrier, const_cast<float*>(this->hanning.data()), carrier, samples);

    /* Calculate LPC */
    this->envelope_lpc.process(envelope, samples, this->envelope_lpc_ord, this->envelope_lpc_coeffs.data(), &this->envelope_lpc_gain);
    this->carrier_lpc.process(carrier, samples, this->carrier_lpc_ord, this->carrier_lpc_coeffs.data(), &this->carrier_lpc_gain);

    float *ae = this->envelope_lpc_coeffs.data() + 1;
    arm_negate_f32(ae, ae, this->envelope_lpc_ord);

    constexpr uint32_t hop_size = config::dsp_vector_size / 4;
    for (unsigned i = 0; i < hop_size; i++)
    {
        float e1;
        float g = 1.0f / this->carrier_lpc_gain;
        arm_scale_f32(this->carrier_lpc_coeffs.data(), g, this->carrier_lpc_coeffs.data(), this->carrier_lpc_coeffs.size());
        arm_dot_prod_f32(this->carrier_lpc_coeffs.data(), carrier + i, this->carrier_lpc_coeffs.size(), &e1);
        arm_dot_prod_f32(ae, out.data(), this->envelope_lpc_ord, &out[i]);
        out[i] += this->envelope_lpc_gain * e1;
    }

#endif
}

const effect_specific_attributes vocoder::get_specific_attributes(void) const
{
    return this->attr;
}

void vocoder::set_clarity(float clarity)
{
    clarity = std::clamp(clarity, 0.0f, 0.999f);

    if (this->attr.ctrl.clarity == clarity)
        return;

    this->attr.ctrl.clarity = clarity;
}

void vocoder::hold(bool state)
{
    if (this->attr.ctrl.hold == state)
        return;

    this->attr.ctrl.hold = state;
}

