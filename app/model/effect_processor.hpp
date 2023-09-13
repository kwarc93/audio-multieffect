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

#include <functional>
#include <variant>
#include <memory>
#include <array>

#include "effect_interface.hpp"

#include "app/model/tremolo/tremolo.hpp"
#include "app/model/echo/echo.hpp"
#include "app/model/overdrive/overdrive.hpp"
#include "app/model/cabinet_sim/cabinet_sim.hpp"

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

    struct bypass
    {
        effect_id id;
        bool bypassed;
    };

    struct volume
    {
        uint8_t input_vol;
        uint8_t output_vol;
    };

    struct effect_controls
    {
        std::variant
        <
            tremolo::controls,
            echo::controls,
            overdrive::controls,
            cabinet_sim::controls
        >
        controls;
    };

    using holder = std::variant
    <
        get_processing_load,
        process_data,
        add_effect,
        remove_effect,
        bypass,
        volume,
        effect_controls
    >;
}

class effect_processor : public middlewares::active_object<effect_processor_events::holder>
{
public:
    effect_processor();
    ~effect_processor();

private:
    void dispatch(const event &e) override;

    /* Event handlers */
    void event_handler(const effect_processor_events::add_effect &e);
    void event_handler(const effect_processor_events::remove_effect& e);
    void event_handler(const effect_processor_events::bypass &e);
    void event_handler(const effect_processor_events::volume &e);
    void event_handler(const effect_processor_events::process_data &e);
    void event_handler(const effect_processor_events::get_processing_load &e);
    void event_handler(const effect_processor_events::effect_controls &e);

    std::unique_ptr<effect> create_new(effect_id id);
    bool find_effect(effect_id id, std::vector<std::unique_ptr<effect>>::iterator &it);

    void audio_capture_cb(const hal::audio_devices::codec::input_sample_t *input, uint16_t length);
    void audio_play_cb(uint16_t sample_index);

    uint8_t get_processing_load(void);

    std::vector<std::unique_ptr<effect>> effects;

    hal::audio_devices::codec audio;
    hal::audio_devices::codec::input_buffer_t<2 * dsp_vector_size> audio_input;
    hal::audio_devices::codec::output_buffer_t<2 * dsp_vector_size> audio_output;

    dsp_input_t dsp_input;
    dsp_output_t dsp_output;

    uint32_t processing_time_us;
};

}

#endif /* MODEL_EFFECT_PROCESSOR_HPP_ */
