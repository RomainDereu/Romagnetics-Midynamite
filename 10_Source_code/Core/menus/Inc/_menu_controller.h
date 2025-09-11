/*
 * menu_controller.h
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 */

#ifndef MIDI_INC_MENU_CONTROLLER_H_
#define MIDI_INC_MENU_CONTROLLER_H_

#include <stdint.h>
#include "memory_main.h"

// ---------------------
// Menu list
// ---------------------
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


// ---------------------
// UI state
// ---------------------
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




// ---------------------
// Modify ops
// ---------------------
typedef enum {
    UI_MODIFY_INCREMENT = 0,
    UI_MODIFY_SET,
} ui_modify_op_t;

// ---------------------
// Field change bits
// ---------------------
#define CHANGE_BITS_WORDS (((SAVE_FIELD_COUNT) + 31) / 32)
extern uint32_t s_field_change_bits[CHANGE_BITS_WORDS];

// ---------------------
// Wrapping options
// ---------------------
#define NO_WRAP  0
#define WRAP     1

// ---------------------
// Menu controls
// ---------------------
typedef void (*save_handler_t)(save_field_t field, uint8_t arg);

typedef struct {
    uint8_t        wrap;
    save_handler_t handler;
    uint8_t        handler_arg;
    uint32_t       groups;
} menu_controls_t;

extern const menu_controls_t menu_controls[SAVE_FIELD_COUNT];

// ---------------------
// Controller groups (bit flags)
// ---------------------
typedef enum {
    CTRL_G_TEMPO = 1,

	CTRL_G_MODIFY_CHANGE,
    CTRL_G_MODIFY_SPLIT,
    CTRL_G_MODIFY_BOTH,
    CTRL_G_MODIFY_VEL_CHANGED,
    CTRL_G_MODIFY_VEL_FIXED,

    CTRL_G_TRANSPOSE_SHIFT,
    CTRL_G_TRANSPOSE_SCALED,
    CTRL_G_TRANSPOSE_BOTH,

    CTRL_G_SETTINGS_GLOBAL1,
    CTRL_G_SETTINGS_GLOBAL2,
    CTRL_G_SETTINGS_FILTER,
    CTRL_G_SETTINGS_ALL,
    CTRL_G_SETTINGS_ABOUT,

} ctrl_group_id_t;

// ---------------------
// Active list
// ---------------------
#ifndef MENU_ACTIVE_LIST_CAP
#define MENU_ACTIVE_LIST_CAP 64
#endif

typedef struct {
    uint16_t fields_idx[MENU_ACTIVE_LIST_CAP];
    uint8_t  count;
} CtrlActiveList;

// ---------------------
// UI API
// ---------------------

void update_menu(menu_list_t menu);

void select_press_menu_change(ui_state_field_t sel_field);

uint8_t ui_is_field_selected(save_field_t f);

uint32_t ui_active_groups(void);


void menu_nav_begin_and_update(ui_state_field_t field);
uint8_t menu_nav_end_auto(ui_state_field_t field);

void save_mark_all_changed(void);

uint8_t build_select_states(ui_group_t group,
                            uint8_t current_select,
                            uint8_t *states,
                            uint8_t states_cap);


uint8_t handle_menu_toggle(GPIO_TypeDef *port, uint16_t pin1, uint16_t pin2);

void    menu_nav_reset(ui_state_field_t field, uint8_t value);

uint8_t menu_nav_get_select(ui_state_field_t field);

uint8_t ui_state_modify(ui_state_field_t field, ui_modify_op_t op, uint8_t value_if_set);
uint8_t ui_state_get(ui_state_field_t field);

#endif /* MIDI_INC_MENU_CONTROLLER_H_ */
