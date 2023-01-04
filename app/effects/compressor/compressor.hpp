/*
 * compressor.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_COMPRESSOR_COMPRESSOR_HPP_
#define EFFECTS_COMPRESSOR_COMPRESSOR_HPP_

#include "app/effects/effect_interface.hpp"

class compressor : public effect
{
public:
    compressor();
    virtual ~compressor();

    void process(const input_t &in, output_t &out) override;
};

#endif /* EFFECTS_COMPRESSOR_COMPRESSOR_HPP_ */
