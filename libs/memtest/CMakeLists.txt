# =============================================================================
# Simple RAM memory test utility (memtest.c/.h). No upstream CMake support.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/memtest.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
