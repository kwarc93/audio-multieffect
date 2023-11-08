# Digital Multi Effect

A concept of digital multi-effect for guitar running on STM32F746-DISCO board

![screenshot](guitar_mfx.png)

## Overview

[PROJECT UNDER DEVELOPMENT]

Device allows to add multiple effects to signal chain and control them via GUI. Currently six simple guitar effects are available: tremolo, echo/delay, chorus, reverb, overdrive and speaker cabinet emulator. Effects order (in signal chain) can be changed through settings menu and each effect can be enabled/disabled separately. Changing between effects is done by left/right 'swipe' gesture. Settings screen can be accessed by 'swipe down' gesture.

Audio quality is set to 16bit 48kHz. Audio latency is determined by size of audio buffer, for now it is equal to 5ms.

## Demo
https://youtu.be/xXm61wA0C68?feature=shared

