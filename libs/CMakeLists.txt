# =============================================================================
# Subdirectory: Third-Party & Vendored Libraries (libs/)
#
# Manages the integration of external software components and git submodules.
# Upstream CMake projects are consumed natively via target propagation.
# Codebases lacking native CMake infrastructure are encapsulated into isolated
# static library targets to maintain strict architectural separation.
# =============================================================================

# -----------------------------------------------------------------------------
# FreeRTOS
# -----------------------------------------------------------------------------
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${FREERTOS_PORT_DIR}"
)
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)

set(FREERTOS_HEAP "3" CACHE STRING "FreeRTOS heap implementation")
add_subdirectory(FreeRTOS)

target_link_libraries(${PROJECT_NAME} PRIVATE freertos_kernel)
target_link_libraries(freertos_kernel PRIVATE audio_mfx_common)
target_compile_options(freertos_kernel PRIVATE $<BUILD_INTERFACE:-w>)
if(TARGET freertos_kernel_port)
    target_link_libraries(freertos_kernel_port PRIVATE audio_mfx_common)
endif()

# -----------------------------------------------------------------------------
# TinyUSB
# -----------------------------------------------------------------------------
if(HAS_TINYUSB)
    add_subdirectory(tinyusb/src)
    tinyusb_target_add(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c"
        "${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/portable/synopsys/dwc2/dwc2_common.c"
    )
endif()

# -----------------------------------------------------------------------------
# LVGL
# -----------------------------------------------------------------------------
if(HAS_VIEW)
    set(LV_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" CACHE STRING "")
    
    # EXCLUDE_FROM_ALL prevents building unused demos/examples automatically
    add_subdirectory(lvgl EXCLUDE_FROM_ALL)
    
    target_link_libraries(lvgl PRIVATE audio_mfx_common)
    target_compile_options(lvgl PRIVATE $<BUILD_INTERFACE:-w>)
    target_include_directories(lvgl PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS/include"
        "${FREERTOS_PORT_DIR}"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE lvgl)
endif()

# -----------------------------------------------------------------------------
# nlohmann_json
# -----------------------------------------------------------------------------
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_ImplicitConversions OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)
target_compile_definitions(nlohmann_json INTERFACE JSON_NO_IO)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# -----------------------------------------------------------------------------
# littlefs (Encapsulated Component)
# -----------------------------------------------------------------------------
if(HAS_LITTLEFS)
    add_library(littlefs STATIC
        "${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs.c"
        "${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs_util.c"
    )
    target_include_directories(littlefs PUBLIC "${CMAKE_CURRENT_LIST_DIR}/littlefs")
    target_compile_options(littlefs PRIVATE $<BUILD_INTERFACE:-w>)
    target_link_libraries(littlefs PRIVATE audio_mfx_common)
    target_link_libraries(${PROJECT_NAME} PRIVATE littlefs)
endif()

# -----------------------------------------------------------------------------
# willpirkle (Encapsulated Component)
# -----------------------------------------------------------------------------
add_library(willpirkle STATIC "${CMAKE_CURRENT_LIST_DIR}/willpirkle/fxobjects.cpp")
target_include_directories(willpirkle PUBLIC "${CMAKE_CURRENT_LIST_DIR}/willpirkle")
target_link_libraries(willpirkle PRIVATE audio_mfx_common)
target_link_libraries(${PROJECT_NAME} PRIVATE willpirkle)

# -----------------------------------------------------------------------------
# memtest (Encapsulated Component)
# -----------------------------------------------------------------------------
add_library(memtest STATIC "${CMAKE_CURRENT_LIST_DIR}/memtest/memtest.c")
target_include_directories(memtest PUBLIC "${CMAKE_CURRENT_LIST_DIR}/memtest")
target_link_libraries(memtest PRIVATE audio_mfx_common)
target_link_libraries(${PROJECT_NAME} PRIVATE memtest)

# -----------------------------------------------------------------------------
# nam-a2-embedded (Encapsulated Component)
# -----------------------------------------------------------------------------
if(HAS_EFFECT_NAM)
    add_library(nam_a2_embedded STATIC
        "${CMAKE_CURRENT_LIST_DIR}/nam-a2-embedded/engines/cortex-m7/a2lite/nam_a2lite.c"
        "${CMAKE_CURRENT_LIST_DIR}/nam-a2-embedded/namb/namb_reader.c")
    target_include_directories(nam_a2_embedded PUBLIC "${CMAKE_CURRENT_LIST_DIR}/nam-a2-embedded/engines/cortex-m7/a2lite")
    target_include_directories(nam_a2_embedded PUBLIC "${CMAKE_CURRENT_LIST_DIR}/nam-a2-embedded/namb")
    target_compile_definitions(nam_a2_embedded PUBLIC NAM_A2LITE_MAX_BUFFER_SIZE=128)
    target_link_libraries(nam_a2_embedded PRIVATE audio_mfx_common)
    target_link_libraries(${PROJECT_NAME} PRIVATE nam_a2_embedded)
endif()
