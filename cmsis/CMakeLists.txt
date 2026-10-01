# =============================================================================
# CMSIS headers and a prebuilt CMSIS-DSP static library. The .a archives
# under dsp/ are precompiled binaries, so they are exposed as an IMPORTED
# target rather than being compiled from source.
# =============================================================================

# -----------------------------------------------------------------------------
# CMSIS-DSP
# -----------------------------------------------------------------------------
set(CMSISCORE "${CMAKE_CURRENT_SOURCE_DIR}/cmsis_6/CMSIS/Core" CACHE INTERNAL "")

add_subdirectory(cmsis-dsp)

target_compile_definitions(CMSISDSP PRIVATE DISABLEFLOAT16)
target_compile_options(CMSISDSP PRIVATE -O3 -ffast-math)
target_link_libraries(CMSISDSP PRIVATE audio_mfx_common)
target_link_libraries(${PROJECT_NAME} PRIVATE CMSISDSP::CMSISDSP)

# Static library
# add_library(cmsis_dsp STATIC IMPORTED GLOBAL)
# set_target_properties(cmsis_dsp PROPERTIES
#     IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/dsp/lib${CMSIS_DSP_LIB}.a"
# )

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/cmsis_6/CMSIS/Core/Include
)

