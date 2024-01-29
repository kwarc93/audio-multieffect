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

    arm_fir_init_f32(&this->pred_filt, this->carrier_lpc_ord + 1, this->carrier_lpc_coeffs.data(), this->pred_filt_state.data(), 1);
    arm_fir_init_f32(&this->inv_filt, this->envelope_lpc_ord, this->envelope_lpc_coeffs.data() + 1, this->inv_filt_state.data(), 1);

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

    constexpr uint32_t hop_size = config::dsp_vector_size;
    const uint32_t move_size = this->envelope.size() - hop_size;

//    /* Sliding window of input blocks */
//    arm_copy_f32(this->envelope.data() + hop_size, this->envelope.data(), move_size);
//    arm_copy_f32(const_cast<float*>(this->aux_in->data()), this->envelope.data() + move_size, hop_size);
//    arm_copy_f32(this->carrier.data() + hop_size, this->carrier.data(), move_size);
//    arm_copy_f32(const_cast<float*>(in.data()), this->carrier.data() + move_size, hop_size);
//
//    /* Window the signals */
//    arm_mult_f32(this->envelope.data(), const_cast<float*>(this->hanning.data()), this->envelope.data(), this->envelope.size());
//    arm_mult_f32(this->carrier.data(), const_cast<float*>(this->hanning.data()), this->carrier.data(), this->carrier.size());

    /* Calculate LPC */
    this->envelope_lpc.process(this->envelope.data(), this->envelope.size()/8, this->envelope_lpc_ord, this->envelope_lpc_coeffs.data(), &this->envelope_lpc_gain);
    this->carrier_lpc.process(this->carrier.data(), this->carrier.size()/8, this->carrier_lpc_ord, this->carrier_lpc_coeffs.data(), &this->carrier_lpc_gain);

//    float *ae = this->envelope_lpc_coeffs.data() + 1;
//    arm_negate_f32(ae, ae, this->envelope_lpc_ord);
//
//    float g = 1.0f / this->carrier_lpc_gain;
//    arm_scale_f32(this->carrier_lpc_coeffs.data(), g, this->carrier_lpc_coeffs.data(), this->carrier_lpc_coeffs.size());
//
//    for (unsigned i = 0; i < hop_size; i++)
//    {
//        float e1, o;
//        arm_fir_f32(&this->pred_filt, &this->carrier[i], &e1, 1);
//        arm_fir_f32(&this->inv_filt, &this->out_save[i], &o, 1);
//        this->out_save[i] = o + this->envelope_lpc_gain * e1;
//    }
//
//    /* Copy result to output */
//    arm_copy_f32(this->out_save.data(), out.data(), hop_size);

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

