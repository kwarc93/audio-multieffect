# =============================================================================
# Application layer sources. Which parts are compiled depends on the board's
# role (full single-core app, dual-core UI/storage core, or dual-core DSP
# core) - see the HAS_* variables set in the top-level CMakeLists.txt.
#
# app/ipc/*.hpp is header-only (included directly by main.cpp), so it needs
# no entry here beyond the include path set up in the top-level CMakeLists.txt.
# =============================================================================

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(HAS_CONTROLLER)
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/controller/controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/presets.cpp
    )
endif()

if(HAS_MODEL)
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/model/effect_processor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/amp_sim/amp_sim.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/cabinet_sim/cabinet_sim.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/chorus/chorus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/echo/echo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/overdrive/overdrive.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/phaser/phaser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/reverb/reverb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/tremolo/tremolo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/tuner/tuner.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/vocoder/vocoder.cpp
    )
    if(HAS_MODEL_NAM)
        target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/model/nam/nam.cpp)
    endif()
endif()

if(HAS_VIEW)
    # ui*.c/.h and the components/fonts/images/screens sources are SquareLine
    # Studio-generated LVGL UI code.
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/view/console_view/console_view.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/lcd_view.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/ui_events.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/ui.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/ui_helpers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/components/ui_comp_hook.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/fonts/ui_font_14_bold.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/images/ui_img_pot_ver_line_png.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/images/ui_img_pot_ver_knob_png.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/images/ui_img_logo_gmfx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/images/ui_img_btn_knob_png.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/images/ui_img_btn_1_inact_png.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/images/ui_img_btn_1_act_png.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_splash.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_settings.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_vocoder.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_tuner.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_tremolo.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_reverb.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_phaser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_overdrive.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_nam.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_echo.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_chorus.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_cabinet_sim.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_fx_amp_sim.c
        ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view/screens/ui_blank.c
    )
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/view/lcd_view)
endif()

