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

#include <drivers/touch_ft5336.hpp>

#include <hal/hal_system.hpp>
#include <hal/hal_delay.hpp>
#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>
#include <hal/hal_sdram.hpp>
#include <hal/hal_lcd.hpp>

#include "cmsis_os2.h"

#include "app/blinky.hpp"
#include "app/echo.hpp"
#include "app/gui.hpp"
#include "app/effects/effect_manager.hpp"
#include "app/controller/controller.hpp"

#include "middlewares/i2c_manager.hpp"

#include "libs/memtest/memtest.h"


void init_thread(void *arg)
{
    /* Test SDRAM */
    uint32_t tick_start = osKernelGetTickCount();
    int result  = memTestAll((datum*)hal::sdram::start_addr(), hal::sdram::size());
    uint32_t test_time = osKernelGetTickCount() - tick_start;
    printf("SDRAM memtest %s! Duration: %lu ms\n", result ? "failed" : "passed", test_time);
    assert(result == 0);

    /* Test of 'i2c_manager' */
    auto touch = drivers::touch_ft5336 { middlewares::i2c_managers::main::get_instance() };
    tick_start = osKernelGetTickCount();
    uint32_t cnt = 1000;
    while (cnt--)
    {
        uint8_t id = touch.read_id();
        assert(id == drivers::touch_ft5336::FT5336_ID);
    }
    test_time = osKernelGetTickCount() - tick_start;
    printf("I2C test %s! Duration: %lu ms\n", false ? "failed" : "passed", test_time);

    /* Create and test active objects */

    /* Test of Active Object 'gui' */
    auto gui_ao = std::make_unique<gui>();
    const gui::event e { gui::demo_test_evt_t {} };
    gui_ao->send(e);

    /* Test of Active Object 'echo' */
    auto echo_ao = std::make_unique<echo>();

    /* Test of Active Object 'blinky' */
    auto blinky_ao = std::make_unique<blinky>();

    /* Test of Active Object 'controller' */
    controller ctrl;

    /* Test of Active Object 'effect_manager' */
    effect_manager em;

    static const std::array<effect_manager::event, 8> em_events =
    {{
        { effect_manager::add_effect_evt_t {effect_id::compressor} },
        { effect_manager::add_effect_evt_t {effect_id::equalizer} },
        { effect_manager::add_effect_evt_t {effect_id::reverb} },
        { effect_manager::process_data_evt_t {} },
        { effect_manager::bypass_evt_t {effect_id::reverb, true} },
        { effect_manager::remove_effect_evt_t {effect_id::compressor} },
        { effect_manager::process_data_evt_t {} },
        { effect_manager::bypass_evt_t {effect_id::reverb, false} }
    }};

    while (true)
    {
        printf("\n--- Start of test loop ---\n");

        for (const auto &e : em_events)
        {
            em.send(e);
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

