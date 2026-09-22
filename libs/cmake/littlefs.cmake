# =============================================================================
# littlefs - small embedded filesystem used for settings/presets storage.
#
# littlefs (libs/littlefs) is a git submodule pinned to an upstream commit and
# must stay byte-for-byte identical to that commit - so instead of dropping a
# CMakeLists.txt inside the submodule (which would make it "dirty"/modified),
# this file lives outside the submodule and just points at its sources.
# Upstream ships no CMake support of its own.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../littlefs/lfs.c
    ${CMAKE_CURRENT_LIST_DIR}/../littlefs/lfs_util.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../littlefs
)
