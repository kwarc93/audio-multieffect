/*
 * freertos_hooks.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <cassert>

#include "FreeRTOS.h"
#include "task.h"

extern "C" void vApplicationIdleHook(void)
{

}

extern "C" void vApplicationTickHook(void)
{

}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName )
{
    assert(0);
}

extern "C" void vApplicationMallocFailedHook(void)
{
    assert(0);
}
