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
#include "memory_ui_state.h"

typedef void (*save_handler_t)(save_field_t field, uint8_t arg);

typedef struct {
    uint8_t      wrap;          // WRAP / NO_WRAP
    save_handler_t handler;     // update_value / update_contrast / update_channel_filter / no_update
    uint8_t       handler_arg;  // e.g. 1, 10, 12...
    uint32_t       groups;      // controller group bitmask (CTRL_G_*)
} menu_controls_t;




extern const menu_controls_t menu_controls[SAVE_FIELD_COUNT];

// ---- Controller API ----

// Bit-flags for controller groups (decoupled from UI enums)
typedef enum {
    CTRL_G_TEMPO               = 1u << 0,
    CTRL_G_SETTINGS            = 1u << 1,

    CTRL_G_TRANSPOSE_SHIFT     = 1u << 2,
    CTRL_G_TRANSPOSE_SCALED    = 1u << 3,
    CTRL_G_TRANSPOSE_BOTH      = 1u << 4,

    CTRL_G_MODIFY_CHANGE       = 1u << 5,
    CTRL_G_MODIFY_SPLIT        = 1u << 6,
    CTRL_G_MODIFY_BOTH         = 1u << 7,

    CTRL_G_MODIFY_VEL_CHANGED  = 1u << 8,
    CTRL_G_MODIFY_VEL_FIXED    = 1u << 9,
} ctrl_group_flag_t;

#ifndef MENU_ACTIVE_LIST_CAP
#define MENU_ACTIVE_LIST_CAP 64
#endif

typedef struct {
    uint16_t fields_idx[MENU_ACTIVE_LIST_CAP];
    uint8_t  count;
} CtrlActiveList;

// Build active groups mask from a UI "family" request.
// We accept your existing ui_group_t so views don't change much.
uint32_t ctrl_active_groups_from_ui_group(ui_group_t requested);

// Build an active field list from a group mask (uses internal field→group mapping).
void ctrl_build_active_fields(uint32_t active_groups, CtrlActiveList *out);

// Count rows (expands 16-bit virtual strip for SETTINGS_FILTERED_CHANNELS).
uint8_t ctrl_row_count(const CtrlActiveList *list);

// Apply "click/toggle" at a row (invokes handler or toggles a bit in channel filter).
void ctrl_toggle_row(const CtrlActiveList *list, uint8_t row_index);


#endif /* MIDI_INC_MENU_CONTROLLER_H_ */
