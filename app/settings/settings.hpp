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
#include <cstdint>

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
        this->settings[k_dark_mode] = bool(true);
        this->settings[k_displ_brightness] = uint8_t(100);

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
            this->settings.update(json::from_cbor(data));
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

    // TOOD: Add checks for safer access & modification
    unsigned get_boot_counter(void) { return this->settings[k_boot_counter].get<unsigned>(); };
    void set_boot_counter(unsigned volume) { this->settings[k_boot_counter] = volume; }

    bool get_dark_mode(void) { return this->settings[k_dark_mode].get<bool>(); };
    void set_dark_mode(bool enabled) { this->settings[k_dark_mode] = enabled; }

    uint8_t get_display_brightness(void) { return this->settings[k_displ_brightness].get<uint8_t>(); };
    void set_display_brightness(uint8_t value) { this->settings[k_displ_brightness] = value; }

private:
    json settings;
    std::unique_ptr<settings_storage> storage;

    // Setting keys
    static constexpr const char *k_boot_counter = "boot_cnt";
    static constexpr const char *k_dark_mode = "dark_mode";
    static constexpr const char *k_displ_brightness = "displ_brightness";
};

#endif /* SETTINGS_HPP_ */
