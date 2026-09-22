cmake_minimum_required(VERSION 3.23)

# =============================================================================
# Top-level project: selects the target board, derives all board-specific
# settings from it, declares a single executable target, and delegates to
# per-directory CMakeLists.txt files via add_subdirectory() to populate its
# sources (each one calls target_sources()/target_include_directories() on
# ${PROJECT_NAME} rather than building separate static libraries).
# =============================================================================

# Make the helper modules below (toolchain, git version) includable by name.
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(audio-multieffect LANGUAGES C CXX ASM)

# -----------------------------------------------------------------------------
# 1. Board selection
# -----------------------------------------------------------------------------
set(BOARD "" CACHE STRING "Target board: one of STM32F746G-DISCO, STM32H745I-DISCO, STM32H745I-DISCO-CM4, STM32H745I-DISCO-CM7")
set_property(CACHE BOARD PROPERTY STRINGS
    "STM32F746G-DISCO"
    "STM32H745I-DISCO"
    "STM32H745I-DISCO-CM4"
    "STM32H745I-DISCO-CM7"
)

set(_valid_boards "STM32F746G-DISCO" "STM32H745I-DISCO" "STM32H745I-DISCO-CM4" "STM32H745I-DISCO-CM7")
if(NOT BOARD IN_LIST _valid_boards)
    message(FATAL_ERROR "BOARD is '${BOARD}' but must be one of: ${_valid_boards}. Use one of the CMake presets, e.g. `cmake --preset STM32F746G-DISCO`.")
endif()
message(STATUS "Building for BOARD = ${BOARD}")

# -----------------------------------------------------------------------------
# 2. Per-board configuration: MCU, defines, linker script and which optional
#    parts of the source tree (HAS_*) this board needs. Consumed by the
#    per-directory CMakeLists.txt files added further down.
# -----------------------------------------------------------------------------
set(MCU_FLOAT_ABI hard)

set(HAS_CONTROLLER OFF)   # app/controller (UI/settings controller actor)
set(HAS_MODEL OFF)        # app/model (real DSP effect chain)
set(HAS_MODEL_NAM OFF)    # app/model/nam + libs/nam-core (Neural Amp Modeler)
set(HAS_VIEW OFF)         # app/view (console_view + lcd_view + lvgl)
set(HAS_LITTLEFS OFF)     # libs/littlefs (settings/presets storage)
set(DUAL_CORE_APP OFF)    # this image is one half of a dual-core (H745) build

if(BOARD STREQUAL "STM32F746G-DISCO")
    set(MCU_FAMILY        STM32F7)
    set(MCU_CORE          CM7)
    set(MCU_CPU           cortex-m7)
    set(MCU_FPU           fpv5-sp-d16)
    set(LINKER_SCRIPT     "${PROJECT_SOURCE_DIR}/system/stm32f746/arm-cortex-m.ld")
    set(SYSTEM_DIR        "${PROJECT_SOURCE_DIR}/system/stm32f746")
    set(DRIVERS_MCU_DIR   "${PROJECT_SOURCE_DIR}/drivers/stm32f7")
    set(HAL_MCU_DIR       "${PROJECT_SOURCE_DIR}/hal/stm32f7")
    set(CMSIS_DSP_LIB     arm_cortexM7lfsp_math)
    set(FREERTOS_PORT     GCC_ARM_CM7)
    set(FREERTOS_PORT_DIR "${PROJECT_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM7/r0p1")

    set(HAS_CONTROLLER ON)
    set(HAS_MODEL ON)
    set(HAS_MODEL_NAM OFF)          # CFG_DISABLE_NEURAL_AMP_MODELER: no NAM on the F7
    set(HAS_VIEW ON)
    set(HAS_LITTLEFS ON)

    set(BOARD_DEFINES
        STM32F746xx
        ARM_MATH_CM7
        STM32F7
        CORE_CM7
        CFG_FS_CALIB=-9
        CFG_TUSB_MCU=OPT_MCU_STM32F7
        CFG_DISABLE_NEURAL_AMP_MODELER
    )

elseif(BOARD STREQUAL "STM32H745I-DISCO")
    set(MCU_FAMILY        STM32H7)
    set(MCU_CORE          CM7)
    set(MCU_CPU           cortex-m7)
    set(MCU_FPU           fpv5-d16)
    set(LINKER_SCRIPT     "${PROJECT_SOURCE_DIR}/system/stm32h745/arm-cortex-m.ld")
    set(SYSTEM_DIR        "${PROJECT_SOURCE_DIR}/system/stm32h745")
    set(DRIVERS_MCU_DIR   "${PROJECT_SOURCE_DIR}/drivers/stm32h7")
    set(HAL_MCU_DIR       "${PROJECT_SOURCE_DIR}/hal/stm32h7")
    set(CMSIS_DSP_LIB     arm_cortexM7lfsp_math)
    set(FREERTOS_PORT     GCC_ARM_CM4F)
    set(FREERTOS_PORT_DIR "${PROJECT_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM4F")

    set(HAS_CONTROLLER ON)
    set(HAS_MODEL ON)
    set(HAS_MODEL_NAM ON)
    set(HAS_VIEW ON)
    set(HAS_LITTLEFS ON)

    set(BOARD_DEFINES
        STM32H745xx
        STM32H7
        ARM_MATH_CM7
        CORE_CM7
        CFG_FS_CALIB=-2
        CFG_TUSB_MCU=OPT_MCU_STM32H7
        "NAM_DTCM=__attribute__((section(\".dtcmram\")))"
    )

elseif(BOARD STREQUAL "STM32H745I-DISCO-CM7")
    set(MCU_FAMILY        STM32H7)
    set(MCU_CORE          CM7)
    set(MCU_CPU           cortex-m7)
    set(MCU_FPU           fpv5-d16)
    set(LINKER_SCRIPT     "${PROJECT_SOURCE_DIR}/system/stm32h745/arm-cortex-m7.ld")
    set(SYSTEM_DIR        "${PROJECT_SOURCE_DIR}/system/stm32h745")
    set(DRIVERS_MCU_DIR   "${PROJECT_SOURCE_DIR}/drivers/stm32h7")
    set(HAL_MCU_DIR       "${PROJECT_SOURCE_DIR}/hal/stm32h7")
    set(CMSIS_DSP_LIB     arm_cortexM7lfsp_math)
    set(FREERTOS_PORT     GCC_ARM_CM4F)
    set(FREERTOS_PORT_DIR "${PROJECT_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM4F")

    set(DUAL_CORE_APP ON)
    set(HAS_CONTROLLER OFF)   # UI lives on the CM4 core, this is the DSP core
    set(HAS_MODEL ON)
    set(HAS_MODEL_NAM ON)
    set(HAS_VIEW OFF)
    set(HAS_LITTLEFS OFF)     # storage/filesystem lives on the CM4 core

    set(BOARD_DEFINES
        STM32H745xx
        STM32H7
        ARM_MATH_CM7
        CORE_CM7
        DUAL_CORE_APP
        CFG_FS_CALIB=-2
        CFG_TUSB_MCU=OPT_MCU_STM32H7
        "NAM_DTCM=__attribute__((section(\".dtcmram\")))"
    )

elseif(BOARD STREQUAL "STM32H745I-DISCO-CM4")
    set(MCU_FAMILY        STM32H7)
    set(MCU_CORE          CM4)
    set(MCU_CPU           cortex-m4)
    set(MCU_FPU           fpv4-sp-d16)
    set(LINKER_SCRIPT     "${PROJECT_SOURCE_DIR}/system/stm32h745/arm-cortex-m4.ld")
    set(SYSTEM_DIR        "${PROJECT_SOURCE_DIR}/system/stm32h745")
    set(DRIVERS_MCU_DIR   "${PROJECT_SOURCE_DIR}/drivers/stm32h7")
    set(HAL_MCU_DIR       "${PROJECT_SOURCE_DIR}/hal/stm32h7")
    set(CMSIS_DSP_LIB     arm_cortexM4lf_math)
    set(FREERTOS_PORT     GCC_ARM_CM4F)
    set(FREERTOS_PORT_DIR "${PROJECT_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM4F")

    set(DUAL_CORE_APP ON)
    set(HAS_CONTROLLER ON)    # UI + settings + presets live on the CM4 core
    set(HAS_MODEL OFF)        # real effect DSP lives on the CM7 core (IPC proxy instead)
    set(HAS_MODEL_NAM OFF)
    set(HAS_VIEW ON)
    set(HAS_LITTLEFS ON)

    set(BOARD_DEFINES
        STM32H745xx
        STM32H7
        ARM_MATH_CM4
        CORE_CM4
        DUAL_CORE_APP
        CFG_FS_CALIB=-2
        CFG_TUSB_MCU=OPT_MCU_STM32H7
    )
endif()

# -----------------------------------------------------------------------------
# 3. Git revision string (replaces makefile.defs' GIT_REVISION shell command)
# -----------------------------------------------------------------------------
include(git-version)
get_git_revision(GIT_REVISION)
message(STATUS "GIT_REVISION = ${GIT_REVISION}")

# -----------------------------------------------------------------------------
# 4. Firmware executable target
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    SUFFIX ".elf"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

add_library(audio_mfx_common INTERFACE)

target_compile_definitions(audio_mfx_common INTERFACE
    __HEAP_SIZE=0x00020000
    __STACK_SIZE=0x00001000
    _LITE_EXIT
    LFS_DEFINES=libs/lfs_conf.h
    CFG_TUSB_OS=OPT_OS_FREERTOS
    GIT_REVISION="${GIT_REVISION}"
    ${BOARD_DEFINES}
)

target_include_directories(audio_mfx_common INTERFACE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/cmsis
    ${PROJECT_SOURCE_DIR}/libs
    ${PROJECT_SOURCE_DIR}/libs/tinyusb/src
    ${PROJECT_SOURCE_DIR}/hal
    ${HAL_MCU_DIR}
)
# nlohmann_json is linked in as nlohmann_json::nlohmann_json (see libs/CMakeLists.txt),
# which brings in its own include path.

target_link_libraries(${PROJECT_NAME} PRIVATE audio_mfx_common)

# -----------------------------------------------------------------------------
# 5. Compiler/linker flags, equivalent to the Eclipse ARM Cross GCC toolchain
#    settings shared by all 4 configurations (per-board CPU/FPU flags above).
# -----------------------------------------------------------------------------
set(MCU_FLAGS
    -mcpu=${MCU_CPU}
    -mthumb
    -mfpu=${MCU_FPU}
    -mfloat-abi=${MCU_FLOAT_ABI}
)

target_compile_options(audio_mfx_common INTERFACE
    ${MCU_FLAGS}
    -O3
    -fmessage-length=0
    -fsigned-char
    -ffunction-sections
    -fdata-sections
    -fsingle-precision-constant
    -g
    -Wall
    -Wextra
    -Wuninitialized
    -Wno-unused-parameter
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions;-fno-rtti>
)

target_link_options(audio_mfx_common INTERFACE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -u_printf_float
    -u_scanf_float
    --specs=nano.specs
    --specs=nosys.specs
    -L${PROJECT_SOURCE_DIR}/cmsis/dsp
)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

# -----------------------------------------------------------------------------
# 6. Source tree
# -----------------------------------------------------------------------------
add_subdirectory(app)
add_subdirectory(drivers)
add_subdirectory(middlewares)
add_subdirectory(system)
add_subdirectory(libs)
add_subdirectory(cmsis)

target_link_libraries(audio_mfx_common INTERFACE cmsis_dsp)
# -----------------------------------------------------------------------------
# 7. Post-build steps: .hex/.bin generation and size report (equivalent of
#    Eclipse's "Create flash image" / "Print size" post-build steps).
# -----------------------------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}>
    BYPRODUCTS ${PROJECT_NAME}.hex ${PROJECT_NAME}.bin
    COMMENT "Generating .hex/.bin and printing size for ${BOARD}"
    VERBATIM
)

# Extended disassembly listing (equivalent of Eclipse's "Create extended
# listing" step), built on demand only: `cmake --build . --target listing`.
add_custom_target(listing
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers --wide $<TARGET_FILE:${PROJECT_NAME}> > ${PROJECT_NAME}.lst
    DEPENDS ${PROJECT_NAME}
    BYPRODUCTS ${PROJECT_NAME}.lst
    COMMENT "Generating extended disassembly listing"
    VERBATIM
)
