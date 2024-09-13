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
#include <hal/hal_nvm.hpp>

#include "cmsis_os2.h"

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/view/console_view/console_view.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"

void init_thread(void *arg)
{
    /* Create active objects */

    auto model = std::make_unique<mfx::effect_processor>();
    auto lcd_view = std::make_unique<mfx::lcd_view>();
    auto ctrl = std::make_unique<mfx::controller>(std::move(model), std::move(lcd_view));

    osThreadSuspend(osThreadGetId());
}

int main(void)
{
    hal::system::init();

    printf("System started\r\n");

    // QSPI FLASH TEST
    char *data = "abcde";
    char buffer[5];
    hal::nvms::qspi_flash storage;
    uint8_t first_byte = 0xFF;

    if (storage.read(reinterpret_cast<std::byte*>(&first_byte), 0, 1))
    {
        if (first_byte != 0)
        {
            printf("Erasing QSPI FLASH first subsector...\n");
            bool result = storage.erase(0, 4096);
            printf("QSPI FLASH erasing %s\n", result ? "done" : "error");
        }
    }

    if (storage.write(reinterpret_cast<std::byte*>(data), 0, 5))
    {
        if (storage.read(reinterpret_cast<std::byte*>(buffer), 0, 5))
        {
            for (unsigned i = 0; i < 5; i++)
                printf("buffer[%u]: %c\n", i, buffer[i]);
            while (1);
        }
    }

    printf("QSPI FLASH write-read data mismatch\n");
    while (1);

    osKernelInitialize();
    osThreadNew(init_thread, NULL, NULL);
    if (osKernelGetState() == osKernelReady)
        osKernelStart();

    assert(!"OS kernel start error");

    while (1);

    return 0;
}

