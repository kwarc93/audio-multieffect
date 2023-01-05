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
#include <hal/hal_delay.hpp>
#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>

#include "cmsis_os2.h"

#include "app/blinky.hpp"
#include "app/effects/effect_manager.hpp"
#include "app/controller/controller.hpp"

void blinky_timer_callback(void *arg)
{
    blinky *blinky_ao = static_cast<blinky*>(arg);

    static const blinky::event e{ blinky::timer_evt_t {}, blinky::event::flags::static_storage };
    blinky_ao->send(e);
}

void init_thread(void *arg)
{
    auto backlight_led = std::make_unique<hal::leds::backlight>();
    backlight_led->set(false);

    /* Test of Active Object 'blinky' */
    blinky blinky_ao;

    osTimerId_t blinky_tim = osTimerNew(blinky_timer_callback, osTimerPeriodic, &blinky_ao, NULL);
    assert(blinky_tim != nullptr);
    osTimerStart(blinky_tim, 500);

    /* Test of Active Object 'controller' */
    controller ctrl;

    /* Test of Active Object 'effect_manager' */
    effect_manager em;

//    static const std::array<effect_manager::event, 8> em_events =
//    {{
//        { effect_manager::add_effect_evt_t {effect_id::compressor} },
//        { effect_manager::add_effect_evt_t {effect_id::equalizer} },
//        { effect_manager::add_effect_evt_t {effect_id::reverb} },
//        { effect_manager::process_data_evt_t {} },
//        { effect_manager::bypass_evt_t {effect_id::reverb, true} },
//        { effect_manager::remove_effect_evt_t {effect_id::compressor} },
//        { effect_manager::process_data_evt_t {} },
//        { effect_manager::bypass_evt_t {effect_id::reverb, false} }
//    }};
//
//    while (true)
//    {
//        printf("\n--- Start of test loop ---\n");
//
//        for (const auto &e : em_events)
//        {
//            em.send(e);
//            osDelay(500);
//        }
//
//        printf("\n--- End of test loop ---\n");
//    }

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

