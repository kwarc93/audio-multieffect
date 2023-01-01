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
#include "FreeRTOS.h"
#include "task.h"

auto debug_led = hal::leds::debug();

void blinky_thread(void *param)
{
    std::cout << "blinky_thread started" << std::endl;

    while (true)
    {
        debug_led.set(true);
        osDelay(500);
        debug_led.set(false);
        osDelay(500);
    }
}

void printf1_thread(void *param)
{
    std::cout << "printf1_thread started" << std::endl;
    osDelay(1000);

    while (true)
    {
        std::string *s = new std::string("****************");
        std::cout << *s << std::endl;
        delete s;
        osDelay(10);
    }
}

void printf2_thread(void *param)
{
    std::cout << "printf2_thread started" << std::endl;
    osDelay(1000);

    while (true)
    {
        std::string *s = new std::string("----------------");
        std::cout << *s << std::endl;
        delete s;
        osDelay(10);
    }
}

int main(void)
{
    hal::system::init();

    std::cout << "System started" << std::endl;

    auto backlight_led = std::make_unique<hal::leds::backlight>();
    backlight_led->set(false);

    osKernelInitialize();

    osThreadNew(blinky_thread, NULL, NULL);
    osThreadNew(printf1_thread, NULL, NULL);
    osThreadNew(printf2_thread, NULL, NULL);

    if (osKernelGetState() == osKernelReady)
      osKernelStart();

    assert(!"OS kernel start error");

    while(1);

    return 0;
}

