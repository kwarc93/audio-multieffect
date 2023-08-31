/*
 * effect_interface.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EFFECT_INTERFACE_HPP_
#define MODEL_EFFECT_INTERFACE_HPP_

#include <vector>
#include <string_view>
#include <cstdint>

#include "data_types.hpp"

namespace mfx
{

class effect
{
public:
    effect(const effect_id &id, const std::string_view &name) : id {id}, name {name}, bypassed {true} {};
    virtual ~effect() {};

    virtual void process(const dsp_input_t &in, dsp_output_t &out) = 0;

    effect_id get_id() const { return this->id; };
    const std::string_view& get_name() const { return this->name; };
    bool is_bypassed() const { return this->bypassed; };
    void bypass(bool state) { this->bypassed = state; };
protected:
    const effect_id id;
    const std::string_view name;
    bool bypassed;
};

}

#endif /* MODEL_EFFECT_INTERFACE_HPP_ */
