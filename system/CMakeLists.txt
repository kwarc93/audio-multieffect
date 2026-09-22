# =============================================================================
# MCU startup code: interrupt vector table (.S), CMSIS SystemInit/clock setup
# (system.cpp) and C++ interrupt handlers. SYSTEM_DIR is set in the top-level
# CMakeLists.txt based on BOARD.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${SYSTEM_STARTUP}
    ${SYSTEM_DIR}/system.cpp
    ${SYSTEM_DIR}/interrupt_handlers.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SYSTEM_DIR}
)
