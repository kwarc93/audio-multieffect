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

#include "cmsis_os2.h"

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"

#include "middlewares/filesystem.hpp"

#ifdef DUAL_CORE_APP
#ifdef CORE_CM4
#include "app/ipc/ipc_effect_processor.hpp"

static void init_thread(void *arg)
{
    /* Test filesystem */
    //middlewares::filesystem::test();

    /* Create active objects */
    auto model = std::make_unique<mfx::ipc_effect_processor>();
    auto lcd_view = std::make_unique<mfx::lcd_view>();
    auto ctrl = std::make_unique<mfx::controller>(std::move(model), std::move(lcd_view));

    osThreadSuspend(osThreadGetId());
}
#endif /* CORE_CM4 */
#ifdef CORE_CM7
#include "app/ipc/ipc_controller.hpp"
static void init_thread(void *arg)
{
    /* Create active objects */
    auto model = std::make_unique<mfx::effect_processor>();
    auto ctrl = std::make_unique<mfx::ipc_controller>(std::move(model));

    osThreadSuspend(osThreadGetId());
}
#endif /* CORE_CM7 */
#else
static void init_thread(void *arg)
{
    /* Test filesystem */
    //middlewares::filesystem::test();

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
