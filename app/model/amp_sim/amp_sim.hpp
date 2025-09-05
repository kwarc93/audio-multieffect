/*
 * amp_sim.hpp
 *
 *  Created on: 5 sty 2025
 *      Author: kwarc
 */

#ifndef MODEL_AMP_SIM_AMP_SIM_HPP_
#define MODEL_AMP_SIM_AMP_SIM_HPP_


#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>
#include <libs/willpirkle/valves.h>

namespace mfx
{

class amp_sim : public effect
{
public:
    amp_sim();
    virtual ~amp_sim();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attr get_specific_attributes(void) const override;

    void set_input(float input);
    void set_drive(float drive);
    void set_compression(float compression);
    void set_mode(amp_sim_attr::controls::mode_type mode);

private:

    OneMarkAmp amp;
    OneMarkAmpParameters amp_params;

    amp_sim_attr attr {0};
};

}



#endif /* MODEL_AMP_SIM_AMP_SIM_HPP_ */
