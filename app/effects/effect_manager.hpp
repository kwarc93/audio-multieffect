/*
 * effect_manager.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_EFFECT_MANAGER_HPP_
#define EFFECTS_EFFECT_MANAGER_HPP_

#include <middlewares/active_object.hpp>

#include <hal/hal_audio.hpp>

#include <variant>
#include <memory>

#include "effect_interface.hpp"

struct effect_manager_event
{
    struct process_data_evt_t
    {

    };

    struct add_effect_evt_t
    {
        effect_id id;
    };

    struct remove_effect_evt_t
    {
        effect_id id;
    };

    struct bypass_evt_t
    {
        effect_id id;
        bool bypassed;
    };

    using holder = std::variant<process_data_evt_t, add_effect_evt_t, remove_effect_evt_t, bypass_evt_t>;
};

class effect_manager : public effect_manager_event, public middlewares::active_object<effect_manager_event::holder>
{
public:
    effect_manager();
    ~effect_manager();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const add_effect_evt_t &e);
    void event_handler(const remove_effect_evt_t& e);
    void event_handler(const bypass_evt_t &e);
    void event_handler(const process_data_evt_t &e);

    std::unique_ptr<effect> create_new(effect_id id);
    bool find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it);

    std::vector<std::unique_ptr<effect>> effects;

    hal::audio_devices::codec audio;
};

#endif /* EFFECTS_EFFECT_MANAGER_HPP_ */
