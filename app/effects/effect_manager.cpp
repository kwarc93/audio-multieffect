/*
 * effect_manager.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "effect_manager.hpp"

#include <cstring>
#include <functional>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>

#include "middlewares/i2c_manager.hpp"

#include "app/effects/equalizer/equalizer.hpp"
#include "app/effects/reverb/reverb.hpp"
#include "app/effects/compressor/compressor.hpp"

#include <stm32f7xx.h> // For managing D-Cache & I-Cache

//-----------------------------------------------------------------------------
/* helpers */


//-----------------------------------------------------------------------------
/* private */

void effect_manager::dispatch(const event &e)
{
    std::visit([this](const auto &e) { this->event_handler(e); }, e.data);
}

void effect_manager::event_handler(const add_effect_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (!this->find_effect(e.id, it))
    {
        auto effect = this->create_new(e.id);
//        printf("Effect '%s' added\n", effect->get_name().data());
        this->effects.push_back(std::move(effect));
    }
}

void effect_manager::event_handler(const remove_effect_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        auto &effect = *it;
//        printf("Effect '%s' removed\n", effect->get_name().data());
        this->effects.erase(it);
    }
}

void effect_manager::event_handler(const bypass_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        auto &effect = *it;
        effect->bypass(e.bypassed);

//        printf("Effect '%s' bypass state: %s\n", effect->get_name().data(), effect->is_bypassed() ? "on" : "off");
    }
}

void effect_manager::event_handler(const process_data_evt_t &e)
{
    input_t *current_input {&this->dsp_input_buf};
    output_t *current_output {&this->dsp_output_buf};

    for (auto &effect : this->effects)
    {
        if (!(effect->is_bypassed()))
        {
            effect->process(*current_input, *current_output);

            /* Swap current buffers pointers after each effect process */
            input_t *tmp = current_input;
            current_input = current_output;
            current_output = tmp;
        }
    }

    /* Set correct output buffer after all processing */
    current_output = current_input;

    /* Copy processed samples to output buffer */
    std::transform(current_output->begin(), current_output->end(), this->audio_output_buf.begin() + this->outbuf_idx,
    [](auto x)
    {
        return static_cast<hal::audio_devices::codec::audio::output_sample_t>(x);
    }
    );

    // If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
    // Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes.
    SCB_CleanDCache_by_Addr(&this->audio_output_buf[this->outbuf_idx], sizeof(this->audio_output_buf) / 2);
}

std::unique_ptr<effect> effect_manager::create_new(effect_id id)
{
    static const std::map<effect_id, std::function<std::unique_ptr<effect>()>> effect_factory =
    {
        { effect_id::equalizer,     []() { return std::make_unique<equalizer>(); } },
        { effect_id::reverb,        []() { return std::make_unique<reverb>(); } },
        { effect_id::compressor,    []() { return std::make_unique<compressor>(); } },
    };

    return effect_factory.at(id)();
}

bool effect_manager::find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it)
{
    auto effect_it = std::find_if(begin(this->effects), end(this->effects),
                                  [id](const auto &effect) { return effect->get_id() == id; });

    it = effect_it;

    return effect_it != std::end(this->effects);
}

void effect_manager::audio_capture_cb(const hal::audio_devices::codec::audio::input_sample_t *input, uint16_t length)
{
    /* WARINING: This method may be called from interrupt */

    // If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
    // Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes.

    SCB_InvalidateDCache_by_Addr(const_cast<int16_t*>(input), length * sizeof(*input));

    /* Copy new samples to DSP input buffer */
    this->inbuf_idx = (input == &this->audio_input_buf[0]) ? 0 : this->audio_input_buf.size() / 2;
    auto audio_input_begin {this->audio_input_buf.begin() + this->inbuf_idx};
    auto audio_input_end {this->audio_input_buf.begin() + this->inbuf_idx + this->audio_input_buf.size() / 2};
    std::transform(audio_input_begin, audio_input_end, this->dsp_input_buf.begin(),
    [](auto x)
    {
        return static_cast<float>(x);
    }
    );

    static const event e{ process_data_evt_t {}, event::immutable };
    this->send(e, 0);
}

void effect_manager::audio_play_cb(uint16_t output_sample_index)
{
    /* WARINING: This method may be called from interrupt */

    this->outbuf_idx = (output_sample_index == this->audio_output_buf.size()) ? this->audio_output_buf.size() / 2 : 0;
}

//-----------------------------------------------------------------------------
/* public */

effect_manager::effect_manager() : active_object("effect_manager", osPriorityHigh, 4096),
audio{middlewares::i2c_managers::main::get_instance()}
{
    this->audio.capture(this->audio_input_buf.data(), this->audio_input_buf.size(),
    [this](auto... params)
    {
        this->audio_capture_cb(params...);
    },
    true);

    this->audio.play(this->audio_output_buf.data(), this->audio_output_buf.size(),
    [this](auto... params)
    {
        this->audio_play_cb(params...);
    },
    true);
}

effect_manager::~effect_manager()
{

}


