# =============================================================================
# CMake toolchain file for the GNU Arm Embedded (arm-none-eabi) cross compiler.
#
# This tells CMake: "don't use the host (Windows/Linux) compiler, cross-compile
# for a bare-metal ARM target instead". It is loaded once, very early, via
# --toolchain (or the "toolchainFile" field of a CMake preset) BEFORE
# project() runs its compiler checks.
#
# The toolchain is NOT expected to be on the system PATH, so its location must
# be supplied by the user as the cache variable TOOLCHAIN_BIN_DIR, e.g.:
#   cmake --preset STM32F746G-DISCO -DTOOLCHAIN_BIN_DIR=C:/path/to/gcc/bin
# or (preferred) set it once per-machine in a local CMakeUserPresets.json
# (see CMakeUserPresets.json.example in the repository root).
# =============================================================================

# Tell CMake we are building for a generic/bare-metal target (no OS), running
# on an ARM CPU. This disables host-specific assumptions and link tests that
# would otherwise try to produce/run a native executable.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Directory containing arm-none-eabi-gcc.exe & friends. Must be provided by
# the user (see header comment above), as the TOOLCHAIN_BIN_DIR *environment*
# variable (set by CMakePresets.json/CMakeUserPresets.json "environment"
# block). An environment variable is used here (rather than a plain cache
# variable) because this toolchain file is also re-run, from scratch, by
# CMake's internal try_compile() compiler checks, which do NOT automatically
# forward ordinary -D/cache variables, but DO inherit the parent process'
# environment.
if(DEFINED ENV{TOOLCHAIN_BIN_DIR})
    set(TOOLCHAIN_BIN_DIR "$ENV{TOOLCHAIN_BIN_DIR}" CACHE PATH "Directory containing arm-none-eabi-gcc(.exe) and friends" FORCE)
else()
    set(TOOLCHAIN_BIN_DIR "" CACHE PATH "Directory containing arm-none-eabi-gcc(.exe) and friends")
endif()

if(NOT TOOLCHAIN_BIN_DIR)
    message(FATAL_ERROR
        "TOOLCHAIN_BIN_DIR is not set. Set it as an ENVIRONMENT variable "
        "pointing at the folder that contains arm-none-eabi-gcc(.exe) (e.g. "
        "via the \"environment\" block of your CMakeUserPresets.json - see "
        "CMakeUserPresets.json.example), or export it in your shell before "
        "running cmake.")
endif()

# Toolchain executable prefix, standard for the GNU Arm Embedded toolchain.
set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Executable suffix on Windows hosts (empty string on Linux/macOS hosts).
if(CMAKE_HOST_WIN32)
    set(_TOOLCHAIN_EXE_SUFFIX ".exe")
else()
    set(_TOOLCHAIN_EXE_SUFFIX "")
endif()

# The actual compilers/binutils used for every source file & link step.
set(CMAKE_C_COMPILER   "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}gcc${_TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}g++${_TOOLCHAIN_EXE_SUFFIX}")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}gcc${_TOOLCHAIN_EXE_SUFFIX}")

# Extra binutils used by post-build steps (hex/bin generation, size report,
# disassembly listing) - equivalent of Eclipse's "Create flash image" /
# "Print size" / "Create extended listing" build steps.
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}objcopy${_TOOLCHAIN_EXE_SUFFIX}" CACHE FILEPATH "objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}objdump${_TOOLCHAIN_EXE_SUFFIX}" CACHE FILEPATH "objdump")
set(CMAKE_SIZE    "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}size${_TOOLCHAIN_EXE_SUFFIX}"    CACHE FILEPATH "size")
set(CMAKE_AR      "${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN_PREFIX}ar${_TOOLCHAIN_EXE_SUFFIX}"      CACHE FILEPATH "ar")

# A bare-metal cross compiler cannot link a "hello world" test program
# (there is no OS/CRT to link against by default), so we only ask CMake to
# check that it can produce a static library during its internal compiler
# sanity checks, instead of a full executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Never try to root-search the host system's own /usr/include, /usr/lib etc.
# for a cross build - only look inside directories we explicitly add.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
