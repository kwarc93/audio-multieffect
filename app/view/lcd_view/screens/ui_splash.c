// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: gmfx

#include "../ui.h"

void ui_splash_screen_init(void)
{
    ui_splash = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_splash, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_splash, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_splash, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_lbl_splash = lv_label_create(ui_splash);
    lv_obj_set_width(ui_lbl_splash, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_lbl_splash, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_lbl_splash, LV_ALIGN_CENTER);
    lv_label_set_text(ui_lbl_splash, "GUITAR MFX");
    lv_obj_set_style_text_color(ui_lbl_splash, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_lbl_splash, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_lbl_splash, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_lbl_splash, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_splash, ui_event_splash, LV_EVENT_ALL, NULL);

}