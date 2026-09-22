# =============================================================================
# LCD UI (LVGL based). lcd_view.cpp/hpp is our own actor wrapper; ui*.c/.h and
# the components/fonts/images/screens subfolders are SquareLine
# Studio-generated LVGL UI code. Being generated/vendored code with many
# files, its own folder + the generated subfolders are globbed rather than
# listed by hand - re-run cmake configure after regenerating the UI project.
# =============================================================================

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lcd_view.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ui_events.cpp
)

file(GLOB _ui_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
file(GLOB _ui_component_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/components/*.c)
file(GLOB _ui_font_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.c)
file(GLOB _ui_image_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images/*.c)
file(GLOB _ui_screen_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/screens/*.c)

target_sources(${PROJECT_NAME} PRIVATE
    ${_ui_sources}
    ${_ui_component_sources}
    ${_ui_font_sources}
    ${_ui_image_sources}
    ${_ui_screen_sources}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
