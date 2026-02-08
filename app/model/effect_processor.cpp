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
#include <array>

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
#include "app/model/amp_sim/amp_sim.hpp"
#include "app/utils.hpp"

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

    if (e.usb_audio_if_enabled)
        this->usb_audio.enable();
    this->usb_direct_mon = e.usb_direct_mon_enabled;

    this->usb_audio.set_volume_changed_callback(
    [this](float output_volume_db)
    {
        const auto vol_range = this->audio.get_output_volume_range();
        const uint8_t volume = utils::map_range<float>(vol_range.min_db, vol_range.max_db, vol_range.min_val, vol_range.max_val, output_volume_db);

        this->send({events::set_output_volume {volume}});
        this->notify(events::output_volume_changed {volume});
    });

    this->usb_audio.set_mute_changed_callback(
    [this](bool muted)
    {
        this->send({events::set_mute {muted}});
        this->notify(events::mute_changed {muted});
    });
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

void effect_processor::event_handler(const events::enable_usb_audio_if &e)
{
    e.value ? this->usb_audio.enable() : this->usb_audio.disable();
}

void effect_processor::event_handler(const events::enable_usb_direct_mon &e)
{
    this->usb_direct_mon = e.value;
}

void effect_processor::event_handler(const events::process_audio &e)
{
    const uint32_t cycles_start = hal::system::clock::cycles();

    /* Handle USB audio */
    const bool usb_enabled = this->usb_audio.is_enabled();
    const bool unmute_sample = !usb_enabled || this->usb_direct_mon;
    if (usb_enabled)
        this->usb_audio.process();

    /* Process effects */
    std::reference_wrapper<decltype(this->dsp_output)> current_output = this->dsp_output;
    std::reference_wrapper<decltype(this->dsp_main_input)> current_input = this->dsp_main_input;
    for (auto &&effect : this->effects)
    {
        if (!effect->is_bypassed())
        {
            effect->set_aux_input(this->dsp_aux_input);
            effect->process(current_input, current_output);

            /* Swap current buffers so that old output is new input */
            std::swap(current_input, current_output);
        }
    }

    /* Set correct output buffer after all processing */
    current_output = current_input;

    const auto out_buf_idx = this->audio_output.sample_index;
    for (unsigned i = 0; i < current_output.get().size(); i++)
    {
        /* Transform normalized DSP samples to RAW buffer (24bit onto 32bit MSB) */
        decltype(this->audio_output.buffer)::value_type sample;

        constexpr decltype(sample) min = -(1 << (this->audio_input.bps - 1));
        constexpr decltype(sample) max = -min - 1;
        constexpr decltype(sample) scale = -min;

        sample = current_output.get()[i] * scale;
        sample = std::clamp(sample, min, max) << 8;

        /* Duplicate left channel to right channel & mix with received USB audio */
        const auto j = 2 * i;
        const auto out = sample * unmute_sample;
        this->audio_output.buffer[out_buf_idx + j] = out + this->usb_audio.audio_from_host.buffer[j];
        this->audio_output.buffer[out_buf_idx + j + 1] = out + this->usb_audio.audio_from_host.buffer[j + 1];

        /* Fill USB audio buffer */
        this->usb_audio.audio_to_host.buffer[i] = sample;
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
        this->notify_effect_attributes_changed(effect);
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

    tuner_effect->set_mute_mode(ctrl.mute);
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

    /* Notify about change in internal structure of effect */
    if (current_mode != ctrl.mode)
        this->notify_effect_attributes_changed(vocoder_effect);
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

void effect_processor::set_controls(const amp_sim_attr::controls &ctrl)
{
    auto amp_sim_effect = static_cast<amp_sim*>(this->find_effect(effect_id::amplifier_sim));

    if (amp_sim_effect == nullptr)
        return;

    amp_sim_effect->set_mode(ctrl.mode);
    amp_sim_effect->set_input(ctrl.input);
    amp_sim_effect->set_drive(ctrl.drive);
    amp_sim_effect->set_compression(ctrl.compression);
    amp_sim_effect->set_tone_stack(ctrl.bass, ctrl.mids, ctrl.treb);
}

void effect_processor::notify_effect_attributes_changed(const effect *e)
{
    this->notify(events::effect_attributes_changed {e->get_basic_attributes(), e->get_specific_attributes()});
}

std::unique_ptr<effect> effect_processor::create_new(effect_id id)
{
    constexpr std::array<std::unique_ptr<effect>(*)(), static_cast<uint8_t>(effect_id::_count)> effect_factory
    {{
        []() -> std::unique_ptr<effect> { return std::make_unique<tuner>();       },
        []() -> std::unique_ptr<effect> { return std::make_unique<tremolo>();     },
        []() -> std::unique_ptr<effect> { return std::make_unique<echo>();        },
        []() -> std::unique_ptr<effect> { return std::make_unique<chorus>();      },
        []() -> std::unique_ptr<effect> { return std::make_unique<reverb>();      },
        []() -> std::unique_ptr<effect> { return std::make_unique<overdrive>();   },
        []() -> std::unique_ptr<effect> { return std::make_unique<cabinet_sim>(); },
        []() -> std::unique_ptr<effect> { return std::make_unique<vocoder>();     },
        []() -> std::unique_ptr<effect> { return std::make_unique<phaser>();      },
        []() -> std::unique_ptr<effect> { return std::make_unique<amp_sim>();     }
    }};

    std::unique_ptr<effect> e = effect_factory.at(static_cast<uint8_t>(id))();
    e->set_callback([this](effect* e) { this->notify_effect_attributes_changed(e); });
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
    this->audio_input.sample_index = input - this->audio_input.buffer.begin();

    /* Transform RAW samples (24bit extended onto 32bit MSB) to normalized DSP buffer */
    for (unsigned i = this->audio_input.sample_index, j = 0; i < this->audio_input.sample_index + this->audio_input.buffer.size() / 2; i+=2, j++)
    {
        constexpr float scale = 1.0f / (1 << (this->audio_input.bps - 1));
        this->dsp_main_input[j] = (this->audio_input.buffer[i] >> 8) * scale;       // Left
        this->dsp_aux_input[j] = (this->audio_input.buffer[i + 1] >> 8) * scale;    // Right
    }

    /* Send event to process data */
    static const event e{ events::process_audio {}, true };
    this->send(e);
}

void effect_processor::audio_play_cb(uint16_t sample_index)
{
    /* WARNING: This method could have been called from interrupt */

    /* Set current write index for output buffer (double buffering) */
    this->audio_output.sample_index = sample_index - this->audio_output.buffer.size() / 2;
}

uint8_t effect_processor::get_processing_load(void)
{
    constexpr uint32_t max_processing_time_us = 1e6 * config::dsp_vector_size / config::sampling_frequency_hz;
    return 100 * this->processing_time_us / max_processing_time_us;
}

//-----------------------------------------------------------------------------
/* public */

effect_processor::effect_processor() :
audio{middlewares::i2c_managers::main::get_instance()},
usb_audio {audio.get_output_volume_range()}
{
    this->processing_time_us = 0;
    this->usb_direct_mon = false;

    this->send({events::initialize {}});
}

effect_processor::~effect_processor()
{

}


