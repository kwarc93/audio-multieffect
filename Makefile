######################################
# Target
######################################
TARGET = audio_multieffect

######################################
# Include makefile.defs for version info
######################################
-include makefile.defs

######################################
# Board Selection
######################################
# Available boards:
#   STM32F746G-DISCO (default)
#   STM32H745I-DISCO
#   STM32H745I-DISCO-CM4
#   STM32H745I-DISCO-CM7
# Usage: make BOARD=STM32H745I-DISCO
BOARD ?= STM32F746G-DISCO

######################################
# Building variables
######################################
# Debug build?
DEBUG = 1
# Optimization
OPT = -Og

#######################################
# Paths
#######################################
# Build path - use underscore instead of hyphen to avoid Make parsing issues
BOARD_SAFE = $(subst -,_,$(BOARD))
BUILD_DIR = build/$(BOARD_SAFE)

######################################
# Board-specific configuration
######################################
ifeq ($(BOARD),STM32F746G-DISCO)
    MCU_FAMILY = stm32f7
    MCU_MODEL = stm32f746
    CPU = -mcpu=cortex-m7
    FPU = -mfpu=fpv5-sp-d16
    FLOAT-ABI = -mfloat-abi=hard
    C_DEFS = -DSTM32F7 -DSTM32F746xx -DARM_MATH_CM7 -D__FPU_PRESENT=1 -DCORE_CM7 -DCFG_FS_CALIB=0 -DCFG_TUSB_MCU=OPT_MCU_STM32F7 -DARM_MATH_LOOPUNROLL -DGIT_REVISION='"$(GIT_REVISION)"'
    LDSCRIPT = system/stm32f746/arm-cortex-m.ld
    LIBS = -lc -lm -lnosys -lstdc++
    SYSTEM_SRC = system/stm32f746/system.cpp system/stm32f746/interrupt_handlers.cpp
    ASM_SOURCES = system/stm32f746/startup-cortex-m7.S
else ifeq ($(BOARD),STM32H745I-DISCO)
    MCU_FAMILY = stm32h7
    MCU_MODEL = stm32h745
    CPU = -mcpu=cortex-m7
    FPU = -mfpu=fpv5-d16
    FLOAT-ABI = -mfloat-abi=hard
    C_DEFS = -DSTM32H7 -DSTM32H745xx -DARM_MATH_CM7 -D__FPU_PRESENT=1 -DCORE_CM7 -DCFG_FS_CALIB=0 -DCFG_TUSB_MCU=OPT_MCU_STM32H7 -DGIT_REVISION='"$(GIT_REVISION)"'
    LDSCRIPT = system/stm32h745/arm-cortex-m7.ld
    LIBS = -lc -lm -lnosys -lstdc++ -Lcmsis/dsp -larm_cortexM7lfdp_math
    SYSTEM_SRC = system/stm32h745/system.cpp system/stm32h745/interrupt_handlers.cpp
    ASM_SOURCES = system/stm32h745/startup-cortex-m7.S
else ifeq ($(BOARD),STM32H745I-DISCO-CM7)
    MCU_FAMILY = stm32h7
    MCU_MODEL = stm32h745
    CPU = -mcpu=cortex-m7
    FPU = -mfpu=fpv5-d16
    FLOAT-ABI = -mfloat-abi=hard
    C_DEFS = -DSTM32H7 -DSTM32H745xx -DARM_MATH_CM7 -D__FPU_PRESENT=1 -DCORE_CM7 -DDUAL_CORE -DCFG_FS_CALIB=0 -DCFG_TUSB_MCU=OPT_MCU_STM32H7 -DGIT_REVISION='"$(GIT_REVISION)"'
    LDSCRIPT = system/stm32h745/arm-cortex-m7-dual.ld
    LIBS = -lc -lm -lnosys -lstdc++ -Lcmsis/dsp -larm_cortexM7lfdp_math
    SYSTEM_SRC = system/stm32h745/system_cm7.cpp system/stm32h745/interrupt_handlers_cm7.cpp
    ASM_SOURCES = system/stm32h745/startup-cortex-m7.S
else ifeq ($(BOARD),STM32H745I-DISCO-CM4)
    MCU_FAMILY = stm32h7
    MCU_MODEL = stm32h745
    CPU = -mcpu=cortex-m4
    FPU = -mfpu=fpv4-sp-d16
    FLOAT-ABI = -mfloat-abi=hard
    C_DEFS = -DSTM32H7 -DSTM32H745xx -DARM_MATH_CM4 -D__FPU_PRESENT=1 -DCORE_CM4 -DDUAL_CORE -DCFG_FS_CALIB=0 -DCFG_TUSB_MCU=OPT_MCU_STM32H7 -DGIT_REVISION='"$(GIT_REVISION)"'
    LDSCRIPT = system/stm32h745/arm-cortex-m4-dual.ld
    LIBS = -lc -lm -lnosys -lstdc++ -Lcmsis/dsp -larm_cortexM4lf_math
    SYSTEM_SRC = system/stm32h745/system_cm4.cpp system/stm32h745/interrupt_handlers_cm4.cpp
    ASM_SOURCES = system/stm32h745/startup-cortex-m4.S
else
    $(error Invalid BOARD specified: $(BOARD). Valid options: STM32F746G-DISCO, STM32H745I-DISCO, STM32H745I-DISCO-CM7, STM32H745I-DISCO-CM4)
endif

######################################
# Source
######################################
# Common C sources
C_SOURCES = \
$(SYSTEM_SRC) \
drivers/$(MCU_FAMILY)/core.cpp \
drivers/$(MCU_FAMILY)/delay.cpp \
drivers/$(MCU_FAMILY)/dma2d.cpp \
drivers/$(MCU_FAMILY)/exti.cpp \
drivers/$(MCU_FAMILY)/flash.cpp \
drivers/$(MCU_FAMILY)/fmc.cpp \
drivers/$(MCU_FAMILY)/gpio.cpp \
drivers/$(MCU_FAMILY)/i2c.cpp \
drivers/$(MCU_FAMILY)/i2c_sw.cpp \
drivers/$(MCU_FAMILY)/i2c_timing_utility.c \
drivers/$(MCU_FAMILY)/ltdc.cpp \
drivers/$(MCU_FAMILY)/one_wire.cpp \
drivers/$(MCU_FAMILY)/qspi.cpp \
drivers/$(MCU_FAMILY)/rcc.cpp \
drivers/$(MCU_FAMILY)/rng.cpp \
drivers/$(MCU_FAMILY)/sai.cpp \
drivers/$(MCU_FAMILY)/timer.cpp \
drivers/$(MCU_FAMILY)/usart.cpp \
drivers/$(MCU_FAMILY)/usb.cpp \
drivers/audio_wm8994ecs.cpp \
drivers/button_gpio.cpp \
drivers/lcd_rk043fn48h.cpp \
drivers/led_gpio.cpp \
drivers/led_pwm.cpp \
drivers/qspi_mt25ql512a.cpp \
drivers/qspi_n25q128a.cpp \
drivers/touch_ft5336.cpp \
middlewares/usb_descriptors.c \
app/controller/controller.cpp \
app/model/effect_processor.cpp \
app/model/amp_sim/amp_sim.cpp \
app/model/arpeggiator/arpeggiator.cpp \
app/model/cabinet_sim/cabinet_sim.cpp \
app/model/chorus/chorus.cpp \
app/model/compressor/compressor.cpp \
app/model/echo/echo.cpp \
app/model/overdrive/overdrive.cpp \
app/model/phaser/phaser.cpp \
app/model/reverb/reverb.cpp \
app/model/tremolo/tremolo.cpp \
app/model/tuner/tuner.cpp \
app/model/vocoder/vocoder.cpp \
app/modules/presets.cpp \
app/view/console_view/console_view.cpp \
app/view/lcd_view/lcd_view.cpp \
app/view/lcd_view/ui_events.cpp \
app/view/lcd_view/ui_fx_arpeggiator.c \
app/view/lcd_view/ui_fx_compressor.c \
app/view/lcd_view/ui_helpers.c \
app/view/lcd_view/ui.c \
app/view/lcd_view/components/ui_comp_hook.c \
app/view/lcd_view/fonts/ui_font_14_bold.c \
app/view/lcd_view/images/ui_img_btn_1_act_png.c \
app/view/lcd_view/images/ui_img_btn_1_inact_png.c \
app/view/lcd_view/images/ui_img_btn_knob_png.c \
app/view/lcd_view/images/ui_img_logo_gmfx.c \
app/view/lcd_view/images/ui_img_pot_ver_knob_png.c \
app/view/lcd_view/images/ui_img_pot_ver_line_png.c \
app/view/lcd_view/screens/ui_blank.c \
app/view/lcd_view/screens/ui_fx_amp_sim.c \
app/view/lcd_view/screens/ui_fx_cabinet_sim.c \
app/view/lcd_view/screens/ui_fx_chorus.c \
app/view/lcd_view/screens/ui_fx_echo.c \
app/view/lcd_view/screens/ui_fx_overdrive.c \
app/view/lcd_view/screens/ui_fx_phaser.c \
app/view/lcd_view/screens/ui_fx_reverb.c \
app/view/lcd_view/screens/ui_fx_tremolo.c \
app/view/lcd_view/screens/ui_fx_tuner.c \
app/view/lcd_view/screens/ui_fx_vocoder.c \
app/view/lcd_view/screens/ui_settings.c \
app/view/lcd_view/screens/ui_splash.c \
app/view/lcd_view/lv_font_montserrat_28_compressed.c \
main.cpp

# Add H7-specific sources for dual-core builds
ifeq ($(MCU_FAMILY),stm32h7)
    C_SOURCES += drivers/stm32h7/hsem.cpp
endif

# FreeRTOS sources
FREERTOS_DIR = rtos/FreeRTOS
FREERTOS_SOURCES = \
$(FREERTOS_DIR)/Source/croutine.c \
$(FREERTOS_DIR)/Source/event_groups.c \
$(FREERTOS_DIR)/Source/list.c \
$(FREERTOS_DIR)/Source/queue.c \
$(FREERTOS_DIR)/Source/stream_buffer.c \
$(FREERTOS_DIR)/Source/tasks.c \
$(FREERTOS_DIR)/Source/timers.c \
$(FREERTOS_DIR)/Source/portable/MemMang/heap_4.c \
cmsis/rtos2/os_systick.c \
cmsis/rtos2/FreeRTOS/cmsis_os2.c

# FreeRTOS port selection based on CPU
ifeq ($(CPU),-mcpu=cortex-m7)
    FREERTOS_SOURCES += $(FREERTOS_DIR)/Source/portable/GCC/ARM_CM7/r0p1/port.c
else ifeq ($(CPU),-mcpu=cortex-m4)
    FREERTOS_SOURCES += $(FREERTOS_DIR)/Source/portable/GCC/ARM_CM4F/port.c
endif

# LVGL sources
LVGL_DIR = libs/lvgl
LVGL_SOURCES = $(shell find $(LVGL_DIR)/src -name '*.c')

# LittleFS sources
LFS_DIR = libs/littlefs
LFS_SOURCES = \
$(LFS_DIR)/lfs.c \
$(LFS_DIR)/lfs_util.c

# WillPirkle FX Objects
WILLPIRKLE_SOURCES = \
libs/willpirkle/fxobjects.cpp

# TinyUSB sources
TUSB_DIR = libs/tinyusb
TUSB_SOURCES = \
$(TUSB_DIR)/src/tusb.c \
$(TUSB_DIR)/src/common/tusb_fifo.c \
$(TUSB_DIR)/src/device/usbd.c \
$(TUSB_DIR)/src/device/usbd_control.c \
$(TUSB_DIR)/src/class/audio/audio_device.c \
$(TUSB_DIR)/src/portable/synopsys/dwc2/dcd_dwc2.c

# CMSIS-DSP sources (only the modules we need)
CMSIS_DSP_DIR = libs/CMSIS-DSP
CMSIS_DSP_SOURCES = \
$(CMSIS_DSP_DIR)/Source/TransformFunctions/arm_bitreversal2.c \
$(CMSIS_DSP_DIR)/Source/TransformFunctions/arm_cfft_f32.c \
$(CMSIS_DSP_DIR)/Source/TransformFunctions/arm_cfft_init_f32.c \
$(CMSIS_DSP_DIR)/Source/TransformFunctions/arm_cfft_radix8_f32.c \
$(CMSIS_DSP_DIR)/Source/TransformFunctions/arm_rfft_fast_f32.c \
$(CMSIS_DSP_DIR)/Source/TransformFunctions/arm_rfft_fast_init_f32.c \
$(CMSIS_DSP_DIR)/Source/CommonTables/arm_common_tables.c \
$(CMSIS_DSP_DIR)/Source/CommonTables/arm_const_structs.c \
$(CMSIS_DSP_DIR)/Source/BasicMathFunctions/arm_mult_f32.c \
$(CMSIS_DSP_DIR)/Source/BasicMathFunctions/arm_add_f32.c \
$(CMSIS_DSP_DIR)/Source/BasicMathFunctions/arm_sub_f32.c \
$(CMSIS_DSP_DIR)/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c \
$(CMSIS_DSP_DIR)/Source/ComplexMathFunctions/arm_cmplx_mag_squared_f32.c \
$(CMSIS_DSP_DIR)/Source/ComplexMathFunctions/arm_cmplx_mult_cmplx_f32.c \
$(CMSIS_DSP_DIR)/Source/ComplexMathFunctions/arm_cmplx_mult_real_f32.c \
$(CMSIS_DSP_DIR)/Source/FilteringFunctions/arm_biquad_cascade_df2T_f32.c \
$(CMSIS_DSP_DIR)/Source/FilteringFunctions/arm_biquad_cascade_df2T_init_f32.c \
$(CMSIS_DSP_DIR)/Source/FilteringFunctions/arm_fir_decimate_f32.c \
$(CMSIS_DSP_DIR)/Source/FilteringFunctions/arm_fir_decimate_init_f32.c \
$(CMSIS_DSP_DIR)/Source/FilteringFunctions/arm_fir_interpolate_f32.c \
$(CMSIS_DSP_DIR)/Source/FilteringFunctions/arm_fir_interpolate_init_f32.c \
$(CMSIS_DSP_DIR)/Source/FastMathFunctions/arm_sin_f32.c \
$(CMSIS_DSP_DIR)/Source/FastMathFunctions/arm_cos_f32.c \
$(CMSIS_DSP_DIR)/Source/StatisticsFunctions/arm_max_f32.c \
$(CMSIS_DSP_DIR)/Source/StatisticsFunctions/arm_mean_f32.c \
$(CMSIS_DSP_DIR)/Source/SupportFunctions/arm_copy_f32.c \
$(CMSIS_DSP_DIR)/Source/SupportFunctions/arm_fill_f32.c

# Combine all C sources
C_SOURCES += $(FREERTOS_SOURCES) $(LVGL_SOURCES) $(LFS_SOURCES) $(TUSB_SOURCES) $(WILLPIRKLE_SOURCES) $(CMSIS_DSP_SOURCES)

#######################################
# Binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
CXX = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# MCU
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# AS includes
AS_INCLUDES = $(C_INCLUDES)

# C includes
C_INCLUDES = \
-I. \
-Iapp \
-Iapp/controller \
-Iapp/model \
-Iapp/model/vocoder \
-Iapp/modules \
-Iapp/view \
-Iapp/view/console_view \
-Iapp/view/lcd_view \
-Icmsis \
-Ilibs/CMSIS-DSP/Include \
-Ilibs/CMSIS-DSP/PrivateInclude \
-Icmsis/rtos2 \
-Idrivers \
-Idrivers/$(MCU_FAMILY) \
-Ihal \
-Ihal/$(MCU_FAMILY) \
-Ilibs \
-Ilibs/lvgl \
-Ilibs/littlefs \
-Ilibs/nlohmann_json/include \
-Ilibs/tinyusb/src \
-Ilibs/willpirkle \
-Imiddlewares \
-Irtos \
-I$(FREERTOS_DIR)/Source/include \
-Isystem/$(MCU_MODEL)

# Add FreeRTOS port include based on CPU
ifeq ($(CPU),-mcpu=cortex-m7)
    C_INCLUDES += -I$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM7/r0p1
else ifeq ($(CPU),-mcpu=cortex-m4)
    C_INCLUDES += -I$(FREERTOS_DIR)/Source/portable/GCC/ARM_CM4F
endif

# Compile gcc flags
ASFLAGS = $(MCU) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:.o=.d)"

# C++ flags
CXXFLAGS = $(CFLAGS)
CXXFLAGS += -std=c++17
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-exceptions
CXXFLAGS += -fno-threadsafe-statics
CXXFLAGS += -fno-use-cxa-atexit

#######################################
# LDFLAGS
#######################################
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,--cref -Wl,--gc-sections -Wl,--no-warn-rwx-segments

# Target files (use immediate expansion to avoid parsing issues)
ELF_TARGET := $(BUILD_DIR)/$(TARGET).elf
HEX_TARGET := $(BUILD_DIR)/$(TARGET).hex
BIN_TARGET := $(BUILD_DIR)/$(TARGET).bin
MAP_FILE := $(BUILD_DIR)/$(TARGET).map

# Default action: build all
all: build-elf build-hex build-bin
	@echo "Built for board: $(BOARD)"

#######################################
# Build the application
#######################################
# List of objects
# Separate C and C++ sources
C_ONLY_SOURCES = $(filter %.c,$(C_SOURCES))
CPP_ONLY_SOURCES = $(filter %.cpp,$(C_SOURCES))

# Create object file lists
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_ONLY_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_ONLY_SOURCES:.cpp=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))

# Set up vpath for source file lookup
vpath %.c $(sort $(dir $(C_ONLY_SOURCES)))
vpath %.cpp $(sort $(dir $(CPP_ONLY_SOURCES)))
vpath %.S $(sort $(dir $(ASM_SOURCES)))

# Compilation rules - using simple suffix rules instead of pattern rules
.SUFFIXES: .c .cpp .S .o

.c.o:
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $(BUILD_DIR)/$(notdir $@)

.cpp.o:
	@mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) $< -o $(BUILD_DIR)/$(notdir $@)

.S.o:
	@mkdir -p $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $(BUILD_DIR)/$(notdir $@)

.PHONY: build-elf build-hex build-bin compile-all

compile-all: | $(BUILD_DIR)
	@echo "Compiling source files..."
	@for src in $(C_SOURCES); do \
		obj=$(BUILD_DIR)/$$(basename $$src .c | sed 's/\.cpp$$//').o; \
		echo "Compiling $$src..."; \
		case $$src in \
			*.c) $(CC) -c $(CFLAGS) $$src -o $$obj || exit 1 ;; \
			*.cpp) $(CXX) -c $(CXXFLAGS) $$src -o $$obj || exit 1 ;; \
		esac; \
	done
	@for src in $(ASM_SOURCES); do \
		obj=$(BUILD_DIR)/$$(basename $$src .S | sed 's/\.cpp$$//').o; \
		echo "Assembling $$src..."; \
		$(AS) -c $(ASFLAGS) $$src -o $$obj || exit 1; \
	done

build-elf: compile-all
	@echo "Linking..."
	$(CXX) $(OBJECTS) $(LDFLAGS) -Wl,-Map=$(MAP_FILE) -o $(ELF_TARGET)
	$(SZ) $(ELF_TARGET)

build-hex: build-elf
	@echo "Creating HEX file..."
	$(HEX) $(ELF_TARGET) $(HEX_TARGET)

build-bin: build-elf
	@echo "Creating BIN file..."
	$(BIN) $(ELF_TARGET) $(BIN_TARGET)
	
$(BUILD_DIR):
	mkdir -p $@

#######################################
# Build all boards
#######################################
.PHONY: all-boards
all-boards:
	@echo "Building for all boards..."
	$(MAKE) clean BOARD=STM32F746G-DISCO
	$(MAKE) all BOARD=STM32F746G-DISCO
	$(MAKE) clean BOARD=STM32H745I-DISCO
	$(MAKE) all BOARD=STM32H745I-DISCO
	$(MAKE) clean BOARD=STM32H745I-DISCO-CM7
	$(MAKE) all BOARD=STM32H745I-DISCO-CM7
	$(MAKE) clean BOARD=STM32H745I-DISCO-CM4
	$(MAKE) all BOARD=STM32H745I-DISCO-CM4
	@echo "All boards built successfully!"

#######################################
# Clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

clean-all:
	-rm -fR build

#######################################
# Flash
#######################################
flash: all
	st-flash write $(BIN_TARGET) 0x8000000

#######################################
# Help
#######################################
help:
	@echo "Available targets:"
	@echo "  make                    - Build for default board (STM32F746G-DISCO)"
	@echo "  make BOARD=<board>      - Build for specific board"
	@echo "  make all-boards         - Build for all supported boards"
	@echo "  make clean              - Clean current board build"
	@echo "  make clean-all          - Clean all board builds"
	@echo "  make flash              - Flash firmware to board"
	@echo ""
	@echo "Supported boards:"
	@echo "  STM32F746G-DISCO        - STM32F746 Discovery (default)"
	@echo "  STM32H745I-DISCO        - STM32H745 Discovery (single core)"
	@echo "  STM32H745I-DISCO-CM7    - STM32H745 Discovery (dual core, M7)"
	@echo "  STM32H745I-DISCO-CM4    - STM32H745 Discovery (dual core, M4)"
	@echo ""
	@echo "Examples:"
	@echo "  make BOARD=STM32H745I-DISCO -j4"
	@echo "  make BOARD=STM32H745I-DISCO-CM7 DEBUG=0 OPT=-O3"

#######################################
# Dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
