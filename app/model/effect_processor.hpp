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

#include <hal_audio.hpp>

#include "effect_interface.hpp"

namespace mfx
{

namespace effect_processor_events
{

struct initialize
{

};

struct ipc_data
{
    /* Used only in dual-core configuration */
};

struct shutdown
{

};

struct configuration
{
    uint8_t main_input_vol;
    uint8_t aux_input_vol;
    uint8_t output_vol;
    bool output_muted;
    bool mic_routed_to_aux;
};

struct start_audio
{

};

struct process_audio
{

};

struct get_dsp_load
{

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

struct set_input_volume
{
    uint8_t main_input_vol;
    uint8_t aux_input_vol;
};

struct set_output_volume
{
    uint8_t output_vol;
};

struct route_mic_to_aux
{
    bool value;
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
        reverb_attr::controls,
        overdrive_attr::controls,
        cabinet_sim_attr::controls,
        vocoder_attr::controls,
        phaser_attr::controls
    >
    ctrl;
};

struct get_effect_attributes
{
    effect_id id;
};

struct effect_attributes_changed
{
    effect_attr basic;
    effect_specific_attributes specific;
};

struct dsp_load_changed
{
    uint8_t load_pct;
};

using input_volume_changed = set_input_volume;
using output_volume_changed = set_output_volume;

using incoming = std::variant
<
    initialize,
    ipc_data,
    shutdown,
    configuration,
    start_audio,
    process_audio,
    get_dsp_load,
    add_effect,
    remove_effect,
    move_effect,
    bypass_effect,
    set_input_volume,
    set_output_volume,
    route_mic_to_aux,
    set_mute,
    set_effect_controls,
    get_effect_attributes
>;

using outgoing = std::variant
<
    dsp_load_changed,
    input_volume_changed,
    output_volume_changed,
    effect_attributes_changed
>;

}

class effect_processor_base : public middlewares::active_object<effect_processor_events::incoming>,
                              public middlewares::subject<effect_processor_events::outgoing>
{
public:
    effect_processor_base() : active_object("effect_processor", osPriorityHigh, 4096) {}
private:
    virtual void dispatch(const event &e) {};
};

class effect_processor : public effect_processor_base
{
public:
    effect_processor();
    ~effect_processor();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const effect_processor_events::initialize &e);
    void event_handler(const effect_processor_events::ipc_data &e){};
    void event_handler(const effect_processor_events::shutdown &e);
    void event_handler(const effect_processor_events::configuration &e);
    void event_handler(const effect_processor_events::start_audio &e);
    void event_handler(const effect_processor_events::add_effect &e);
    void event_handler(const effect_processor_events::remove_effect& e);
    void event_handler(const effect_processor_events::move_effect& e);
    void event_handler(const effect_processor_events::bypass_effect &e);
    void event_handler(const effect_processor_events::set_input_volume &e);
    void event_handler(const effect_processor_events::set_output_volume &e);
    void event_handler(const effect_processor_events::route_mic_to_aux &e);
    void event_handler(const effect_processor_events::set_mute &e);
    void event_handler(const effect_processor_events::process_audio &e);
    void event_handler(const effect_processor_events::get_dsp_load &e);
    void event_handler(const effect_processor_events::set_effect_controls &e);
    void event_handler(const effect_processor_events::get_effect_attributes &e);

    void set_controls(const tremolo_attr::controls &ctrl);
    void set_controls(const echo_attr::controls &ctrl);
    void set_controls(const chorus_attr::controls &ctrl);
    void set_controls(const reverb_attr::controls &ctrl);
    void set_controls(const overdrive_attr::controls &ctrl);
    void set_controls(const cabinet_sim_attr::controls &ctrl);
    void set_controls(const vocoder_attr::controls &ctrl);
    void set_controls(const phaser_attr::controls &ctrl);

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

    effect::dsp_input dsp_main_input;
    effect::dsp_input dsp_aux_input;
    effect::dsp_output dsp_output;

    uint32_t processing_time_us;
};

}

#endif /* MODEL_EFFECT_PROCESSOR_HPP_ */
