# =============================================================================
# Third-party / vendored libraries under libs/ (mostly git submodules).
#
# FreeRTOS, lvgl and nlohmann_json ship their own upstream CMakeLists.txt and
# are consumed via add_subdirectory(). littlefs, tinyusb and nam-core are git
# submodules with no CMake support of their own; adding a CMakeLists.txt
# inside a submodule would leave it locally modified, so their sources are
# listed directly in this file instead and referenced with a relative path.
#
# willpirkle/ and memtest/ are not submodules (see .gitmodules), so their own
# CMakeLists.txt next to their sources is unproblematic.
# =============================================================================

# --- FreeRTOS kernel ---------------------------------------------------------
# libs/FreeRTOS/CMakeLists.txt requires an INTERFACE target "freertos_config"
# providing FreeRTOSConfig.h's include directory, the FREERTOS_PORT variable,
# and optionally FREERTOS_HEAP to select a portable/MemMang/heap_N.c allocator.
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}          # FreeRTOSConfig.h
    ${FREERTOS_PORT_DIR}                 # portmacro.h for the selected port
)
target_compile_definitions(freertos_config INTERFACE
    projCOVERAGE_TEST=0
)

set(FREERTOS_HEAP "3" CACHE STRING "FreeRTOS heap implementation (heap_3 = malloc/free wrapper)")
add_subdirectory(FreeRTOS)
target_link_libraries(${PROJECT_NAME} PRIVATE freertos_kernel)
target_link_libraries(freertos_kernel PRIVATE audio_mfx_common)
if(TARGET freertos_kernel_port)
    target_link_libraries(freertos_kernel_port PRIVATE audio_mfx_common)
endif()

# --- nlohmann_json (single-header JSON library, ships its own CMakeLists) ---
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# --- littlefs (submodule, no upstream CMake support) -------------------------
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/littlefs/lfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/littlefs/lfs_util.c
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/littlefs)

# --- willpirkle / memtest (vendored, not submodules) -------------------------
add_subdirectory(willpirkle)
add_subdirectory(memtest)

# --- TinyUSB (submodule, no upstream CMake support) --------------------------
# Every portable/<vendor>/<driver> MCU driver is compiled in: each is guarded
# internally by CFG_TUSB_MCU and reduces to an empty translation unit when it
# doesn't match the current target, with unused code dropped by
# --gc-sections at link time.
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/tusb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/common/tusb_fifo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/typec/usbc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/device/usbd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/host/hub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/host/usbh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/audio/audio_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/bth/bth_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/cdc/cdc_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/cdc/cdc_host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/dfu/dfu_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/dfu/dfu_rt_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/hid/hid_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/hid/hid_host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/midi/midi2_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/midi/midi2_host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/midi/midi_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/midi/midi_host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/msc/msc_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/msc/msc_host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/mtp/mtp_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/net/ecm_rndis_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/net/ncm_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/printer/printer_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/usbtmc/usbtmc_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/vendor/vendor_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/vendor/vendor_host.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/video/video_device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/analog/max3421/hcd_max3421.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/bridgetek/ft9xx/dcd_ft9xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/chipidea/ci_fs/dcd_ci_fs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/chipidea/ci_hs/dcd_ci_hs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/chipidea/ci_hs/hcd_ci_hs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/dialog/da146xx/dcd_da146xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/ehci/ehci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/mentor/musb/dcd_musb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/mentor/musb/hcd_musb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/microchip/pic/dcd_pic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/microchip/pic32mz/dcd_pic32mz.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/microchip/samd/dcd_samd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/microchip/samd/hcd_samd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/microchip/samg/dcd_samg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/microchip/samx7x/dcd_samx7x.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/mindmotion/mm32/dcd_mm32f327x_otg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nordic/nrf5x/dcd_nrf5x.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nuvoton/nuc120/dcd_nuc120.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nuvoton/nuc121/dcd_nuc121.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nuvoton/nuc505/dcd_nuc505.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nxp/khci/dcd_khci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nxp/khci/hcd_khci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nxp/lpc17_40/dcd_lpc17_40.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nxp/lpc_ip3511/dcd_lpc_ip3511.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/nxp/lpc_ip3516/hcd_lpc_ip3516.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/ohci/ohci.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/raspberrypi/rp2040/hcd_rp2040.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/raspberrypi/rp2040/rp2040_usb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/renesas/rusb2/dcd_rusb2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/renesas/rusb2/hcd_rusb2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/renesas/rusb2/rusb2_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/sony/cxd56/dcd_cxd56.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/st/stm32_fsdev/dcd_stm32_fsdev.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/st/stm32_fsdev/fsdev_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/st/stm32_fsdev/hcd_stm32_fsdev.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/st/typec/typec_stm32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/sunxi/dcd_sunxi_musb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/synopsys/dwc2/dwc2_common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/synopsys/dwc2/hcd_dwc2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/template/dcd_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/template/hcd_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/ti/msp430x5xx/dcd_msp430x5xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/valentyusb/eptri/dcd_eptri.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/wch/dcd_ch32_usbfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/wch/dcd_ch32_usbhs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/wch/hcd_ch32_usbfs.c
)
# libs/tusb_config.h and libs/tinyusb/src are already on the include path
# set up in the top-level CMakeLists.txt.

if(HAS_VIEW)
    # lvgl ships its own CMakeLists.txt (env_support/cmake/custom.cmake).
    set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "" FORCE)
    add_subdirectory(lvgl)
    target_link_libraries(lvgl PRIVATE audio_mfx_common)
    # lvgl's CMakeLists.txt also defines lvgl_examples/lvgl_demos targets,
    # which are not used here and would otherwise be built by default.
    set_target_properties(lvgl_examples lvgl_demos PROPERTIES EXCLUDE_FROM_ALL TRUE)
    # lv_conf.h sets LV_TICK_CUSTOM_INCLUDE "FreeRTOS.h", so lvgl needs
    # FreeRTOS's include directories too.
    target_include_directories(lvgl PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/include
        ${FREERTOS_PORT_DIR}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE lvgl)
endif()

if(HAS_MODEL_NAM)
    # nam-core (submodule, no upstream CMake support). NAMPedal.cpp is an
    # alternate entry point and is intentionally not compiled here.
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nam-core/nam_model.c)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nam-core)
endif()

