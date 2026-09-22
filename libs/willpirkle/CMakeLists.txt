# =============================================================================
# Will Pirkle's fxobjects DSP building blocks (filters, delays, ...) used by
# several effects (amp_sim, cabinet_sim, etc.). No upstream CMake support.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/fxobjects.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
