/*
 * ui_comp_fx_ctrl_pnl.c
 *
 *  Created on: 16 maj 2025
 *      Author: kwarc
 */

#include "../ui.h"

lv_obj_t * ui_comp_fx_ctrl_pnl(lv_obj_t * parent)
{
    lv_obj_t * fx_ctrl_pnl = lv_obj_create(parent);
    lv_obj_set_width(fx_ctrl_pnl, lv_pct(90));
    lv_obj_set_height(fx_ctrl_pnl, lv_pct(60));
    lv_obj_set_x(fx_ctrl_pnl, 0);
    lv_obj_set_y(fx_ctrl_pnl, 35);
    lv_obj_set_align(fx_ctrl_pnl, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(fx_ctrl_pnl, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(fx_ctrl_pnl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(fx_ctrl_pnl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(fx_ctrl_pnl, lv_color_hex(0x747682), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(fx_ctrl_pnl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    return fx_ctrl_pnl;
}
