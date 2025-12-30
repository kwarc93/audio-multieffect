/*
 * usb_audio.hpp
 *
 *  Created on: 8 wrz 2025
 *      Author: kwarc
 */

#ifndef USB_AUDIO_HPP_
#define USB_AUDIO_HPP_

#include <hal_interface.hpp>
#include "app/config.hpp"

#include "FreeRTOS.h"
#include "task.h"

namespace middlewares
{

class usb_audio
{
public:
    usb_audio();
    ~usb_audio();

    void enable(void);
    void disable(void);
    bool is_enabled(void) const;
    void process();

    hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 1, 24> audio_to_host;
    hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 2, 24> audio_from_host;

private:
    TaskHandle_t usb_task;
};

} // namespace middlewares

#endif /* USB_AUDIO_HPP_ */
