# =============================================================================
# Subdirectory: Application Layer Configuration
#
# Coordinates compilation targets for the application layer. Source selection 
# is conditionally orchestrated through feature flags (HAS_*) mapped from the 
# high-level hardware-software capability matrix in the root configuration.
#
# Architectural Layout:
#   - Controller: System actor managing UI state, device events, and presets.
#   - Model: Core DSP execution pipeline
#   - View: Concrete user-interface abstractions (Console, LCD, etc.).
# =============================================================================


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

if(HAS_CONTROLLER)
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/controller/controller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/modules/presets.cpp
    )
endif()

if(HAS_MODEL)
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/model/effect_processor.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/amp_sim/amp_sim.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/cabinet_sim/cabinet_sim.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/chorus/chorus.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/echo/echo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/overdrive/overdrive.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/phaser/phaser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/reverb/reverb.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/tremolo/tremolo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/tuner/tuner.cpp
        ${CMAKE_CURRENT_LIST_DIR}/model/vocoder/vocoder.cpp
    )
    if(HAS_EFFECT_NAM)
        target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/model/nam/nam.cpp)
    endif()
endif()

if(HAS_VIEW)
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/view/console_view/console_view.cpp
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/lcd_view.cpp
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/ui_events.cpp
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/ui.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/ui_helpers.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/components/ui_comp_hook.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/fonts/ui_font_14_bold.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/images/ui_img_pot_ver_line_png.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/images/ui_img_pot_ver_knob_png.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/images/ui_img_logo_gmfx.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/images/ui_img_btn_knob_png.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/images/ui_img_btn_1_inact_png.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/images/ui_img_btn_1_act_png.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_splash.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_settings.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_vocoder.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_tuner.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_tremolo.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_reverb.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_phaser.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_overdrive.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_nam.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_echo.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_chorus.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_cabinet_sim.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_fx_amp_sim.c
        ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view/screens/ui_blank.c
    )
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/view/lcd_view)
endif()

