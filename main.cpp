/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <memory>
#include <vector>
#include <cassert>
#include <cstdio>

#include <hal/hal_system.hpp>

#include "cmsis_os2.h"

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/view/console_view/console_view.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"

void init_thread(void *arg)
{
    /* Create active objects */

    /* Active Object 'lcd_view' */
    auto lcd_view = std::make_unique<mfx::lcd_view>();

    /* Active Object 'console_view' */
    auto console_view = std::make_unique<mfx::console_view>();

    /* Available views */
    std::vector<mfx::view_interface*> views = {lcd_view.get(), console_view.get()};

    /* Active Object 'model' */
    auto model = std::make_unique<mfx::effect_processor>();

    /* Active Object 'controller' */
    auto ctrl = std::make_unique<mfx::controller>(model.get(), views);

    /* Add some effects */
//    static const std::array<mfx::effect_processor::event, 3> em_events =
//    {{
//        { mfx::effect_processor::add_effect_evt_t {mfx::effect_id::tremolo} },
//        { mfx::effect_processor::add_effect_evt_t {mfx::effect_id::equalizer} },
//        { mfx::effect_processor::add_effect_evt_t {mfx::effect_id::noise_gate} },
//    }};
//
//    for (const auto &e : em_events)
//        em->send(e);

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

