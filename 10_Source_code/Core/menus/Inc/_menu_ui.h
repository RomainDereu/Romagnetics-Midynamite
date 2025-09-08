/*
 * _menu_ui.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MENU_INC_MENU_UI_H_
#define MENU_INC_MENU_UI_H_

#include "memory_main.h"

//Menu list
typedef enum {
    MIDI_TEMPO = 0,
    MIDI_MODIFY,
    MIDI_TRANSPOSE,
    SETTINGS,
    AMOUNT_OF_MENUS
} menu_list_t;

typedef enum {
	TEMPO_PRINT = 0,
	MIDI_OUT_PRINT,
	AMOUNT_OF_TEMPO_ITEMS
} midi_tempo_ui_states_t;


#define UI_STATE_BUSY 0xFF

typedef enum {
    UI_MIDI_TEMPO_SELECT,
    UI_MIDI_MODIFY_SELECT,
    UI_MIDI_TRANSPOSE_SELECT,
    UI_SETTINGS_SELECT,
    UI_CURRENT_MENU,
	UI_OLD_MENU,
    UI_STATE_FIELD_COUNT
} ui_state_field_t;

// ---------------------
// UI submenu id
// ---------------------
typedef enum {
    UI_GROUP_TEMPO = 0,
    UI_GROUP_MODIFY,

    UI_GROUP_TRANSPOSE_SHIFT,
    UI_GROUP_TRANSPOSE_SCALED,
	UI_GROUP_TRANSPOSE_BOTH,

    UI_GROUP_MODIFY_CHANGE,
    UI_GROUP_MODIFY_SPLIT,
    UI_GROUP_MODIFY_BOTH,

	UI_GROUP_MODIFY_VEL_CHANGED,
	UI_GROUP_MODIFY_VEL_FIXED,

    UI_GROUP_SETTINGS,
    UI_GROUP_NONE = 0xFF
} ui_group_t;




typedef enum {
    UI_MODIFY_INCREMENT = 0,
	UI_MODIFY_SET,
} ui_modify_op_t;




#define CHANGE_BITS_WORDS (((SAVE_FIELD_COUNT) + 31) / 32)
extern uint32_t s_field_change_bits[CHANGE_BITS_WORDS];

// ---------------------
// API
// ---------------------



void toggle_underline_items(ui_group_t group, uint8_t index);

uint8_t build_select_states(ui_group_t group,
                            uint8_t current_select,
                            uint8_t *states,
                            uint8_t states_cap);

// Begin a frame for a specific menu GROUP: snapshot only fields that are visible/active now.
void    menu_nav_begin(ui_group_t group);

uint8_t menu_nav_end(ui_state_field_t field, uint8_t current_select);
void    menu_nav_reset(ui_state_field_t field, uint8_t value);


uint8_t menu_nav_get_select(ui_state_field_t field);
void menu_nav_update_select(ui_state_field_t field, ui_group_t group);


uint8_t ui_state_modify(ui_state_field_t field, ui_modify_op_t op, uint8_t value_if_set);
uint8_t ui_state_get(ui_state_field_t field);


#endif /* MENU_INC_MENU_UI_H_ */
