# =============================================================================
# MCU startup code: reset/interrupt vector table (.S), CMSIS SystemInit and
# clock configuration (system.cpp), and C++ interrupt handlers.
#
# SYSTEM_DIR was set in the top-level CMakeLists.txt to either
# system/stm32f746 or system/stm32h745, depending on BOARD.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${SYSTEM_DIR}/startup-cortex-m7.S
    ${SYSTEM_DIR}/system.cpp
    ${SYSTEM_DIR}/interrupt_handlers.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SYSTEM_DIR}
)
