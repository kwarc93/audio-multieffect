# =============================================================================
# CMSIS headers + prebuilt CMSIS-DSP static library.
#
# cmsis/dsp/*.a are prebuilt binary archives (one per Cortex-M variant/FPU
# combination), not source code, so there is nothing to compile here - we just
# tell CMake "here is an already-built library file" via an IMPORTED target,
# which is the standard modern-CMake way to consume a prebuilt .a/.lib.
# =============================================================================

add_library(cmsis_dsp STATIC IMPORTED GLOBAL)
set_target_properties(cmsis_dsp PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/dsp/lib${CMSIS_DSP_LIB}.a"
)

# CMSIS core + CMSIS-DSP headers (arm_math.h and friends), needed by anything
# that #include <cmsis/...> or <arm_math.h>.
target_include_directories(cmsis_dsp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dsp
)
