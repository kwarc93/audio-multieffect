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
    reverb(float bandwidth = 0.99f, float damping = 0.3f, float decay = 0.86f);
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
        allpass(float delay, float gain) : h{0}, g{gain}, del{delay, config::sampling_frequency_hz} {}
        float process(float in)
        {
            const float d = this->del.get();
            this->h = in - this->g * d;
            this->del.put(this->h);
            return d + this->g * this->h;
        }
        float at(float delay)
        {
            return this->del.at(delay);
        }
    private:
        float h;
        float g;
        libs::adsp::delay_line<libs::adsp::delay_line_intrpl::none> del;
    };

    class mod_allpass
    {
    public:
        mod_allpass(float delay, float depth, float gain) : del_tap{delay - depth}, del_depth{depth}, h{0}, g{gain}, osc{libs::adsp::oscillator::shape::sine, config::sampling_frequency_hz}, del{delay, config::sampling_frequency_hz}
        {
            this->osc.set_frequency(1.0f);
        }
        float process(float in)
        {
            this->del.set_delay(this->del_tap + this->osc.generate() * this->del_depth);
            const float d = this->del.get();
            this->h = in - this->g * d;
            this->del.put(this->h);
            return d + this->g * this->h;
        }
    private:
        const float del_tap;
        const float del_depth;
        float h;
        float g;
        libs::adsp::oscillator osc;
        libs::adsp::delay_line<libs::adsp::delay_line_intrpl::allpass> del;
    };

    libs::adsp::delay_line<libs::adsp::delay_line_intrpl::none> pdel, del1, del2, del3, del4;
    libs::adsp::iir_lowpass lpf1, lpf2, lpf3;
    allpass apf1, apf2, apf3, apf4, apf5, apf6;
    mod_allpass mapf1, mapf2;

    reverb_attr attr;
};

}

#endif /* MODEL_REVERB_REVERB_HPP_ */
