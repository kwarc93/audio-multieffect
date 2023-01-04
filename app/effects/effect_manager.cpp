/*
 * effect_manager.cpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#include "app/effects/effect_manager.hpp"

//-----------------------------------------------------------------------------
/* public */

effect_manager::effect_manager() : active_object("effect_manager", osPriorityNormal, 2048)
{

}

effect_manager::~effect_manager()
{

}

//-----------------------------------------------------------------------------
/* private */

void effect_manager::dispatch(const event &e)
{
    std::visit([this](const auto &e) { return this->event_handler(e); }, e.data);
}

void effect_manager::event_handler(const add_effect_evt_t &e)
{
//    this->effects.push_back(std::make_unique<>)
}

void effect_manager::event_handler(const remove_effect_evt_t &e)
{

}

void effect_manager::event_handler(const bypass_evt_t &e)
{

}

