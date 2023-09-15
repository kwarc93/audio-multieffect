/*
 * overdrive.hpp
 *
 *  Created on: 30 sie 2023
 *      Author: kwarc
 */

#ifndef MODEL_OVERDRIVE_OVERDRIVE_HPP_
#define MODEL_OVERDRIVE_OVERDRIVE_HPP_

#include "app/model/effect_interface.hpp"

#include <array>

#include <libs/audio_dsp.hpp>

namespace mfx
{

class overdrive : public effect
{
public:
    overdrive(float low = 0.5f, float high = 0.5f, float gain = 40.0f, float mix = 0.5f, overdrive_attributes::controls::mode_type mode = overdrive_attributes::controls::mode_type::soft);
    virtual ~overdrive();

    void process(const dsp_input_t &in, dsp_output_t &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_high(float high);
    void set_gain(float gain);
    void set_low(float low);
    void set_mix(float mix);
    void set_mode(overdrive_attributes::controls::mode_type mode);

private:
    float soft_clip(float in);
    float hard_clip(float in);

    /* Low-pass FIR filter for anti-aliasing */
    constexpr static inline std::array<float, 97> fir_coeffs
    {{
        -0.00007726, -0.00054562, -0.00006974, 0.00060528, 0.00026125,
        -0.00066688, -0.00053010, 0.00069328, 0.00089668, -0.00062799,
        -0.00135850, 0.00040278, 0.00188195, 0.00005076, -0.00239801,
        -0.00078692, 0.00280298, 0.00183069, -0.00296491, -0.00316375,
        0.00273538, 0.00471352, -0.00196578, -0.00634692, 0.00052614,
        0.00786968, 0.00167556, -0.00903115, -0.00467613, 0.00953286,
        0.00844433, -0.00903749, -0.01287456, 0.00717167, 0.01778817,
        -0.00350995, -0.02294301, -0.00248744, 0.02805055, 0.01165379,
        -0.03279911, -0.02563864, 0.03688079, 0.04871686, -0.04001944,
        -0.09684767, 0.04199643, 0.31535539, 0.45766032, 0.31535539,
        0.04199643, -0.09684767, -0.04001944, 0.04871686, 0.03688079,
        -0.02563864, -0.03279911, 0.01165379, 0.02805055, -0.00248744,
        -0.02294301, -0.00350995, 0.01778817, 0.00717167, -0.01287456,
        -0.00903749, 0.00844433, 0.00953286, -0.00467613, -0.00903115,
        0.00167556, 0.00786968, 0.00052614, -0.00634692, -0.00196578,
        0.00471352, 0.00273538, -0.00316375, -0.00296491, 0.00183069,
        0.00280298, -0.00078692, -0.00239801, 0.00005076, 0.00188195,
        0.00040278, -0.00135850, -0.00062799, 0.00089668, 0.00069328,
        -0.00053010, -0.00066688, 0.00026125, 0.00060528, -0.00006974,
        -0.00054562, -0.00007726
    }};

    libs::adsp::fir<fir_coeffs.size(), fir_coeffs, dsp_vector_size> fir_lp;

    /* Tunable high-pass 2nd order IIR filter */
    libs::adsp::iir_highpass iir_hp;

    /* Tunable low-pass 2-nd order IIR filter */
    libs::adsp::iir_lowpass iir_lp;

    overdrive_attributes attributes;
};

}


#endif /* MODEL_OVERDRIVE_OVERDRIVE_HPP_ */
