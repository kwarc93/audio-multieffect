/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <iostream>
#include <memory>

#include <hal/hal_system.hpp>
#include <hal/hal_delay.hpp>
#include <hal/hal_led.hpp>

#include "FreeRTOS.h"
#include "task.h"

auto debug_led = hal::leds::debug();

void blinky_task(void *param)
{
    std::cout << "blinky_task started" << std::endl;

    while (true)
    {
        debug_led.set(true);
        vTaskDelay(500);
        debug_led.set(false);
        vTaskDelay(500);
    }
}

int main(void)
{
    hal::system::init();

    std::cout << "System started" << std::endl;

    auto backlight_led = std::make_unique<hal::leds::backlight>();

    backlight_led->set(false);

    TaskHandle_t task_h = NULL;

    /* Create the task, storing the handle. */
    BaseType_t result = xTaskCreate(
                    blinky_task,            /* Function that implements the task. */
                    "blinky_task",          /* Text name for the task. */
                    100,                    /* Stack size in words, not bytes. */
                    ( void * ) 1,           /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 1,   /* Priority at which the task is created. */
                    &task_h );              /* Used to pass out the created task's handle. */

    if( result != pdPASS )
    {
        std::cout << "Failed to create blinky_task" << std::endl;
    }

    vTaskStartScheduler();

    return 0;
}

