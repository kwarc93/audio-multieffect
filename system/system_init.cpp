/*
 * system_init.c
 *
 *  Created on: 19 paź 2020
 *      Author: kwarc
 */

#include <cmsis/stm32f7xx.h>

extern "C" void system_init(void)
{
#if (__FPU_USED == 1)
    /* Set bits 20-23 to enable full access to CP10 and CP11 coprocessors. */
    SCB->CPACR |= (3UL << 20) | (3UL << 22);
    __DSB();
    __ISB();
#endif
}

