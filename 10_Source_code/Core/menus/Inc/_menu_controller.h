/*
 * _menu_controller.h
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 */

#ifndef MIDI_INC_MENU_CONTROLLER_H_
#define MIDI_INC_MENU_CONTROLLER_H_

#include <stdint.h>
#include "memory_main.h"   // for save_field_t, SAVE_FIELD_COUNT, etc.

// ---------------------
// Menu list
// ---------------------
typedef enum {
    MENU_TEMPO = 0,
    MENU_MODIFY,
    MENU_TRANSPOSE,
    MENU_SETTINGS,
    AMOUNT_OF_MENUS,   // number of menus
    CURRENT_MENU,      // UI state index
    OLD_MENU,          // UI state index
    STATE_FIELD_COUNT  // total UI state slots
} menu_list_t;

// ---------------------
// UI submenu id
// ---------------------
typedef enum {
    CTRL_TEMPO_ALL = 1,

    CTRL_MODIFY_CHANGE,
    CTRL_MODIFY_SPLIT,
    CTRL_MODIFY_ALL,
    CTRL_MODIFY_VEL_CHANGED,
    CTRL_MODIFY_VEL_FIXED,

    CTRL_TRANSPOSE_SHIFT,
    CTRL_TRANSPOSE_SCALED,
    CTRL_TRANSPOSE_ALL,

    CTRL_SETTINGS_GLOBAL1,
    CTRL_SETTINGS_GLOBAL2,
    CTRL_SETTINGS_FILTER,
    CTRL_SETTINGS_ALL,
    CTRL_SETTINGS_ABOUT,
	CTRL_SETTINGS_ALWAYS,

} ctrl_group_id_t;



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
// Active list
// ---------------------
#ifndef MENU_ACTIVE_LIST_CAP
#define MENU_ACTIVE_LIST_CAP 64
#endif

typedef struct {
    uint16_t fields_idx[MENU_ACTIVE_LIST_CAP];
    uint8_t  count;
} CtrlActiveList;


// =====================
// Display flag helpers
// =====================

// 1) Display flags
typedef enum {
    FLAG_TEMPO      = (1u << 0),
    FLAG_MODIFY     = (1u << 1),
    FLAG_TRANSPOSE  = (1u << 2),
    FLAG_SETTINGS   = (1u << 3)
} DisplayFlags_t;

// 2) Forward declarations used by helpers (implemented elsewhere)
void    threads_display_notify(uint32_t flags);

// 3) Menu → flag lookup
static const DisplayFlags_t kMenuFlag[AMOUNT_OF_MENUS] = {
    /* MIDI_TEMPO     */ FLAG_TEMPO,
    /* MIDI_MODIFY    */ FLAG_MODIFY,
    /* MIDI_TRANSPOSE */ FLAG_TRANSPOSE,
    /* SETTINGS       */ FLAG_SETTINGS
};

// 4) Menu → "sending" save_field_t lookup
static const save_field_t kMenuSendingField[AMOUNT_OF_MENUS] = {
    /* MIDI_TEMPO     */ TEMPO_CURRENTLY_SENDING,
    /* MIDI_MODIFY    */ MODIFY_SENDING,
    /* MIDI_TRANSPOSE */ TRANSPOSE_SENDING,
    /* SETTINGS       */ SAVE_FIELD_INVALID
};

// 5) Small helpers
static inline DisplayFlags_t flag_for_menu(menu_list_t m) {
    return (m < AMOUNT_OF_MENUS) ? kMenuFlag[m] : FLAG_TEMPO;
}

static inline save_field_t sending_field_for_menu(menu_list_t m) {
    return (m < AMOUNT_OF_MENUS) ? kMenuSendingField[m] : SAVE_FIELD_INVALID;
}



// ---------------------
// UI API
// ---------------------
void select_press_menu_change(menu_list_t sel_field);

int8_t ui_selected_bit(save_field_t f);
uint8_t  ui_is_field_selected(save_field_t f);
uint32_t ui_active_groups(void);

void     menu_nav_begin_and_update(menu_list_t field);
void     save_mark_all_changed(void);

uint8_t  menu_nav_get_select(menu_list_t field);

uint8_t  ui_state_modify(menu_list_t field, ui_modify_op_t op, uint8_t value_if_set);
uint8_t  ui_state_get(menu_list_t field);

static inline void screen_refresh(void) {
    threads_display_notify(flag_for_menu((menu_list_t)ui_state_get(CURRENT_MENU)));
}


int8_t filter_selected_bits(save_field_t f);
void     update_menu(menu_list_t menu);

#endif /* MIDI_INC_MENU_CONTROLLER_H_ */
