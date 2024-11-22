# Digital Multi Effect

A concept of digital multi-effect for guitar running on STM32F746G-DISCO or STM32H745I-DISCO board

![screenshot](guitar_mfx.png)

## Overview

Device captures audio signal from line-in jack (left channel, labeled as MAIN), then processes audio samples according to selected effect and finally outputs processed audio to line-out jack. User controls device by touchscreen. It is possible to add multiple effects to signal chain. Several basic guitar effects are implemented:
- tremolo
- echo/delay
- chorus
- reverb
- overdrive
- speaker cabinet emulator
- vocoder

Effects order (in signal chain) can be changed through settings menu and each effect can be enabled/disabled separately. Changing between effects is done by left/right *swipe* gesture. Settings screen can be accessed by *swipe down* gesture.

The vocoder effect needs an additional signal input as modulator, which can be:
- right channel of the line-in, labeled as AUX (default)
- signal form the onboard digital microphone


This signal can be selected from the audio settings screen.

Audio quality is set to 24bit 48kHz. Audio latency is determined by the size of audio buffer, which by default is 128 samples that gives around 6ms in-out delay.

## Demo
https://soundcloud.com/kwarc-1/sets/gmfx  
https://youtu.be/xXm61wA0C68?feature=shared

## Supported boards

- STM32F746G-DISCO
- STM32H745I-DISCO

*Note 1: On STM32H745I-DISCO board, the application runs on single core (Cortex-M7) and the second core (Cortex-M4) should be put in stop mode.*  
*Note 2: To use digital microphone on STM32H745I-DISCO board, some solder bridges need to be changed (open: SB41 & SB42, close: SB40 & SB32).*

## How to build

Project was created using **Eclipse IDE for Embedded C/C++ Developers**

Follow these steps:
1. Install **Eclipse IDE for Embedded C/C++ Developers** and **gcc-arm-none-eabi** toolchain. Setup Eclipse's global ARM toolchain path in **Window->Preferences->MCU**.
2. Clone repo: `git clone --recurse-submodules https://github.com/kwarc93/audio-multieffect.git`
3. In Eclipse go to: **File->Import->Existing projects into workspace**, select folder with cloned repo and check **Copy projects into workspace**. Click **Finish**.

Now the project should have four build configurations: 
- **STM32F746G-DISCO-Debug**
- **STM32F746G-DISCO-Release**
- **STM32H745I-DISCO-CM7-Debug**
- **STM32H745I-DISCO-CM7-Release**

it should be possible to build each one with no errors.

## How to add new effect

Here is a brief description of how to add new effect to the system:

1. Define new effect ID, name, its attributes and controls in **app/model/effect_features.hpp**. Do not forget to add new effect attributes to the **effect_specific_attributes** variant at the end.
2. Add new effect module (.cpp/.hpp pair) to the **app/model/new_effect_name** location. Write new effect class that inherits from base **effect** class. Look at other effects implementation as a guideline.
3. Go to **app/model/effect_processor.hpp** module and add new effect controls variant to the event **set_effect_controls**. Add another **set_controls** method overload to the **effect_processor** class. In the source file, include header of new effect and add new entry to the **effect_factory** map. Define **set_controls** method.
4. Go to the **app/view/lcd_view/screens** and create new screen for the effect. This is the most complicated step and would take a lot of writting to describe it in detail here :(. You can use Squareline Studio tool to easily create screen without writting code. Another way is to look at existing screens implementation and take it as a guideline. Go to the **app/view/lcd_view/** and write event handling code for new screen in **ui_events.cpp** & **ui.c** source files.
5. Go to **app/view/lcd_view/lcd_view.hpp** module and add new effect controls to the **effect_controls_changed** event. Add another **set_effect_attr** method overload to the **lcd_view** class and define it in the source file. Also, in method **change_effect_screen** add new case for handling new effect screen.

After completing these steps it should be possible to compile the project. However this instruction is not very detailed so there may be compilation errors if something is missing. If so, follow the compiler error messages.

