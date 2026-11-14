/*
 * nam.cpp
 *
 *  Created on: 1 lip 2026
 *      Author: kwarc
 */

#include "nam.hpp"
#include "nam_models.hpp"
#include "namb_reader.h"

#include <algorithm>

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{

constexpr std::array<std::pair<const char*, const nam_a2_lite_t*>, 10> nam_map
{{
    { "Fender Pro Reverb 1967", &Fender_Pro_Reverb_1967 },
    { "Laney LA100BL 1969", &Laney_LA100BL_pre_Supergroup_1969 },
    { "Roland JC 120B Jazz Chorus", &Roland_JC_120B_Jazz_Chorus },
    { "Orange OTR 120 2x12", &Orange_OTR_120_2x12 },
    { "Gibson GA-20 Tweed 1961", &Gibson_GA_20_Tweed_1961 },
    { "Vox AC30/4 1961 Fawn EF86", &Vox_AC30_4_1961_Fawn_EF86 },
    { "Marshall Super Lead 12000", &Marshall_Super_Lead_12_000 },
    { "Soldano SLO 100", &Soldano_SLO_100 },
    { "Peavey 5150", &Peavey_5150 },
    { "Mesa Dual Rectifier MW", &Mesa_Dual_Rectifier_MW }
}};

static_assert(nam_map.size() == neural_amp_modeler_attr{}.model_names.size());
static_assert(NAM_A2LITE_MAX_BUFFER_SIZE == mfx::config::dsp_buffer_size);
static_assert((static_cast<uint32_t>(NAM_A2LITE_SAMPLE_RATE) + 500u) / 1000u == (mfx::config::sampling_frequency_hz + 500u) / 1000u);

}

//-----------------------------------------------------------------------------
/* private */

bool neural_amp_modeler::prewarm(int samples_to_prewarm)
{
    if (this->prewarmed_samples == samples_to_prewarm)
        return true;

    // Feed silence through the model until the receptive field is full

    static constexpr float zero_buf[NAM_A2LITE_MAX_BUFFER_SIZE] {};
    static float out_buf[NAM_A2LITE_MAX_BUFFER_SIZE] {};

    const float *in_ptrs[NAM_A2LITE_IN_CHANNELS] {zero_buf};
    float *out_ptrs[NAM_A2LITE_OUT_CHANNELS] {out_buf};

    if (this->prewarmed_samples < samples_to_prewarm)
    {
        int n = NAM_A2LITE_MAX_BUFFER_SIZE;
        if (n > samples_to_prewarm - this->prewarmed_samples)
            n = samples_to_prewarm - this->prewarmed_samples;
        nam_a2lite_process(&this->nam, in_ptrs, out_ptrs, n);
        this->prewarmed_samples += n;
    }

    return this->prewarmed_samples == samples_to_prewarm;
}

bool neural_amp_modeler::load_model(const uint8_t* model, size_t size)
{
    namb_model_t namb_model;
    if (namb_parse(model, size, &namb_model) != NAMB_OK)
    {
        return false;
    }

    if (namb_check_a2lite(&namb_model) != NAMB_OK)
    {
        return false;
    }

    if (nam_a2lite_load_weights(&this->nam_weights, namb_model.weights, (int)namb_model.weight_count) != 0)
    {
        return false;
    }

    nam_a2lite_init(&this->nam, &this->nam_weights, &this->nam_state);
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
    this->model_ready = this->prewarm(NAM_A2LITE_PREWARM_SAMPLES);

    if (this->model_ready)
    {
        const float *in_ptrs[NAM_A2LITE_IN_CHANNELS] {in.data()};
        float *out_ptrs[NAM_A2LITE_OUT_CHANNELS] {out.data()};

        nam_a2lite_process(&this->nam, in_ptrs, out_ptrs, mfx::config::dsp_buffer_size);

        std::transform(out.begin(), out.end(), out.begin(),
        [this](auto input)
        {
            return input * this->out_gain;
        });
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
    // Range: -10db .. +10db
    this->out_gain = std::pow(10.0f, vol - 0.5f);
}

