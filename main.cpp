/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <memory>
#include <cassert>
#include <cstdio>

#include <hal/hal_system.hpp>
#include <hal/hal_delay.hpp>
#include <hal/hal_led.hpp>
#include <hal/hal_button.hpp>

#include "cmsis_os2.h"

#include "app/blinky.hpp"
#include "app/echo.hpp"

osTimerId_t blinky_tim = nullptr;
uint32_t blinky_tim_period = 500;

void blinky_timer_callback(void *arg)
{
    blinky *blinky_ao = static_cast<blinky*>(arg);

    static const blinky::event e{ blinky::timer_evt_t{}, blinky::event::flags::static_storage };
    blinky_ao->send(e);
}

void button_timer_callback(void *arg)
{
    hal::buttons::blue_btn *btn = static_cast<hal::buttons::blue_btn*>(arg);

    btn->debounce();

    if (btn->was_pressed())
    {
        blinky_tim_period = blinky_tim_period == 500 ? 100 : 500;
        osTimerStart(blinky_tim, blinky_tim_period);

        static const echo::event e2{ echo::button_evt_t{}, blinky::event::flags::static_storage };
        echo::instance->send(e2);
    }
}

void init_thread(void *arg)
{
    auto backlight_led = std::make_unique<hal::leds::backlight>();
    backlight_led->set(false);

    blinky blinky_ao;
    echo echo_ao;
    hal::buttons::blue_btn button;

    blinky_tim = osTimerNew(blinky_timer_callback, osTimerPeriodic, &blinky_ao, NULL);
    assert(blinky_tim != nullptr);
    osTimerStart(blinky_tim, blinky_tim_period);

    osTimerId_t button_tim = osTimerNew(button_timer_callback, osTimerPeriodic, &button, NULL);
    assert(button_tim != nullptr);
    osTimerStart(button_tim, 20);

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

    while(1);

    return 0;
}

