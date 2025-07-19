/*
 * presets_storage.hpp
 *
 *  Created on: 17 maj 2025
 *      Author: kwarc
 */

#ifndef PRESETS_STORAGE_HPP_
#define PRESETS_STORAGE_HPP_

#include <vector>
#include <cstdint>
#include <string>

#include "libs/littlefs/lfs.h"
#include "middlewares/filesystem.hpp"

//------------------------------------------------------------------------------

// Strategy pattern (for different presets storage types)
class presets_storage
{
public:
    virtual ~presets_storage() = default;

    virtual bool save(std::string_view name, const std::vector<std::uint8_t> &data) = 0;
    virtual bool load(std::string_view name, std::vector<std::uint8_t> &data) = 0;
    virtual bool remove(std::string_view name) = 0;
};

//------------------------------------------------------------------------------
// One of the ways of presets storage - filesystem
class presets_storage_file : public presets_storage
{
public:
    presets_storage_file(std::string_view directory) : dir(directory)
    {
        auto fs = &middlewares::filesystem::lfs;
        lfs_mkdir(fs, this->dir.c_str());
    }

    bool save(std::string_view name, const std::vector<std::uint8_t> &data) override
    {
        bool result = false;
        auto fs = &middlewares::filesystem::lfs;

        const std::string path = this->dir + "/" + std::string(name);

        lfs_file_t file;
        if (lfs_file_open(fs, &file, path.c_str(), LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) == LFS_ERR_OK)
        {
            const lfs_ssize_t bytes_to_write = data.size();
            result =  (lfs_file_write(fs, &file, data.data(), bytes_to_write) == bytes_to_write);
            result &= (lfs_file_close(fs, &file) == LFS_ERR_OK);
        }

        return result;
    }

    bool load(std::string_view name, std::vector<std::uint8_t> &data) override
    {
        bool result = false;
        auto fs = &middlewares::filesystem::lfs;

        data.clear();

        const std::string path = this->dir + "/" + std::string(name);

        lfs_file_t file;
        if (lfs_file_open(fs, &file, path.c_str(), LFS_O_RDONLY) == LFS_ERR_OK)
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

    bool remove(std::string_view name)
    {
        auto fs = &middlewares::filesystem::lfs;

        const std::string path = this->dir + "/" + std::string(name);

        return lfs_remove(fs, path.c_str()) == LFS_ERR_OK;
    }
private:
    std::string dir;
};

//------------------------------------------------------------------------------
// One of the ways of presets storage - EEPROM
class presets_storage_eeprom : public presets_storage
{

};

//------------------------------------------------------------------------------
// One of the ways of presets storage - RAM
class presets_storage_ram : public presets_storage
{

};

#endif /* PRESETS_STORAGE_HPP_ */
