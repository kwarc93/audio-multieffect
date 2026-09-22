# =============================================================================
# Board peripheral drivers.
#
# Common drivers (audio codec, LCD, touch, QSPI flash, LEDs, buttons) live
# directly in drivers/, MCU-specific low-level drivers (GPIO/DMA/I2C/... register
# access) live in drivers/stm32f7 or drivers/stm32h7 depending on BOARD
# (DRIVERS_MCU_DIR, set in the top-level CMakeLists.txt).
#
# These are first-party project files (not a vendored third-party library),
# so a simple file(GLOB) for "all .cpp in this folder" is used instead of
# listing every file by hand - re-run cmake configure after adding/removing a
# driver source file so the new file is picked up.
# =============================================================================

file(GLOB _driver_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _driver_mcu_sources CONFIGURE_DEPENDS ${DRIVERS_MCU_DIR}/*.cpp ${DRIVERS_MCU_DIR}/*.c)

target_sources(${PROJECT_NAME} PRIVATE
    ${_driver_sources}
    ${_driver_mcu_sources}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DRIVERS_MCU_DIR}
)
