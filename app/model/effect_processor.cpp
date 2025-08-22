/*
 * effect_processor.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "effect_processor.hpp"

#include <cstring>
#include <cmath>
#include <functional>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>

#include <hal_system.hpp>

#include <middlewares/i2c_manager.hpp>

#include <cmsis_device.h> // For managing D-Cache & I-Cache

#include "app/model/tuner/tuner.hpp"
#include "app/model/tremolo/tremolo.hpp"
#include "app/model/echo/echo.hpp"
#include "app/model/chorus/chorus.hpp"
#include "app/model/reverb/reverb.hpp"
#include "app/model/overdrive/overdrive.hpp"
#include "app/model/cabinet_sim/cabinet_sim.hpp"
#include "app/model/vocoder/vocoder.hpp"
#include "app/model/phaser/phaser.hpp"

using namespace mfx;
namespace events = effect_processor_events;

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
    uint32_t cpu_cycles_to_us(uint32_t start, uint32_t end)
    {
        constexpr uint32_t cycles_per_us = hal::system::system_clock / 1000000ul;
        uint32_t total_cycles = end - start;
        return total_cycles / cycles_per_us;
    }
}

//-----------------------------------------------------------------------------
/* private */

void effect_processor::dispatch(const event &e)
{
    std::visit([this](auto &&e) { this->event_handler(e); }, e.data);
}

void effect_processor::event_handler(const events::initialize &e)
{
    /* DSP buffers contain only one channel */
    this->dsp_main_input.resize(config::dsp_vector_size);
    this->dsp_aux_input.resize(config::dsp_vector_size);
    this->dsp_output.resize(config::dsp_vector_size);
}

void effect_processor::event_handler(const events::shutdown &e)
{
    this->audio.mute(true);
    this->audio.stop();
    this->audio.stop_capture();
}

void effect_processor::event_handler(const events::configuration &e)
{
    this->audio.set_input_volume(e.main_input_vol, 0);
    this->audio.set_input_volume(e.aux_input_vol, 1);
    this->audio.set_output_volume(e.output_vol);
    this->audio.route_onboard_mic_to_aux(e.mic_routed_to_aux);
    this->audio.mute(e.output_muted);
}

void effect_processor::event_handler(const events::start_audio &e)
{
    /* Start audio capture */
    this->audio.capture(this->audio_input.buffer.data(), this->audio_input.buffer.size(),
    [this](auto && ...params)
    {
        this->audio_capture_cb(params...);
    },
    true);

    /* Start audio playback */
    this->audio.play(this->audio_output.buffer.data(), this->audio_output.buffer.size(),
    [this](auto && ...params)
    {
        this->audio_play_cb(params...);
    },
    true);
}

void effect_processor::event_handler(const events::add_effect &e)
{
    /* Don't allow duplicates */
    if (!this->find_effect(e.id))
        this->effects.push_back(std::move(this->create_new(e.id)));
}

void effect_processor::event_handler(const events::remove_effect &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
        this->effects.erase(it);
}

void effect_processor::event_handler(const events::move_effect &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        if (it == this->effects.begin() && e.step < 0)
            return;

        if (it == (this->effects.end() - 1) && e.step > 0)
            return;

        /* Only moves by +1/-1 are supported */
        std::swap(*it, *std::next(it, std::clamp(e.step, -1L, 1L)));
    }
}

void effect_processor::event_handler(const events::bypass_effect &e)
{
    auto effect = this->find_effect(e.id);

    if (effect)
        effect->bypass(e.bypassed);

}

void effect_processor::event_handler(const events::set_input_volume &e)
{
    this->audio.set_input_volume(e.main_input_vol, 0);
    this->audio.set_input_volume(e.aux_input_vol, 1);
}

void effect_processor::event_handler(const events::set_output_volume &e)
{
    this->audio.set_output_volume(e.output_vol);
}

void effect_processor::event_handler(const events::route_mic_to_aux &e)
{
    this->audio.route_onboard_mic_to_aux(e.value);

    /* Re-start audio capture */
    this->audio.capture(this->audio_input.buffer.data(), this->audio_input.buffer.size(),
    [this](auto && ...params)
    {
        this->audio_capture_cb(params...);
    },
    true);
}

void effect_processor::event_handler(const events::set_mute &e)
{
    this->audio.mute(e.value);
}

void effect_processor::event_handler(const events::process_audio &e)
{
    const uint32_t cycles_start = hal::system::clock::cycles();

    effect::dsp_input *current_input {&this->dsp_main_input};
    effect::dsp_output *current_output {&this->dsp_output};

    for (auto &&effect : this->effects)
    {
        if (!effect->is_bypassed())
        {
            effect->set_aux_input(this->dsp_aux_input);
            effect->process(*current_input, *current_output);

            /* Swap current buffers pointers after each effect process so that old output is new input */
            effect::dsp_input *tmp = current_input;
            current_input = current_output;
            current_output = tmp;
        }
    }

    /* Set correct output buffer after all processing */
    current_output = current_input;

    /* Transform normalized DSP samples to RAW buffer (24bit onto 32bit MSB) */
    for (unsigned i = 0; i < current_output->size(); i++)
    {
        decltype(this->audio_output.buffer)::value_type sample;

        constexpr decltype(sample) min = -(1 << (this->audio_input.bps - 1));
        constexpr decltype(sample) max = -min - 1;
        constexpr decltype(sample) scale = -min;

        sample = current_output->at(i) * scale;
        sample = std::clamp(sample, min, max) << 8;

        /* Duplicate left channel to right channel */
        const auto index = this->audio_output.sample_index + 2 * i;
        this->audio_output.buffer[index] = sample;
        this->audio_output.buffer[index + 1] = sample;
    }

#ifdef CORE_CM7
    /* If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
       Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes. */
    SCB_CleanDCache_by_Addr(&this->audio_output.buffer[this->audio_output.sample_index], sizeof(this->audio_output.buffer) / 2);
#endif /* CORE_CM7 */

    const uint32_t cycles_end = hal::system::clock::cycles();
    this->processing_time_us = cpu_cycles_to_us(cycles_start, cycles_end);
}

void effect_processor::event_handler(const events::get_dsp_load &e)
{
    this->notify(events::dsp_load_changed {this->get_processing_load()});
}

void effect_processor::event_handler(const events::set_effect_controls &e)
{
    std::visit([this](auto &&ctrl) { this->set_controls(ctrl); }, e.ctrl);
}

void effect_processor::event_handler(const events::get_effect_attributes &e)
{
    auto effect = this->find_effect(e.id);
    if (effect)
    {
        this->notify_effect_attributes_changed(effect);
    }
}

void effect_processor::event_handler(const effect_processor_events::enumerate_effects_attributes &e)
{
    for (auto it = this->effects.begin(); it != this->effects.end(); ++it)
    {
        bool is_last = std::next(it) == this->effects.end();
        this->notify(events::effect_attributes_enumerated {is_last, (*it)->get_basic_attributes(), (*it)->get_specific_attributes()});
    }
}

void effect_processor::set_controls(const tuner_attr::controls &ctrl)
{
    auto tuner_effect = static_cast<tuner*>(this->find_effect(effect_id::tuner));

    if (tuner_effect == nullptr)
        return;

    tuner_effect->set_a4_tuning(ctrl.a4_tuning);
}

void effect_processor::set_controls(const tremolo_attr::controls &ctrl)
{
    auto tremolo_effect = static_cast<tremolo*>(this->find_effect(effect_id::tremolo));

    if (tremolo_effect == nullptr)
        return;

    tremolo_effect->set_rate(ctrl.rate);
    tremolo_effect->set_depth(ctrl.depth);
    tremolo_effect->set_shape(ctrl.shape);
}

void effect_processor::set_controls(const echo_attr::controls &ctrl)
{
    auto echo_effect = static_cast<echo*>(this->find_effect(effect_id::echo));

    if (echo_effect == nullptr)
        return;

    echo_effect->set_mode(ctrl.mode);
    echo_effect->set_blur(ctrl.blur);
    echo_effect->set_time(ctrl.time);
    echo_effect->set_feedback(ctrl.feedback);
}

void effect_processor::set_controls(const chorus_attr::controls &ctrl)
{
    auto chorus_effect = static_cast<chorus*>(this->find_effect(effect_id::chorus));

    if (chorus_effect == nullptr)
        return;

    chorus_effect->set_depth(ctrl.depth);
    chorus_effect->set_rate(ctrl.rate);
    chorus_effect->set_tone(ctrl.tone);
    chorus_effect->set_mix(ctrl.mix);
    chorus_effect->set_mode(ctrl.mode);
}

void effect_processor::set_controls(const reverb_attr::controls &ctrl)
{
    auto reverb_effect = static_cast<reverb*>(this->find_effect(effect_id::reverb));

    if (reverb_effect == nullptr)
        return;

    reverb_effect->set_bandwidth(ctrl.bandwidth);
    reverb_effect->set_damping(ctrl.damping);
    reverb_effect->set_decay(ctrl.decay);
    reverb_effect->set_mode(ctrl.mode);
}

void effect_processor::set_controls(const overdrive_attr::controls &ctrl)
{
    auto overdrive_effect = static_cast<overdrive*>(this->find_effect(effect_id::overdrive));

    if (overdrive_effect == nullptr)
        return;

    overdrive_effect->set_mode(ctrl.mode);
    overdrive_effect->set_high(ctrl.high);
    overdrive_effect->set_low(ctrl.low);
    overdrive_effect->set_gain(ctrl.gain);
    overdrive_effect->set_mix(ctrl.mix);
}

void effect_processor::set_controls(const cabinet_sim_attr::controls &ctrl)
{
    auto cab_sim_effect = static_cast<cabinet_sim*>(this->find_effect(effect_id::cabinet_sim));

    if (cab_sim_effect == nullptr)
        return;

    cab_sim_effect->set_ir(ctrl.ir_idx);
}

void effect_processor::set_controls(const vocoder_attr::controls &ctrl)
{
    auto vocoder_effect = static_cast<vocoder*>(this->find_effect(effect_id::vocoder));

    if (vocoder_effect == nullptr)
        return;

    const auto current_mode = std::get<vocoder_attr>(vocoder_effect->get_specific_attributes()).ctrl.mode;

    vocoder_effect->set_mode(ctrl.mode);
    vocoder_effect->hold(ctrl.hold);
    vocoder_effect->set_tone(ctrl.tone);
    vocoder_effect->set_clarity(ctrl.clarity);
    vocoder_effect->set_bands(ctrl.bands);

    if (current_mode != ctrl.mode)
    {
        /* Notify about change in internal structure of effect */
        this->notify_effect_attributes_changed(vocoder_effect);
    }
}

void effect_processor::set_controls(const phaser_attr::controls &ctrl)
{
    auto phaser_effect = static_cast<phaser*>(this->find_effect(effect_id::phaser));

    if (phaser_effect == nullptr)
        return;

    phaser_effect->set_rate(ctrl.rate);
    phaser_effect->set_depth(ctrl.depth);
    phaser_effect->set_contour(ctrl.contour);
}

void effect_processor::notify_effect_attributes_changed(const effect *e)
{
    this->notify(events::effect_attributes_changed {e->get_basic_attributes(), e->get_specific_attributes()});
}

std::unique_ptr<effect> effect_processor::create_new(effect_id id)
{
    static const std::map<effect_id, std::function<std::unique_ptr<effect>()>> effect_factory =
    {
        { effect_id::tuner,         []() { return std::make_unique<tuner>(); } },
        { effect_id::tremolo,       []() { return std::make_unique<tremolo>(); } },
        { effect_id::echo,          []() { return std::make_unique<echo>(); } },
        { effect_id::chorus,        []() { return std::make_unique<chorus>(); } },
        { effect_id::reverb,        []() { return std::make_unique<reverb>(); } },
        { effect_id::overdrive,     []() { return std::make_unique<overdrive>(); } },
        { effect_id::cabinet_sim,   []() { return std::make_unique<cabinet_sim>(); } },
        { effect_id::vocoder,       []() { return std::make_unique<vocoder>(); } },
        { effect_id::phaser,        []() { return std::make_unique<phaser>(); } }
    };

    std::unique_ptr<effect> e = effect_factory.at(id)();
    e->set_notify_callback([this](effect* e) { this->notify_effect_attributes_changed(e); });
    return e;
}

bool effect_processor::find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it)
{
    auto effect_it = std::find_if(begin(this->effects), end(this->effects),
                                  [id](auto &&effect) { return effect->get_basic_attributes().id == id; });

    it = effect_it;

    return effect_it != std::end(this->effects);
}

effect* effect_processor::find_effect(effect_id id)
{
    std::vector<std::unique_ptr<effect>>::iterator it;
    return this->find_effect(id, it) ? (*it).get() : nullptr;
}

void effect_processor::audio_capture_cb(const hal::audio_devices::codec::input_sample_t *input, uint16_t length)
{
    /* WARNING: This method could have been called from interrupt */

#ifdef CORE_CM7
    /* If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
       Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes. */
    SCB_InvalidateDCache_by_Addr(const_cast<hal::audio_devices::codec::input_sample_t*>(input), length * sizeof(*input));
#endif /* CORE_CM7 */

    /* Set current read index for input buffer (double buffering) */
    if (input == &this->audio_input.buffer[0])
        this->audio_input.sample_index = 0;
    else
        this->audio_input.sample_index = this->audio_input.buffer.size() / 2;

    /* Transform RAW samples (24bit extended onto 32bit MSB) to normalized DSP buffer */
    for (unsigned i = this->audio_input.sample_index, j = 0; i < this->audio_input.sample_index + this->audio_input.buffer.size() / 2; i+=2, j++)
    {
        constexpr float scale = 1.0f / (1 << (this->audio_input.bps - 1));
        this->dsp_main_input[j] = (this->audio_input.buffer[i] >> 8) * scale; // Left
        this->dsp_aux_input[j] = (this->audio_input.buffer[i + 1] >> 8) * scale; // Right
    }

    /* Send event to process data */
    static const event e{ events::process_audio {}, event::immutable };
    this->send(e, 0);
}

void effect_processor::audio_play_cb(uint16_t sample_index)
{
    /* WARNING: This method could have been called from interrupt */

    /* Set current write index for output buffer (double buffering) */
    if (sample_index == this->audio_output.buffer.size())
        this->audio_output.sample_index = this->audio_output.buffer.size() / 2;
    else
        this->audio_output.sample_index = 0;
}

uint8_t effect_processor::get_processing_load(void)
{
    constexpr uint32_t max_processing_time_us = 1e6 * config::dsp_vector_size / config::sampling_frequency_hz;
    return 100 * this->processing_time_us / max_processing_time_us;
}

//-----------------------------------------------------------------------------
/* public */

effect_processor::effect_processor() :
audio{middlewares::i2c_managers::main::get_instance()}
{
    this->processing_time_us = 0;

    this->send({events::initialize {}});
}

effect_processor::~effect_processor()
{

}


