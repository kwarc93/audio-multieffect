/*
 * interrupt_handlers.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <hal/hal_system.hpp>

#include <drivers/stm32f7/usart.hpp>
#include <drivers/stm32f7/ltdc.hpp>

//-----------------------------------------------------------------------------
/* Core interrupt handlers */

#ifndef HAL_SYSTEM_RTOS_ENABLED
extern "C" void SysTick_Handler(void)
{
    hal::system::systick++;
}
#endif

extern "C" void NMI_Handler(void)
{
    while(true);
}

extern "C" void HardFault_Handler(void)
{
    while(true);
}

extern "C" void MemManage_Handler(void)
{
    while(true);
}

extern "C" void BusFault_Handler(void)
{
    while(true);
}

extern "C" void UsageFault_Handler(void)
{
    while(true);
}

//-----------------------------------------------------------------------------
/* Peripheral interrupt handlers */

extern "C" void USART1_IRQHandler(void)
{
    drivers::usart::instance[static_cast<uint8_t>(drivers::usart::id::usart1)]->irq_handler();
}

extern "C" void LTDC_ER_IRQHandler(void)
{
    drivers::ltdc::irq_handler();
}

extern "C" void LTDC_IRQHandler(void)
{
    drivers::ltdc::irq_handler();
}
