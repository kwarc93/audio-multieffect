cmake_minimum_required(VERSION 3.23)

# =============================================================================
# Top-level CMake project for the audio-multieffect firmware.
#
# This file is the single "control room" of the whole build:
#   1. it figures out which of the 4 hardware targets (BOARD) we are building,
#   2. it derives all MCU/board specific settings (CPU, FPU, defines, linker
#      script, ...) from that one BOARD variable,
#   3. it declares ONE executable target (the firmware image) and then walks
#      down into app/, drivers/, hal/, middlewares/, system/ and libs/ via
#      add_subdirectory() - each of those directories has its own small
#      CMakeLists.txt that calls target_sources()/target_include_directories()
#      on this same executable target to add "its" files.
#
# This mirrors the Eclipse CDT project, which had 4 build configurations
# (STM32F746G-DISCO, STM32H745I-DISCO, STM32H745I-DISCO-CM4,
# STM32H745I-DISCO-CM7), each compiling a different subset of the source tree
# for a different MCU core.
# =============================================================================

# Make our helper .cmake files (toolchain, git version, ...) available via
# include(...) without needing a relative "cmake/" prefix everywhere.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(audio-multieffect LANGUAGES C CXX ASM)

# -----------------------------------------------------------------------------
# 1. Board selection
#
# BOARD picks which of the 4 original Eclipse build configurations we produce.
# It must be set before the rest of this file can do anything useful, which is
# why CMakePresets.json defines one preset per board (see that file).
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
# 2. Per-board configuration
#
# Everything that differed between the 4 Eclipse configurations (MCU core,
# FPU, preprocessor defines, linker script, CMSIS-DSP variant, FreeRTOS port,
# and which parts of app/ + libs/ get compiled) is derived here, once, from
# BOARD. The rest of the CMake tree only looks at the variables set below.
# -----------------------------------------------------------------------------

# --- flags shared by every board -------------------------------------------
set(MCU_INSTRUCTION_SET thumb)
set(MCU_FLOAT_ABI hard)

# Which optional parts of the source tree this board needs. Read by the
# CMakeLists.txt files in app/, libs/ etc.
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
    set(LINKER_SCRIPT     "${CMAKE_SOURCE_DIR}/system/stm32f746/arm-cortex-m.ld")
    set(SYSTEM_DIR        "${CMAKE_SOURCE_DIR}/system/stm32f746")
    set(DRIVERS_MCU_DIR   "${CMAKE_SOURCE_DIR}/drivers/stm32f7")
    set(HAL_MCU_DIR       "${CMAKE_SOURCE_DIR}/hal/stm32f7")
    set(CMSIS_DSP_LIB     arm_cortexM7lfsp_math)
    set(FREERTOS_PORT     GCC_ARM_CM7)
    set(FREERTOS_PORT_DIR "${CMAKE_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM7/r0p1")

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
    set(LINKER_SCRIPT     "${CMAKE_SOURCE_DIR}/system/stm32h745/arm-cortex-m.ld")
    set(SYSTEM_DIR        "${CMAKE_SOURCE_DIR}/system/stm32h745")
    set(DRIVERS_MCU_DIR   "${CMAKE_SOURCE_DIR}/drivers/stm32h7")
    set(HAL_MCU_DIR       "${CMAKE_SOURCE_DIR}/hal/stm32h7")
    set(CMSIS_DSP_LIB     arm_cortexM7lfsp_math)
    set(FREERTOS_PORT     GCC_ARM_CM4F)
    set(FREERTOS_PORT_DIR "${CMAKE_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM4F")

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
    set(LINKER_SCRIPT     "${CMAKE_SOURCE_DIR}/system/stm32h745/arm-cortex-m7.ld")
    set(SYSTEM_DIR        "${CMAKE_SOURCE_DIR}/system/stm32h745")
    set(DRIVERS_MCU_DIR   "${CMAKE_SOURCE_DIR}/drivers/stm32h7")
    set(HAL_MCU_DIR       "${CMAKE_SOURCE_DIR}/hal/stm32h7")
    set(CMSIS_DSP_LIB     arm_cortexM7lfsp_math)
    set(FREERTOS_PORT     GCC_ARM_CM4F)
    set(FREERTOS_PORT_DIR "${CMAKE_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM4F")

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
    set(LINKER_SCRIPT     "${CMAKE_SOURCE_DIR}/system/stm32h745/arm-cortex-m4.ld")
    set(SYSTEM_DIR        "${CMAKE_SOURCE_DIR}/system/stm32h745")
    set(DRIVERS_MCU_DIR   "${CMAKE_SOURCE_DIR}/drivers/stm32h7")
    set(HAL_MCU_DIR       "${CMAKE_SOURCE_DIR}/hal/stm32h7")
    set(CMSIS_DSP_LIB     arm_cortexM4lf_math)
    set(FREERTOS_PORT     GCC_ARM_CM4F)
    set(FREERTOS_PORT_DIR "${CMAKE_SOURCE_DIR}/libs/FreeRTOS/portable/GCC/ARM_CM4F")

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
# 4. The firmware executable target
#
# Every add_subdirectory() below adds its own sources/includes to this one
# target with target_sources()/target_include_directories() - there is
# intentionally only a single link step, just like the original Makefile.
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}" SUFFIX ".elf")

# Defines common to every board (identical to the "Defined symbols" shared by
# all 4 Eclipse configurations).
target_compile_definitions(${PROJECT_NAME} PRIVATE
    __HEAP_SIZE=0x00020000
    __STACK_SIZE=0x00001000
    _LITE_EXIT
    LFS_DEFINES=libs/lfs_conf.h
    CFG_TUSB_OS=OPT_OS_FREERTOS
    GIT_REVISION="${GIT_REVISION}"
    ${BOARD_DEFINES}
)

# Repo root, CMSIS and third-party single-header libs used everywhere.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/cmsis
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/libs/tinyusb/src
    ${CMAKE_SOURCE_DIR}/hal
    ${HAL_MCU_DIR}
)
# nlohmann_json's own CMakeLists.txt (libs/nlohmann_json) is added later via
# add_subdirectory(libs) and linked in as the nlohmann_json::nlohmann_json
# target, which brings in its single_include path automatically.

# -----------------------------------------------------------------------------
# 5. Common ARM GCC compiler/linker flags
#
# These reproduce the "Optimization"/"Debugging"/"Warnings" option pages of
# the Eclipse "ARM Cross GCC" toolchain settings, applied identically to every
# board (per-board CPU/FPU flags are added separately below).
# -----------------------------------------------------------------------------
set(MCU_FLAGS
    -mcpu=${MCU_CPU}
    -mthumb
    -mfpu=${MCU_FPU}
    -mfloat-abi=${MCU_FLOAT_ABI}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -O3                              # "Optimize most (-O3)"
    -fmessage-length=0
    -fsigned-char
    -ffunction-sections
    -fdata-sections
    -fsingle-precision-constant
    -g                               # "Debug level: default"
    -Wall
    -Wextra                          # "Warn on various unused elements (-Wunused)" + uninitialized
    -Wuninitialized
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
)

target_link_options(${PROJECT_NAME} PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -u_printf_float
    -u_scanf_float
    --specs=nano.specs
    --specs=nosys.specs
    -L${CMAKE_SOURCE_DIR}/cmsis/dsp
)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

# CMSIS-DSP ships as prebuilt static libraries (see cmsis/CMakeLists.txt).
add_subdirectory(cmsis)
target_link_libraries(${PROJECT_NAME} PRIVATE cmsis_dsp)

# -----------------------------------------------------------------------------
# 6. Walk down into the rest of the source tree.
# -----------------------------------------------------------------------------
add_subdirectory(app)
add_subdirectory(drivers)
add_subdirectory(middlewares)
add_subdirectory(system)
add_subdirectory(libs)

# -----------------------------------------------------------------------------
# 7. Post-build steps: .hex/.bin generation, size report, disassembly listing
#    (equivalent of Eclipse's "Create flash image" / "Print size" /
#    "Create extended listing" post-build steps).
# -----------------------------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}>
    BYPRODUCTS ${PROJECT_NAME}.hex ${PROJECT_NAME}.bin
    COMMENT "Generating .hex/.bin and printing size for ${BOARD}"
    VERBATIM
)

# Optional extended disassembly listing, only built on demand (`cmake --build . --target listing`)
# since it is slow and not needed for a normal build/flash cycle.
add_custom_target(listing
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers --wide $<TARGET_FILE:${PROJECT_NAME}> > ${PROJECT_NAME}.lst
    DEPENDS ${PROJECT_NAME}
    BYPRODUCTS ${PROJECT_NAME}.lst
    COMMENT "Generating extended disassembly listing"
    VERBATIM
)
