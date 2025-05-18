/*
 * settings.hpp
 *
 *  Created on: 17 maj 2025
 *      Author: kwarc
 */

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

#include <string>
#include <memory>

#include "settings_storage.hpp"

#define JSON_NO_IO
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class settings_manager
{
public:
    settings_manager(std::unique_ptr<settings_storage> ss) : storage(std::move(ss))
    {
        // Set default values
        this->settings[k_boot_counter] = unsigned(0);

        // Save default values if load failed
        if (!this->load())
        {
            this->save();
        }
    }

    bool load(void)
    {
        std::vector<uint8_t> data;
        bool result = this->storage->load(data);
        if (result)
            this->settings = json::from_cbor(data);
        result &= this->settings.is_object();

        return result;
    }

    bool save(void)
    {
        const auto binary = json::to_cbor(this->settings);
        return this->storage->save(binary);
    }

    std::string dump(void)
    {
        return this->settings.dump(4);
    }

    unsigned get_boot_counter(void) { return this->settings[k_boot_counter].get<unsigned>(); };
    void set_boot_counter(unsigned volume) { this->settings[k_boot_counter] = volume; }

private:
    json settings;
    std::unique_ptr<settings_storage> storage;

    // Setting keys
    static constexpr const char *k_boot_counter = "boot_cnt";
};

#endif /* SETTINGS_HPP_ */
