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
    inline uint8_t clkValid;
    inline uint32_t sampFreq;
    inline bool mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];       // +1 for master channel 0
    inline uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; // +1 for master channel 0
    inline audio_control_range_4_n_t(1) sampleFreqRng;              // Sample frequency range state
}

extern "C"
{
//--------------------------------------------------------------------+
// Driver callbacks
//--------------------------------------------------------------------+
bool dwc2_dcache_clean(const void* addr, uint32_t data_size) {
    SCB_CleanDCache_by_Addr((void*)addr, data_size);
    return true;
}

bool dwc2_dcache_invalidate(const void* addr, uint32_t data_size) {
    SCB_InvalidateDCache_by_Addr((void*)addr, data_size);
    return true;
}

bool dwc2_dcache_clean_invalidate(const void* addr, uint32_t data_size) {
    SCB_CleanInvalidateDCache_by_Addr((void*)addr, data_size);
    return true;
}
//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
//  ready = tud_mounted();
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) ep;

  return false;// Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void) rhport;
  (void) pBuff;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) itf;

  return false;// Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *pBuff) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  (void) itf;

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  // If request is for our feature unit
  if (entityID == 2) {
    switch (ctrlSel) {
      case AUDIO_FU_CTRL_MUTE:
        // Request uses format layout 1
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

        mute[channelNum] = ((audio_control_cur_1_t *) pBuff)->bCur;

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", mute[channelNum], channelNum);
        return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        volume[channelNum] = (uint16_t) ((audio_control_cur_2_t *) pBuff)->bCur;

        TU_LOG2("    Set Volume: %d dB of channel: %u\r\n", volume[channelNum], channelNum);
        return true;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }
  return false;// Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t ep = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) ep;

  //    return tud_control_xfer(rhport, p_request, &tmp, 1);

  return false;// Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);

  (void) channelNum;
  (void) ctrlSel;
  (void) itf;

  return false;// Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  // uint8_t itf = TU_U16_LOW(p_request->wIndex);           // Since we have only one audio function implemented, we do not need the itf value
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  // Input terminal (Microphone input)
  if (entityID == 1) {
    switch (ctrlSel) {
      case AUDIO_TE_CTRL_CONNECTOR: {
        // The terminal connector control only has a get request with only the CUR attribute.
        audio_desc_channel_cluster_t ret;

        // Those are dummy values for now
        ret.bNrChannels = 1;
        ret.bmChannelConfig = (audio_channel_config_t) 0;
        ret.iChannelNames = 0;

        TU_LOG2("    Get terminal connector\r\n");

        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void *) &ret, sizeof(ret));
      } break;

        // Unknown/Unsupported control selector
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Feature unit
  if (entityID == 2) {
    switch (ctrlSel) {
      case AUDIO_FU_CTRL_MUTE:
        // Audio control mute cur parameter block consists of only one byte - we thus can send it right away
        // There does not exist a range parameter block for mute
        TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &mute[channelNum], 1);

      case AUDIO_FU_CTRL_VOLUME:
        switch (p_request->bRequest) {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &volume[channelNum], sizeof(volume[channelNum]));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

            // Copy values - only for testing - better is version below
            audio_control_range_2_n_t(1)
                ret;

            ret.wNumSubRanges = 1;
            ret.subrange[0].bMin = -90;// -90 dB
            ret.subrange[0].bMax = 90; // +90 dB
            ret.subrange[0].bRes = 1;  // 1 dB steps

            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void *) &ret, sizeof(ret));

            // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
        break;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Clock Source unit
  if (entityID == 4) {
    switch (ctrlSel) {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch (p_request->bRequest) {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Sample Freq.\r\n");
            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &sampFreq, sizeof(sampFreq));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Sample Freq. range\r\n");
            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

            // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
        break;

      case AUDIO_CS_CTRL_CLK_VALID:
        // Only cur attribute exists for this request
        TU_LOG2("    Get Sample Freq. valid\r\n");
        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

      // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  TU_LOG2("  Unsupported entity: %d\r\n", entityID);
  return false;// Yet not implemented
}

bool tud_audio_set_itf_close_ep_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
  (void) rhport;
  (void) p_request;

  return true;
}
}

namespace middlewares
{

class usb_audio_source
{
public:
    usb_audio_source()
    {
        this->samples.buffer.fill(0);

        // Enable low-level USBFS stuff
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin10 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // ID
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin11 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // D-
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin12 }, drivers::gpio::mode::af, drivers::gpio::af::af10); // D+

        drivers::rcc::enable_periph_clock(RCC_PERIPH_BUS(AHB2, OTGFS), true);

        NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority( NVIC_GetPriorityGrouping(), 5+1, 0 ));

#if OTG_FS_VBUS_SENSE
        // Configure VBUS Pin (or leave at its default state after reset)
        drivers::gpio::configure({ drivers::gpio::port::porta, drivers::gpio::pin::pin9 }, drivers::gpio::mode::input); // VBUS

        // Enable VBUS sense (B device) via pin PA9
        USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_VBDEN;
#else
        // Disable VBUS sense (B device) via pin PA9
        USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

        // B-peripheral session valid override enable
        USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
        USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
#endif // vbus sense

        // init device stack on configured roothub port
        tusb_rhport_init_t dev_init = {
            .role = TUSB_ROLE_DEVICE,
            .speed = TUSB_SPEED_AUTO};
        tusb_init(BOARD_TUD_RHPORT, &dev_init);

        // Init values
        sampFreq = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
        clkValid = 1;

        sampleFreqRng.wNumSubRanges = 1;
        sampleFreqRng.subrange[0].bMin = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
        sampleFreqRng.subrange[0].bMax = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
        sampleFreqRng.subrange[0].bRes = 0;

        osThreadAttr_t attr {};
        attr.name = "tusb_thread";
        attr.stack_size = 4096;
        attr.priority = osPriorityRealtime1;
        osThreadNew([](void *arg){ while(1) { tud_task(); }; }, this, &attr);
    }

    ~usb_audio_source()
    {
    }

    bool write()
    {
        if (!tud_audio_mounted()) return false;

        uint16_t bytes_to_write = samples.buffer.size() * sizeof(int32_t);
        uint16_t bytes_written = tud_audio_write((uint8_t *)samples.buffer.data(), bytes_to_write);
        return bytes_written == bytes_to_write;
    }

    hal::interface::audio_buffer<int32_t, mfx::config::dsp_vector_size, 1, 24> samples;
};

}


#endif /* USB_AUDIO_SOURCE_HPP_ */
