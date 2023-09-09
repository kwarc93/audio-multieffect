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
#include <map>

#include <hal/hal_system.hpp>

#include <middlewares/i2c_manager.hpp>

#include <stm32f7xx.h> // For managing D-Cache & I-Cache

using namespace mfx;

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

void effect_processor::event_handler(const add_effect_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (!this->find_effect(e.id, it))
    {
        auto effect = this->create_new(e.id);
        this->effects.push_back(std::move(effect));
    }
}

void effect_processor::event_handler(const remove_effect_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        this->effects.erase(it);
    }
}

void effect_processor::event_handler(const bypass_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        auto &effect = *it;
        effect->bypass(e.bypassed);
    }
}

void effect_processor::event_handler(const volume_evt_t &e)
{
    this->audio.set_input_volume(e.input_vol);
    this->audio.set_output_volume(e.output_vol);
}

void effect_processor::event_handler(const process_data_evt_t &e)
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

void effect_processor::event_handler(const effect_controls_evt_t &e)
{
    std::visit([this](auto &&controls)
    {
        using T = std::decay_t<decltype(controls)>;
        if constexpr (std::is_same_v<T, equalizer::controls>)
        {
            /* Do something specific to this effect */
        }
        else if constexpr (std::is_same_v<T, noise_gate::controls>)
        {
            /* Do something specific to this effect */
        }
        else if constexpr (std::is_same_v<T, tremolo::controls>)
        {
            std::vector<std::unique_ptr<mfx::effect>>::iterator it;

            if (this->find_effect(effect_id::tremolo, it))
            {
                auto &effect = *it;
                auto tremolo = static_cast<mfx::tremolo*>(effect.get());
                tremolo->set_rate(controls.rate);
                tremolo->set_depth(controls.depth);
                tremolo->set_shape(controls.shape);
            }
        }
        else if constexpr (std::is_same_v<T, echo::controls>)
        {
            std::vector<std::unique_ptr<mfx::effect>>::iterator it;

            if (this->find_effect(effect_id::echo, it))
            {
                auto &effect = *it;
                auto echo = static_cast<mfx::echo*>(effect.get());
                echo->set_mode(controls.mode);
                echo->set_decay(controls.decay);
                echo->set_time(controls.time);
                echo->set_feedback(controls.feedback);
            }
        }
        else if constexpr (std::is_same_v<T, overdrive::controls>)
        {
            std::vector<std::unique_ptr<mfx::effect>>::iterator it;

            if (this->find_effect(effect_id::overdrive, it))
            {
                auto &effect = *it;
                auto overdrive = static_cast<mfx::overdrive*>(effect.get());
                overdrive->set_mode(controls.mode);
                overdrive->set_high(controls.high);
                overdrive->set_low(controls.low);
                overdrive->set_gain(controls.gain);
                overdrive->set_mix(controls.mix);
            }
        }
        else if constexpr (std::is_same_v<T, cabinet_sim::controls>)
        {
            /* Do something specific to this effect */
        }
    }, e.controls);
}

std::unique_ptr<effect> effect_processor::create_new(effect_id id)
{
    static const std::map<effect_id, std::function<std::unique_ptr<effect>()>> effect_factory =
    {
        { effect_id::equalizer,     []() { return std::make_unique<equalizer>(); } },
        { effect_id::noise_gate,    []() { return std::make_unique<noise_gate>(); } },
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
                                  [id](const auto &effect) { return effect->get_id() == id; });

    it = effect_it;

    return effect_it != std::end(this->effects);
}

void effect_processor::audio_capture_cb(const hal::audio_devices::codec::input_sample_t *input, uint16_t length)
{
    /* WARINING: This method may be called from interrupt */

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
    static const event e{ process_data_evt_t {}, event::immutable };
    this->send(e, 0);
}

void effect_processor::audio_play_cb(uint16_t sample_index)
{
    /* WARINING: This method may be called from interrupt */

    /* Set current write index for output buffer (double buffering) */
    if (sample_index == this->audio_output.buffer.size())
        this->audio_output.sample_index = this->audio_output.buffer.size() / 2;
    else
        this->audio_output.sample_index = 0;
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

uint8_t effect_processor::get_processing_load(void)
{
    constexpr uint32_t max_processing_time_us = 10e6 * 2 * dsp_vector_size / sampling_frequency_hz;
    return 100 * this->processing_time_us / max_processing_time_us;
}


