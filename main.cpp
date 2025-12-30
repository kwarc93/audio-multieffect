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

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"
#include "app/modules/settings.hpp"
#include "app/modules/presets.hpp"

#include "middlewares/filesystem.hpp"

#ifdef DUAL_CORE_APP
#ifdef CORE_CM4
#include "app/ipc/ipc_effect_processor.hpp"

static void init_thread(void *arg)
{
    /* Init filesystem */
    middlewares::filesystem::init();

    /* Create settings manager */
    auto settings = std::make_unique<settings_manager>(std::make_unique<settings_storage_file>("settings.cbor"));

    /* Create preset manager */
    auto presets = std::make_unique<presets_manager>(std::make_unique<presets_storage_file>("presets"));

    /* Create actors */
    auto view = std::make_unique<mfx::lcd_view>();
    auto model = std::make_unique<mfx::ipc_effect_processor>();
    auto ctrl = std::make_unique<mfx::controller>(std::move(model), std::move(view), std::move(settings), std::move(presets));

    osThreadSuspend(osThreadGetId());
}
#endif /* CORE_CM4 */
#ifdef CORE_CM7
#include "app/ipc/ipc_controller.hpp"
static void init_thread(void *arg)
{
    /* Create actors */
    auto model = std::make_unique<mfx::effect_processor>();
    auto ctrl = std::make_unique<mfx::ipc_controller>(std::move(model));

    osThreadSuspend(osThreadGetId());
}
#endif /* CORE_CM7 */
#else
static void init_thread(void *arg)
{
    /* Init filesystem */
    middlewares::filesystem::init();

    /* Create settings manager */
    auto settings = std::make_unique<settings_manager>(std::make_unique<settings_storage_file>("settings.cbor"));

    /* Create preset manager */
    auto presets = std::make_unique<presets_manager>(std::make_unique<presets_storage_file>("presets"));

    /* Create actors */
    auto view = std::make_unique<mfx::lcd_view>();
    auto model = std::make_unique<mfx::effect_processor>();
    auto ctrl = std::make_unique<mfx::controller>(std::move(model), std::move(view), std::move(settings), std::move(presets));

    vTaskSuspend(xTaskGetCurrentTaskHandle());
}
#endif /* DUAL_CORE_APP */

int main(int argc, const char* argv[])
{
    hal::system::init();

    printf("System started\r\n");
    printf("Software version: " GIT_REVISION "\r\n");

    auto result = xTaskCreate(init_thread, "init_thread", 2048 / sizeof(StackType_t), nullptr, configTASK_PRIO_BACKGROUND, nullptr);
    assert(result == pdPASS);

    vTaskStartScheduler();

    assert(!"OS kernel start error");

    while (1);

    return 0;
}

extern "C"
{
/* Callback function prototypes */
extern void vApplicationIdleHook (void);
extern void vApplicationMallocFailedHook (void);
extern void vApplicationDaemonTaskStartupHook (void);

/**
  Dummy implementation of the callback function vApplicationIdleHook().
*/
#if (configUSE_IDLE_HOOK == 1)
__WEAK void vApplicationIdleHook (void){ __WFI(); }
#endif

/**
  Dummy implementation of the callback function vApplicationTickHook().
*/
#if (configUSE_TICK_HOOK == 1)
 __WEAK void vApplicationTickHook (void){}
#endif

/**
  Dummy implementation of the callback function vApplicationMallocFailedHook().
*/
#if (configUSE_MALLOC_FAILED_HOOK == 1)
__WEAK void vApplicationMallocFailedHook (void) {
  /* Assert when malloc failed hook is enabled but no application defined function exists */
  configASSERT(0);
}
#endif

/**
  Dummy implementation of the callback function vApplicationDaemonTaskStartupHook().
*/
#if (configUSE_DAEMON_TASK_STARTUP_HOOK == 1)
__WEAK void vApplicationDaemonTaskStartupHook (void){}
#endif

/**
  Dummy implementation of the callback function vApplicationStackOverflowHook().
*/
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
__WEAK void vApplicationStackOverflowHook (TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;

  /* Assert when stack overflow is enabled but no application defined function exists */
  configASSERT(0);
}
#endif
}

