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
    overdrive(float low = 0.5f, float high = 0.5f, float gain = 40.0f, float mix = 0.5f, overdrive_attr::controls::mode_type mode = overdrive_attr::controls::mode_type::soft);
    virtual ~overdrive();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_high(float high);
    void set_gain(float gain);
    void set_low(float low);
    void set_mix(float mix);
    void set_mode(overdrive_attr::controls::mode_type mode);

private:
    float soft_clip(float in);
    float hard_clip(float in);

    /* Interpolator & decimator */
    constexpr static uint32_t oversampling_factor = 4;
    constexpr static std::array<float, 33> intrpl_x4_fir_coeffs{0.00000000,-0.00455932,-0.00677751,-0.00517776,-0.00000000,0.02578440,0.03945777,0.03118661,0.00000000,-0.08770110,-0.14265809,-0.12204653,-0.00000000,0.29100579,0.60983636,0.87130542,1.00000000,0.87130542,0.60983636,0.29100579,-0.00000000,-0.12204653,-0.14265809,-0.08770110,0.00000000,0.03118661,0.03945777,0.02578440,-0.00000000,-0.00517776,-0.00677751,-0.00455932,0.00000000};
    constexpr static std::array<float, 31> decim_x4_fir_coeffs{-0.00120388,-0.00205336,-0.00207963,0.00000000,0.00476490,0.00989603,0.00997846,-0.00000000,-0.01896379,-0.03629332,-0.03476203,0.00000000,0.06863228,0.15326576,0.22345846,0.25072021,0.22345846,0.15326576,0.06863228,0.00000000,-0.03476203,-0.03629332,-0.01896379,-0.00000000,0.00997846,0.00989603,0.00476490,0.00000000,-0.00207963,-0.00205336,-0.00120388};
//    constexpr static inline std::array<float, 17> intrpl_x2_fir_coeffs{-0.00000000,-0.00677751,0.00000000,0.03945777,-0.00000000,-0.14265809,0.00000000,0.60983636,1.00000000,0.60983636,0.00000000,-0.14265809,-0.00000000,0.03945777,0.00000000,-0.00677751,-0.00000000};
//    constexpr static inline std::array<float, 31> decim_x2_fir_coeffs{-0.00170040,0.00000000,0.00293733,-0.00000000,-0.00673009,0.00000000,0.01409389,-0.00000000,-0.02678504,0.00000000,0.04909896,-0.00000000,-0.09693833,0.00000000,0.31561956,0.50080823,0.31561956,0.00000000,-0.09693833,-0.00000000,0.04909896,0.00000000,-0.02678504,-0.00000000,0.01409389,0.00000000,-0.00673009,-0.00000000,0.00293733,0.00000000,-0.00170040};
    std::array<float, ((intrpl_x4_fir_coeffs.size() - 1) / oversampling_factor) + config::dsp_vector_size - 1> intrpl_state;
    std::array<float, decim_x4_fir_coeffs.size() + (oversampling_factor * config::dsp_vector_size) - 1> decim_state;
    std::array<float, oversampling_factor * config::dsp_vector_size> os_buffer;
    arm_fir_interpolate_instance_f32 intrpl;
    arm_fir_decimate_instance_f32 decim;

    /* Tunable high-pass 2nd order IIR filter */
    libs::adsp::iir_highpass iir_hp;

    /* Tunable low-pass 2-nd order IIR filter */
    libs::adsp::iir_lowpass iir_lp;

    overdrive_attr attr;
};

}


#endif /* MODEL_OVERDRIVE_OVERDRIVE_HPP_ */
