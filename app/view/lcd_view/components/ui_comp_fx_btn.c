/*
 * ui_comp_fx_btn.c
 *
 *  Created on: 16 maj 2025
 *      Author: kwarc
 */

#include "../ui.h"

lv_obj_t * ui_comp_fx_btn(lv_obj_t * parent, const char * name, lv_align_t align)
{
    lv_obj_t * fx_btn = lv_imgbtn_create(parent);
    lv_imgbtn_set_src(fx_btn, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_btn_1_inact_png, NULL);
    lv_imgbtn_set_src(fx_btn, LV_IMGBTN_STATE_PRESSED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(fx_btn, LV_IMGBTN_STATE_DISABLED, NULL, &ui_img_btn_1_inact_png, NULL);
    lv_imgbtn_set_src(fx_btn, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(fx_btn, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &ui_img_btn_1_act_png, NULL);
    lv_imgbtn_set_src(fx_btn, LV_IMGBTN_STATE_CHECKED_DISABLED, NULL, &ui_img_btn_1_inact_png, NULL);
    lv_obj_set_height(fx_btn, 70);
    lv_obj_set_width(fx_btn, LV_SIZE_CONTENT);
    lv_obj_set_x(fx_btn, 15);
    lv_obj_set_y(fx_btn, 0);
    lv_obj_set_align(fx_btn, align);
    lv_obj_add_flag(fx_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_clear_flag(fx_btn, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_t * fx_btn_lbl = lv_label_create(fx_btn);
    lv_obj_set_width(fx_btn_lbl, LV_SIZE_CONTENT);
    lv_obj_set_height(fx_btn_lbl, LV_SIZE_CONTENT);
    lv_obj_set_align(fx_btn_lbl, LV_ALIGN_CENTER);
    lv_label_set_text(fx_btn_lbl, name);
    lv_obj_set_style_text_color(fx_btn_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(fx_btn_lbl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(fx_btn_lbl, &ui_font_14_bold, LV_PART_MAIN | LV_STATE_DEFAULT);

    return fx_btn;
}

