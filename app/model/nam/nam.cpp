/*
 * nam.cpp
 *
 *  Created on: 1 lip 2026
 *      Author: kwarc
 */

#include "nam.hpp"
#include "nam_models.hpp"

#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr std::array<std::pair<const char*, const nam_a2_lite_t*>, 9> nam_map
{{
    { "Fender Pro Reverb 1967", &Fender_Pro_Reverb_1967 },
    { "Fender Twin Reverb 1965", &Fender_Twin_Reverb_1965 },
    { "Roland JC 120B Jazz Chorus", &Roland_JC_120B_Jazz_Chorus },
    { "Orange OTR 120 2x12", &Orange_OTR_120_2x12 },
    { "Vox AC30/4 1961 Fawn EF86", &Vox_AC30_4_1961_Fawn_EF86 },
    { "Marshall Super Lead 12000", &Marshall_Super_Lead_12_000 },
    { "Soldano SLO 100", &Soldano_SLO_100 },
    { "Peavey 5150", &Peavey_5150 },
    { "Mesa Dual Rectifier MW", &Mesa_Dual_Rectifier_MW }
}};

static_assert(nam_map.size() == neural_amp_modeler_attr{}.model_names.size());

// A2-lite receptive field. Dilations x (kernel_size - 1) summed across layers:
//   layers  0–13: kernel=6,  dilations [1,3,7,17,41,101,239, 1,3,7,17,41,101,239] -> 4090
//   layers 14–15: kernel=15, dilations [1,13]                                     ->  196
//   layers 16–22: kernel=6,  dilations [1,3,7,17,41,101,239]                      -> 2045
constexpr unsigned prewarm_samples = 1 + 4090 + 196 + 2045;

// A2-lite has 1871 float weights -> 7484 bytes payload + 32-byte .namb header.
constexpr size_t max_model_size = 8 * 1024;

// .namb magic number
constexpr uint32_t namb_magic = 0x4E414D42; // "NAMB"

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
    in_ptrs[0] = zero_buf;
    out_ptrs[0] = out_buf;

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
    if (magic != namb_magic)
    {
        // Bad magic number
        return false;
    }

    uint32_t weights_offset, num_weights;
    memcpy(&weights_offset, model + 12, 4);
    memcpy(&num_weights, model + 16, 4);
    if (weights_offset + num_weights * 4 > size)
    {
        // Weights extend beyond file
        return false;
    }

    nam_init(&nam_state);

    const float* weights = reinterpret_cast<const float*>(model + weights_offset);
    if (nam_load_weights(weights, (int)num_weights) != 0)
    {
        // nam_load_weights failed (not an A2-lite model?)
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

    this->attr.ctrl.model_idx = def.model_idx;
    this->set_input_volume(def.in_vol);
    this->set_output_volume(def.out_vol);

    for (unsigned i = 0; i < nam_map.size(); i++)
        this->attr.model_names.at(i) = (nam_map.at(i).first);

    this->model_ready = false;
    this->prewarmed_samples = 0;
    this->load_model(nam_map.at(def.model_idx).second->data(), nam_map.at(def.model_idx).second->size());
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


void neural_amp_modeler::set_model(uint8_t idx)
{
    if (idx >= this->attr.model_names.size() || this->attr.ctrl.model_idx == idx)
        return;

    this->attr.ctrl.model_idx = idx;

    this->model_ready = false;
    this->prewarmed_samples = 0;
    this->load_model(nam_map.at(idx).second->data(), nam_map.at(idx).second->size());
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

