/* _menu_controller.h
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 */

#ifndef MIDI_INC_MENU_CONTROLLER_H_
#define MIDI_INC_MENU_CONTROLLER_H_

#include <stdint.h>
#include "memory_main.h"   // for save_field_t, SAVE_FIELD_COUNT, etc.

// Will be defined somewhere else once functions to be exposed aren't related to memory
#ifndef STATIC_PRODUCTION
#  ifdef UNIT_TEST
#    define STATIC_PRODUCTION   /* empty: export symbol to linker */
#  else
#    define STATIC_PRODUCTION static
#  endif
#endif

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

// Forward declaration used elsewhere
void threads_display_notify(uint32_t flags);

// Return a single-bit mask for a menu.
static inline uint32_t flag_for_menu(menu_list_t m) {
    return (m < AMOUNT_OF_MENUS) ? (1u << (uint32_t)m) : (1u << (uint32_t)MENU_TEMPO);
}

// Menu → "sending" save_field_t lookup
static inline save_field_t sending_field_for_menu(menu_list_t m) {
    switch (m) {
        case MENU_TEMPO:     return TEMPO_CURRENTLY_SENDING;
        case MENU_MODIFY:    return MODIFY_SENDING;
        case MENU_TRANSPOSE: return TRANSPOSE_SENDING;
        case MENU_SETTINGS:  return SAVE_FIELD_INVALID;
        default:             return SAVE_FIELD_INVALID;
    }
}

// ---------------------
// UI API
// ---------------------
void select_press_menu_change(menu_list_t sel_field);

int8_t   ui_selected_bit(save_field_t f);
uint8_t  ui_is_field_selected(save_field_t f);
uint32_t ui_active_groups(void);

void     menu_nav_begin_and_update(menu_list_t field);
void     save_mark_all_changed(void);

uint8_t  menu_nav_get_select(menu_list_t field);

int8_t   filter_selected_bits(save_field_t f);
void     update_menu(menu_list_t menu);

#ifdef UNIT_TEST
void no_update(save_field_t field, uint8_t arg);
void shadow_select(save_field_t field, uint8_t arg);
void update_value(save_field_t field, uint8_t multiplier);
void update_contrast(save_field_t f, uint8_t step);
void update_channel_filter(save_field_t field, uint8_t bit_index);
#endif

#endif /* MIDI_INC_MENU_CONTROLLER_H_ */
