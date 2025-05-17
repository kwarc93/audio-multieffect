/*
 * ui_components.h
 *
 *  Created on: 17 maj 2025
 *      Author: kwarc
 */

#ifndef UI_COMPONENTS_H_
#define UI_COMPONENTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

lv_obj_t * ui_comp_fx_base_create(lv_obj_t * parent, const char * name);
lv_obj_t * ui_comp_fx_btn_create(lv_obj_t * parent, const char * name, lv_align_t align);
lv_obj_t * ui_comp_fx_ctrl_pnl_create(lv_obj_t * parent);

lv_obj_t * ui_comp_fx_knob_create(lv_obj_t * parent, const char * name, uint8_t value, lv_align_t align);
uint8_t    ui_comp_fx_knob_get_value(lv_obj_t * obj);
void       ui_comp_fx_knob_set_value(lv_obj_t * obj, uint8_t value);
void       ui_comp_fx_knob_add_event_cb(lv_obj_t * obj, lv_event_cb_t cb, lv_event_code_t filter, void * user_data);

lv_obj_t * ui_comp_fx_switch_create(lv_obj_t * parent, const char * left_txt, const char * right_txt);
bool 	     ui_comp_fx_switch_get_state(lv_obj_t * widget);
void       ui_comp_fx_switch_set_state(lv_obj_t * widget, bool state);
void       ui_comp_fx_switch_add_event_cb(lv_obj_t * widget, lv_event_cb_t cb, lv_event_code_t filter, void * user_data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* UI_COMPONENTS_H_ */
