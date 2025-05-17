/*
 * ui_comp_fx_knob.c
 *
 *  Created on: 16 maj 2025
 *      Author: kwarc
 */

#include "../ui.h"

LV_IMG_DECLARE(ui_img_btn_knob_png);

typedef struct
{
    lv_obj_t * image;
    lv_obj_t * arc;
    lv_obj_t * label;
} ui_fx_knob_t;

static void delete_event_cb(lv_event_t * e)
{
    ui_fx_knob_t * data = lv_event_get_user_data(e);
    lv_mem_free(data);
}

lv_obj_t * ui_comp_fx_knob_create(lv_obj_t * parent, const char * name, uint8_t value, lv_align_t align)
{
    ui_fx_knob_t * data = lv_mem_alloc(sizeof(ui_fx_knob_t));
    if (!data) return NULL;

    lv_obj_t * fx_knob = lv_obj_create(parent);
	   lv_obj_set_user_data(fx_knob, data);
    lv_obj_add_event_cb(fx_knob, delete_event_cb, LV_EVENT_DELETE, data);

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

    data->image = lv_img_create(fx_knob);
    lv_img_set_src(data->image, &ui_img_btn_knob_png);
    lv_obj_set_width(data->image, 104);
    lv_obj_set_height(data->image, 105);
    lv_obj_set_x(data->image, 0);
    lv_obj_set_y(data->image, -12);
    lv_obj_set_align(data->image, LV_ALIGN_CENTER);
    lv_obj_add_flag(data->image, LV_OBJ_FLAG_ADV_HITTEST);

    data->arc = lv_arc_create(fx_knob);
    lv_obj_set_width(data->arc, 95);
    lv_obj_set_height(data->arc, 97);
    lv_obj_set_x(data->arc, 0);
    lv_obj_set_y(data->arc, -12);
    lv_obj_set_align(data->arc, LV_ALIGN_CENTER);
    lv_obj_clear_flag(data->arc, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_arc_set_range(data->arc, 0, 100);
    lv_arc_set_value(data->arc, value);
    lv_arc_set_bg_angles(data->arc, 129, 51);
    lv_obj_set_style_pad_left(data->arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(data->arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(data->arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(data->arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(data->arc, lv_color_hex(0x4040FF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(data->arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(data->arc, 20, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(data->arc, lv_color_hex(0x50FF7D), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(data->arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(data->arc, 2, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(data->arc, true, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(data->arc, lv_color_hex(0x50FF7D), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(data->arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    data->label = lv_label_create(fx_knob);
    lv_obj_set_width(data->label, 68);
    lv_obj_set_height(data->label, 17);
    lv_obj_set_align(data->label, LV_ALIGN_BOTTOM_MID);
    lv_label_set_text(data->label, name);
    lv_obj_set_style_text_color(data->label, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(data->label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(data->label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(data->label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    return fx_knob;
}

uint8_t ui_comp_fx_knob_get_value(lv_obj_t * obj)
{
    ui_fx_knob_t * data = lv_obj_get_user_data(obj);
    return data ? lv_arc_get_value(data->arc) : 0;
}

void ui_comp_fx_knob_set_value(lv_obj_t * obj, uint8_t value)
{
    ui_fx_knob_t * data = lv_obj_get_user_data(obj);
    if (data) lv_arc_set_value(data->arc, value);
}

void ui_comp_fx_knob_add_event_cb(lv_obj_t * obj, lv_event_cb_t cb, lv_event_code_t filter, void * user_data)
{
    ui_fx_knob_t * data = lv_obj_get_user_data(obj);
    if (data) lv_obj_add_event_cb(data->arc, cb, filter, user_data);
}
