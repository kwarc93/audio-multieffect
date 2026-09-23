cmake_minimum_required(VERSION 3.23)

# =============================================================================
# Top-level Project Configuration
# Defines the target board topology, compiler toolchain requirements, 
# architecture flags, and coordinates the modular source subdirectories.
# =============================================================================

# Make the helper modules below (toolchain, git version) includable by name.
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(audio-multieffect LANGUAGES C CXX ASM)

# -----------------------------------------------------------------------------
# 1. Target Board Selection
# -----------------------------------------------------------------------------
set(BOARD "" CACHE STRING "Target hardware platform selection")
set_property(CACHE BOARD PROPERTY STRINGS
    "STM32F746G-DISCO"
    "STM32H745I-DISCO"
    "STM32H745I-DISCO-CM4"
    "STM32H745I-DISCO-CM7"
)

set(VALID_BOARDS "STM32F746G-DISCO" "STM32H745I-DISCO" "STM32H745I-DISCO-CM4" "STM32H745I-DISCO-CM7")
if(NOT BOARD IN_LIST VALID_BOARDS)
    message(FATAL_ERROR "BOARD '${BOARD}' is invalid. Must be one of: ${VALID_BOARDS}. Please configure using a valid CMake preset.")
endif()
message(STATUS "Configuring project for target board: ${BOARD}")

# -----------------------------------------------------------------------------
# 2. Hardware Architecture & Feature Matrix Definitions
# -----------------------------------------------------------------------------
set(MCU_FLOAT_ABI hard)

# Initialize global hardware capability flags
set(HAS_CONTROLLER OFF)
set(HAS_MODEL OFF)
set(HAS_VIEW OFF)
set(HAS_LITTLEFS OFF)
set(HAS_TINYUSB OFF)
set(HAS_EFFECT_NAM OFF)

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
    set(HAS_VIEW ON)
    set(HAS_LITTLEFS ON)
    set(HAS_TINYUSB ON)

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
    set(HAS_VIEW ON)
    set(HAS_LITTLEFS ON)
    set(HAS_TINYUSB ON)
    set(HAS_EFFECT_NAM ON)

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

    set(HAS_MODEL ON)
    set(HAS_EFFECT_NAM ON)
    set(HAS_TINYUSB ON)

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

    set(HAS_CONTROLLER ON)
    set(HAS_VIEW ON)
    set(HAS_LITTLEFS ON)

    set(BOARD_DEFINES
        STM32H745xx
        STM32H7
        ARM_MATH_CM4
        CORE_CM4
        DUAL_CORE_APP
        CFG_FS_CALIB=-2
    )
endif()

# -----------------------------------------------------------------------------
# 3. Dynamic Versioning Integration
# -----------------------------------------------------------------------------
include(git-version)
get_git_revision(GIT_REVISION)
message(STATUS "Current Project Git Revision: ${GIT_REVISION}")

# -----------------------------------------------------------------------------
# 4. Target Definition & Common Interface Library
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    SUFFIX ".elf"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Common settings container to propagate configuration definitions across directories
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

target_link_libraries(${PROJECT_NAME} PRIVATE audio_mfx_common)

# -----------------------------------------------------------------------------
# 5. Compiler and Linker Toolchain Flags
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
    -Wunused
    -Wuninitialized
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions;-fno-rtti>
)

target_link_options(audio_mfx_common INTERFACE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -u_printf_float
#   -u_scanf_float
    --specs=nano.specs
    --specs=nosys.specs
    -L${PROJECT_SOURCE_DIR}/cmsis/dsp
)

# Track linker script changes to force a target relink
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT}")

# -----------------------------------------------------------------------------
# 6. Source Tree Subdirectories Execution
# -----------------------------------------------------------------------------
add_subdirectory(app)
add_subdirectory(drivers)
add_subdirectory(middlewares)
add_subdirectory(system)
add_subdirectory(libs)
add_subdirectory(cmsis)

# Bind the processed CMSIS library back into the configuration interface target
target_link_libraries(audio_mfx_common INTERFACE cmsis_dsp)

# -----------------------------------------------------------------------------
# 7. Post-Build Artifact Generation (Hex, Binary & Memory Metrics)
# -----------------------------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}>
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"
    COMMENT "Generating Flash image deployment files (.hex/.bin) and evaluating memory size for ${BOARD}"
    VERBATIM
)

# Optional diagnostic target for compilation analysis: run `cmake --build . --target listing`
add_custom_target(listing
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers --wide $<TARGET_FILE:${PROJECT_NAME}> > "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.lst"
    DEPENDS ${PROJECT_NAME}
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.lst"
    COMMENT "Generating extended application disassembly listing"
    VERBATIM
)
