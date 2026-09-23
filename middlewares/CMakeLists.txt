# =============================================================================
# Middleware source files.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/usb/usb_audio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb/usb_descriptors.c
)
