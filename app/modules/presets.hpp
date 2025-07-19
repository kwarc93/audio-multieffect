/*
 * presets.hpp
 *
 *  Created on: 17 maj 2025
 *      Author: kwarc
 */

#ifndef PRESETS_HPP_
#define PRESETS_HPP_

#include <memory>
#include <cstdint>
#include <optional>
#include <functional>
#include <string_view>

#include "nlohmann/json_fwd.hpp"

#include "app/model/effect_features.hpp"

#include "presets_storage.hpp"

class presets_manager
{
public:
    typedef std::function<void(mfx::effect_id id, const char *name, bool bypassed, const mfx::effect_controls &ctrl)> effect_cb;

    presets_manager(std::unique_ptr<presets_storage> ps);

    bool verify(std::string_view name);
    bool remove(std::string_view name);
    bool load(std::string_view name, effect_cb cb);
    void create(std::string_view name);
    void add(mfx::effect_id id, const char *name, bool bypassed, const mfx::effect_controls &ctrl);
    bool save(void);

private:
    nlohmann::json tmp;
    std::unique_ptr<presets_storage> storage;
};

#endif /* PRESETS_HPP_ */
