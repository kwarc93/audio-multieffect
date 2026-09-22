# =============================================================================
# Middleware: actor.hpp/filesystem.hpp/i2c_manager.hpp are header-only and
# reached via the repo-root include path. usb/ contains the USB audio class
# implementation built on top of TinyUSB.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/usb/usb_audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/usb/usb_descriptors.c
)
