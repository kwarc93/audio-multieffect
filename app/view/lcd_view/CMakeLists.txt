SET(SOURCES screens/ui_splash.c
    screens/ui_fx_tremolo.c
    screens/ui_fx_echo.c
    screens/ui_fx_noise_gate.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.cpp
    images/ui_img_btn_1_inact_png.c
    images/ui_img_btn_1_act_png.c
    images/ui_img_btn_knob_png.c
    fonts/ui_font_14_bold.c)

add_library(ui ${SOURCES})
