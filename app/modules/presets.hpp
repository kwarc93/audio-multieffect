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

    void list(std::vector<std::string> &names);
    bool verify(std::string_view name);
    bool rename(std::string_view old_name, std::string_view new_name);
    bool remove(std::string_view name);
    bool load(std::string_view name, effect_cb cb);

    /* These three methods are used together to create a new preset */
    void create(std::string_view name);
    void add(mfx::effect_id id, const char *name, bool bypassed, const mfx::effect_controls &ctrl);
    bool save(void);

private:
    std::string append_extension(std::string_view name) const;

    nlohmann::json tmp;
    std::unique_ptr<presets_storage> storage;
};

#endif /* PRESETS_HPP_ */
