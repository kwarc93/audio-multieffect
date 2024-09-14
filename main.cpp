/*
 * main.cpp
 *
 *  Created on: 30 gru 2022
 *      Author: kwarc
 */

#include <memory>
#include <cassert>
#include <cstdio>
#include <array>
#include <algorithm>

#include <hal/hal_system.hpp>
#include <hal/hal_nvm.hpp>
#include <hal/hal_random.hpp>

#include "cmsis_os2.h"

#include "app/view/lcd_view/lcd_view.hpp"
#include "app/view/console_view/console_view.hpp"
#include "app/model/effect_processor.hpp"
#include "app/controller/controller.hpp"

#include "libs/littlefs/lfs.h"

static void littlefs_test(void)
{
    // variables used by the filesystem
    static lfs_t lfs;
    static lfs_file_t file;
    static struct lfs_config cfg;

    hal::random::enable(true);
    hal::buttons::blue_btn button;
    hal::nvms::qspi_flash storage;

    if (button.is_pressed())
    {
        printf("Erasing QSPI FLASH...\r\n");
        const bool result = storage.erase();
        printf("QSPI FLASH erasing %s\r\n", result ? "done" : "error");
    }

//------------------------------------------------------------------------------
//    uint32_t address = 4096 * 115;
//    if (storage.erase(address, storage.erase_size()))
//    {
//        static std::array<std::byte, 4096> data;
//        std::generate(data.begin(), data.end(), [](){ return (std::byte)(hal::random::get() % 256); });
//
//        if (storage.write(data.data(), address, data.size()))
//        {
//            static std::array<std::byte, 4096> readback;
//            if (storage.read(readback.data(), address, readback.size()))
//            {
//                for (unsigned i = 0; i < 4096; i++)
//                    assert(data[i] == readback[i]);
//
//                printf("QSPI FLASH write & read at 0x%lx successful\r\n", address);
//            }
//        }
//    }
//------------------------------------------------------------------------------

    cfg.context = &storage;

    // block device operations
    cfg.read = [](const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) -> int
                {
                    auto nvm = static_cast<hal::nvm*>(c->context);
                    int result = nvm->read((std::byte*)buffer, block * c->block_size + off, size) ? LFS_ERR_OK : LFS_ERR_IO;
                    assert(result == LFS_ERR_OK);
                    return result;
                };
    cfg.prog = [](const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) -> int
                {
                    auto nvm = static_cast<hal::nvm*>(c->context);
                    int result = nvm->write((std::byte*)buffer, block * c->block_size + off, size) ? LFS_ERR_OK : LFS_ERR_IO;
                    assert(result == LFS_ERR_OK);
                    return result;
                };
    cfg.erase = [](const struct lfs_config *c, lfs_block_t block) -> int
                {
                    auto nvm = static_cast<hal::nvm*>(c->context);
                    int result = nvm->erase(block * c->block_size, c->block_size) ? LFS_ERR_OK : LFS_ERR_IO;
                    assert(result == LFS_ERR_OK);
                    return result;
                };
    cfg.sync = [](const struct lfs_config *c) -> int
               {
                    return LFS_ERR_OK; // No buffering so return
               };

    // block device configuration
    cfg.read_size = 1;
    cfg.prog_size = storage.prog_size();
    cfg.block_size = storage.erase_size();
    cfg.block_count = storage.total_size() / storage.erase_size();
    cfg.cache_size = cfg.prog_size;
    cfg.lookahead_size = 64;
    cfg.block_cycles = 512;

    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

     // Test file benchmark
    #define SIZE (128ul * 1024ul)
    #define CHUNK_SIZE (128ul)

    lfs_size_t chunks = (SIZE+CHUNK_SIZE-1)/CHUNK_SIZE;

    static std::array<uint8_t, CHUNK_SIZE> pattern;
    std::generate(pattern.begin(), pattern.end(), [](){ return hal::random::get() % 256; });

    // first write the file
    uint32_t start = hal::system::clock::cycles();
    assert(lfs_file_open(&lfs, &file, "file", LFS_O_WRONLY | LFS_O_CREAT) == 0);
    for (lfs_size_t i = 0; i < chunks; i++)
        assert(lfs_file_write(&lfs, &file, pattern.data(), CHUNK_SIZE) == CHUNK_SIZE);
    assert(lfs_file_write(&lfs, &file, pattern.data(), CHUNK_SIZE) == CHUNK_SIZE);
    assert(lfs_file_close(&lfs, &file) == 0);

    uint32_t duration = hal::system::clock::cycles() - start;
    constexpr uint32_t cycles_per_us = hal::system::system_clock / 1000000ul;
    printf("lfs write benchmark: %.2f MBit/s\r\n", (float)(SIZE * 8 * cycles_per_us) / (duration));

    // then read the file
    start = hal::system::clock::cycles();
    assert(lfs_file_open(&lfs, &file, "file", LFS_O_RDONLY) == 0);
    std::array<uint8_t, CHUNK_SIZE> buffer;
    for (lfs_size_t i = 0; i < chunks; i++)
    {
        assert(lfs_file_read(&lfs, &file, buffer.data(), CHUNK_SIZE) == CHUNK_SIZE);

        for (lfs_size_t j = 0; j < CHUNK_SIZE; j++)
            assert(buffer[j] == pattern[j]);
    }
    assert(lfs_file_close(&lfs, &file) == 0);
    duration = hal::system::clock::cycles() - start;
    printf("lfs read benchmark: %.1f MBit/s\r\n", (float)(SIZE * 8 * cycles_per_us) / (duration));

    assert(lfs_unmount(&lfs) == 0);
}

static void init_thread(void *arg)
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

    littlefs_test();

    osKernelInitialize();
    osThreadNew(init_thread, NULL, NULL);
    if (osKernelGetState() == osKernelReady)
        osKernelStart();

    assert(!"OS kernel start error");

    while (1);

    return 0;
}
