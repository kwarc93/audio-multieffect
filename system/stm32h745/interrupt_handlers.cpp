/*
 * interrupt_handlers.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <hal_system.hpp>

#include <drivers/stm32h7/usart.hpp>
#include <drivers/stm32h7/ltdc.hpp>
#include <drivers/stm32h7/dma2d.hpp>
#include <drivers/stm32h7/sai.hpp>
#include <drivers/stm32h7/exti.hpp>
#include <drivers/stm32h7/hsem.hpp>

extern "C" void tusb_int_handler(uint8_t rhport, bool in_isr);

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

extern "C" void OTG_FS_IRQHandler(void)
{
    tusb_int_handler(0, true);
}

extern "C"  void OTG_HS_IRQHandler(void)
{
    tusb_int_handler(1, true);
}

extern "C" void USART3_IRQHandler(void)
{
    drivers::usart::instance[static_cast<uint8_t>(drivers::usart::id::usart3)]->irq_handler();
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
    drivers::sai_base::instance[static_cast<uint8_t>(drivers::sai_base::id::sai2)]->block_a.dma_irq_handler();
}

extern "C" void DMA2_Stream6_IRQHandler(void)
{
    drivers::sai_base::instance[static_cast<uint8_t>(drivers::sai_base::id::sai2)]->block_b.dma_irq_handler();
}

extern "C" void HSEM1_IRQHandler(void)
{
    drivers::hsem::irq_handler();
}

extern "C" void HSEM2_IRQHandler(void)
{
    drivers::hsem::irq_handler();
}

extern "C" void EXTI0_IRQHandler(void)
{
    drivers::exti::irq_handler(drivers::exti::line::line0);
}

extern "C" void EXTI1_IRQHandler(void)
{
    drivers::exti::irq_handler(drivers::exti::line::line1);
}

extern "C" void EXTI2_IRQHandler(void)
{
    drivers::exti::irq_handler(drivers::exti::line::line2);
}

extern "C" void EXTI3_IRQHandler(void)
{
    drivers::exti::irq_handler(drivers::exti::line::line3);
}

extern "C" void EXTI4_IRQHandler(void)
{
    drivers::exti::irq_handler(drivers::exti::line::line4);
}

extern "C" void EXTI9_5_IRQHandler(void)
{
    drivers::exti::irq_handler(drivers::exti::line::line5, drivers::exti::line::line9);
}

extern "C" void EXTI15_10_IRQHandler(void)
{
    drivers::exti::irq_handler(drivers::exti::line::line10, drivers::exti::line::line15);
}
