/*
 * effect_manager.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "effect_manager.hpp"

#include <functional>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>

#include "app/effects/equalizer/equalizer.hpp"
#include "app/effects/reverb/reverb.hpp"
#include "app/effects/compressor/compressor.hpp"

#include "drivers/stm32f7/sai.hpp"

static int16_t inbuf[16];
static int16_t outbuf[16];

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

effect_manager::effect_manager() : active_object("effect_manager", osPriorityHigh, 4096)
{
    // SAI2 test
    using audio_sai = drivers::sai<int16_t>;

    auto sai = std::make_unique<audio_sai>(audio_sai::id::sai2);

    static const audio_sai::block::config a_cfg
    {
        audio_sai::block::mode_type::master_tx,
        audio_sai::block::protocol_type::generic,
        audio_sai::block::data_size::_16bit,
        audio_sai::block::sync_type::none,
        audio_sai::block::frame_type::stereo,
        audio_sai::block::audio_freq::_48kHz,
    };

    sai->block_a.configure(a_cfg);

    static const audio_sai::block::config b_cfg
    {
        audio_sai::block::mode_type::slave_rx,
        audio_sai::block::protocol_type::generic,
        audio_sai::block::data_size::_16bit,
        audio_sai::block::sync_type::internal,
        audio_sai::block::frame_type::stereo,
        audio_sai::block::audio_freq::_48kHz,
    };

    sai->block_b.configure(b_cfg);

    static audio_sai::transfer_desc xfer
    {
        outbuf,
        sizeof(outbuf),
        inbuf,
        sizeof(inbuf),
    };

    sai->transfer(xfer, [](const audio_sai::transfer_desc &xfer){}, true);
}

effect_manager::~effect_manager()
{

}


