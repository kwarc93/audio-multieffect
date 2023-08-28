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
#include "app/view/lcd_view/gui.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"

void init_thread(void *arg)
{
    /* Create and test active objects */

    /* Test of Active Object 'echo' */
    auto echo_ao = std::make_unique<echo>();

    /* Test of Active Object 'blinky' */
    auto blinky_ao = std::make_unique<blinky>();

    /* Active Object 'gui' */
    auto gui_ao = std::make_unique<gui>();
    const gui::event e { gui::demo_test_evt_t {} };
    gui_ao->send(e);

    /* Active Object 'controller' */
    auto ctrl = std::make_unique<controller>();

    /* Active Object 'effect_processor' */
    auto em = std::make_unique<effect_processor>();

    static const std::array<effect_processor::event, 3> em_events =
    {{
        { effect_processor::add_effect_evt_t {effect_id::tremolo} },
        { effect_processor::add_effect_evt_t {effect_id::equalizer} },
        { effect_processor::add_effect_evt_t {effect_id::noise_gate} },
    }};

    /* Add some effects */
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

