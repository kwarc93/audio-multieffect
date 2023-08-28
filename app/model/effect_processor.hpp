/*
 * effect_processor.hpp
 *
 *  Created on: 4 sty 2023
 *      Author: kwarc
 */

#ifndef MODEL_EFFECT_PROCESSOR_HPP_
#define MODEL_EFFECT_PROCESSOR_HPP_

#include <middlewares/active_object.hpp>

#include <hal/hal_audio.hpp>

#include <variant>
#include <memory>
#include <array>

#include "effect_interface.hpp"

#include "app/model/equalizer/equalizer.hpp"
#include "app/model/noise_gate/noise_gate.hpp"
#include "app/model/tremolo/tremolo.hpp"

namespace mfx
{

struct effect_processor_event
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

    struct effect_controls_evt_t
    {
        std::variant<equalizer::controls, noise_gate::controls, tremolo::controls> controls;
    };

    using holder = std::variant<process_data_evt_t, add_effect_evt_t, remove_effect_evt_t, bypass_evt_t, effect_controls_evt_t>;
};

class effect_processor : public effect_processor_event, public middlewares::active_object<effect_processor_event::holder>
{
public:
    effect_processor();
    ~effect_processor();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const add_effect_evt_t &e);
    void event_handler(const remove_effect_evt_t& e);
    void event_handler(const bypass_evt_t &e);
    void event_handler(const process_data_evt_t &e);
    void event_handler(const effect_controls_evt_t &e);

    std::unique_ptr<effect> create_new(effect_id id);
    bool find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it);

    void audio_capture_cb(const hal::audio_devices::codec::input_sample_t *input, uint16_t length);
    void audio_play_cb(uint16_t sample_index);

    std::vector<std::unique_ptr<effect>> effects;

    hal::audio_devices::codec audio;
    hal::audio_devices::codec::input_buffer_t<256> audio_input;
    hal::audio_devices::codec::output_buffer_t<256> audio_output;

    dsp_input_t dsp_input;
    dsp_output_t dsp_output;
};

}

#endif /* MODEL_EFFECT_PROCESSOR_HPP_ */
