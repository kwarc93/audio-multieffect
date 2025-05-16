/*
 * ui_comp_fx_knob.c
 *
 *  Created on: 16 maj 2025
 *      Author: kwarc
 */

#include "../ui.h"

lv_obj_t * ui_comp_fx_knob(lv_obj_t * parent, const char * name, int16_t min, int16_t max, int16_t val, lv_align_t align)
{
    lv_obj_t * fx_knob = lv_obj_create(parent);
    lv_obj_set_width(fx_knob, lv_pct(33));
    lv_obj_set_height(fx_knob, lv_pct(100));
    lv_obj_set_align(fx_knob, align);
    lv_obj_clear_flag(fx_knob, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_style_bg_color(fx_knob, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(fx_knob, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(fx_knob, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(fx_knob, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(fx_knob, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(fx_knob, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(fx_knob, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * fx_knob_img = lv_img_create(fx_knob);
    lv_img_set_src(fx_knob_img, &ui_img_btn_knob_png);
    lv_obj_set_width(fx_knob_img, 104);
    lv_obj_set_height(fx_knob_img, 105);
    lv_obj_set_x(fx_knob_img, 0);
    lv_obj_set_y(fx_knob_img, -12);
    lv_obj_set_align(fx_knob_img, LV_ALIGN_CENTER);
    lv_obj_add_flag(fx_knob_img, LV_OBJ_FLAG_ADV_HITTEST);

    lv_obj_t * fx_knob_arc = lv_arc_create(fx_knob);
    lv_obj_set_width(fx_knob_arc, 95);
    lv_obj_set_height(fx_knob_arc, 97);
    lv_obj_set_x(fx_knob_arc, 0);
    lv_obj_set_y(fx_knob_arc, -12);
    lv_obj_set_align(fx_knob_arc, LV_ALIGN_CENTER);
    lv_obj_clear_flag(fx_knob_arc, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_arc_set_range(fx_knob_arc, min, max);
    lv_arc_set_value(fx_knob_arc, val);
    lv_arc_set_bg_angles(fx_knob_arc, 129, 51);
    lv_obj_set_style_pad_left(fx_knob_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(fx_knob_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(fx_knob_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(fx_knob_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(fx_knob_arc, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(fx_knob_arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(fx_knob_arc, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(fx_knob_arc, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(fx_knob_arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(fx_knob_arc, 2, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(fx_knob_arc, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(fx_knob_arc, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(fx_knob_arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    lv_obj_t * fx_knob_lbl = lv_label_create(fx_knob);
    lv_obj_set_width(fx_knob_lbl, 68);
    lv_obj_set_height(fx_knob_lbl, 17);
    lv_obj_set_align(fx_knob_lbl, LV_ALIGN_BOTTOM_MID);
    lv_label_set_text(fx_knob_lbl, name);
    lv_obj_set_style_text_color(fx_knob_lbl, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(fx_knob_lbl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(fx_knob_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(fx_knob_lbl, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    return fx_knob_arc;
}


