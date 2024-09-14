/*
 * rng.cpp
 *
 *  Created on: 12 mar 2024
 *      Author: kwarc
 */

#include "rng.hpp"

#include <cmsis/stm32f7xx.h>

#include <drivers/stm32f7/rcc.hpp>

using namespace drivers;

void rng::enable(bool state)
{
    if (state)
    {
        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB2, RNG), true);
        RNG->CR |= RNG_CR_RNGEN;
    }
    else
    {
        RNG->CR &= ~RNG_CR_RNGEN;
        rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB2, RNG), false);
    }
}

uint32_t rng::get(void)
{
    while ((RNG->SR & RNG_SR_CECS) || (RNG->SR & RNG_SR_SECS) || !(RNG->SR & RNG_SR_DRDY));
    return RNG->DR;
}

