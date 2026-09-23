# =============================================================================
# Third-party / vendored libraries under libs/ (mostly git submodules).
#
# FreeRTOS, lvgl, nlohmann_json, and TinyUSB ship CMake integration and are
# consumed without modifying their submodules. littlefs and nam-core have no
# CMake support of their own, so their sources are listed directly here.
#
# willpirkle/ and memtest/ are not submodules (see .gitmodules), so their own
# CMakeLists.txt next to their sources is unproblematic.
# =============================================================================

# --- FreeRTOS kernel ---------------------------------------------------------
# libs/FreeRTOS/CMakeLists.txt requires an INTERFACE target "freertos_config"
# providing FreeRTOSConfig.h's include directory, the FREERTOS_PORT variable,
# and optionally FREERTOS_HEAP to select a portable/MemMang/heap_N.c allocator.
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
target_link_libraries(freertos_kernel PRIVATE audio_mfx_common)
if(TARGET freertos_kernel_port)
    target_link_libraries(freertos_kernel_port PRIVATE audio_mfx_common)
endif()

# --- nlohmann_json (single-header JSON library, ships its own CMakeLists) ---
# CMake 3.13+ honors this normal variable when nlohmann_json calls option().
set(JSON_BuildTests OFF)
add_subdirectory(nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# --- littlefs (submodule, no upstream CMake support) -------------------------
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/littlefs/lfs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/littlefs/lfs_util.c
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/littlefs)

# --- willpirkle / memtest (vendored, not submodules) -------------------------
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/willpirkle/fxobjects.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/willpirkle)
target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/memtest/memtest.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/memtest)

# --- TinyUSB ---------------------------------------------------------------
# TinyUSB's source-level CMake integration adds its portable core and class
# sources to an existing target. MCU drivers are selected by the application.
add_subdirectory(tinyusb/src)
tinyusb_target_add(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/synopsys/dwc2/dwc2_common.c
)

if(HAS_VIEW)
    # lvgl ships its own CMakeLists.txt (env_support/cmake/custom.cmake).
    set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "" FORCE)
    add_subdirectory(lvgl)
    target_link_libraries(lvgl PRIVATE audio_mfx_common)
    # lvgl's CMakeLists.txt also defines lvgl_examples/lvgl_demos targets,
    # which are not used here and would otherwise be built by default.
    set_target_properties(lvgl_examples lvgl_demos PROPERTIES EXCLUDE_FROM_ALL TRUE)
    # lv_conf.h sets LV_TICK_CUSTOM_INCLUDE "FreeRTOS.h", so lvgl needs
    # FreeRTOS's include directories too.
    target_include_directories(lvgl PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/include
        ${FREERTOS_PORT_DIR}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE lvgl)
endif()

if(HAS_MODEL_NAM)
    # nam-core (submodule, no upstream CMake support). NAMPedal.cpp is an
    # alternate entry point and is intentionally not compiled here.
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nam-core/nam_model.c)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nam-core)
endif()

