# =============================================================================
# Subdirectory: Third-Party & Vendored Libraries (libs/)
#
# Manages the integration of external software components and git submodules.
# Upstream CMake projects are consumed natively via target propagation.
# Codebases lacking native CMake infrastructure are encapsulated into isolated
# static library targets to maintain strict architectural separation.
# =============================================================================

# --- FreeRTOS Kernel ---------------------------------------------------------
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

# --- TinyUSB -----------------------------------------------------------------
add_subdirectory(tinyusb/src)
tinyusb_target_add(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/portable/synopsys/dwc2/dcd_dwc2.c"
    "${CMAKE_CURRENT_LIST_DIR}/tinyusb/src/portable/synopsys/dwc2/dwc2_common.c"
)

# --- LVGL Graphical Stack ----------------------------------------------------
if(HAS_VIEW)
    set(LV_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" CACHE STRING "" FORCE)
    
    # EXCLUDE_FROM_ALL prevents building unused demos/examples automatically
    add_subdirectory(lvgl EXCLUDE_FROM_ALL)
    
    target_link_libraries(lvgl PRIVATE audio_mfx_common)
    target_compile_options(lvgl PRIVATE $<BUILD_INTERFACE:-w>)
    target_include_directories(lvgl SYSTEM PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS/include"
        "${FREERTOS_PORT_DIR}"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE lvgl)
endif()

# --- nlohmann_json -----------------------------------------------------------
set(JSON_BuildTests OFF)
add_subdirectory(nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# --- littlefs (Encapsulated Component) ---------------------------------------
add_library(littlefs STATIC
    "${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs.c"
    "${CMAKE_CURRENT_LIST_DIR}/littlefs/lfs_util.c"
)
target_include_directories(littlefs SYSTEM PUBLIC "${CMAKE_CURRENT_LIST_DIR}/littlefs")
target_link_libraries(littlefs PRIVATE audio_mfx_common)
target_link_libraries(${PROJECT_NAME} PRIVATE littlefs)

# --- willpirkle (Encapsulated Component) -------------------------------------
add_library(willpirkle STATIC "${CMAKE_CURRENT_LIST_DIR}/willpirkle/fxobjects.cpp")
target_include_directories(willpirkle SYSTEM PUBLIC "${CMAKE_CURRENT_LIST_DIR}/willpirkle")
target_link_libraries(willpirkle PRIVATE audio_mfx_common)
target_link_libraries(${PROJECT_NAME} PRIVATE willpirkle)

# --- memtest (Encapsulated Component) ----------------------------------------
add_library(memtest STATIC "${CMAKE_CURRENT_LIST_DIR}/memtest/memtest.c")
target_include_directories(memtest SYSTEM PUBLIC "${CMAKE_CURRENT_LIST_DIR}/memtest")
target_link_libraries(memtest PRIVATE audio_mfx_common)
target_link_libraries(${PROJECT_NAME} PRIVATE memtest)

# --- nam-core (Encapsulated Component) --------------------
if(HAS_MODEL_NAM)
    add_library(nam_core STATIC "${CMAKE_CURRENT_LIST_DIR}/nam-core/nam_model.c")
    target_include_directories(nam_core SYSTEM PUBLIC "${CMAKE_CURRENT_LIST_DIR}/nam-core")
    target_link_libraries(nam_core PRIVATE audio_mfx_common)
    target_link_libraries(${PROJECT_NAME} PRIVATE nam_core)
endif()

