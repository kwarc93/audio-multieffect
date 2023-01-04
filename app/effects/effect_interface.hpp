/*
 * effect_interface.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_EFFECT_INTERFACE_HPP_
#define EFFECTS_EFFECT_INTERFACE_HPP_

#include <vector>
#include <string_view>
#include <cstdint>
#include <cstdio> // TODO: Remove after tests

#include "effect_types.hpp"

class effect
{
public:
    effect(const effect_id &id, const std::string_view &name) : id {id}, name {name}, bypassed {false} {};
    virtual ~effect() {};

    virtual void process(const input_t &in, output_t &out) = 0;
protected:
    const effect_id id;
    const std::string_view name;
    bool bypassed;
};

#endif /* EFFECTS_EFFECT_INTERFACE_HPP_ */
