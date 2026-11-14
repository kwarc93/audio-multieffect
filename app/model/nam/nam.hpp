/*
 * nam.hpp
 *
 *  Created on: 1 lip 2026
 *      Author: kwarc
 */

#ifndef MODEL_NAM_NAM_HPP_
#define MODEL_NAM_NAM_HPP_

#include "app/model/effect_interface.hpp"

#include <libs/audio_dsp.hpp>

extern "C"
{
#include "nam_a2lite.h"
}

namespace mfx
{

class neural_amp_modeler : public effect
{
public:
    neural_amp_modeler();
    virtual ~neural_amp_modeler();

    void process(const dsp_input &in, dsp_output &out) override;
    const effect_specific_attr get_specific_attributes(void) const override;

    void set_model(uint8_t idx);
    void set_input_volume(float vol);
    void set_output_volume(float vol);
private:

    bool prewarm(int samples_to_prewarm);
    bool load_model(const uint8_t* model, size_t size);

    inline static nam_a2lite_t nam FAST_RAM {};
    inline static nam_a2lite_weights_t nam_weights FAST_RAM {};
    nam_a2lite_state_t nam_state {};

    bool model_ready {false};
    int prewarmed_samples {0};
    float out_gain {1};

    neural_amp_modeler_attr attr {0};
};

}

#endif /* MODEL_NAM_NAM_HPP_ */
