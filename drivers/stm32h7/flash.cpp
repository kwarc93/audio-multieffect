/*
 * flash.cpp
 *
 *  Created on: 8 gru 2020
 *      Author: kwarc
 */

#include "flash.hpp"

#include <cmsis/stm32h7xx.h>

using namespace drivers;

void flash::set_wait_states(uint32_t sysclk_freq)
{
#ifdef CORE_CM7
    /* FLASH is clocked from AXI */
    sysclk_freq /= 2;
#endif

    /* Default value */
    uint32_t wait_states = 7;

    /* Calculate wait_states (valid for VOS1 core voltage range) */
    if (sysclk_freq <= 70000000)
    {
        wait_states = 0;
    }
    else if (sysclk_freq <= 140000000)
    {
        wait_states = 1;
    }
    else if (sysclk_freq <= 185000000)
    {
        wait_states = 2;
    }
    else if (sysclk_freq <= 210000000)
    {
        wait_states = 3;
    }
    else
    {
        /* Unsupported */
    }


    /* Enable prefetch & ART accelerator & set wait states */
    //ART->CTR |= ART_CTR_EN;
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, wait_states);

    __ISB();
}
