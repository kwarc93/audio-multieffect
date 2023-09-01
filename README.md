# Digital Multi Effect

A concept of digital multi-effect for guitar running on STM32F746-DISCO board

![screenshot](guitar_mfx.png)

## Overview

[PROJECT UNDER DEVELOPMENT]

Device allows to add multiple effects to signal chain and control them via GUI. Currently, four simple effects are available: tremolo, echo/delay, overdrive and speaker cabinet emulator. Effects are in fixed order (to be changed in future) and can be enabled/disabled separately. Changing between effects is done by left/right 'swipe' gesture. Settings screen contain input & output volume control. Settings screen can be accessed by 'swipe down' gesture.

Audio quality is set to 16bit 48kHz. Audio latency is determined by size of audio buffer, for now it is equal to 5ms.