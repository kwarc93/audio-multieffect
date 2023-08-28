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

#include "middlewares/i2c_manager.hpp"

#include <stm32f7xx.h> // For managing D-Cache & I-Cache

using namespace mfx;

//-----------------------------------------------------------------------------
/* helpers */


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
//        printf("Effect '%s' added\n", effect->get_name().data());
        this->effects.push_back(std::move(effect));
    }
}

void effect_processor::event_handler(const remove_effect_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
//        auto &effect = *it;
//        printf("Effect '%s' removed\n", effect->get_name().data());
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

//        printf("Effect '%s' bypass state: %s\n", effect->get_name().data(), effect->is_bypassed() ? "on" : "off");
    }
}

void effect_processor::event_handler(const process_data_evt_t &e)
{
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
    for (unsigned i = 0; i < current_output->size(); i++)
    {
        auto sample = std::lround(current_output->at(i));
        /* Left */
        this->audio_output.buffer[this->audio_output.sample_index + 2 * i] = sample;
        /* Right */
        this->audio_output.buffer[this->audio_output.sample_index + 2 * i + 1] = sample;
    }

    // If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
    // Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes.
    SCB_CleanDCache_by_Addr(&this->audio_output.buffer[this->audio_output.sample_index], sizeof(this->audio_output.buffer) / 2);
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
    }, e.controls);
}

std::unique_ptr<effect> effect_processor::create_new(effect_id id)
{
    static const std::map<effect_id, std::function<std::unique_ptr<effect>()>> effect_factory =
    {
        { effect_id::equalizer,     []() { return std::make_unique<equalizer>(); } },
        { effect_id::noise_gate,    []() { return std::make_unique<noise_gate>(); } },
        { effect_id::tremolo,       []() { return std::make_unique<tremolo>(); } },
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
    for (unsigned i = this->audio_input.sample_index, j = 0; i < this->audio_input.sample_index + this->audio_input.buffer.size() / 2; i+=2, j++)
    {
        /* Copy only left channel */
        this->dsp_input.at(j) = this->audio_input.buffer.at(i);
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
    /* DSP buffers have only one channel */
    this->dsp_input.resize(this->audio_input.samples / 2);
    this->dsp_output.resize(this->audio_output.samples / 2);

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


