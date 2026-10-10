# =============================================================================
# CMSIS headers and CMSIS-DSP library
# =============================================================================

# -----------------------------------------------------------------------------
# CMSIS-DSP
# -----------------------------------------------------------------------------
set(CMSISCORE "${CMAKE_CURRENT_SOURCE_DIR}/cmsis_6/CMSIS/Core" CACHE INTERNAL "")

add_subdirectory(cmsis-dsp)

target_compile_definitions(CMSISDSP PRIVATE DISABLEFLOAT16)
target_compile_options(CMSISDSP PRIVATE -Ofast -ffast-math)
target_link_libraries(CMSISDSP PRIVATE audio_mfx_common)
target_link_libraries(${PROJECT_NAME} PRIVATE CMSISDSP::CMSISDSP)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/cmsis_6/CMSIS/Core/Include
)

