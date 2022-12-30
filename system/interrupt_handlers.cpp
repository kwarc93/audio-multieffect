/*
 * interrupt_handlers.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <hal/hal_system.hpp>

//-----------------------------------------------------------------------------
/* interrupt handlers */

#ifndef HAL_SYSTEM_FREERTOS_ENABLED
extern "C" void SysTick_Handler(void)
{
    hal::system::systick++;
}
#endif
