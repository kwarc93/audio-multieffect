/*
 * tremolo.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_TREMOLO_TREMOLO_HPP_
#define MODEL_TREMOLO_TREMOLO_HPP_

#include "app/model/effect_interface.hpp"

namespace mfx
{

class tremolo : public effect
{
public:
    enum class shape_type {sine, triangle};

    struct controls
    {
        float rate;  // LFO frequency in Hz, range: [1, 20]
        float depth; // Effect depth, range: [0, 0.5]
        shape_type shape; // LFO shape
    };

    struct state_t
    {
        int error_code;
    };

    tremolo(float rate = 8, float depth = 0.3f, shape_type shape = shape_type::triangle);
    virtual ~tremolo();

    void process(const dsp_input_t &in, dsp_output_t &out) override;

    void set_depth(float depth);
    void set_rate(float rate);
    void set_shape(shape_type shape);
private:
    float lfo(void);

    float depth;

    float lfo_freq;
    float lfo_counter;
    float lfo_counter_dir;
    float lfo_counter_limit;
    shape_type lfo_shape;
};

}

#endif /* MODEL_TREMOLO_TREMOLO_HPP_ */