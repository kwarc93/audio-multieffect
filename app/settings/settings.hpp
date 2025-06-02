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
        this->settings[k_boot_counter] = 0;            // Boot counter
        this->settings[k_dark_mode] = true;            // GUI theme, values: true/false
        this->settings[k_displ_brightness] = 100;      // Display brightness, values: 0 - 100%
        this->settings[k_main_input_volume] = 11;      // MAIN input volume, values: 0 - 31 (-16.5dB to 30dB)
        this->settings[k_aux_input_volume] = 11;       // AUX input volume, values: 0 - 31 (-16.5dB to 30dB)
        this->settings[k_output_volume] = 57;          // Output volume, values: 0 - 63 (-57dB to 6dB)
        this->settings[k_output_muted] = false;        // Output muted, values: true/false
        this->settings[k_mic_routed_to_aux] = false;   // Route microphone signal to AUX, values: true/false

        // Save default values if load failed
        if (!this->load())
        {
            this->save();
        }

        this->dirty = false;
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
        if (!this->dirty)
            return true;

        const auto binary = json::to_cbor(this->settings);
        bool result = this->storage->save(binary);
        this->dirty = !result;
        return result;
    }

    std::string_view dump(void)
    {
        return this->settings.dump(4);
    }

    // TOOD: Add checks for safer access & modification
    unsigned get_boot_counter(void) { return this->settings[k_boot_counter].get<unsigned>(); };
    void set_boot_counter(unsigned value) { this->settings[k_boot_counter] = value; this->dirty = true; }

    bool get_dark_mode(void) { return this->settings[k_dark_mode].get<bool>(); };
    void set_dark_mode(bool value) { this->settings[k_dark_mode] = value; this->dirty = true; }

    uint8_t get_display_brightness(void) { return this->settings[k_displ_brightness].get<uint8_t>(); };
    void set_display_brightness(uint8_t value) { this->settings[k_displ_brightness] = value; this->dirty = true; }

    uint8_t get_main_input_volume(void) { return this->settings[k_main_input_volume].get<uint8_t>(); }
    void set_main_input_volume(uint8_t value) { this->settings[k_main_input_volume] = value; this->dirty = true; }

    uint8_t get_aux_input_volume(void) { return this->settings[k_aux_input_volume].get<uint8_t>(); }
    void set_aux_input_volume(uint8_t value) { this->settings[k_aux_input_volume] = value; this->dirty = true; }

    uint8_t get_output_volume(void) { return this->settings[k_output_volume].get<uint8_t>(); }
    void set_output_volume(uint8_t value) { this->settings[k_output_volume] = value; this->dirty = true; }

    bool get_output_muted(void) { return this->settings[k_output_muted].get<bool>(); };
    void set_output_muted(bool value) { this->settings[k_output_muted] = value; this->dirty = true; }

    bool get_mic_routed_to_aux(void) { return this->settings[k_mic_routed_to_aux].get<bool>(); };
    void set_mic_routed_to_aux(bool value) { this->settings[k_mic_routed_to_aux] = value; this->dirty = true; }

private:
    json settings;
    std::unique_ptr<settings_storage> storage;
    bool dirty;

    // Setting keys
    static constexpr const char *k_boot_counter = "boot_cnt";
    static constexpr const char *k_dark_mode = "dark_mode";
    static constexpr const char *k_displ_brightness = "displ_brightness";
    static constexpr const char *k_main_input_volume = "main_input_volume";
    static constexpr const char *k_aux_input_volume = "aux_input_volume";
    static constexpr const char *k_output_volume = "output_volume";
    static constexpr const char *k_output_muted = "output_muted";
    static constexpr const char *k_mic_routed_to_aux = "mic_routed_to_aux";
};

#endif /* SETTINGS_HPP_ */
