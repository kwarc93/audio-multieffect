/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "middlewares/usb/usb_descriptors.h"

//-----------------------------------------------------------------------------
// Board Specific Configuration
//-----------------------------------------------------------------------------

// RHPort max operational speed can defined by board.mk
#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED   OPT_MODE_FULL_SPEED
#endif

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT      (BOARD_TUD_MAX_SPEED == OPT_MODE_HIGH_SPEED)
#endif

//-----------------------------------------------------------------------------
// COMMON CONFIGURATION
//-----------------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS         OPT_OS_NONE
#endif

// Enable Device stack
#define CFG_TUD_ENABLED     1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED   BOARD_TUD_MAX_SPEED

// Enable debugging level (0/1/2/3 <-> none/err/warn/info)
#define CFG_TUSB_DEBUG      0

// Enable DMA for device controller (STM32 USB HS only)
#define CFG_TUD_DWC2_DMA_ENABLE         0
#define CFG_TUD_MEM_DCACHE_ENABLE       0
#define CFG_TUSB_MEM_DCACHE_LINE_SIZE   32

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//-----------------------------------------------------------------------------
// DEVICE CONFIGURATION
//-----------------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif

//-----------------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//-----------------------------------------------------------------------------
#define CFG_TUD_AUDIO                                                1

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN                                TUD_AUDIO_GMFX_INTERFACE_DESC_LEN

// Audio format type I specifications
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE                         48000

#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                           1
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX                           2

// 24bit in 32bit slots
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX                   4
#define CFG_TUD_AUDIO_FUNC_1_RESOLUTION_TX                           24
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX                   4
#define CFG_TUD_AUDIO_FUNC_1_RESOLUTION_RX                           24

// Enable input endpoint
#define CFG_TUD_AUDIO_ENABLE_EP_IN                                   1

#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX        TUD_AUDIO_EP_SIZE(TUD_OPT_HIGH_SPEED, CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ     (TUD_OPT_HIGH_SPEED ? 48 : 6) * CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX // It should be 8 times larger for HS device

// Enable output endpoint
#define CFG_TUD_AUDIO_ENABLE_EP_OUT                                  1

#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX        TUD_AUDIO_EP_SIZE(TUD_OPT_HIGH_SPEED, CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ     (TUD_OPT_HIGH_SPEED ? 48 : 6) * CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX // It should be 8 times larger for HS device

// Enable feedback endpoint
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP                             1

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
