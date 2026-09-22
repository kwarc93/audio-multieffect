# =============================================================================
# Toolchain file for the GNU Arm Embedded (arm-none-eabi) cross compiler.
# Loaded via --toolchain / a preset's "toolchainFile" field, before project()
# runs its compiler checks.
#
# The toolchain directory is not assumed to be on PATH; it must be supplied
# via the TOOLCHAIN_BIN_DIR environment variable, e.g.:
#   cmake --preset STM32F746G-DISCO
# with TOOLCHAIN_BIN_DIR set in the environment (see CMakeUserPresets.json.example).
# =============================================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# TOOLCHAIN_BIN_DIR must come from the environment, not a plain cache/-D
# variable: CMake's internal try_compile() compiler checks re-run this
# toolchain file in a fresh process that inherits the environment but does
# not forward arbitrary cache variables.
if(DEFINED ENV{TOOLCHAIN_BIN_DIR})
    set(TOOLCHAIN_BIN_DIR "$ENV{TOOLCHAIN_BIN_DIR}" CACHE PATH "Directory containing arm-none-eabi-gcc(.exe) and friends" FORCE)
else()
    set(TOOLCHAIN_BIN_DIR "" CACHE PATH "Directory containing arm-none-eabi-gcc(.exe) and friends")
endif()

if(NOT TOOLCHAIN_BIN_DIR)
    message(FATAL_ERROR
        "TOOLCHAIN_BIN_DIR is not set. Set it as an environment variable "
        "pointing at the folder that contains arm-none-eabi-gcc(.exe), e.g. "
        "via the \"environment\" block of CMakeUserPresets.json (see "
        "CMakeUserPresets.json.example), or export it before running cmake.")
endif()

set(TOOLCHAIN_PREFIX arm-none-eabi-)

if(CMAKE_HOST_WIN32)
    set(_TOOLCHAIN_EXE_SUFFIX ".exe")
else()
    set(_TOOLCHAIN_EXE_SUFFIX "")
endif()

set(CMAKE_C_COMPILER   "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}gcc${_TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}g++${_TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}gcc${_TOOLCHAIN_EXE_SUFFIX}")

# Used by the post-build steps in the top-level CMakeLists.txt (hex/bin
# generation, size report, disassembly listing).
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}objcopy${_TOOLCHAIN_EXE_SUFFIX}" CACHE FILEPATH "objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}objdump${_TOOLCHAIN_EXE_SUFFIX}" CACHE FILEPATH "objdump")
set(CMAKE_SIZE    "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}size${_TOOLCHAIN_EXE_SUFFIX}"    CACHE FILEPATH "size")
set(CMAKE_AR      "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}ar${_TOOLCHAIN_EXE_SUFFIX}"      CACHE FILEPATH "ar")

# A bare-metal cross compiler cannot link an executable without a target-
# specific CRT/linker script, so compiler sanity checks only need to produce
# sanity checks, instead of a full executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Never try to root-search the host system's own /usr/include, /usr/lib etc.
# for a cross build - only look inside directories we explicitly add.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
