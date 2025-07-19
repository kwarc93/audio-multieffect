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
#include <string>

#include "libs/littlefs/lfs.h"
#include "middlewares/filesystem.hpp"

//------------------------------------------------------------------------------

// Strategy pattern (for different settings storage types)
class settings_storage
{
public:
    virtual ~settings_storage() = default;

    virtual bool save(const std::vector<std::uint8_t> &data) = 0;
    virtual bool load(std::vector<std::uint8_t> &data) = 0;
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

        lfs_file_t file;
        if (lfs_file_open(fs, &file, this->file_name.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) == LFS_ERR_OK)
        {
            const lfs_ssize_t bytes_to_write = data.size();
            result =  (lfs_file_write(fs, &file, data.data(), bytes_to_write) == bytes_to_write);
            result &= (lfs_file_close(fs, &file) == LFS_ERR_OK);
        }

        return result;
    }

    bool load(std::vector<std::uint8_t> &data) override
    {
        bool result = false;
        auto fs = &middlewares::filesystem::lfs;

        data.clear();

        lfs_file_t file;
        if (lfs_file_open(fs, &file, this->file_name.c_str(), LFS_O_RDONLY) == LFS_ERR_OK)
        {
            const lfs_soff_t size = lfs_file_size(fs, &file);
            if (size > 0)
            {
                data.resize(size);
                result =  (lfs_file_read(fs, &file, data.data(), size) == size);
                result &= (lfs_file_close(fs, &file) == LFS_ERR_OK);
            }
        }

        return result;
    }
private:
    std::string file_name;
};

//------------------------------------------------------------------------------
// One of the ways of settings storage - EEPROM
class settings_storage_eeprom : public settings_storage
{

};

//------------------------------------------------------------------------------
// One of the ways of settings storage - RAM
class settings_storage_ram : public settings_storage
{

};

#endif /* SETTINGS_STORAGE_HPP_ */
