/*
 * nam.cpp
 *
 *  Created on: 1 lip 2026
 *      Author: kwarc
 */

#include "nam.hpp"
#include "test_models.hpp"

#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

}

//-----------------------------------------------------------------------------
/* private */

bool neural_amp_modeler::prewarm(int samples_to_prewarm)
{
    if (this->prewarmed_samples == samples_to_prewarm)
        return true;

    // Feed silence through the model until the receptive field is full

    float zero_buf[NAM_MAX_BUFFER_SIZE];
    float out_buf[NAM_MAX_BUFFER_SIZE];
    memset(zero_buf, 0, sizeof(zero_buf));

    float *in_ptrs[NAM_IN_CHANNELS];
    float *out_ptrs[NAM_OUT_CHANNELS];
    for (int i = 0; i < NAM_IN_CHANNELS; i++)
        in_ptrs[i] = zero_buf;
    for (int i = 0; i < NAM_OUT_CHANNELS; i++)
        out_ptrs[i] = out_buf;

    if (this->prewarmed_samples < samples_to_prewarm)
    {
        int n = NAM_MAX_BUFFER_SIZE;
        if (n > samples_to_prewarm - this->prewarmed_samples)
            n = samples_to_prewarm - this->prewarmed_samples;
        nam_process(&this->nam_state, (const float* const*) in_ptrs, out_ptrs, n);
        this->prewarmed_samples += n;
    }

    return this->prewarmed_samples == samples_to_prewarm;
}

bool neural_amp_modeler::load_model(const uint8_t* model, size_t size)
{
    // .namb header (32 bytes):
    //   u32 magic, u16 version, u16 flags, u32 total_size,
    //   u32 weights_offset, u32 num_weights, u32 model_block_size, u32 checksum
    uint32_t magic;
    memcpy(&magic, model, 4);
    if(magic != namb_magic)
    {
        printf("  bad magic: 0x%08lX", (unsigned long)magic);
        return false;
    }

    uint32_t weights_offset, num_weights;
    memcpy(&weights_offset, model + 12, 4);
    memcpy(&num_weights, model + 16, 4);

    printf("  weights: offset=%lu count=%lu",
                      (unsigned long)weights_offset,
                      (unsigned long)num_weights);

    if(weights_offset + num_weights * 4 > size)
    {
        printf("  weights extend beyond file");
        return false;
    }

    nam_init(&nam_state);

    const float* weights
        = reinterpret_cast<const float*>(model + weights_offset);
    int rc = nam_load_weights(weights, (int)num_weights);
    if(rc != 0)
    {
        printf("  nam_load_weights failed (not an A2-lite model?)");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/* public */

neural_amp_modeler::neural_amp_modeler() : effect { effect_id::neural_amp_modeler },
attr {}
{
    const auto& def = neural_amp_modeler_attr::default_ctrl;

    this->set_input_volume(def.in_vol);
    this->set_output_volume(def.out_vol);

    // TODO: Move loading models to separate method
    this->prewarmed_samples = 0;
    this->model_ready = false;
    this->load_model(soldano_slo100_ovd_sm57.data(), soldano_slo100_ovd_sm57.size());
}

neural_amp_modeler::~neural_amp_modeler()
{

}

void neural_amp_modeler::process(const dsp_input& in, dsp_output& out)
{

    this->model_ready = this->prewarm(prewarm_samples);

    if (this->model_ready)
    {
        float *in_ptrs[NAM_IN_CHANNELS];
        float *out_ptrs[NAM_OUT_CHANNELS];

        in_ptrs[0] = const_cast<float*>(in.data());
        out_ptrs[0] = out.data();
        nam_process(&this->nam_state, static_cast<const float* const*>(in_ptrs), out_ptrs, mfx::config::dsp_buffer_size / 2);

        in_ptrs[0] = const_cast<float*>(in.data() + mfx::config::dsp_buffer_size / 2);
        out_ptrs[0] = out.data() + mfx::config::dsp_buffer_size / 2;
        nam_process(&this->nam_state, static_cast<const float* const*>(in_ptrs), out_ptrs, mfx::config::dsp_buffer_size / 2);
    }
    else
    {
        out = in;
    }
}

const effect_specific_attr neural_amp_modeler::get_specific_attributes(void) const
{
    return this->attr;
}

void neural_amp_modeler::set_input_volume(float vol)
{
    vol = std::clamp(vol, 0.0f, 1.0f);

    if (this->attr.ctrl.in_vol == vol)
        return;

    this->attr.ctrl.in_vol = vol;
}

void neural_amp_modeler::set_output_volume(float vol)
{
    vol = std::clamp(vol, 0.0f, 1.0f);

    if (this->attr.ctrl.out_vol == vol)
        return;

    this->attr.ctrl.out_vol = vol;
}

