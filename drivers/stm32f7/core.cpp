/*
 * core.cpp
 *
 *  Created on: 28 paź 2020
 *      Author: kwarc
 */


#include "core.hpp"

#include <cmsis/stm32f7xx.h>

using namespace drivers;

void core::enable_cycles_counter(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

core_critical_section::core_critical_section(void)
{
    this->primask = __get_PRIMASK();
    __disable_irq();
}

core_critical_section::~core_critical_section(void)
{
    if (!this->primask)
        __enable_irq();
}

core_temperature_sensor::core_temperature_sensor(void)
{
    /* TODO: Implement internal temperature sensor initialization */
}

float core_temperature_sensor::read_temperature(void)
{
    /* TODO: Implement internal temperature sensor reading */
    return 27.6;
}
