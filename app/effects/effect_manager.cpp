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

#include <stm32f7xx.h>

//-----------------------------------------------------------------------------
/* helpers */

namespace
{
    constexpr uint16_t inbuf_samples = 256;
    constexpr uint16_t outbuf_samples = inbuf_samples;
    __attribute__((aligned(32))) int16_t inbuf[inbuf_samples] {0};
    __attribute__((aligned(32))) int16_t outbuf[outbuf_samples] {0};
    volatile uint16_t inbuf_idx {0};
    volatile uint16_t outbuf_idx {0};

    /* Simple pass through */
    void capture_cb(const int16_t *input, uint16_t length)
    {
        // If D-Cache is enabled, it must be cleaned/invalidated for buffers used by DMA.
        // Moreover, functions 'SCB_*_by_Addr()' require address alignment of 32 bytes.

        std::size_t bytes = length * sizeof(*input);
        SCB_InvalidateDCache_by_Addr(const_cast<int16_t*>(input), bytes);
        std::memcpy(&outbuf[outbuf_idx], input, bytes);
        SCB_CleanDCache_by_Addr(&outbuf[outbuf_idx], bytes);
    }

    void play_cb(uint16_t output_sample_index)
    {
        outbuf_idx = (output_sample_index == outbuf_samples) ? outbuf_samples / 2 : 0;
    }
}

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
        printf("Effect '%s' added\n", effect->get_name().data());
        this->effects.push_back(std::move(effect));
    }
}

void effect_manager::event_handler(const remove_effect_evt_t &e)
{
    std::vector<std::unique_ptr<effect>>::iterator it;

    if (this->find_effect(e.id, it))
    {
        auto &effect = *it;
        printf("Effect '%s' removed\n", effect->get_name().data());
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

        printf("Effect '%s' bypass state: %s\n", effect->get_name().data(), effect->is_bypassed() ? "on" : "off");
    }
}

void effect_manager::event_handler(const process_data_evt_t &e)
{
    std::vector<uint32_t> input, output;

    for (auto &effect : this->effects)
        if (!(effect->is_bypassed()))
            effect->process(input, output);
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

//-----------------------------------------------------------------------------
/* public */

effect_manager::effect_manager() : active_object("effect_manager", osPriorityHigh, 4096),
audio{middlewares::i2c_managers::main::get_instance(), drivers::audio_wm8994ecs::i2c_address,
      drivers::audio_wm8994ecs::input::line1, drivers::audio_wm8994ecs::output::headphone}
{
    this->audio.capture(inbuf, inbuf_samples, capture_cb, true);
    this->audio.play(outbuf, outbuf_samples, play_cb, true);
}

effect_manager::~effect_manager()
{

}


