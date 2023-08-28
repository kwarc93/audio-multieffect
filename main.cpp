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

#include "app/echo.hpp"
#include "app/view/lcd_view/gui.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"

void init_thread(void *arg)
{
    /* Create active objects */

    /* Active Object 'gui' */
    auto gui_ao = std::make_unique<gui>();
    const gui::event e { gui::demo_test_evt_t {} };
    gui_ao->send(e);

    /* Active Object 'controller' */
    auto ctrl = std::make_unique<mfx::controller>();

    /* Active Object 'effect_processor' */
    auto em = std::make_unique<mfx::effect_processor>();

    /* Add some effects */
    static const std::array<mfx::effect_processor::event, 3> em_events =
    {{
        { mfx::effect_processor::add_effect_evt_t {mfx::effect_id::tremolo} },
        { mfx::effect_processor::add_effect_evt_t {mfx::effect_id::equalizer} },
        { mfx::effect_processor::add_effect_evt_t {mfx::effect_id::noise_gate} },
    }};

    for (const auto &e : em_events)
        em->send(e);

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

