/*
 * memory_ui_state.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MEMORY_INC_MEMORY_UI_STATE_H_
#define MEMORY_INC_MEMORY_UI_STATE_H_

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
    int32_t min;
    int32_t max;
    uint8_t wrap;   // 0 = clamp, 1 = wrap
    int32_t def;

    void (*handler)(save_field_t field, uint8_t arg);
    uint8_t handler_arg;
    ui_group_t ui_group;
} menu_items_parameters_t;



// ---------------------
// UI structs
// ---------------------




// Variant selection per menu family
typedef struct {
    save_field_t key;               // field whose current value selects one option
    const ui_group_t *options;      // array of groups; index by clamped key value
    uint8_t option_count;           // number of options
} VariantSelector;



// Menu definition: always-active groups + independent variant selectors
typedef struct {
    const ui_group_t *always_groups;
    uint8_t always_count;

    const VariantSelector *selectors;
    uint8_t selector_count;
} MenuDef;




static volatile uint8_t ui_state_busy = 0;
extern const menu_items_parameters_t menu_items_parameters[SAVE_FIELD_COUNT];




// ---------------------
// API
// ---------------------

void save_mark_all_changed(void);
void mark_field_changed(save_field_t f);

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


#endif /* MEMORY_INC_MEMORY_UI_STATE_H_ */
