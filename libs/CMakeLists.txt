# =============================================================================
# Third-party / vendored libraries (mostly git submodules under libs/).
#
# Where a library ships its own modern CMakeLists.txt as part of its upstream
# commit (FreeRTOS, lvgl, nlohmann_json), we reuse it via add_subdirectory()
# and link against the target it defines.
#
# Submodules with NO upstream CMake support (littlefs, tinyusb, nam-core) must
# NOT get a CMakeLists.txt written inside them - that would locally modify a
# submodule that is supposed to stay pinned to an untouched upstream commit.
# Instead, their source lists live in libs/cmake/<name>.cmake (a plain
# directory we own) and are pulled in with include(), which only *reads* the
# submodule's files and adds no files to it.
#
# willpirkle/ and memtest/ are NOT submodules (see .gitmodules) - they are
# vendored directly in this repo, so a CMakeLists.txt next to their sources is
# fine.
# =============================================================================

# --- FreeRTOS kernel ---------------------------------------------------------
# The upstream FreeRTOS-Kernel CMakeLists.txt (libs/FreeRTOS/CMakeLists.txt)
# expects the consumer to provide:
#   * an INTERFACE target called "freertos_config" with the include directory
#     for FreeRTOSConfig.h (+ the port's portmacro.h),
#   * the FREERTOS_PORT cache variable (which GCC port to build),
#   * optionally FREERTOS_HEAP to auto-add one of the portable/MemMang/heap_N.c
#     allocators (project uses heap_3, the malloc()/free() wrapper).
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}          # FreeRTOSConfig.h
    ${FREERTOS_PORT_DIR}                 # portmacro.h for the selected port
)
target_compile_definitions(freertos_config INTERFACE
    projCOVERAGE_TEST=0
)

set(FREERTOS_HEAP "3" CACHE STRING "FreeRTOS heap implementation (heap_3 = malloc/free wrapper)")
add_subdirectory(FreeRTOS)
target_link_libraries(${PROJECT_NAME} PRIVATE freertos_kernel)
# freertos_kernel (and its port-specific object library, freertos_kernel_port)
# are separate CMake targets (not our executable), so they need the same
# -mcpu/-mfpu/-mfloat-abi flags applied explicitly - otherwise their objects
# would be compiled without hardware FPU support and fail to build/link with
# the rest of the (hard-float) firmware.
target_compile_options(freertos_kernel PRIVATE ${MCU_FLAGS} -O3 -fmessage-length=0)
if(TARGET freertos_kernel_port)
    target_compile_options(freertos_kernel_port PRIVATE ${MCU_FLAGS} -O3 -fmessage-length=0)
endif()

# --- nlohmann_json (single-header JSON library, ships its own CMakeLists) ---
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# --- Header-only / hand-written wrapper libraries ---------------------------
include(cmake/littlefs.cmake)
add_subdirectory(willpirkle)
add_subdirectory(memtest)
include(cmake/tinyusb.cmake)

if(HAS_VIEW)
    # LVGL ships its own CMakeLists.txt (env_support/cmake/custom.cmake),
    # which builds a static "lvgl" target - point it at our lv_conf.h.
    set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "" FORCE)
    add_subdirectory(lvgl)
    target_compile_options(lvgl PRIVATE ${MCU_FLAGS} -O3 -fmessage-length=0)
    # lvgl's own CMakeLists.txt also defines lvgl_examples/lvgl_demos targets
    # (LVGL's bundled example/demo UIs) - we don't use them, so keep them out
    # of the default build to save a lot of compile time.
    set_target_properties(lvgl_examples lvgl_demos PROPERTIES EXCLUDE_FROM_ALL TRUE)
    # lv_conf.h asks for a FreeRTOS-based tick source (LV_TICK_CUSTOM_INCLUDE
    # "FreeRTOS.h"), so lvgl also needs FreeRTOS's include directories.
    target_include_directories(lvgl PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/include
        ${FREERTOS_PORT_DIR}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE lvgl)
endif()

if(HAS_MODEL_NAM)
    include(cmake/nam-core.cmake)
endif()
