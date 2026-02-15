/*
 * usb_audio.hpp
 *
 *  Created on: 8 wrz 2025
 *      Author: kwarc
 */

#ifndef USB_AUDIO_HPP_
#define USB_AUDIO_HPP_

#include <functional>

#include <hal_interface.hpp>
#include "app/config.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace middlewares
{

class usb_audio
{
public:
    using input_buffer_t = hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 1, 24>;
    using output_buffer_t = hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 2, 24>;

    usb_audio(const hal::interface::audio_volume_range &in_volume_range, const hal::interface::audio_volume_range &out_volume_range);
    ~usb_audio();

    void enable(void);
    void disable(void);
    bool is_enabled(void) const;
    void process();

    void set_input_volume_changed_callback(std::function<void(float volume_db)> callback);
    void set_output_volume_changed_callback(std::function<void(float volume_db)> callback);
    void set_mute_changed_callback(std::function<void(bool muted)> callback);

    void notify_input_volume_changed(float volume_db);
    void notify_output_volume_changed(float volume_db);
    void notify_mute_changed(bool muted);

    input_buffer_t audio_to_host;
    output_buffer_t audio_from_host;

private:
    TaskHandle_t usb_task;
};

} // namespace middlewares

#endif /* USB_AUDIO_HPP_ */
