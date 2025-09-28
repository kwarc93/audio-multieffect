# Digital Multi Effect

A concept of digital multi-effect for guitar running on STM32F746G-DISCO or STM32H745I-DISCO board

![screenshot](guitar_mfx.png)

## Overview

Device captures audio signal from line-in jack (blue), then processes audio samples according to selected effect and finally outputs processed audio to line-out/headphone jack (green). User controls device by touchscreen. It is possible to add multiple effects to signal chain. Several basic guitar effects are implemented:
- chromatic tuner
- tremolo
- echo/delay
- chorus
- reverb
- overdrive
- tube amplifier simulator
- speaker cabinet simulator
- vocoder
- phaser

Effects order (in signal chain) can be changed through settings menu and each effect can be enabled/disabled separately. Changing between effects is done by left/right *swipe* gesture. Settings screen can be accessed by *swipe down* gesture.
The guitar signal (MAIN) is captured as left channel from input jack. The right channel input (AUX) can be used for other purposes. For example, the vocoder effect needs an additional input signal as a modulator, which can be:
- signal form the onboard digital microphone (default)
- right channel of the line-in, labeled as AUX


This signal can be selected from the audio settings screen.

Audio quality is set to 24bit 48kHz. Audio latency is determined by the size of audio buffer, which by default is 128 samples that gives around 6ms in-out delay.

Device stores its settings in a filesystem so they are persistent after power-off. User can also create, save & load their effect presets.

Device can work as USB audio interface (can be enabled in settings). When connected to host (Linux/Windows) through *USBFS* connector, the device shows up in system as a sound card with stereo output (headphones) and mono input (line-in). The downside of enabling USB is increased DSP load which may limit the maximum number of effects in chain. The overall USB audio latency may be too high to play comfortably. In that case user can enable *direct monitoring* which feeds signal directly to output (as in normal usecase).

## Demo
https://soundcloud.com/kwarc-1/sets/gmfx  
https://youtu.be/vwppcxMA08A?feature=shared

## Supported boards

- STM32F746G-DISCO
- STM32H745I-DISCO

*Note 1: The STM32H745I-DISCO board supports parallel dual-core operation, where DSP operations are performed on Cortex-M7 and GUI on the Cortex-M4 core.*  
*Note 2: On STM32H745I-DISCO board, when the application runs on single core (Cortex-M7), the second core (Cortex-M4) should be put in stop mode.*  
*Note 3: To use digital microphone on STM32H745I-DISCO board, some solder bridges need to be changed (open: SB41 & SB42, close: SB40 & SB32).*

## How to build

Project was created using **Eclipse IDE for Embedded C/C++ Developers**

Follow these steps:
1. Install **Eclipse IDE for Embedded C/C++ Developers** and **gcc-arm-none-eabi** toolchain. Setup Eclipse's global ARM toolchain path in **Window->Preferences->MCU**.
2. Clone repo: `git clone --recurse-submodules https://github.com/kwarc93/audio-multieffect.git`
3. In Eclipse go to: **File->Import->Existing projects into workspace**, select folder with cloned repo and check **Copy projects into workspace**. Click **Finish**.

Now the project should have four build configurations: 
- **STM32F746G-DISCO**
- **STM32H745I-DISCO** *(single core)*
- **STM32H745I-DISCO-CM4** *(dual core)*
- **STM32H745I-DISCO-CM7** *(dual core)*

it should be possible to build each one with no errors.

## How to add new effect

Here is a brief description of how to add new effect to the system:

1. Define new effect ID, name, its attributes and controls in **app/model/effect_features.hpp**. Do not forget to add new effect attributes to the **effect_specific_attributes** variant at the end.
2. Add new effect module (.cpp/.hpp pair) to the **app/model/new_effect_name** location. Write new effect class that inherits from base **effect** class. Look at other effects implementation as a guideline.
3. Go to **app/model/effect_processor.hpp** module and add new effect controls variant to the event **set_effect_controls**. Add another **set_controls** method overload to the **effect_processor** class. In the source file, include header of new effect and add new entry to the **effect_factory** map. Define **set_controls** method.
4. Go to the **app/view/lcd_view/screens** and create new screen for the effect. This is the most complicated step and would take a lot of writting to describe it in detail here :(. You can use Squareline Studio tool to easily create screen without writting code. Another way is to look at existing screens implementation and take it as a guideline. Go to the **app/view/lcd_view/** and write event handling code for new screen in **ui_events.cpp** & **ui.c** source files.
5. Go to **app/view/lcd_view/lcd_view.hpp** module and add new effect controls to the **effect_controls_changed** event. Add another **set_effect_attr** method overload to the **lcd_view** class and define it in the source file. Also, in method **change_effect_screen** add new case for handling new effect screen.

After completing these steps it should be possible to compile the project. However this instruction is not very detailed so there may be compilation errors if something is missing. If so, follow the compiler error messages.

## Known issues and limitations
- changing the LCD brightness does not work (hardware does not support it)
- USB audio interface on MacOS was not tested (may not work)