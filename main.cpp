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

#include "libs/littlefs/lfs.h"

static void littlefs_test(void)
{
    // variables used by the filesystem
    static lfs_t lfs;
    static lfs_file_t file;
    static struct lfs_config cfg;

    hal::nvms::qspi_flash storage;

    // configuration of the filesystem is provided by this struct

    cfg.context = &storage;
    // block device operations
    cfg.read = [](const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) -> int
                {
                    auto nvm = static_cast<hal::nvm*>(c->context);
                    return nvm->read((std::byte*)buffer, block * c->block_size + off, size) ? LFS_ERR_OK : LFS_ERR_IO;
                };
    cfg.prog = [](const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) -> int
                {
                    auto nvm = static_cast<hal::nvm*>(c->context);
                    return nvm->write((std::byte*)buffer, block * c->block_size + off, size) ? LFS_ERR_OK : LFS_ERR_IO;
                };
    cfg.erase = [](const struct lfs_config *c, lfs_block_t block) -> int
                {
                    auto nvm = static_cast<hal::nvm*>(c->context);
                    return nvm->erase(block * c->block_size, c->block_size) ? LFS_ERR_OK : LFS_ERR_IO;
                };
    cfg.sync = [](const struct lfs_config *c) -> int
               {
                    return LFS_ERR_OK; // No buffering so return
               };

    // block device configuration
    cfg.read_size = 1;
    cfg.prog_size = 1;
    cfg.block_size = 4096;
    cfg.block_count = 512; // 2 MB
    cfg.cache_size = 16;
    cfg.lookahead_size = 16;
    cfg.block_cycles = 500;

    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %lu\n", boot_count);
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

