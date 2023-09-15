/*
 * effect_processor.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "effect_processor.hpp"

#include <cstring>
#include <cmath>
#include <limits>
#include <functional>
#include <algorithm>
#include <memory>
#include <vector>
#include <tuple>
#include <map>

#include <hal/hal_system.hpp>

#include <middlewares/i2c_manager.hpp>

#include <stm32f7xx.h> // For managing D-Cache & I-Cache

#include "app/model/tremolo/tremolo.hpp"
#include "app/model/echo/echo.hpp"
#include "app/model/overdrive/overdrive.hpp"
#include "app/model/cabinet_sim/cabinet_sim.hpp"

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
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void effect_processor::event_handler(const events::add_effect &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    /* Don't allow duplicates */
    if (!this->find_effect(e.id, it))
    {
        auto effect = this->create_new(e.id);
        this->effects.push_back(std::move(effect));
    }
}

void effect_processor::event_handler(const events::remove_effect &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        this->effects.erase(it);
    }
}

void effect_processor::event_handler(const events::bypass_effect &e)
{
    auto effect = this->find_effect(e.id);
    if (effect == nullptr)
        return;

    effect->bypass(e.bypassed);
}

void effect_processor::event_handler(const events::set_volume &e)
{
    this->audio.set_input_volume(e.input_vol);
    this->audio.set_output_volume(e.output_vol);
}

void effect_processor::event_handler(const events::process_data &e)
{
    const uint32_t cycles_start = hal::system::clock::cycles();

    dsp_input_t *current_input {&this->dsp_input};
    dsp_output_t *current_output {&this->dsp_output};

    for (auto &effect : this->effects)
    {
        if (!(effect->is_bypassed()))
        {
            effect->process(*current_input, *current_output);

            /* Swap current buffers pointers after each effect process so that old output is new input */
            dsp_input_t *tmp = current_input;
            current_input = current_output;
            current_output = tmp;
        }
    }

    /* Set correct output buffer after all processing */
    current_output = current_input;

    /* Transform DSP samples to RAW buffer */
    constexpr float min = std::numeric_limits<hal::audio_devices::codec::output_sample_t>::min();
    constexpr float max = std::numeric_limits<hal::audio_devices::codec::output_sample_t>::max();
    constexpr float scale = -min;
    for (unsigned i = 0; i < current_output->size(); i++)
    {
        auto sample = std::clamp(current_output->at(i) * scale, min, max);

        /* Left */
        this->audio_output.buffer[this->audio_output.sample_index + 2 * i] = sample;
        /* Right */
        this->audio_output.buffer[this->audio_output.sample_index + 2 * i + 1] = sample;
    }

    // If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
    // Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes.
    SCB_CleanDCache_by_Addr(&this->audio_output.buffer[this->audio_output.sample_index], sizeof(this->audio_output.buffer) / 2);

    const uint32_t cycles_end = hal::system::clock::cycles();
    this->processing_time_us = cpu_cycles_to_us(cycles_start, cycles_end);
}

void effect_processor::event_handler(const events::get_processing_load &e)
{
    e.response(this->get_processing_load());
}

void effect_processor::event_handler(const events::set_effect_controls &e)
{
    std::visit([this](auto &&ctrl) { this->set_controls(ctrl); }, e.controls);
}

void effect_processor::event_handler(const events::get_effect_attributes &e)
{
    auto effect = this->find_effect(e.id);
    if (effect == nullptr)
        return;

    e.response(effect->get_basic_attributes(), effect->get_specific_attributes());
}

void effect_processor::set_controls(const tremolo_attributes::controls &ctrl)
{
    auto tremolo_effect = static_cast<tremolo*>(this->find_effect(effect_id::tremolo));

    if (tremolo_effect == nullptr)
        return;

    tremolo_effect->set_rate(ctrl.rate);
    tremolo_effect->set_depth(ctrl.depth);
    tremolo_effect->set_shape(ctrl.shape);
}

void effect_processor::set_controls(const echo_attributes::controls &ctrl)
{
    auto echo_effect = static_cast<echo*>(this->find_effect(effect_id::echo));

    if (echo_effect == nullptr)
        return;

    echo_effect->set_mode(ctrl.mode);
    echo_effect->set_blur(ctrl.blur);
    echo_effect->set_time(ctrl.time);
    echo_effect->set_feedback(ctrl.feedback);
}

void effect_processor::set_controls(const overdrive_attributes::controls &ctrl)
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

void effect_processor::set_controls(const cabinet_sim_attributes::controls &ctrl)
{
    auto cab_sim_effect = static_cast<cabinet_sim*>(this->find_effect(effect_id::cabinet_sim));

    if (cab_sim_effect == nullptr)
        return;
}

void effect_processor::notify_effect_attributes_changed(effect *e)
{
    this->notify(events::effect_attributes_changed {e->get_basic_attributes(), e->get_specific_attributes()});
}

std::unique_ptr<effect> effect_processor::create_new(effect_id id)
{
    static const std::map<effect_id, std::function<std::unique_ptr<effect>()>> effect_factory =
    {
        { effect_id::tremolo,       []() { return std::make_unique<tremolo>(); } },
        { effect_id::echo,          []() { return std::make_unique<echo>(); } },
        { effect_id::overdrive,     []() { return std::make_unique<overdrive>(); } },
        { effect_id::cabinet_sim,   []() { return std::make_unique<cabinet_sim>(); } }
    };

    return effect_factory.at(id)();
}

bool effect_processor::find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it)
{
    auto effect_it = std::find_if(begin(this->effects), end(this->effects),
                                  [id](const auto &effect) { return effect->get_basic_attributes().id == id; });

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
    /* WARINING: This method could have been called from interrupt */

    // If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
    // Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes.
    SCB_InvalidateDCache_by_Addr(const_cast<hal::audio_devices::codec::input_sample_t*>(input), length * sizeof(*input));

    /* Set current read index for input buffer (double buffering) */
    if (input == &this->audio_input.buffer[0])
        this->audio_input.sample_index = 0;
    else
        this->audio_input.sample_index = this->audio_input.buffer.size() / 2;

    /* Transform RAW samples to DSP buffer */
    constexpr float scale = 1.0f / -std::numeric_limits<hal::audio_devices::codec::input_sample_t>::min();
    for (unsigned i = this->audio_input.sample_index, j = 0; i < this->audio_input.sample_index + this->audio_input.buffer.size() / 2; i+=2, j++)
    {
        /* Copy only left channel */
        this->dsp_input[j] = this->audio_input.buffer[i] * scale;
    }

    /* Send event to process data */
    static const event e{ events::process_data {}, event::immutable };
    this->send(e, 0);
}

void effect_processor::audio_play_cb(uint16_t sample_index)
{
    /* WARINING: This method could have been called from interrupt */

    /* Set current write index for output buffer (double buffering) */
    if (sample_index == this->audio_output.buffer.size())
        this->audio_output.sample_index = this->audio_output.buffer.size() / 2;
    else
        this->audio_output.sample_index = 0;
}

uint8_t effect_processor::get_processing_load(void)
{
    constexpr uint32_t max_processing_time_us = 1e6 * dsp_vector_size / sampling_frequency_hz;
    return 100 * this->processing_time_us / max_processing_time_us;
}

//-----------------------------------------------------------------------------
/* public */

effect_processor::effect_processor() : active_object("effect_processor", osPriorityHigh, 4096),
audio{middlewares::i2c_managers::main::get_instance()}
{
    this->processing_time_us = 0;

    /* DSP buffers contain only one channel (left) */
    this->dsp_input.resize(dsp_vector_size);
    this->dsp_output.resize(dsp_vector_size);

    /* Start audio capture */
    this->audio.capture(this->audio_input.buffer.data(), this->audio_input.buffer.size(),
    [this](auto... params)
    {
        this->audio_capture_cb(params...);
    },
    true);

    /* Start audio playback */
    this->audio.play(this->audio_output.buffer.data(), this->audio_output.buffer.size(),
    [this](auto... params)
    {
        this->audio_play_cb(params...);
    },
    true);
}

effect_processor::~effect_processor()
{

}


