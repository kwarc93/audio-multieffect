# =============================================================================
# Middleware: actor framework (header-only, actor.hpp/filesystem.hpp/
# i2c_manager.hpp, included via the repo-root include path) plus the USB
# audio class implementation built on top of TinyUSB.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/usb/usb_audio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/usb/usb_descriptors.c
)
