/*
 * usb_audio_source.hpp
 *
 *  Created on: 8 wrz 2025
 *      Author: kwarc
 */

#ifndef USB_AUDIO_SOURCE_HPP_
#define USB_AUDIO_SOURCE_HPP_

#include <hal_interface.hpp>
#include <hal_audio.hpp>
//#include <hal_usbd.hpp>

#include <cmsis_device.h> // For managing D-Cache & I-Cache

#include "app/config.hpp"
#include "cmsis_os2.h"
#include "tusb.h"

namespace
{

// List of supported sample rates
const uint32_t sample_rates[] = {48000};

const uint32_t current_sample_rate = 48000;

#define N_SAMPLE_RATES TU_ARRAY_SIZE(sample_rates)

/* Blink pattern
 * - 25 ms   : streaming data
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_STREAMING = 25,
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

enum {
  VOLUME_CTRL_0_DB = 0,
  VOLUME_CTRL_10_DB = 2560,
  VOLUME_CTRL_20_DB = 5120,
  VOLUME_CTRL_30_DB = 7680,
  VOLUME_CTRL_40_DB = 10240,
  VOLUME_CTRL_50_DB = 12800,
  VOLUME_CTRL_60_DB = 15360,
  VOLUME_CTRL_70_DB = 17920,
  VOLUME_CTRL_80_DB = 20480,
  VOLUME_CTRL_90_DB = 23040,
  VOLUME_CTRL_100_DB = 25600,
  VOLUME_CTRL_SILENCE = 0x8000,
};

uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

// Audio controls
// Current states
int8_t mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1];   // +1 for master channel 0
int16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1];// +1 for master channel 0

}

extern "C"
{
//-----------------------------------------------------------------------------
// Driver callbacks
//-----------------------------------------------------------------------------
bool dwc2_dcache_clean(const void *addr, uint32_t data_size)
{
    SCB_CleanDCache_by_Addr((void*) addr, data_size);
    return true;
}

bool dwc2_dcache_invalidate(const void *addr, uint32_t data_size)
{
    SCB_InvalidateDCache_by_Addr((void*) addr, data_size);
    return true;
}

bool dwc2_dcache_clean_invalidate(const void *addr, uint32_t data_size)
{
    SCB_CleanInvalidateDCache_by_Addr((void*) addr, data_size);
    return true;
}
//-----------------------------------------------------------------------------
// Device callbacks
//-----------------------------------------------------------------------------

// Invoked when device is mounted
void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//-----------------------------------------------------------------------------
// Application Callback API Implementations
//-----------------------------------------------------------------------------

// Helper for clock get requests
static bool tud_audio_clock_get_request(uint8_t rhport, audio_control_request_t const *request)
{
    TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);

    if (request->bControlSelector == AUDIO_CS_CTRL_SAM_FREQ)
    {
        if (request->bRequest == AUDIO_CS_REQ_CUR)
        {
            TU_LOG1("Clock get current freq %lu\r\n", current_sample_rate);

            audio_control_cur_4_t curf = { (int32_t) tu_htole32(current_sample_rate) };
            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &curf, sizeof(curf));
        }
        else if (request->bRequest == AUDIO_CS_REQ_RANGE)
        {
            audio_control_range_4_n_t(N_SAMPLE_RATES) rangef = { tu_htole16(N_SAMPLE_RATES),{} };
            TU_LOG1("Clock get %d freq ranges\r\n", N_SAMPLE_RATES);

            for (uint8_t i = 0; i < N_SAMPLE_RATES; i++)
            {
                rangef.subrange[i].bMin = (int32_t) sample_rates[i];
                rangef.subrange[i].bMax = (int32_t) sample_rates[i];
                rangef.subrange[i].bRes = 0;

                TU_LOG1("Range %d (%d, %d, %d)\r\n", i, (int) rangef.subrange[i].bMin, (int) rangef.subrange[i].bMax, (int) rangef.subrange[i].bRes);
            }

            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &rangef, sizeof(rangef));
        }
    }
    else if (request->bControlSelector == AUDIO_CS_CTRL_CLK_VALID && request->bRequest == AUDIO_CS_REQ_CUR)
    {
        audio_control_cur_1_t cur_valid = { .bCur = 1 };
        TU_LOG1("Clock get is valid %u\r\n", cur_valid.bCur);
        return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &cur_valid, sizeof(cur_valid));
    }

    TU_LOG1("Clock get request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

// Helper for clock set requests
static bool tud_audio_clock_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf)
{
    (void) rhport;

    TU_ASSERT(request->bEntityID == UAC2_ENTITY_CLOCK);
    TU_VERIFY(request->bRequest == AUDIO_CS_REQ_CUR);

    TU_LOG1("Clock set request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

// Helper for feature unit get requests
static bool tud_audio_feature_unit_get_request(uint8_t rhport, audio_control_request_t const *request)
{
    TU_ASSERT(request->bEntityID == UAC2_ENTITY_HPH_FEATURE_UNIT);

    if (request->bControlSelector == AUDIO_FU_CTRL_MUTE && request->bRequest == AUDIO_CS_REQ_CUR)
    {
        audio_control_cur_1_t mute1 = { .bCur = mute[request->bChannelNumber] };
        TU_LOG1("Get channel %u mute %d\r\n", request->bChannelNumber, mute1.bCur);
        return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &mute1, sizeof(mute1));
    }
    else if (request->bControlSelector == AUDIO_FU_CTRL_VOLUME)
    {
        if (request->bRequest == AUDIO_CS_REQ_RANGE)
        {
            audio_control_range_2_n_t(1) range_vol = { tu_htole16(1), { tu_htole16(-VOLUME_CTRL_50_DB), tu_htole16(VOLUME_CTRL_0_DB), tu_htole16(256) } };
            TU_LOG1("Get channel %u volume range (%d, %d, %u) dB\r\n", request->bChannelNumber,
                    range_vol.subrange[0].bMin / 256, range_vol.subrange[0].bMax / 256, range_vol.subrange[0].bRes / 256);

            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &range_vol, sizeof(range_vol));
        }
        else if (request->bRequest == AUDIO_CS_REQ_CUR)
        {
            audio_control_cur_2_t cur_vol = { .bCur = tu_htole16(volume[request->bChannelNumber]) };
            TU_LOG1("Get channel %u volume %d dB\r\n", request->bChannelNumber, cur_vol.bCur / 256);

            return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const*) request, &cur_vol, sizeof(cur_vol));
        }
    }

    TU_LOG1("Feature unit get request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);

    return false;
}

// Helper for feature unit set requests
static bool tud_audio_feature_unit_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf)
{
    (void) rhport;

    TU_ASSERT(request->bEntityID == UAC2_ENTITY_HPH_FEATURE_UNIT);
    TU_VERIFY(request->bRequest == AUDIO_CS_REQ_CUR);

    if (request->bControlSelector == AUDIO_FU_CTRL_MUTE)
    {
        TU_VERIFY(request->wLength == sizeof(audio_control_cur_1_t));

        mute[request->bChannelNumber] = ((audio_control_cur_1_t const*)buf)->bCur;

        TU_LOG1("Set channel %d Mute: %d\r\n", request->bChannelNumber, mute[request->bChannelNumber]);

        return true;
    }
    else if (request->bControlSelector == AUDIO_FU_CTRL_VOLUME)
    {
        TU_VERIFY(request->wLength == sizeof(audio_control_cur_2_t));

        volume[request->bChannelNumber] = ((audio_control_cur_2_t const*) buf)->bCur;

        TU_LOG1("Set channel %d volume: %d dB\r\n", request->bChannelNumber, volume[request->bChannelNumber] / 256);

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
    audio_control_request_t const *request = (audio_control_request_t const*) p_request;

    if (request->bEntityID == UAC2_ENTITY_CLOCK)
    {
        return tud_audio_clock_get_request(rhport, request);
    }
    else if (request->bEntityID == UAC2_ENTITY_HPH_FEATURE_UNIT)
    {
        return tud_audio_feature_unit_get_request(rhport, request);
    }
    else
    {
        TU_LOG1("Get request not handled, entity = %d, selector = %d, request = %d\r\n",
                request->bEntityID, request->bControlSelector, request->bRequest);
    }

    return false;
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *buf)
{
    audio_control_request_t const *request = (audio_control_request_t const*) p_request;

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

    if (ITF_NUM_AUDIO_STREAMING_HPH == itf && alt == 0)
        blink_interval_ms = BLINK_MOUNTED;

    return true;
}

bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
    (void) rhport;
    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

    TU_LOG2("Set interface %d alt %d\r\n", itf, alt);
    if (ITF_NUM_AUDIO_STREAMING_HPH == itf && alt != 0)
        blink_interval_ms = BLINK_STREAMING;

    return true;
}

void tud_audio_feedback_params_cb(uint8_t func_id, uint8_t alt_itf, audio_feedback_params_t *feedback_param)
{
    (void) func_id;
    (void) alt_itf;

    // Set feedback method to fifo counting
    feedback_param->method = AUDIO_FEEDBACK_METHOD_FIFO_COUNT;
    feedback_param->sample_freq = mfx::config::sampling_frequency_hz;
}

}

namespace middlewares
{

class usb_audio_source
{
public:
    usb_audio_source()
    {
        this->audio_from_host.buffer.fill(0);
        this->audio_to_host.buffer.fill(0);

#if BOARD_TUD_RHPORT == 0 // USB FS
        // Enable low-level USB FS stuff
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin10 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // ID
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin11 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // D-
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin12 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // D+

        drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB2, OTGFS), true);

        NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 5+1, 0 ));

        // Disable VBUS sense (B device) via pin PA9
        USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

        // B-peripheral session valid override enable
        USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
#else // USB HS
        // Enable low-level USB HS stuff
        // MCU with external ULPI PHY

        /* ULPI CLK */
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin5 }, drivers::gpio::mode::af, drivers::gpio::af::af10);

        /* ULPI D0 */
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin3 }, drivers::gpio::mode::af, drivers::gpio::af::af10);

        /* ULPI D1 D2 D3 D4 D5 D6 D7 */
        drivers::gpio::configure({ drivers::gpio::port::portb, drivers::gpio::pin::pin0 }, drivers::gpio::mode::af, drivers::gpio::af::af10);
        drivers::gpio::configure({ drivers::gpio::port::portb, drivers::gpio::pin::pin1 }, drivers::gpio::mode::af, drivers::gpio::af::af10);
        drivers::gpio::configure({ drivers::gpio::port::portb, drivers::gpio::pin::pin10 }, drivers::gpio::mode::af, drivers::gpio::af::af10);
        drivers::gpio::configure({ drivers::gpio::port::portb, drivers::gpio::pin::pin11 }, drivers::gpio::mode::af, drivers::gpio::af::af10);
        drivers::gpio::configure({ drivers::gpio::port::portb, drivers::gpio::pin::pin12 }, drivers::gpio::mode::af, drivers::gpio::af::af10);
        drivers::gpio::configure({ drivers::gpio::port::portb, drivers::gpio::pin::pin13 }, drivers::gpio::mode::af, drivers::gpio::af::af10);
        drivers::gpio::configure({ drivers::gpio::port::portb, drivers::gpio::pin::pin5 }, drivers::gpio::mode::af, drivers::gpio::af::af10);

        /* ULPI STP */
        drivers::gpio::configure({ drivers::gpio::port::portc, drivers::gpio::pin::pin0 }, drivers::gpio::mode::af, drivers::gpio::af::af10);
        drivers::gpio::configure({ drivers::gpio::port::portc, drivers::gpio::pin::pin2 }, drivers::gpio::mode::af, drivers::gpio::af::af10);

        /* NXT */
        drivers::gpio::configure({ drivers::gpio::port::porth, drivers::gpio::pin::pin4 }, drivers::gpio::mode::af, drivers::gpio::af::af10);

        /* ULPI DIR */
        drivers::gpio::configure({ drivers::gpio::port::porti, drivers::gpio::pin::pin11 }, drivers::gpio::mode::af, drivers::gpio::af::af10);

        NVIC_SetPriority(OTG_HS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 5+1, 0 ));

        // Enable USB HS & ULPI Clocks
        drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, OTGHS), true);
        drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB1, OTGHSULPI), true);

        // Disable VBUS sense (B device) via pin PA9
        USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

        // B-peripheral session valid override enable
        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
#endif // BOARD_TUD_RHPORT

        // init device stack on configured roothub port
        tusb_rhport_init_t dev_init = {
            .role = TUSB_ROLE_DEVICE,
            .speed = TUSB_SPEED_AUTO};
        tusb_init(BOARD_TUD_RHPORT, &dev_init);

        osThreadAttr_t attr {};
        attr.name = "tusb_thread";
        attr.stack_size = 4096;
        attr.priority = osPriorityRealtime;
        osThreadNew([](void *arg){ while(1) { tud_task(); }; }, this, &attr);
    }

    ~usb_audio_source()
    {
    }

    bool write()
    {
        if (!tud_audio_mounted()) return false;

        uint16_t bytes_to_write = audio_to_host.buffer.size() * sizeof(int32_t);
        uint16_t bytes_written = tud_audio_write((uint8_t *)audio_to_host.buffer.data(), bytes_to_write);
        return bytes_written == bytes_to_write;
    }

    bool read()
    {
        if (!tud_audio_mounted()) return false;

        uint16_t bytes_to_read = audio_from_host.buffer.size() * sizeof(int32_t);
        uint16_t bytes_read = tud_audio_read((uint8_t *)audio_from_host.buffer.data(), bytes_to_read);
        /* If not enough data, fill remaining buffer with zeroes */
        std::memset(audio_from_host.buffer.data() + bytes_read, 0, bytes_to_read - bytes_read);
        return bytes_to_read == bytes_read;
    }

    hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 1, 24> audio_to_host;
    hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 2, 24> audio_from_host;
};

}


#endif /* USB_AUDIO_SOURCE_HPP_ */
