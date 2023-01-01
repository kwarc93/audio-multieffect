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

auto debug_led = hal::leds::debug();

void blinky_task(void *param)
{
    std::cout << "blinky_task started" << std::endl;

    while (true)
    {
        debug_led.set(true);
        osDelay(500);
        debug_led.set(false);
        osDelay(500);
    }
}

int main(void)
{
    hal::system::init();

    std::cout << "System started" << std::endl;

    auto backlight_led = std::make_unique<hal::leds::backlight>();

    backlight_led->set(false);

    osKernelInitialize();                       // Initialize CMSIS-RTOS
    osThreadNew(blinky_task, NULL, NULL);       // Create application main thread
    if (osKernelGetState() == osKernelReady)
      osKernelStart();                          // Start thread execution

    assert(0);
    while(1);

    return 0;
}

