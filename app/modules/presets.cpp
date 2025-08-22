/*
 * presets.cpp
 *
 *  Created on: 19 lip 2025
 *      Author: kwarc
 */

#define JSON_NO_IO
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "nlohmann/json.hpp"

#include "presets.hpp"

//-----------------------------------------------------------------------------
/* helpers */

using json = nlohmann::json;

// JSON serializers/deserializers for effects
namespace mfx
{

// tuner
void from_json(const json& j, tuner_attr::controls& c)
{
    const auto& def = tuner_attr::default_ctrl;
    c.a4_tuning = j.value("a4_tuning", def.a4_tuning);
}

void to_json(json& j, const tuner_attr::controls& c)
{
    j = json{ {"a4_tuning", c.a4_tuning} };
}

// tremolo
void from_json(const json& j, tremolo_attr::controls& c)
{
    const auto& def = tremolo_attr::default_ctrl;
    c.rate = j.value("rate", def.rate);
    c.depth = j.value("depth", def.depth);
    c.shape = j.value("shape", def.shape);
}

void to_json(json& j, const tremolo_attr::controls& c)
{
    j = json{ {"rate", c.rate}, {"depth", c.depth}, {"shape", c.shape} };
}

// echo
void from_json(const json& j, echo_attr::controls& c)
{
    const auto& def = echo_attr::default_ctrl;
    c.blur = j.value("blur", def.blur);
    c.time = j.value("time", def.time);
    c.feedback = j.value("feedback", def.feedback);
    c.mode = j.value("mode", def.mode);
}

void to_json(json& j, const echo_attr::controls& c)
{
    j = json{ {"blur", c.blur}, {"time", c.time}, {"feedback", c.feedback}, {"mode", c.mode} };
}

// chorus
void from_json(const json& j, chorus_attr::controls& c)
{
    const auto& def = chorus_attr::default_ctrl;
    c.depth = j.value("depth", def.depth);
    c.rate = j.value("rate", def.rate);
    c.tone = j.value("tone", def.tone);
    c.mix = j.value("mix", def.mix);
    c.mode = j.value("mode", def.mode);
}

void to_json(json& j, const chorus_attr::controls& c)
{
    j = json{ {"depth", c.depth}, {"rate", c.rate}, {"tone", c.tone}, {"mix", c.mix}, {"mode", c.mode} };
}

// reverb
void from_json(const json& j, reverb_attr::controls& c)
{
    const auto& def = reverb_attr::default_ctrl;
    c.bandwidth = j.value("bandwidth", def.bandwidth);
    c.damping = j.value("damping", def.damping);
    c.decay = j.value("decay", def.decay);
    c.mode = j.value("mode", def.mode);
}

void to_json(json& j, const reverb_attr::controls& c)
{
    j = json{ {"bandwidth", c.bandwidth}, {"damping", c.damping}, {"decay", c.decay}, {"mode", c.mode} };
}

// overdrive
void from_json(const json& j, overdrive_attr::controls& c)
{
    const auto& def = overdrive_attr::default_ctrl;
    c.low = j.value("low", def.low);
    c.gain = j.value("gain", def.gain);
    c.high = j.value("high", def.high);
    c.mix = j.value("mix", def.mix);
    c.mode = j.value("mode", def.mode);
}

void to_json(json& j, const overdrive_attr::controls& c)
{
    j = json{ {"low", c.low}, {"gain", c.gain}, {"high", c.high}, {"mix", c.mix}, {"mode", c.mode} };
}

// cabinet_sim
void from_json(const json& j, cabinet_sim_attr::controls& c)
{
    const auto& def = cabinet_sim_attr::default_ctrl;
    c.ir_idx = j.value("ir_idx", def.ir_idx);
    c.ir_res = j.value("ir_res", def.ir_res);
}

void to_json(json& j, const cabinet_sim_attr::controls& c)
{
    j = json{ {"ir_idx", c.ir_idx}, {"ir_res", c.ir_res} };
}

// vocoder
void from_json(const json& j, vocoder_attr::controls& c)
{
    const auto& def = vocoder_attr::default_ctrl;
    c.bands = j.value("bands", def.bands);
    c.clarity = j.value("clarity", def.clarity);
    c.tone = j.value("tone", def.tone);
    c.hold = j.value("hold", def.hold);
    c.mode = j.value("mode", def.mode);
}

void to_json(json& j, const vocoder_attr::controls& c)
{
    j = json{ {"bands", c.bands}, {"clarity", c.clarity}, {"tone", c.tone}, {"hold", c.hold}, {"mode", c.mode} };
}

// phaser
void from_json(const json& j, phaser_attr::controls& c)
{
    const auto& def = phaser_attr::default_ctrl;
    c.rate = j.value("rate", def.rate);
    c.depth = j.value("depth", def.depth);
    c.contour = j.value("contour", def.contour);
}

void to_json(json& j, const phaser_attr::controls& c)
{
    j = json{ {"rate", c.rate}, {"depth", c.depth}, {"contour", c.contour} };
}

std::optional<effect_controls> get_controls(effect_id id, const json& ctrl_json)
{
    switch (id)
    {
        case mfx::effect_id::tuner:
            return ctrl_json.get<mfx::tuner_attr::controls>();
        case mfx::effect_id::tremolo:
            return ctrl_json.get<mfx::tremolo_attr::controls>();
        case mfx::effect_id::echo:
            return ctrl_json.get<mfx::echo_attr::controls>();
        case mfx::effect_id::chorus:
            return ctrl_json.get<mfx::chorus_attr::controls>();
        case mfx::effect_id::reverb:
            return ctrl_json.get<mfx::reverb_attr::controls>();
        case mfx::effect_id::overdrive:
            return ctrl_json.get<mfx::overdrive_attr::controls>();
        case mfx::effect_id::cabinet_sim:
            return ctrl_json.get<mfx::cabinet_sim_attr::controls>();
        case mfx::effect_id::vocoder:
            return ctrl_json.get<mfx::vocoder_attr::controls>();
        case mfx::effect_id::phaser:
            return ctrl_json.get<mfx::phaser_attr::controls>();
        default:
            return std::nullopt;
    }
}

}

//-----------------------------------------------------------------------------
/* private */

std::string presets_manager::append_extension(std::string_view name) const
{
    return std::string(name) + ".cbor";
}

//-----------------------------------------------------------------------------
/* public */

presets_manager::presets_manager(std::unique_ptr<presets_storage> ps) : storage(std::move(ps))
{

}

void presets_manager::list(std::vector<std::string> &names)
{
    this->storage->list(names);
}

bool presets_manager::verify(std::string_view name)
{
    bool result = false;
    std::vector<uint8_t> data;

    result = this->storage->load(this->append_extension(name), data);

    if (result)
    {
        /* Parse from CBOR without exceptions */
        auto obj = json::from_cbor(data, true, false);
        result &= !obj.is_discarded();
        result &= obj.contains("effects");

        // TODO: Check "version" field
    }

    return result;
}

bool presets_manager::remove(std::string_view name)
{
    return this->storage->remove(this->append_extension(name));
}

bool presets_manager::rename(std::string_view old_name, std::string_view new_name)
{
    return this->storage->rename(this->append_extension(old_name), this->append_extension(new_name));
}

bool presets_manager::load(std::string_view name, effect_cb cb)
{
    bool result = false;
    std::vector<uint8_t> data;

    result = this->storage->load(this->append_extension(name), data);
    if (result)
    {
        /* Parse from CBOR without exceptions */
        auto obj = json::from_cbor(data, true, false);
        result &= !obj.is_discarded();

        if (obj.contains("effects"))
        {
            for (const auto &e : obj["effects"])
            {
                const mfx::effect_id id = e.value("id", mfx::effect_id::_count);
                const std::string name = e.value("name", "unknown");
                const bool bypassed = e.value("bypassed", true);

                if (const auto controls = mfx::get_controls(id, e["ctrl"]))
                    cb(id, name.c_str(), bypassed, *controls);
            }
        }
    }

    return result;
}

void presets_manager::create(std::string_view name)
{
    this->tmp.clear();
    this->tmp["version"] = 0;
    this->tmp["name"] = name;
    this->tmp["effects"] = json::array();
}

void presets_manager::add(mfx::effect_id id, const char *name, bool bypassed, const mfx::effect_controls &ctrl)
{
    if (this->tmp.empty())
        return;

    json j { {"id", id}, {"name", name}, {"bypassed", bypassed} };

    std::visit([&j](auto&& arg) { j["ctrl"] = arg; }, ctrl);

    this->tmp["effects"].push_back(j);
}

bool presets_manager::save(void)
{
    if (this->tmp.empty() || this->tmp["effects"].empty())
        return false;

    bool result = this->storage->save(this->append_extension(this->tmp["name"].get<std::string>()), json::to_cbor(this->tmp));
    this->tmp.clear();
    return result;
}
