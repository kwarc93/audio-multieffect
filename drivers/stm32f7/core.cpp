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
    /* Current inplementation: DWT-based cycles counter.
     * Not all Cortex-M cores have it so SysTick-based cycles
     * counter would be more generic (but has lower, 24bit resolution). */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

__attribute__((__always_inline__))
uint32_t core::get_cycles_counter(void)
{
    return DWT->CYCCNT;
}

void core::enter_sleep_mode(void)
{
    __DSB();
    __ISB();
    __WFI();

    /* Exit upon interrupt */
}

void core::enter_stop_mode(void)
{
    /* Set regulator & flash in low power mode (STOP LP-FPD) */
    PWR->CR1 |= PWR_CR1_LPDS | PWR_CR1_FPDS;
    SCB->SCR |= (1 << SCB_SCR_SLEEPDEEP_Pos);

    __DSB();
    __ISB();
    __SEV();
    __WFE();
    __WFE();

    /* Exit upon event */
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
