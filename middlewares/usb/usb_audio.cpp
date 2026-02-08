/*
 * usb_audio.cpp
 *
 *  Created on: 21 gru 2025
 *      Author: kwarc
 */

#include "usb_audio.hpp"

#include <cmath>
#include <array>
#include <cstring>
#include <algorithm>

#include <hal_usb.hpp>

#include "app/config.hpp"
#include "tusb.h"
#include "usb_descriptors.h"

namespace
{

//-----------------------------------------------------------------------------
// TUSB types & definitions

enum tusb_status
{
    USB_STREAMING,
    USB_NOT_MOUNTED,
    USB_MOUNTED,
    USB_SUSPENDED,
};

struct tusb_context
{
    uint32_t usb_status;

    float vol_min_db;
    float vol_max_db;
    float vol_step_db;

    std::array<int8_t, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1> mute;    // +1 for master channel 0
    std::array<int16_t, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1> volume; // +1 for master channel 0

    std::function<void(float)> usb_volume_callback;
    std::function<void(bool)> usb_mute_callback;

    static constexpr uint32_t current_sample_rate {static_cast<uint32_t>(std::round(mfx::config::sampling_frequency_hz / 1000.0f) * 1000)};
    static constexpr std::array<uint32_t, 1> sample_rates {current_sample_rate};
};

tusb_context tusb;

//-----------------------------------------------------------------------------
// UAC volume level conversions

constexpr int uac_db_unit  = 256;  // 1 dB = 256 units

constexpr int16_t db_to_uac(float db)
{
    // Convert dB → 1/256 dB units with rounding
    return static_cast<int16_t>(std::lround(db * uac_db_unit));
}

constexpr float uac_to_db(int16_t uac)
{
    return static_cast<float>(uac) / uac_db_unit;
}

}

extern "C"
{

//-----------------------------------------------------------------------------
// TUSB Device callbacks

void tud_mount_cb(void)
{
    tusb.usb_status = USB_MOUNTED;
}

void tud_umount_cb(void)
{
    tusb.usb_status = USB_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    tusb.usb_status = USB_SUSPENDED;
}

void tud_resume_cb(void)
{
    tusb.usb_status = tud_mounted() ? USB_MOUNTED : USB_NOT_MOUNTED;
}

bool dcd_deinit(uint8_t rhport)
{
    (void) rhport;
    return true;
}

//-----------------------------------------------------------------------------
// TUSB Application Callbacks


// Helper for clock get requests
static bool tud_audio_clock_get_request(uint8_t rhport, audio20_control_request_t const *request)
{
    TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);

    if (request->bControlSelector == AUDIO20_CS_CTRL_SAM_FREQ)
    {
        if (request->bRequest == AUDIO20_CS_REQ_CUR)
        {
            TU_LOG1("Clock get current freq %lu\r\n", tusb.current_sample_rate);

            audio20_control_cur_4_t curf = { (int32_t) tu_htole32(tusb.current_sample_rate) };
            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &curf, sizeof(curf));
        }
        else if (request->bRequest == AUDIO20_CS_REQ_RANGE)
        {
            audio20_control_range_4_n_t(tusb.sample_rates.size()) rangef = { tu_htole16(tusb.sample_rates.size()),{} };
            TU_LOG1("Clock get %d freq ranges\r\n", tusb.sample_rates.size());

            for (uint8_t i = 0; i < tusb.sample_rates.size(); i++)
            {
                rangef.subrange[i].bMin = (int32_t) tusb.sample_rates[i];
                rangef.subrange[i].bMax = (int32_t) tusb.sample_rates[i];
                rangef.subrange[i].bRes = 0;

                TU_LOG1("Range %d (%d, %d, %d)\r\n", i, (int) rangef.subrange[i].bMin, (int) rangef.subrange[i].bMax, (int) rangef.subrange[i].bRes);
            }

            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &rangef, sizeof(rangef));
        }
    }
    else if (request->bControlSelector == AUDIO20_CS_CTRL_CLK_VALID && request->bRequest == AUDIO20_CS_REQ_CUR)
    {
        audio20_control_cur_1_t cur_valid = { 1 };
        TU_LOG1("Clock get is valid %u\r\n", cur_valid.bCur);
        return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &cur_valid, sizeof(cur_valid));
    }

    TU_LOG1("Clock get request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

// Helper for clock set requests
static bool tud_audio_clock_set_request(uint8_t rhport, audio20_control_request_t const *request, uint8_t const *buf)
{
    (void) rhport;

    TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);
    TU_VERIFY(request->bRequest == AUDIO20_CS_REQ_CUR);

    TU_LOG1("Clock set request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

// Helper for feature unit get requests
static bool tud_audio_feature_unit_get_request(uint8_t rhport, audio20_control_request_t const *request)
{
    TU_ASSERT(request->bEntityID == UAC2_ENTITY_HPH_FEATURE_UNIT);

    if (request->bControlSelector == AUDIO20_FU_CTRL_MUTE && request->bRequest == AUDIO20_CS_REQ_CUR)
    {
        audio20_control_cur_1_t mute1 = { tusb.mute[request->bChannelNumber] };
        TU_LOG1("Get channel %u mute %d\r\n", request->bChannelNumber, mute1.bCur);
        return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &mute1, sizeof(mute1));
    }
    else if (request->bControlSelector == AUDIO20_FU_CTRL_VOLUME)
    {
        if (request->bRequest == AUDIO20_CS_REQ_RANGE)
        {
            audio20_control_range_2_n_t(1) range_vol = { tu_htole16(1), { tu_htole16(db_to_uac(tusb.vol_min_db)), tu_htole16(db_to_uac(tusb.vol_max_db)), tu_htole16(uac_db_unit) } };
            TU_LOG1("Get channel %u volume range (%d, %d, %u) dB\r\n", request->bChannelNumber,
                    range_vol.subrange[0].bMin / uac_db_unit, range_vol.subrange[0].bMax / uac_db_unit, range_vol.subrange[0].bRes / uac_db_unit);

            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &range_vol, sizeof(range_vol));
        }
        else if (request->bRequest == AUDIO20_CS_REQ_CUR)
        {
            audio20_control_cur_2_t cur_vol = { tu_htole16(tusb.volume[request->bChannelNumber]) };
            TU_LOG1("Get channel %u volume %d dB\r\n", request->bChannelNumber, cur_vol.bCur / uac_db_unit);

            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &cur_vol, sizeof(cur_vol));
        }
    }

    TU_LOG1("Feature unit get request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

// Helper for feature unit set requests
static bool tud_audio_feature_unit_set_request(uint8_t rhport, audio20_control_request_t const *request, uint8_t const *buf)
{
    (void) rhport;

    TU_ASSERT(request->bEntityID == UAC2_ENTITY_HPH_FEATURE_UNIT);
    TU_VERIFY(request->bRequest == AUDIO20_CS_REQ_CUR);

    if (request->bControlSelector == AUDIO20_FU_CTRL_MUTE)
    {
        TU_VERIFY(request->wLength == sizeof(audio20_control_cur_1_t));

        tusb.mute[request->bChannelNumber] = ((audio20_control_cur_1_t const*)buf)->bCur;

        TU_LOG1("Set channel %d Mute: %d\r\n", request->bChannelNumber, tusb.mute[request->bChannelNumber]);

        if (tusb.usb_mute_callback && request->bChannelNumber == 0)
            tusb.usb_mute_callback(tusb.mute[request->bChannelNumber]);

        return true;
    }
    else if (request->bControlSelector == AUDIO20_FU_CTRL_VOLUME)
    {
        TU_VERIFY(request->wLength == sizeof(audio20_control_cur_2_t));

        tusb.volume[request->bChannelNumber] = ((audio20_control_cur_2_t const*) buf)->bCur;

        TU_LOG1("Set channel %d volume: %d dB\r\n", request->bChannelNumber, tusb.volume[request->bChannelNumber] / uac_db_unit);

        if (tusb.usb_volume_callback && request->bChannelNumber == 0)
            tusb.usb_volume_callback(uac_to_db(tusb.volume[request->bChannelNumber]));

        return true;
    }
    else
    {
        TU_LOG1("Feature unit set request not supported, entity = %u, selector = %u, request = %u\r\n",
                request->bEntityID, request->bControlSelector, request->bRequest);

        return false;
    }
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
    audio20_control_request_t const *request = (audio20_control_request_t const*) p_request;

    if (request->bEntityID == UAC2_ENTITY_CLOCK)
        return tud_audio_clock_get_request(rhport, request);
    if (request->bEntityID == UAC2_ENTITY_HPH_FEATURE_UNIT)
        return tud_audio_feature_unit_get_request(rhport, request);

    TU_LOG1("Get request not handled, entity = %d, selector = %d, request = %d\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *buf)
{
    audio20_control_request_t const *request = (audio20_control_request_t const*) p_request;

    if (request->bEntityID == UAC2_ENTITY_HPH_FEATURE_UNIT)
        return tud_audio_feature_unit_set_request(rhport, request, buf);
    if (request->bEntityID == UAC2_ENTITY_CLOCK)
        return tud_audio_clock_set_request(rhport, request, buf);

    TU_LOG1("Set request not handled, entity = %d, selector = %d, request = %d\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

bool tud_audio_set_itf_close_ep_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
    (void) rhport;

    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

    if (ITF_NUM_AUDIO20_STREAMING_HPH == itf && alt == 0)
        tusb.usb_status = USB_MOUNTED;

    return true;
}

bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
    (void) rhport;
    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

    TU_LOG2("Set interface %d alt %d\r\n", itf, alt);
    if (ITF_NUM_AUDIO20_STREAMING_HPH == itf && alt != 0)
        tusb.usb_status = USB_STREAMING;

    return true;
}

void tud_audio_feedback_params_cb(uint8_t func_id, uint8_t alt_itf, audio_feedback_params_t *feedback_param)
{
    (void) func_id;
    (void) alt_itf;

    // Set feedback method to fifo counting
    feedback_param->method = AUDIO_FEEDBACK_METHOD_FIFO_COUNT;
    feedback_param->sample_freq = tusb.current_sample_rate;
}

} // extern "C"


namespace middlewares
{

usb_audio::usb_audio(const hal::interface::audio_volume_range &volume_range)
{
    this->usb_task = nullptr;

    tusb.usb_status = USB_NOT_MOUNTED;

    tusb.vol_min_db = volume_range.min_db;
    tusb.vol_max_db = volume_range.max_db;
    tusb.vol_step_db = volume_range.max_db - volume_range.min_db / volume_range.max_val;

    tusb.mute.fill(0);
    tusb.volume.fill(db_to_uac(0));
}

usb_audio::~usb_audio()
{
    disable();
}

void usb_audio::enable()
{
    if (tusb_inited())
        return;

    this->audio_from_host.buffer.fill(0);
    this->audio_to_host.buffer.fill(0);

#if BOARD_TUD_RHPORT == 0
    hal::usbd::init_fs();
#else
    hal::usbd::init_hs();
#endif

    /* Init device stack on configured roothub port */
    tusb_rhport_init_t dev_init =
    {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    auto result = xTaskCreate([](void *arg){ while(1) { tud_task(); }; }, "usb_thread", 4096 / sizeof(StackType_t), this, configTASK_PRIO_CRITICAL, &this->usb_task);
    assert(result == pdPASS);
}

void usb_audio::disable(void)
{
    if (!tusb_inited())
        return;

    bool result = tud_deinit(BOARD_TUD_RHPORT);
    assert(result);

    if (this->usb_task)
    {
        vTaskDelete(this->usb_task);
        this->usb_task = nullptr;
    }
    this->audio_from_host.buffer.fill(0);
}

void usb_audio::set_volume_changed_callback(std::function<void(float volume_db)> callback)
{
    tusb.usb_volume_callback = std::move(callback);
}

void usb_audio::set_mute_changed_callback(std::function<void(bool muted)> callback)
{
    tusb.usb_mute_callback = std::move(callback);
}

void usb_audio::notify_volume_changed(float volume_db)
{
    if (tusb.volume[0] != db_to_uac(volume_db))
    {
        tusb.volume.fill(db_to_uac(volume_db));

        if (tusb.usb_status == USB_NOT_MOUNTED)
            return;

        audio_interrupt_data_t data;
        data.v2.bInfo = 0;
        data.v2.bAttribute = AUDIO20_CS_REQ_CUR;
        data.v2.wValue_cn_or_mcn = 0;
        data.v2.wValue_cs = AUDIO20_FU_CTRL_VOLUME;
        data.v2.wIndex_ep_or_int = 0;
        data.v2.wIndex_entity_id = UAC2_ENTITY_HPH_FEATURE_UNIT;

        tud_audio_int_write(&data);
    }
}

void usb_audio::notify_mute_changed(bool muted)
{
    if (tusb.mute[0] != muted)
    {
        tusb.mute.fill(muted);

        if (tusb.usb_status == USB_NOT_MOUNTED)
            return;

        audio_interrupt_data_t data;
        data.v2.bInfo = 0;
        data.v2.bAttribute = AUDIO20_CS_REQ_CUR;
        data.v2.wValue_cn_or_mcn = 0;
        data.v2.wValue_cs = AUDIO20_FU_CTRL_MUTE;
        data.v2.wIndex_ep_or_int = 0;
        data.v2.wIndex_entity_id = UAC2_ENTITY_HPH_FEATURE_UNIT;

        tud_audio_int_write(&data);
    }
}

bool usb_audio::is_enabled(void) const
{
    return tusb_inited();
}

void usb_audio::process()
{
    if (tusb.usb_status != USB_STREAMING)
        return;

    const uint16_t bytes_to_read = audio_from_host.buffer.size() * sizeof(int32_t);
    const uint16_t bytes_read = tud_audio_read(audio_from_host.buffer.data(), bytes_to_read);
    /* If not enough data, fill remaining buffer with zeroes */
    if (bytes_read < bytes_to_read)
        std::memset(reinterpret_cast<uint8_t*>(audio_from_host.buffer.data()) + bytes_read, 0, bytes_to_read - bytes_read);

    const uint16_t bytes_to_write = audio_to_host.buffer.size() * sizeof(int32_t);
    const uint16_t bytes_written = tud_audio_write(audio_to_host.buffer.data(), bytes_to_write);
    (void) bytes_written;
}

} // namespace middlewares
