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
#include <libs/nam-core/nam_model.h>
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

    void set_input_volume(float vol);
    void set_output_volume(float vol);
private:
    // A2-lite receptive field. Dilations × (kernel_size − 1) summed across layers:
    //   layers  0–13: kernel=6,  dilations [1,3,7,17,41,101,239, 1,3,7,17,41,101,239] → 4090
    //   layers 14–15: kernel=15, dilations [1,13]                                     →  196
    //   layers 16–22: kernel=6,  dilations [1,3,7,17,41,101,239]                      → 2045
    static constexpr unsigned prewarm_samples = 1 + 4090 + 196 + 2045;
    static constexpr size_t max_block_size = NAM_MAX_BUFFER_SIZE;
    // A2-lite has 1871 float weights → 7484 bytes payload + 32-byte .namb header.
    static constexpr size_t max_model_size = 8 * 1024;
    static constexpr uint32_t namb_magic = 0x4E414D42; // "NAMB"

    bool prewarm(int samples_to_prewarm);
    bool load_model(const uint8_t* model, size_t size);

    nam_state_t nam_state {0};
    bool model_ready {false};
    int prewarmed_samples {0};

    neural_amp_modeler_attr attr {0};
};

}

#endif /* MODEL_NAM_NAM_HPP_ */
