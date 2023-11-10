/*
 * reverb.hpp
 *
 *  Created on: 8 lis 2023
 *      Author: kwarc
 */

#ifndef MODEL_REVERB_REVERB_HPP_
#define MODEL_REVERB_REVERB_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

namespace mfx
{

class reverb : public effect
{
public:
    reverb(float bandwidth = 0.9995f, float damping = 0.0005f, float decay = 0.5f);
    virtual ~reverb();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_bandwidth(float bw);
    void set_damping(float d);
    void set_decay(float decay);
    void set_mode(reverb_attr::controls::mode_type mode);

private:
    class allpass
    {
    public:
        allpass(float delay, float gain) : g{gain}, del{delay, config::sampling_frequency_hz}
        {
            this->del.set_delay(delay);
        }

        float process(float in)
        {
            const float d = this->del.get();
            const float h = in - this->g * d;
            this->del.put(h);
            return d + this->g * h;
        }
        float at(float delay)
        {
            return this->del.at(delay);
        }
    private:
        float g;
        libs::adsp::delay_line<libs::adsp::delay_line_intrpl::none> del;
    };

    class mod_allpass
    {
    public:
        mod_allpass(float delay, float depth, float freq, float gain) : del_tap{delay}, del_depth{depth}, g{gain}, osc{libs::adsp::oscillator::shape::sine, config::sampling_frequency_hz}, del{delay + 2 * depth, config::sampling_frequency_hz}
        {
            this->osc.set_frequency(freq);
            this->del.set_delay(delay);
        }
        float process(float in)
        {
            this->del.set_delay(this->del_tap + this->osc.generate() * this->del_depth);
            const float d = this->del.get();
            const float h = in + this->g * d;
            this->del.put(h);
            return d - this->g * h;
        }
    private:
        const float del_tap;
        const float del_depth;
        float g;
        libs::adsp::oscillator osc;
        libs::adsp::delay_line<libs::adsp::delay_line_intrpl::allpass> del;
    };

    libs::adsp::delay_line<libs::adsp::delay_line_intrpl::none> pdel, del1, del2, del3, del4;
    libs::adsp::basic_iir<libs::adsp::basic_iir_type::lowpass> lpf1, lpf2, lpf3;
    allpass apf1, apf2, apf3, apf4, apf5, apf6;
    mod_allpass mapf1, mapf2;

    reverb_attr attr;
};

}

#endif /* MODEL_REVERB_REVERB_HPP_ */
