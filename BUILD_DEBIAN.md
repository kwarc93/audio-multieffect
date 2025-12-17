# Building audio-multieffect on Debian/Ubuntu

This guide explains how to build the STM32F746G-DISCO firmware on Debian/Ubuntu Linux using the command line and Make.

## Prerequisites

### 1. Install Required Packages

```bash
# Update package list
sudo apt-get update

# Install ARM toolchain and build tools
sudo apt-get install -y \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    build-essential \
    git \
    make

# Install st-link tools for flashing (optional, for programming the board)
sudo apt-get install -y stlink-tools

# Install additional development tools (optional but recommended)
sudo apt-get install -y \
    gdb-multiarch \
    openocd
```

### 2. Verify ARM Toolchain Installation

```bash
arm-none-eabi-gcc --version
```

You should see output similar to:
```
arm-none-eabi-gcc (15:10.3-2021.07-4) 10.3.1 20210621 (release)
```

**Note:** The version may vary. This project requires at least GCC 7.0 or newer for C++17 support.

### 3. Clone the Repository

If you haven't already cloned the repository with submodules:

```bash
git clone --recurse-submodules https://github.com/kwarc93/audio-multieffect.git
cd audio-multieffect
```

If you already cloned without submodules, initialize them:

```bash
git submodule update --init --recursive
```

## Building the Project

### Build for STM32F746G-DISCO

```bash
make -j$(nproc)
```

This will:
1. Compile all source files
2. Link the firmware
3. Generate the following files in the `build/` directory:
   - `audio-multieffect.elf` - ELF executable with debug symbols
   - `audio-multieffect.hex` - Intel HEX format for programming
   - `audio-multieffect.bin` - Raw binary format for programming
   - `audio-multieffect.map` - Memory map file

### Build Options

#### Debug Build (default)
```bash
make DEBUG=1
```

#### Release Build (optimized)
```bash
make DEBUG=0 OPT=-O3
```

#### Clean Build
```bash
make clean
make -j$(nproc)
```

#### Parallel Build
Use `-j` flag to speed up compilation:
```bash
make -j$(nproc)  # Uses all CPU cores
make -j4         # Uses 4 cores
```

## Flashing the Firmware

### Using st-flash (Recommended)

Connect your STM32F746G-DISCO board via USB and run:

```bash
make flash
```

Or manually:

```bash
st-flash write build/audio-multieffect.bin 0x8000000
```

### Using OpenOCD

```bash
openocd -f board/stm32f7discovery.cfg -c "program build/audio-multieffect.elf verify reset exit"
```

### Using STM32CubeProgrammer

If you have STM32CubeProgrammer installed, you can use it to flash the `.hex` or `.bin` file through its GUI or CLI.

## Debugging

### Using GDB with OpenOCD

Terminal 1 - Start OpenOCD:
```bash
openocd -f board/stm32f7discovery.cfg
```

Terminal 2 - Start GDB:
```bash
gdb-multiarch build/audio-multieffect.elf
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

## Troubleshooting

### Error: "arm-none-eabi-gcc: command not found"

The ARM toolchain is not installed or not in your PATH. Install it:
```bash
sudo apt-get install gcc-arm-none-eabi
```

### Error: "No rule to make target..."

Make sure you've initialized all git submodules:
```bash
git submodule update --init --recursive
```

### Error: "region RAM overflowed with stack"

The firmware is too large for the available RAM. Try:
1. Build with optimization: `make clean && make OPT=-Os`
2. Reduce the number of enabled features in configuration files
3. Check for memory leaks or excessive stack usage

### Error: "undefined reference to..."

This usually means:
1. Missing source files in the Makefile
2. Missing library dependencies
3. Incorrect linking order

Check that all required source files are listed in the Makefile.

### Compilation is very slow

Use parallel compilation:
```bash
make clean
make -j$(nproc)
```

### st-flash: "Failed to connect to target"

1. Check USB connection
2. Check that ST-LINK drivers are installed
3. Try running with sudo: `sudo st-flash write build/audio-multieffect.bin 0x8000000`
4. Check that no other program is using the ST-LINK interface

## Memory Usage

After building, you can check memory usage:

```bash
arm-none-eabi-size build/audio-multieffect.elf
```

Output example:
```
   text    data     bss     dec     hex filename
 123456    1234   12345  136035   21363 build/audio-multieffect.elf
```

Where:
- **text**: Code size (Flash)
- **data**: Initialized data (Flash + RAM)
- **bss**: Uninitialized data (RAM)

## Project Structure

```
audio-multieffect/
├── Makefile              # Main build file (created)
├── BUILD_DEBIAN.md       # This file
├── main.cpp              # Application entry point
├── app/                  # Application code
│   ├── controller/       # MVC Controller
│   ├── model/            # MVC Model (DSP effects)
│   ├── modules/          # Settings and presets
│   └── view/             # MVC View (UI)
├── drivers/              # Hardware drivers
│   └── stm32f7/          # STM32F7-specific drivers
├── hal/                  # Hardware Abstraction Layer
│   └── stm32f7/          # STM32F7 HAL implementation
├── system/               # System startup code
│   └── stm32f746/        # Startup and linker scripts
├── cmsis/                # ARM CMSIS library
├── rtos/                 # FreeRTOS
├── libs/                 # Third-party libraries
│   ├── lvgl/             # GUI library
│   ├── littlefs/         # Filesystem
│   ├── tinyusb/          # USB stack
│   ├── nlohmann_json/    # JSON library
│   └── willpirkle/       # DSP library
├── middlewares/          # Middleware components
└── build/                # Build output (generated)
```

## Additional Resources

- [STM32F746G-DISCO Board](https://www.st.com/en/evaluation-tools/32f746gdiscovery.html)
- [ARM GCC Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
- [OpenOCD Documentation](http://openocd.org/doc/)
- [ST-LINK Tools](https://github.com/stlink-org/stlink)

## License

See LICENSE.md in the project root.
