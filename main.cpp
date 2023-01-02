/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <iostream>
#include <memory>
#include <cassert>

#include <hal/hal_system.hpp>
#include <hal/hal_delay.hpp>
#include <hal/hal_led.hpp>

#include "cmsis_os2.h"

#include "app/blinky.hpp"

void blinky_timer_callback(void *arg)
{
    blinky *blinky_ao = static_cast<blinky*>(arg);

    blinky_evt::timer_evt_t e;
    blinky_ao->send(e);
}

void init_thread(void *arg)
{
    auto backlight_led = std::make_unique<hal::leds::backlight>();
    backlight_led->set(false);

    blinky blinky_ao;

    osTimerId_t timer = osTimerNew(blinky_timer_callback, osTimerPeriodic, &blinky_ao, NULL);
    assert(timer != nullptr);
    osTimerStart(timer, 500);

    osThreadSuspend(osThreadGetId());
}

int main(void)
{
    hal::system::init();

    std::cout << "System started" << std::endl;

    osKernelInitialize();
    osThreadNew(init_thread, NULL, NULL);
    if (osKernelGetState() == osKernelReady)
        osKernelStart();

    assert(!"OS kernel start error");

    while(1);

    return 0;
}

