/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <memory>
#include <cassert>
#include <cstdio>

#include <hal_system.hpp>
#include <hal_sdram.hpp>
#include <hal_led.hpp>

#include "cmsis_os2.h"

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/view/console_view/console_view.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"

#include "middlewares/filesystem.hpp"

#define DUAL_CORE_APP

#ifdef DUAL_CORE_APP
static void init_thread(void *arg)
{
#if defined(CORE_CM4)
    //middlewares::filesystem::test();
    auto lcd_view = std::make_unique<mfx::lcd_view>();
    auto ctrl = std::make_unique<mfx::controller>(nullptr, std::move(lcd_view));
#endif /* CORE_CM4 */

#ifdef CORE_CM7
    auto model = std::make_unique<mfx::effect_processor>();
    /* HW semaphore Clock enable */
    drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB4, HSEM), true);

    /* Take HSEM */
    bool result = ((HSEM->RLR[0] != (HSEM_RLR_LOCK | HSEM_CR_COREID_CURRENT)) ? 1UL : 0UL);
    (void) result;

    /* Release HSEM in order to notify the Cortex-M4 */
    WRITE_REG(HSEM->R[0], (HSEM_CR_COREID_CURRENT | 0));

    /* Wait until Cortex-M4 wakes up from stop mode */
    while (!(RCC->CR & RCC_CR_D2CKRDY));
#endif /* CORE_CM7 */

    osThreadSuspend(osThreadGetId());
}
//static void init_thread(void *arg)
//{
//#if defined(CORE_CM4)
//    hal::leds::debug led;
//    while (true)
//    {
//        led.set(!led.get());
//        osDelay(500);
//    }
//#endif /* CORE_CM4 */
//
//#ifdef CORE_CM7
//    hal::leds::error led;
//    while (true)
//    {
//        led.set(!led.get());
//        osDelay(200);
//    }
//#endif /* CORE_CM7 */
//
//    osThreadSuspend(osThreadGetId());
//}
#else
static void init_thread(void *arg)
{
    /* Test filesystem */
    middlewares::filesystem::test();

    /* Create active objects */
    auto model = std::make_unique<mfx::effect_processor>();
    auto lcd_view = std::make_unique<mfx::lcd_view>();
    auto ctrl = std::make_unique<mfx::controller>(std::move(model), std::move(lcd_view));

    osThreadSuspend(osThreadGetId());
}
#endif /* DUAL_CORE_APP */

int main(void)
{
    hal::system::init();

    printf("System started\r\n");
    printf("Software version: " GIT_REVISION "\r\n");

    osKernelInitialize();
    osThreadNew(init_thread, NULL, NULL);
    if (osKernelGetState() == osKernelReady)
        osKernelStart();

    assert(!"OS kernel start error");

    while (1);

    return 0;
}
