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

#include <cmsis/stm32f7xx.h>
#include <cmsis/dsp/arm_math.h>

namespace mfx
{

class overdrive : public effect
{
public:
    enum class mode_type {distortion, overdrive};

    struct controls
    {
        float high;
        float low;
        float gain;
        float mix;
        mode_type mode;
    };

    struct state
    {
        int error_code;
    };

    overdrive(float low = 0.5f, float high = 0.3f, float gain = 4.0f, float mix = 1.0f, mode_type mode = mode_type::overdrive);
    virtual ~overdrive();

    void process(const dsp_input_t &in, dsp_output_t &out) override;

    void set_high(float high);
    void set_low(float low);
    void set_gain(float gain);
    void set_mix(float mix);
    void set_mode(mode_type mode);

private:

    dsp_sample_t soft_clip(dsp_sample_t in);
    dsp_sample_t hard_clip(dsp_sample_t in);

    float low;
    float high;
    float gain;
    float mix;
    mode_type mode;

    /* FIR */
    constexpr static unsigned fir_block_size {128};
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

    arm_fir_instance_f32 fir;
    std::array<float, fir_block_size + fir_coeffs.size() - 1> fir_state;

    /* Tunable high-pass 1-st order IIR filter */
    float hp_c;
    dsp_sample_t hp_h;

    /* Tunable high-shelf 1-st order IIR filter */
    float hs_cc;
    float hs_h0;
    dsp_sample_t hs_h;

};

}


#endif /* MODEL_OVERDRIVE_OVERDRIVE_HPP_ */
