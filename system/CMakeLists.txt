# =============================================================================
# Note: SYSTEM_DIR is set in the top-level CMakeLists.txt based on BOARD.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${SYSTEM_DIR}/startup.S
    ${SYSTEM_DIR}/system.cpp
    ${SYSTEM_DIR}/interrupt_handlers.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SYSTEM_DIR}
)
