# Quick Start Guide - Building on Debian/Ubuntu

## TL;DR - Fast Track

```bash
# 1. Install prerequisites
bash install-prerequisites.sh

# 2. Initialize submodules (if not done)
git submodule update --init --recursive

# 3. Build
make -j$(nproc)

# 4. Flash (with board connected)
make flash
```

## Required Debian/Ubuntu Packages

```bash
sudo apt-get update
sudo apt-get install -y \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    build-essential \
    git \
    make \
    stlink-tools
```

## Package Descriptions

| Package | Purpose |
|---------|---------|
| `gcc-arm-none-eabi` | ARM cross-compiler (C/C++) |
| `binutils-arm-none-eabi` | ARM binary utilities (linker, assembler) |
| `libnewlib-arm-none-eabi` | C standard library for ARM |
| `libstdc++-arm-none-eabi-newlib` | C++ standard library for ARM |
| `build-essential` | Essential build tools (make, etc.) |
| `git` | Version control |
| `make` | Build automation |
| `stlink-tools` | ST-LINK programmer utilities |

## Optional Packages (for debugging)

```bash
sudo apt-get install -y gdb-multiarch openocd
```

## Build Commands

### Standard Build
```bash
make -j$(nproc)
```

### Clean Build
```bash
make clean
make -j$(nproc)
```

### Release Build (optimized)
```bash
make clean
make -j$(nproc) DEBUG=0 OPT=-O3
```

## Output Files

After successful build, find these files in `build/` directory:

- `audio-multieffect.elf` - ELF executable with debug symbols
- `audio-multieffect.hex` - Intel HEX format
- `audio-multieffect.bin` - Raw binary (use this for flashing)
- `audio-multieffect.map` - Memory map

## Flashing

### Method 1: Using Make (Recommended)
```bash
make flash
```

### Method 2: Manual with st-flash
```bash
st-flash write build/audio-multieffect.bin 0x8000000
```

### Method 3: Using OpenOCD
```bash
openocd -f board/stm32f7discovery.cfg \
    -c "program build/audio-multieffect.elf verify reset exit"
```

## Troubleshooting

### "make: *** No targets specified and no makefile found"
- You're in the wrong directory. Navigate to the project root where `Makefile` is located.

### "arm-none-eabi-gcc: command not found"
- ARM toolchain not installed. Run: `sudo apt-get install gcc-arm-none-eabi`

### "No rule to make target..."
- Submodules not initialized. Run: `git submodule update --init --recursive`

### "st-flash: Failed to connect to target"
- Check USB connection
- Try with sudo: `sudo make flash`
- Ensure no other program is using ST-LINK

### Build is slow
- Use parallel build: `make -j$(nproc)`
- This uses all CPU cores

## Memory Check

Check memory usage after build:
```bash
arm-none-eabi-size build/audio-multieffect.elf
```

## More Information

See [`BUILD_DEBIAN.md`](BUILD_DEBIAN.md) for detailed documentation.
