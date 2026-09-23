# =============================================================================
# Toolchain file for the GNU Arm Embedded (arm-none-eabi) cross compiler.
# =============================================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 1. Resolve and normalize the toolchain path from environment variables
if(DEFINED ENV{ARM_NONE_EABI_TOOLCHAIN_PATH})
    set(TOOLCHAIN_HINT "$ENV{ARM_NONE_EABI_TOOLCHAIN_PATH}")
    cmake_path(SET TOOLCHAIN_HINT NORMALIZE "${TOOLCHAIN_HINT}")
else()
    message(FATAL_ERROR
        "ARM_NONE_EABI_TOOLCHAIN_PATH environment variable is not set. "
        "Please provide the path to the directory containing arm-none-eabi-gcc.")
endif()

# 2. Find core compilers (CMake handles system suffixes like .exe automatically)
find_program(CMAKE_C_COMPILER_PATH   NAMES arm-none-eabi-gcc   HINTS ${TOOLCHAIN_HINT} REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER_PATH NAMES arm-none-eabi-g++   HINTS ${TOOLCHAIN_HINT} REQUIRED NO_DEFAULT_PATH)
find_program(CMAKE_ASM_COMPILER_PATH NAMES arm-none-eabi-gcc   HINTS ${TOOLCHAIN_HINT} REQUIRED NO_DEFAULT_PATH)

set(CMAKE_C_COMPILER   "${CMAKE_C_COMPILER_PATH}")
set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER_PATH}")
set(CMAKE_ASM_COMPILER "${CMAKE_ASM_COMPILER_PATH}")

# 3. Find auxiliary binary utilities used for post-build steps
find_program(OBJCOPY_PATH NAMES arm-none-eabi-objcopy HINTS ${TOOLCHAIN_HINT} REQUIRED NO_DEFAULT_PATH)
find_program(OBJDUMP_PATH NAMES arm-none-eabi-objdump HINTS ${TOOLCHAIN_HINT} REQUIRED NO_DEFAULT_PATH)
find_program(SIZE_PATH    NAMES arm-none-eabi-size    HINTS ${TOOLCHAIN_HINT} REQUIRED NO_DEFAULT_PATH)
find_program(AR_PATH      NAMES arm-none-eabi-ar      HINTS ${TOOLCHAIN_HINT} REQUIRED NO_DEFAULT_PATH)

set(CMAKE_OBJCOPY "${OBJCOPY_PATH}" CACHE FILEPATH "Path to arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "${OBJDUMP_PATH}" CACHE FILEPATH "Path to arm-none-eabi-objdump")
set(CMAKE_SIZE    "${SIZE_PATH}"    CACHE FILEPATH "Path to arm-none-eabi-size")
set(CMAKE_AR      "${AR_PATH}"      CACHE FILEPATH "Path to arm-none-eabi-ar")

# 4. Skip compiler sanity link checks for bare-metal targets
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# 5. Prevent root-searching host system paths during cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
