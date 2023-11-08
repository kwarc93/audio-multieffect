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
    reverb(float bandwidth = 0.4f, float damping = 0.3f, float decay = 0.7f);
    virtual ~reverb();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attributes get_specific_attributes(void) const override;

    void set_bandwidth(float bw);
    void set_damping(float d);
    void set_decay(float decay);
    void set_mode(reverb_attr::controls::mode_type mode);

private:
    reverb_attr attr;
};

}

#endif /* MODEL_REVERB_REVERB_HPP_ */
