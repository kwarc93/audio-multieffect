/*
 * ui_comp_fx_switch.c
 *
 *  Created on: 16 maj 2025
 *      Author: kwarc
 */

#include "../ui.h"
#include "ui_comp_fx_switch.h"

typedef struct
{
    lv_obj_t * main_switch;
    lv_obj_t * left_lbl;
    lv_obj_t * right_lbl;
} ui_fx_switch_t;

static void delete_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_DELETE)
    {
    	ui_fx_switch_t * data = lv_event_get_user_data(e);
        lv_mem_free(data);
    }
}

static void value_changed_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
    	ui_fx_switch_t * data = lv_event_get_user_data(e);
        _ui_state_modify(data->left_lbl, LV_STATE_CHECKED, _UI_MODIFY_STATE_TOGGLE);
        _ui_state_modify(data->right_lbl, LV_STATE_CHECKED, _UI_MODIFY_STATE_TOGGLE);
    }
}

lv_obj_t * ui_comp_fx_switch_create(lv_obj_t * parent, const char * left_txt, const char * right_txt)
{
	// Allocate and store user data
	ui_fx_switch_t * data = lv_mem_alloc(sizeof(ui_fx_switch_t));
	if (!data)
		return NULL;

	lv_obj_t * fx_switch = lv_obj_create(parent);
	lv_obj_set_width(fx_switch, lv_pct(40));
	lv_obj_set_height(fx_switch, lv_pct(14));
	lv_obj_set_x(fx_switch, 0);
	lv_obj_set_y(fx_switch, 100);
	lv_obj_set_align(fx_switch, LV_ALIGN_CENTER);
	lv_obj_clear_flag(fx_switch, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);
	lv_obj_set_style_bg_color(fx_switch, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(fx_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(fx_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(fx_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(fx_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(fx_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(fx_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_user_data(fx_switch, data);
    lv_obj_add_event_cb(fx_switch, delete_event_cb, LV_EVENT_DELETE, data);

	data->main_switch = lv_switch_create(fx_switch);
	lv_obj_set_width(data->main_switch, 50);
	lv_obj_set_height(data->main_switch, 25);
	lv_obj_set_align(data->main_switch, LV_ALIGN_CENTER);

	lv_obj_set_style_bg_color(data->main_switch, lv_color_hex(0xB9B9B9), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(data->main_switch, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(data->main_switch, lv_color_hex(0xB9B9B9), LV_PART_INDICATOR | LV_STATE_CHECKED);
	lv_obj_set_style_bg_opa(data->main_switch, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

	lv_obj_set_style_bg_color(data->main_switch, lv_color_hex(0x4C4E5B), LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(data->main_switch, 255, LV_PART_KNOB | LV_STATE_DEFAULT);

	lv_obj_add_event_cb(data->main_switch, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, data);

	data->left_lbl = lv_label_create(fx_switch);
	lv_obj_set_width(data->left_lbl, LV_SIZE_CONTENT);
	lv_obj_set_height(data->left_lbl, LV_SIZE_CONTENT);
	lv_obj_set_align(data->left_lbl, LV_ALIGN_LEFT_MID);
	lv_label_set_text(data->left_lbl, left_txt);
	lv_obj_set_style_text_color(data->left_lbl, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(data->left_lbl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(data->left_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(data->left_lbl, &ui_font_14_bold, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(data->left_lbl, lv_color_hex(0x50FF7D), LV_PART_MAIN | LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(data->left_lbl, 255, LV_PART_MAIN | LV_STATE_CHECKED);
	lv_obj_add_state(data->left_lbl, LV_STATE_CHECKED);

	data->right_lbl = lv_label_create(fx_switch);
	lv_obj_set_width(data->right_lbl, LV_SIZE_CONTENT);
	lv_obj_set_height(data->right_lbl, LV_SIZE_CONTENT);
	lv_obj_set_align(data->right_lbl, LV_ALIGN_RIGHT_MID);
	lv_label_set_text(data->right_lbl, right_txt);
	lv_obj_set_style_text_color(data->right_lbl, lv_color_hex(0x9395A1), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_opa(data->right_lbl, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(data->right_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(data->right_lbl, &ui_font_14_bold, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(data->right_lbl, lv_color_hex(0x50FF7D), LV_PART_MAIN | LV_STATE_CHECKED);
	lv_obj_set_style_text_opa(data->right_lbl, 255, LV_PART_MAIN | LV_STATE_CHECKED);

	return fx_switch;
}

bool ui_comp_fx_switch_get_state(lv_obj_t * obj)
{
	ui_fx_switch_t * data = lv_obj_get_user_data(obj);
    return (data && lv_obj_has_state(data->main_switch, LV_STATE_CHECKED));
}

void ui_comp_fx_switch_set_state(lv_obj_t * obj, bool state)
{
	ui_fx_switch_t * data = lv_obj_get_user_data(obj);
    if (data)
    {
        if (state)
        {
            lv_obj_add_state(data->main_switch, LV_STATE_CHECKED);
            lv_obj_add_state(data->right_lbl, LV_STATE_CHECKED);
            lv_obj_clear_state(data->left_lbl, LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_clear_state(data->main_switch, LV_STATE_CHECKED);
            lv_obj_clear_state(data->right_lbl, LV_STATE_CHECKED);
            lv_obj_add_state(data->left_lbl, LV_STATE_CHECKED);
        }
    }
}

void ui_comp_fx_switch_add_event_cb(lv_obj_t * obj, lv_event_cb_t cb, lv_event_code_t filter, void * user_data)
{
	ui_fx_switch_t * data = lv_obj_get_user_data(obj);
    if (data)
    {
        lv_obj_add_event_cb(data->main_switch, cb, filter, user_data);
    }
}
