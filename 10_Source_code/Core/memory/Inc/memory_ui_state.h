/*
 * memory_ui_state.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MEMORY_INC_MEMORY_UI_STATE_H_
#define MEMORY_INC_MEMORY_UI_STATE_H_
#include "stm32f4xx_hal.h"

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


typedef struct {
    uint8_t min;
    uint8_t max;
    uint8_t wrap;  // if 1, loop back to min when exceeding max
} ui_field_limits_t;

static const ui_field_limits_t ui_limits[] = {
    [UI_MIDI_TEMPO_SELECT]    = {TEMPO_PRINT, AMOUNT_OF_TEMPO_ITEMS, 1},
    [UI_MIDI_MODIFY_SELECT]   = {0, 5, 1},   // 0–5 looping
    [UI_MIDI_TRANSPOSE_SELECT]= {0, 11, 1},  // 12 steps looping
    [UI_SETTINGS_SELECT]      = {0, 3, 1},   // 4 settings
    [UI_CURRENT_MENU]         = {MIDI_TEMPO, AMOUNT_OF_MENUS, 1},
};

typedef enum {
    UI_MODIFY_INCREMENT = 0,
	UI_MODIFY_SET,
} ui_modify_op_t;


static volatile uint8_t ui_state_busy = 0;

uint8_t ui_state_modify(ui_state_field_t field, ui_modify_op_t op, uint8_t value_if_set);
uint8_t ui_state_get(ui_state_field_t field);


#endif /* MEMORY_INC_MEMORY_UI_STATE_H_ */
