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
    usb_audio(const hal::interface::audio_volume_range &volume_range);
    ~usb_audio();

    void enable(void);
    void disable(void);
    bool is_enabled(void) const;
    void process();

    void set_volume_changed_callback(std::function<void(float out_volume_db)> callback);
    void set_mute_changed_callback(std::function<void(bool muted)> callback);

    hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 1, 24> audio_to_host;
    hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 2, 24> audio_from_host;

private:
    TaskHandle_t usb_task;
};

} // namespace middlewares

#endif /* USB_AUDIO_HPP_ */
