/*
 * effect_interface.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EFFECT_INTERFACE_HPP_
#define MODEL_EFFECT_INTERFACE_HPP_

#include <cstdint>
#include <vector>
#include <string_view>

#include "effect_features.hpp"

namespace mfx
{

class effect
{
public:
    effect(const effect_id &id, const std::string_view &name) : basic {id, name, true, 0} {};
    virtual ~effect() {};

    virtual void process(const dsp_input_t &in, dsp_output_t &out) = 0;
    virtual const effect_specific_attributes get_specific_attributes(void) const = 0;

    const effect_basic_attributes& get_basic_attributes(void) { return this->basic; };
    bool is_bypassed() const { return this->basic.bypassed; };
    void bypass(bool state) { this->basic.bypassed = state; };

protected:
    effect_basic_attributes basic;
};

}

#endif /* MODEL_EFFECT_INTERFACE_HPP_ */
