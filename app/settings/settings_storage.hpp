/*
 * settings_storage.hpp
 *
 *  Created on: 17 maj 2025
 *      Author: kwarc
 */

#ifndef SETTINGS_STORAGE_HPP_
#define SETTINGS_STORAGE_HPP_

#include <vector>
#include <cstdint>

#include "libs/littlefs/lfs.h"
#include "middlewares/filesystem.hpp"

//------------------------------------------------------------------------------

// Strategy pattern (for different settings storage types)
class settings_storage
{
public:
    virtual ~settings_storage() = default;

    virtual bool save(const std::vector<std::uint8_t> &data) = 0;
    virtual std::vector<std::uint8_t> load(void) = 0;
};

//------------------------------------------------------------------------------
// One of the ways of settings storage - filesystem
class settings_storage_file : public settings_storage
{
public:
    settings_storage_file(const std::string &filename) : file_name(filename)
    {

    }

    bool save(const std::vector<std::uint8_t> &data) override
    {
        bool result = false;
        auto fs = &middlewares::filesystem::lfs;

        if (lfs_file_open(fs, &this->file, this->file_name.c_str(), LFS_O_WRONLY | LFS_O_CREAT) == 0)
        {
            const lfs_ssize_t bytes_to_write = data.size();
            if (lfs_file_write(fs, &this->file, data.data(), bytes_to_write) == bytes_to_write)
            {
                result = true;
            }

            result &= lfs_file_close(fs, &this->file) == 0;
        }

        return result;
    }

    std::vector<std::uint8_t> load(void) override
    {
        auto fs = &middlewares::filesystem::lfs;
        if (lfs_file_open(fs, &this->file, this->file_name.c_str(), LFS_O_RDONLY) == 0)
        {
            const lfs_soff_t size = lfs_file_size(fs, &this->file);
            std::vector<std::uint8_t> contents(size);
            if (lfs_file_read(fs, &file, contents.data(), size) == size)
            {
                lfs_file_close(fs, &this->file);
                return contents;
            }

            lfs_file_close(fs, &this->file);
        }

        return std::vector<std::uint8_t>(); // File error
    }
private:
    lfs_file_t file;
    std::string file_name;
};

//------------------------------------------------------------------------------
// One of the ways of settings storage - EEPROM
class settings_storage_eeprom : public settings_storage
{
    // TODO
};

//------------------------------------------------------------------------------
// One of the ways of settings storage - RAM
class settings_storage_ram : public settings_storage
{
    // TODO
};

#endif /* SETTINGS_STORAGE_HPP_ */
