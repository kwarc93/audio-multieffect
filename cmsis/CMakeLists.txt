# =============================================================================
# CMSIS headers and a prebuilt CMSIS-DSP static library. The .a archives
# under dsp/ are precompiled binaries, so they are exposed as an IMPORTED
# target rather than being compiled from source.
# =============================================================================

add_library(cmsis_dsp STATIC IMPORTED GLOBAL)
set_target_properties(cmsis_dsp PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dsp/lib${CMSIS_DSP_LIB}.a"
)

target_include_directories(cmsis_dsp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dsp
)

