# =============================================================================
# TinyUSB stack (git submodule), used for the composite USB-audio device.
#
# tinyusb (libs/tinyusb) is a git submodule pinned to an upstream commit and
# must stay unmodified - so this file lives outside the submodule and just
# points at its sources, instead of adding a CMakeLists.txt inside it.
#
# Like the original Eclipse project, every portable/<vendor>/<driver> MCU
# driver is compiled in (they are guarded internally by CFG_TUSB_MCU and
# compile down to an empty translation unit when they don't match the
# current target, and unused ones are dropped by --gc-sections at link
# time) - this avoids having to hand-pick the one driver each board needs
# and keeps this file simple and version-independent.
# =============================================================================

file(GLOB_RECURSE _tinyusb_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/../tinyusb/src/*.c)

target_sources(${PROJECT_NAME} PRIVATE ${_tinyusb_sources})

# libs/tusb_config.h and libs/tinyusb/src are already on the include path,
# set up in the top-level CMakeLists.txt.
