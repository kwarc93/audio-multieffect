/*
 * hal_random.cpp
 *
 *  Created on: 12 mar 2024
 *      Author: kwarc
 */

#include "hal_random.hpp"

#include <drivers/stm32f7/rng.hpp>

using namespace hal;

void random::enable(bool state)
{
    drivers::rng::enable(state);
}

uint32_t random::get(void)
{
    return drivers::rng::get();
}


