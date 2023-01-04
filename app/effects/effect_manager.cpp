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

//-----------------------------------------------------------------------------
/* private */

namespace
{
    const std::map<effect_id, std::function<std::unique_ptr<effect>()>> effect_factory =
    {
        { effect_id::equalizer,     []() { return std::make_unique<equalizer>(); } },
        { effect_id::reverb,        []() { return std::make_unique<reverb>(); } },
        { effect_id::compressor,    []() { return std::make_unique<compressor>(); } },
    };
}

void effect_manager::dispatch(const event &e)
{
    std::visit([this](const auto &e) { return this->event_handler(e); }, e.data);
}

void effect_manager::event_handler(const add_effect_evt_t &e)
{
    auto effect = effect_factory.at(e.id)();

    printf("Effect '%s' added\n", effect->get_name().data());

    this->effects.insert({e.id, std::move(effect)});

}

void effect_manager::event_handler(const remove_effect_evt_t &e)
{
    auto &effect = this->effects.at(e.id);

    printf("Effect '%s' removed\n", effect->get_name().data());

    this->effects.erase(e.id);
}

void effect_manager::event_handler(const bypass_evt_t &e)
{
    auto &effect = this->effects.at(e.id);
    effect->bypass(e.bypassed);

    printf("Effect '%s' bypass state: %s\n", effect->get_name().data(), effect->is_bypassed() ? "on" : "off");
}

void effect_manager::event_handler(const process_data_evt_t &e)
{
    std::vector<uint32_t> input, output;

    for (auto &effect : this->effects)
        effect.second->process(input, output);
}

//-----------------------------------------------------------------------------
/* public */

effect_manager::effect_manager() : active_object("effect_manager", osPriorityNormal, 2048)
{

}

effect_manager::~effect_manager()
{

}


