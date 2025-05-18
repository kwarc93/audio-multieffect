/*
 * filesystem.hpp
 *
 *  Created on: 21 lis 2024
 *      Author: kwarc
 */

#ifndef FILESYSTEM_HPP_
#define FILESYSTEM_HPP_

#include <cstdio>
#include <cassert>

#include "libs/littlefs/lfs.h"

#include <hal_system.hpp>
#include <hal_sdram.hpp>
#include <hal/hal_nvm.hpp>
#include <hal/hal_random.hpp>
#include <hal/hal_button.hpp>

namespace middlewares
{

namespace filesystem
{

inline lfs_t lfs;
inline lfs_config lfs_cfg;

inline void init(void)
{
    printf("Initializing filesystem...\r\n");

    static hal::nvms::qspi_flash storage;

    lfs_cfg.context = &storage;

    // block device operations
    lfs_cfg.read = [](const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) -> int
                   {
                       auto nvm = static_cast<hal::nvm*>(c->context);
                       int result = nvm->read((std::byte*)buffer, block * c->block_size + off, size) ? LFS_ERR_OK : LFS_ERR_IO;
                       assert(result == LFS_ERR_OK);
                       return result;
                   };
    lfs_cfg.prog = [](const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) -> int
                   {
                       auto nvm = static_cast<hal::nvm*>(c->context);
                       int result = nvm->write((std::byte*)buffer, block * c->block_size + off, size) ? LFS_ERR_OK : LFS_ERR_IO;
                       assert(result == LFS_ERR_OK);
                       return result;
                   };
    lfs_cfg.erase = [](const struct lfs_config *c, lfs_block_t block) -> int
                    {
                        auto nvm = static_cast<hal::nvm*>(c->context);
                        int result = nvm->erase(block * c->block_size, c->block_size) ? LFS_ERR_OK : LFS_ERR_IO;
                        assert(result == LFS_ERR_OK);
                        return result;
                    };
    lfs_cfg.sync = [](const struct lfs_config *c) -> int
                   {
                        return LFS_ERR_OK; // no buffering so return
                   };

    // block device configuration
    lfs_cfg.read_size = 1;
    lfs_cfg.prog_size = storage.prog_size();
    lfs_cfg.block_size = storage.erase_size();
    lfs_cfg.block_count = storage.total_size() / storage.erase_size();
    lfs_cfg.cache_size = lfs_cfg.prog_size;
    lfs_cfg.lookahead_size = 64;
    lfs_cfg.block_cycles = 512;

    // mount the filesystem
    int err = lfs_mount(&lfs, &lfs_cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err)
    {
        printf("Failed to mount filesystem, formatting...\r\n");
        lfs_format(&lfs, &lfs_cfg);
        err = lfs_mount(&lfs, &lfs_cfg);
    }

    printf("Filesystem initialization %s\r\n", err == LFS_ERR_OK ? "failed" : "successful");
}

inline void test(void)
{
    hal::random::enable(true);
    hal::buttons::blue_btn button;
    hal::nvms::qspi_flash storage;

    printf("Starting file system test...\r\n");

    // test file benchmark
    #define SIZE (128ul * 1024ul)
    #define CHUNK_SIZE (128ul)

    lfs_size_t chunks = (SIZE+CHUNK_SIZE-1)/CHUNK_SIZE;

    static std::array<uint8_t, CHUNK_SIZE> pattern;
    std::generate(pattern.begin(), pattern.end(), [](){ return hal::random::get() % 256; });

    // first write the file
    lfs_file_t file;
    uint32_t start = hal::system::clock::cycles();
    assert(lfs_file_open(&lfs, &file, "file", LFS_O_WRONLY | LFS_O_CREAT) == LFS_ERR_OK);
    for (lfs_size_t i = 0; i < chunks; i++)
        assert(lfs_file_write(&lfs, &file, pattern.data(), CHUNK_SIZE) == CHUNK_SIZE);
    assert(lfs_file_write(&lfs, &file, pattern.data(), CHUNK_SIZE) == CHUNK_SIZE);
    assert(lfs_file_close(&lfs, &file) == LFS_ERR_OK);

    uint32_t duration = hal::system::clock::cycles() - start;
    constexpr uint32_t cycles_per_us = hal::system::system_clock / 1000000ul;
    printf("lfs write benchmark: %.2f MBit/s\r\n", (float)(SIZE * 8 * cycles_per_us) / (duration));

    // then read the file
    start = hal::system::clock::cycles();
    assert(lfs_file_open(&lfs, &file, "file", LFS_O_RDONLY) == LFS_ERR_OK);
    std::array<uint8_t, CHUNK_SIZE> buffer;
    for (lfs_size_t i = 0; i < chunks; i++)
    {
        assert(lfs_file_read(&lfs, &file, buffer.data(), CHUNK_SIZE) == CHUNK_SIZE);

        for (lfs_size_t j = 0; j < CHUNK_SIZE; j++)
            assert(buffer[j] == pattern[j]);
    }
    assert(lfs_file_close(&lfs, &file) == LFS_ERR_OK);
    duration = hal::system::clock::cycles() - start;
    printf("lfs read benchmark: %.1f MBit/s\r\n", (float)(SIZE * 8 * cycles_per_us) / (duration));

    assert(lfs_unmount(&lfs) == LFS_ERR_OK);

    printf("File system test done\r\n");
}

inline void memtest(void)
{
    hal::random::enable(true);
    hal::buttons::blue_btn button;
    hal::nvms::qspi_flash storage;

    // FLASH erase
    if (button.is_pressed())
    {
        printf("Erasing QSPI FLASH...\r\n");
        const bool result = storage.erase();
        printf("QSPI FLASH erasing %s\r\n", result ? "done" : "error");
    }

    printf("Starting QSPI FLASH memory test...\r\n");

    // random subsector test
    constexpr size_t subsector_size = 4096;
    uint32_t address = hal::random::get() % storage.total_size();
    address = address & ~(subsector_size - 1);
    if (storage.erase(address, subsector_size))
    {
        static std::array<std::byte, subsector_size> data;
        std::generate(data.begin(), data.end(), [](){ return (std::byte)(hal::random::get() % 256); });

        if (storage.write(data.data(), address, data.size()))
        {
            static std::array<std::byte, subsector_size> readback;
            if (storage.read(readback.data(), address, readback.size()))
            {
                for (unsigned i = 0; i < subsector_size; i++)
                    assert(data[i] == readback[i]);

                printf("QSPI FLASH write & read at 0x%lx successful\r\n", address);
            }
        }
    }
}

}

}

#endif /* FILESYSTEM_HPP_ */
