/*
 * ui_comp_fx_base.c
 *
 *  Created on: 16 maj 2025
 *      Author: kwarc
 */

#include "../ui.h"

lv_obj_t * ui_comp_fx_base_create(lv_obj_t * parent, const char * name)
{
    lv_obj_t * fx_base = lv_obj_create(parent);
    lv_obj_set_width(fx_base, lv_pct(100));
    lv_obj_set_height(fx_base, lv_pct(100));
    lv_obj_clear_flag(fx_base, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(fx_base, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(fx_base, lv_color_hex(0x4C4E5B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(fx_base, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(fx_base, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(fx_base, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(fx_base, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(fx_base, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(fx_base, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t * fx_base_name_lbl = lv_label_create(fx_base);
    lv_obj_set_width(fx_base_name_lbl, LV_SIZE_CONTENT);
    lv_obj_set_height(fx_base_name_lbl, LV_SIZE_CONTENT);
    lv_obj_set_x(fx_base_name_lbl, 0);
    lv_obj_set_y(fx_base_name_lbl, 10);
    lv_obj_set_align(fx_base_name_lbl, LV_ALIGN_TOP_MID);
    lv_label_set_text(fx_base_name_lbl, name);
    lv_obj_set_style_text_color(fx_base_name_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(fx_base_name_lbl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(fx_base_name_lbl, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    return fx_base;
}
