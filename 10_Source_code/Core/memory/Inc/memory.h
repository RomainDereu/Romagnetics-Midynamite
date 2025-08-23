/*
 * memory.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MEMORY_INC_MEMORY_H_
#define MEMORY_INC_MEMORY_H_



#include "stm32f4xx_hal.h"

/*
typedef struct {
	//Modified in the tempo menu
	int32_t current_tempo;
	int32_t tempo_click_rate;
	uint8_t currently_sending;
	uint8_t send_to_midi_out;
}midi_tempo_data_struct;


typedef struct {
	//Modified in the settings
	uint8_t change_or_split;
	uint8_t velocity_type;
	uint8_t send_to_midi_out;

	//Modified in the menu
	uint8_t send_to_midi_channel_1;
	uint8_t send_to_midi_channel_2;


	uint8_t split_note;
	uint8_t split_midi_channel_1;
	uint8_t split_midi_channel_2;

	int32_t velocity_plus_minus;
	uint8_t velocity_absolute;

	uint8_t currently_sending;


}midi_modify_data_struct;

typedef struct {
	uint8_t transpose_type;

	int32_t midi_shift_value;
	uint8_t send_original;

	uint8_t transpose_base_note;
	uint8_t transpose_interval;
	uint8_t transpose_scale;

	uint8_t currently_sending;

}midi_transpose_data_struct;


typedef struct {
	uint8_t start_menu;
	uint8_t send_to_usb;
	uint8_t brightness;
	uint8_t channel_filter;

	uint8_t midi_thru;
	uint8_t usb_thru;
	uint16_t filtered_channels;

}settings_data_struct;


typedef struct {
	midi_tempo_data_struct midi_tempo_data;
	midi_modify_data_struct midi_modify_data;
	midi_transpose_data_struct midi_transpose_data;
	settings_data_struct settings_data;
	uint32_t check_data_validity;
}save_struct;

 */

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
    UI_CURRENT_MENU
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



typedef struct {
    uint8_t midi_tempo_current_select;
    uint8_t midi_modify_current_select;
    uint8_t midi_transpose_current_select;
    uint8_t settings_current_select;
    uint8_t current_menu;
} ui_state_t;




static volatile uint8_t ui_state_busy = 0;

uint8_t ui_state_modify(ui_state_field_t field, ui_modify_op_t op, uint8_t value_if_set);
uint8_t ui_state_get(ui_state_field_t field);


#endif /* MEMORY_INC_MEMORY_H_ */
