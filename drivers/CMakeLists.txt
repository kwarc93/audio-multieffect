# =============================================================================
# Board peripheral drivers. Common drivers live directly in drivers/;
# MCU-specific register-level drivers live in drivers/stm32f7 or
# drivers/stm32h7, selected via DRIVERS_MCU_DIR (set in the top-level
# CMakeLists.txt based on BOARD).
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/audio_wm8994ecs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/button_gpio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lcd_rk043fn48h.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/led_gpio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/led_pwm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qspi_mt25ql512a.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qspi_n25q128a.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/touch_ft5336.cpp

    ${DRIVERS_MCU_DIR}/core.cpp
    ${DRIVERS_MCU_DIR}/delay.cpp
    ${DRIVERS_MCU_DIR}/dma2d.cpp
    ${DRIVERS_MCU_DIR}/exti.cpp
    ${DRIVERS_MCU_DIR}/flash.cpp
    ${DRIVERS_MCU_DIR}/fmc.cpp
    ${DRIVERS_MCU_DIR}/gpio.cpp
    ${DRIVERS_MCU_DIR}/i2c.cpp
    ${DRIVERS_MCU_DIR}/i2c_sw.cpp
    ${DRIVERS_MCU_DIR}/i2c_timing_utility.c
    ${DRIVERS_MCU_DIR}/ltdc.cpp
    ${DRIVERS_MCU_DIR}/one_wire.cpp
    ${DRIVERS_MCU_DIR}/qspi.cpp
    ${DRIVERS_MCU_DIR}/rcc.cpp
    ${DRIVERS_MCU_DIR}/rng.cpp
    ${DRIVERS_MCU_DIR}/sai.cpp
    ${DRIVERS_MCU_DIR}/timer.cpp
    ${DRIVERS_MCU_DIR}/usart.cpp
    ${DRIVERS_MCU_DIR}/usb.cpp
)

if(MCU_FAMILY STREQUAL "STM32H7")
    # Hardware semaphore driver, used for CM4/CM7 inter-core synchronization.
    target_sources(${PROJECT_NAME} PRIVATE ${DRIVERS_MCU_DIR}/hsem.cpp)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DRIVERS_MCU_DIR}
)

