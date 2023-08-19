/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <memory>
#include <array>
#include <cassert>
#include <cstdio>

#include <hal/hal_system.hpp>

#include "cmsis_os2.h"

#include "app/blinky.hpp"
#include "app/echo.hpp"
#include "app/gui.hpp"
#include "app/effects/effect_manager.hpp"
#include "app/controller/controller.hpp"

void init_thread(void *arg)
{
    /* Create and test active objects */

    /* Test of Active Object 'echo' */
    auto echo_ao = std::make_unique<echo>();

    /* Test of Active Object 'blinky' */
    auto blinky_ao = std::make_unique<blinky>();

    /* Test of Active Object 'gui' */
    auto gui_ao = std::make_unique<gui>();
    const gui::event e { gui::demo_test_evt_t {} };
    gui_ao->send(e);

    /* Test of Active Object 'controller' */
    auto ctrl = std::make_unique<controller>();

    /* Test of Active Object 'effect_manager' */
    auto em = std::make_unique<effect_manager>();

    static const std::array<effect_manager::event, 6> em_events =
    {{
        { effect_manager::add_effect_evt_t {effect_id::compressor} },
        { effect_manager::add_effect_evt_t {effect_id::equalizer} },
        { effect_manager::add_effect_evt_t {effect_id::reverb} },
        { effect_manager::bypass_evt_t {effect_id::reverb, true} },
        { effect_manager::remove_effect_evt_t {effect_id::compressor} },
        { effect_manager::bypass_evt_t {effect_id::reverb, false} }
    }};

    while (true)
    {
        printf("\n--- Start of test loop ---\n");

        for (const auto &e : em_events)
        {
            em->send(e);
            osDelay(500);
        }

        printf("\n--- End of test loop ---\n");
    }

    osThreadSuspend(osThreadGetId());
}

int main(void)
{
    hal::system::init();

    printf("System started\n");

    osKernelInitialize();
    osThreadNew(init_thread, NULL, NULL);
    if (osKernelGetState() == osKernelReady)
        osKernelStart();

    assert(!"OS kernel start error");

    while (1);

    return 0;
}

