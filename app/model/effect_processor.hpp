/*
 * effect_processor.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EFFECT_PROCESSOR_HPP_
#define MODEL_EFFECT_PROCESSOR_HPP_

#include <functional>
#include <variant>
#include <memory>
#include <array>

#include <middlewares/active_object.hpp>
#include <middlewares/observer.hpp>

#include <hal/hal_audio.hpp>

#include "effect_interface.hpp"

namespace mfx
{

namespace effect_processor_events
{

struct process_data
{

};

struct get_processing_load
{
    std::function<void(uint8_t load)> response;
};

struct add_effect
{
    effect_id id;
};

struct remove_effect
{
    effect_id id;
};

struct move_effect
{
    effect_id id;
    int32_t step;
};

struct bypass_effect
{
    effect_id id;
    bool bypassed;
};

struct set_volume
{
    uint8_t input_vol;
    uint8_t output_vol;
};

struct set_mute
{
    bool value;
};

struct set_effect_controls
{
    std::variant
    <
        tremolo_attr::controls,
        echo_attr::controls,
        chorus_attr::controls,
        overdrive_attr::controls,
        cabinet_sim_attr::controls
    >
    ctrl;
};

struct get_effect_attributes
{
    effect_id id;
    std::function<void(const effect_attr &basic, const effect_specific_attributes &specific)> response;
};

struct effect_attributes_changed
{
    effect_attr basic;
    effect_specific_attributes specific;
};

using volume_changed = set_volume;

using incoming = std::variant
<
    process_data,
    get_processing_load,
    add_effect,
    remove_effect,
    move_effect,
    bypass_effect,
    set_volume,
    set_mute,
    set_effect_controls,
    get_effect_attributes
>;

using outgoing = std::variant
<
    volume_changed,
    effect_attributes_changed
>;

}

class effect_processor : public middlewares::active_object<effect_processor_events::incoming>,
                         public middlewares::subject<effect_processor_events::outgoing>
{
public:
    effect_processor();
    ~effect_processor();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const effect_processor_events::add_effect &e);
    void event_handler(const effect_processor_events::remove_effect& e);
    void event_handler(const effect_processor_events::move_effect& e);
    void event_handler(const effect_processor_events::bypass_effect &e);
    void event_handler(const effect_processor_events::set_volume &e);
    void event_handler(const effect_processor_events::set_mute &e);
    void event_handler(const effect_processor_events::process_data &e);
    void event_handler(const effect_processor_events::get_processing_load &e);
    void event_handler(const effect_processor_events::set_effect_controls &e);
    void event_handler(const effect_processor_events::get_effect_attributes &e);

    void set_controls(const tremolo_attr::controls &ctrl);
    void set_controls(const echo_attr::controls &ctrl);
    void set_controls(const chorus_attr::controls &ctrl);
    void set_controls(const overdrive_attr::controls &ctrl);
    void set_controls(const cabinet_sim_attr::controls &ctrl);

    void notify_effect_attributes_changed(effect *eff);

    std::unique_ptr<effect> create_new(effect_id id);
    bool find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it);
    effect* find_effect(effect_id id);

    void audio_capture_cb(const hal::audio_devices::codec::input_sample_t *input, uint16_t length);
    void audio_play_cb(uint16_t sample_index);

    uint8_t get_processing_load(void);

    std::vector<std::unique_ptr<effect>> effects;

    hal::audio_devices::codec audio;
    hal::audio_devices::codec::input_buffer_t<2 * config::dsp_vector_size> audio_input;
    hal::audio_devices::codec::output_buffer_t<2 * config::dsp_vector_size> audio_output;

    effect::dsp_input dsp_input;
    effect::dsp_output dsp_output;

    uint32_t processing_time_us;
};

}

#endif /* MODEL_EFFECT_PROCESSOR_HPP_ */
