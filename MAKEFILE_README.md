# Makefile Build System Guide

This document explains how to use the Makefile to build the audio-multieffect project for different STM32 boards.

## Overview

The Makefile supports building firmware for 4 different board configurations:
1. **STM32F746G-DISCO** - Single core Cortex-M7
2. **STM32H745I-DISCO** - Single core Cortex-M7
3. **STM32H745I-DISCO-CM7** - Dual core, Cortex-M7 (DSP operations)
4. **STM32H745I-DISCO-CM4** - Dual core, Cortex-M4 (GUI operations)

## Quick Start

### Build for Default Board (STM32F746G-DISCO)
```bash
make -j$(nproc)
```

### Build for Specific Board
```bash
# STM32H745I-DISCO (single core)
make BOARD=STM32H745I-DISCO -j$(nproc)

# STM32H745I-DISCO (dual core - Cortex-M7)
make BOARD=STM32H745I-DISCO-CM7 -j$(nproc)

# STM32H745I-DISCO (dual core - Cortex-M4)
make BOARD=STM32H745I-DISCO-CM4 -j$(nproc)
```

### Build All Boards
```bash
make all-boards
```

## Build Options

### Debug vs Release Build

**Debug Build (default):**
```bash
make BOARD=STM32F746G-DISCO DEBUG=1
```

**Release Build (optimized):**
```bash
make BOARD=STM32F746G-DISCO DEBUG=0 OPT=-O3
```

### Optimization Levels
- `-Og` - Optimize for debugging (default)
- `-O0` - No optimization
- `-O1` - Basic optimization
- `-O2` - Moderate optimization
- `-O3` - Aggressive optimization
- `-Os` - Optimize for size

Example:
```bash
make BOARD=STM32H745I-DISCO OPT=-Os -j4
```

### Parallel Compilation
Use the `-j` flag to speed up compilation:
```bash
make BOARD=STM32F746G-DISCO -j$(nproc)  # Use all CPU cores
make BOARD=STM32F746G-DISCO -j4         # Use 4 cores
```

## Cleaning Build Artifacts

### Clean Current Board
```bash
make clean BOARD=STM32F746G-DISCO
```

### Clean All Boards
```bash
make clean-all
```

## Flashing Firmware

After building, flash the firmware to your board:
```bash
make flash BOARD=STM32F746G-DISCO
```

Or manually:
```bash
st-flash write build/STM32F746G-DISCO/audio-multieffect.bin 0x8000000
```

## Build Output

Build artifacts are organized by board in the `build/` directory:
```
build/
├── STM32F746G-DISCO/
│   ├── audio-multieffect.elf    # ELF executable with debug symbols
│   ├── audio-multieffect.hex    # Intel HEX format
│   ├── audio-multieffect.bin    # Raw binary format
│   ├── audio-multieffect.map    # Memory map file
│   ├── *.o                      # Object files
│   └── *.d                      # Dependency files
├── STM32H745I-DISCO/
├── STM32H745I-DISCO-CM7/
└── STM32H745I-DISCO-CM4/
```

## Board-Specific Details

### STM32F746G-DISCO
- **CPU:** Cortex-M7 @ 216 MHz
- **FPU:** Single precision (FPv5-SP-D16)
- **RAM:** 340 KB
- **Flash:** 1 MB
- **Math Library:** `libarm_cortexM7lfsp_math.a`

### STM32H745I-DISCO (Single Core)
- **CPU:** Cortex-M7 @ 480 MHz
- **FPU:** Double precision (FPv5-D16)
- **RAM:** 1 MB
- **Flash:** 2 MB
- **Math Library:** `libarm_cortexM7lfdp_math.a`

### STM32H745I-DISCO-CM7 (Dual Core)
- **CPU:** Cortex-M7 @ 480 MHz (DSP core)
- **FPU:** Double precision (FPv5-D16)
- **Role:** Handles DSP operations
- **Math Library:** `libarm_cortexM7lfdp_math.a`
- **Note:** Requires CM4 firmware to be flashed separately

### STM32H745I-DISCO-CM4 (Dual Core)
- **CPU:** Cortex-M4 @ 240 MHz (GUI core)
- **FPU:** Single precision (FPv4-SP-D16)
- **Role:** Handles GUI operations
- **Math Library:** `libarm_cortexM4lf_math.a`
- **Note:** Requires CM7 firmware to be flashed separately

## Makefile Variables

### User-Configurable Variables
- `BOARD` - Target board (default: `STM32F746G-DISCO`)
- `DEBUG` - Enable debug build (default: `1`)
- `OPT` - Optimization level (default: `-Og`)
- `GCC_PATH` - Path to ARM toolchain (optional)

### Example with Custom Toolchain Path
```bash
make BOARD=STM32F746G-DISCO GCC_PATH=/opt/gcc-arm-none-eabi/bin
```

## Common Build Scenarios

### Development Build (Fast compilation, with debug info)
```bash
make BOARD=STM32F746G-DISCO DEBUG=1 OPT=-Og -j$(nproc)
```

### Production Build (Optimized for performance)
```bash
make BOARD=STM32F746G-DISCO DEBUG=0 OPT=-O3 -j$(nproc)
```

### Size-Optimized Build (Minimize firmware size)
```bash
make BOARD=STM32F746G-DISCO DEBUG=0 OPT=-Os -j$(nproc)
```

### Dual-Core Build (STM32H745I-DISCO)
```bash
# Build and flash CM7 core first
make BOARD=STM32H745I-DISCO-CM7 -j$(nproc)
st-flash write build/STM32H745I-DISCO-CM7/audio-multieffect.bin 0x8000000

# Then build and flash CM4 core
make BOARD=STM32H745I-DISCO-CM4 -j$(nproc)
st-flash write build/STM32H745I-DISCO-CM4/audio-multieffect.bin 0x8100000
```

## Troubleshooting

### Error: "arm-none-eabi-gcc: command not found"
Install the ARM toolchain:
```bash
sudo apt-get install gcc-arm-none-eabi
```

### Error: "No rule to make target..."
Initialize git submodules:
```bash
git submodule update --init --recursive
```

### Error: "region RAM overflowed"
Try optimizing for size:
```bash
make clean BOARD=<your-board>
make BOARD=<your-board> OPT=-Os -j$(nproc)
```

### Build is very slow
Use parallel compilation:
```bash
make BOARD=<your-board> -j$(nproc)
```

### Wrong board configuration
Make sure to specify the correct BOARD variable:
```bash
make clean BOARD=<your-board>
make BOARD=<your-board> -j$(nproc)
```

## Getting Help

Display available make targets and options:
```bash
make help
```

## Memory Usage

Check memory usage after building:
```bash
arm-none-eabi-size build/STM32F746G-DISCO/audio-multieffect.elf
```

Output example:
```
   text    data     bss     dec     hex filename
 123456    1234   12345  136035   21363 build/STM32F746G-DISCO/audio-multieffect.elf
```

Where:
- **text** - Code size (stored in Flash)
- **data** - Initialized data (stored in Flash, copied to RAM at startup)
- **bss** - Uninitialized data (stored in RAM only)

## Advanced Usage

### Custom Compiler Flags
You can add custom flags by modifying the Makefile or using environment variables:
```bash
make BOARD=STM32F746G-DISCO CFLAGS+="-DCUSTOM_DEFINE=1" -j$(nproc)
```

### Verbose Build
To see full compiler commands:
```bash
make BOARD=STM32F746G-DISCO V=1
```

## Integration with CI/CD

Example GitHub Actions workflow:
```yaml
- name: Build all boards
  run: |
    make all-boards
    
- name: Upload artifacts
  uses: actions/upload-artifact@v3
  with:
    name: firmware
    path: build/**/*.bin
```

## See Also

- [BUILD_DEBIAN.md](BUILD_DEBIAN.md) - Detailed Debian/Ubuntu build instructions
- [README.md](README.md) - Project overview and features
- [QUICKSTART.md](QUICKSTART.md) - Quick start guide
