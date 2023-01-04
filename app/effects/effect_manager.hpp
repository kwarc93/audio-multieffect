/*
 * effect_manager.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef EFFECTS_EFFECT_MANAGER_HPP_
#define EFFECTS_EFFECT_MANAGER_HPP_

#include "middlewares/active_object.hpp"

#include <variant>
#include <vector>
#include <memory>
#include <map>

#include "effect_interface.hpp"

struct effect_manager_event
{
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

    using holder = std::variant<add_effect_evt_t, remove_effect_evt_t, bypass_evt_t>;
};

class effect_manager : public effect_manager_event, public ao::active_object<effect_manager_event::holder>
{
public:
    effect_manager();
    ~effect_manager();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const add_effect_evt_t &e);
    void event_handler(const remove_effect_evt_t &e);
    void event_handler(const bypass_evt_t &e);

    std::vector<std::unique_ptr<effect>> effects;

//    std::map<effect_id, std::function<std::unique_ptr<effect>()>> effects_map;
};

#endif /* EFFECTS_EFFECT_MANAGER_HPP_ */
