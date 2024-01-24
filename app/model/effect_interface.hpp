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
#include "app/config.hpp"

namespace mfx
{

class effect
{
public:
    typedef std::vector<float> dsp_input;
    typedef std::vector<float> dsp_output;

    effect(const effect_id id) : basic {id, effect_name[static_cast<uint8_t>(id)], true, 0}, aux_in {nullptr} {};
    virtual ~effect() {};

    virtual void process(const dsp_input &in, dsp_output &out) = 0;
    virtual const effect_specific_attributes get_specific_attributes(void) const = 0;

    const effect_attr& get_basic_attributes(void) { return this->basic; };
    bool is_bypassed() const { return this->basic.bypassed; };
    void bypass(bool state) { this->basic.bypassed = state; };
    void set_aux_input(const dsp_input &aux_in) { this->aux_in = &aux_in; };

protected:
    effect_attr basic;
    const dsp_input *aux_in;
};

}

#endif /* MODEL_EFFECT_INTERFACE_HPP_ */
