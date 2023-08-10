/*
 * interrupt_handlers.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <hal/hal_system.hpp>

#include <drivers/stm32f7/usart.hpp>
#include <drivers/stm32f7/ltdc.hpp>
#include <drivers/stm32f7/dma2d.hpp>
#include <drivers/stm32f7/sai.hpp>

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

extern "C" void DMA2D_IRQHandler(void)
{
    drivers::dma2d::irq_handler();
}

extern "C" void DMA2_Stream4_IRQHandler(void)
{
    drivers::sai_base::dma_irq_handler(drivers::sai_base::id::sai2, drivers::sai_base::block::id::a);
}

extern "C" void DMA2_Stream6_IRQHandler(void)
{
    drivers::sai_base::dma_irq_handler(drivers::sai_base::id::sai2, drivers::sai_base::block::id::b);
}
