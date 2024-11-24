/*
 * flash.cpp
 *
 *  Created on: 8 gru 2020
 *      Author: kwarc
 */

#include "flash.hpp"

#include <cmsis/stm32h7xx.h>

#include <drivers/stm32h7/rcc.hpp>

using namespace drivers;

void flash::set_wait_states(uint32_t hclk)
{
    /* Default value */
    uint32_t wait_states = 7;
    uint32_t prog_delay = 3;

    /* Calculate wait_states (valid for VOS1 core voltage range) */
    if (hclk <= 70000000)
    {
        wait_states = 0;
        prog_delay = 0;
    }
    else if (hclk <= 140000000)
    {
        wait_states = 1;
        prog_delay = 1;
    }
    else if (hclk <= 185000000)
    {
        wait_states = 2;
        prog_delay = 1;
    }
    else if (hclk <= 210000000)
    {
        wait_states = 2;
        prog_delay = 2;
    }
    else if (hclk <= 225000000)
    {
        wait_states = 3;
        prog_delay = 2;
    }
    else
    {
        /* Unsupported */
    }

#ifdef CORE_CM4
    /* Configure Cortex-M4 Instruction cache through ART accelerator */
    rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, ART), true);
    MODIFY_REG(ART->CTR, ART_CTR_PCACHEADDR, ((FLASH_BANK2_BASE >> 12U) & 0x000FFF00UL));
    SET_BIT(ART->CTR, ART_CTR_EN);
#endif /* CORE_CM4 */

    /* Set wait states and programming delay */
    FLASH->ACR = (wait_states << FLASH_ACR_LATENCY_Pos) |
                 (prog_delay << FLASH_ACR_WRHIGHFREQ_Pos);

    __ISB();
}
