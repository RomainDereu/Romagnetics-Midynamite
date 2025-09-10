/*
 * _menu_controller.c  (merged)
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 *
 *  NOTE: This file merges the original _menu_controller.c (controller/groups/handlers)
 *        and _menu_ui.c (UI state + navigation helpers) into a single translation unit.
 */

#include "_menu_controller.h"
#include "_menu_ui.h"
#include "memory_main.h"
#include "utils.h"

// -------------------------
// Controller (from original _menu_controller.c)
// -------------------------

const menu_controls_t menu_controls[SAVE_FIELD_COUNT] = {
                                             //wrap,   handler,    handler_arg, group
    [MIDI_TEMPO_CURRENT_TEMPO]           = { NO_WRAP, update_value,         10, CTRL_G_TEMPO },
    [MIDI_TEMPO_CURRENTLY_SENDING]       = {   WRAP,  no_update,             0, 0 },
    [MIDI_TEMPO_SEND_TO_MIDI_OUT]        = {   WRAP,  update_value,          1, CTRL_G_TEMPO },

    [MIDI_MODIFY_CHANGE_OR_SPLIT]        = {   WRAP,  no_update,             0, 0 }, // selector only
    [MIDI_MODIFY_VELOCITY_TYPE]          = {   WRAP,  no_update,             0, 0 }, // selector only

    [MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1] = { NO_WRAP, update_value,          1, CTRL_G_MODIFY_CHANGE },
    [MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2] = { NO_WRAP, update_value,          1, CTRL_G_MODIFY_CHANGE },

    [MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1]   = { NO_WRAP, update_value,          1, CTRL_G_MODIFY_SPLIT },
    [MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2]   = { NO_WRAP, update_value,          1, CTRL_G_MODIFY_SPLIT },
    [MIDI_MODIFY_SPLIT_NOTE]             = { NO_WRAP, update_value,         12, CTRL_G_MODIFY_SPLIT },

    [MIDI_MODIFY_SEND_TO_MIDI_OUT]       = {   WRAP,  update_value,          1, CTRL_G_MODIFY_BOTH },

    [MIDI_MODIFY_VELOCITY_PLUS_MINUS]    = { NO_WRAP, update_value,         10, CTRL_G_MODIFY_VEL_CHANGED },
    [MIDI_MODIFY_VELOCITY_ABSOLUTE]      = { NO_WRAP, update_value,         10, CTRL_G_MODIFY_VEL_FIXED },

    [MIDI_MODIFY_CURRENTLY_SENDING]      = {   WRAP,  no_update,             0, 0 },

    [MIDI_TRANSPOSE_TRANSPOSE_TYPE]      = {   WRAP,  no_update,             0, 0 }, // selector only
    [MIDI_TRANSPOSE_MIDI_SHIFT_VALUE]    = { NO_WRAP, update_value,         12, CTRL_G_TRANSPOSE_SHIFT },
    [MIDI_TRANSPOSE_BASE_NOTE]           = { NO_WRAP, update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_INTERVAL]            = { NO_WRAP, update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_TRANSPOSE_SCALE]     = {   WRAP,  update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_SEND_ORIGINAL]       = {   WRAP,  update_value,          1, CTRL_G_TRANSPOSE_BOTH },
    [MIDI_TRANSPOSE_CURRENTLY_SENDING]   = {   WRAP,  no_update,             0, 0 },

    [SETTINGS_START_MENU]                = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_SEND_USB]                  = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_BRIGHTNESS]                = { NO_WRAP, update_contrast,       1, CTRL_G_SETTINGS },
    [SETTINGS_MIDI_THRU]                 = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_USB_THRU]                  = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_CHANNEL_FILTER]            = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_FILTERED_CHANNELS]         = {   WRAP,  update_channel_filter, 1, CTRL_G_SETTINGS }, // 16-bit strip
};

static inline uint8_t is_bits_item(save_field_t f) {
    return menu_controls[f].handler == update_channel_filter;
}

uint32_t ctrl_active_groups_from_ui_group(ui_group_t requested)
{
    uint32_t mask = 0;

    switch (requested) {
        case UI_GROUP_TEMPO:
            mask |= CTRL_G_TEMPO;
            break;

        case UI_GROUP_SETTINGS:
            mask |= CTRL_G_SETTINGS;
            break;

        case UI_GROUP_TRANSPOSE_SHIFT:
        case UI_GROUP_TRANSPOSE_SCALED:
        case UI_GROUP_TRANSPOSE_BOTH: {
            mask |= CTRL_G_TRANSPOSE_BOTH;
            int v = (int)save_get(MIDI_TRANSPOSE_TRANSPOSE_TYPE);
            if (v <= MIDI_TRANSPOSE_SHIFT) mask |= CTRL_G_TRANSPOSE_SHIFT;
            else                           mask |= CTRL_G_TRANSPOSE_SCALED;
        } break;

        case UI_GROUP_MODIFY_CHANGE:
        case UI_GROUP_MODIFY_SPLIT:
        case UI_GROUP_MODIFY_BOTH:
        case UI_GROUP_MODIFY_VEL_CHANGED:
        case UI_GROUP_MODIFY_VEL_FIXED: {
            mask |= CTRL_G_MODIFY_BOTH;
            int page = (int)save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
            mask |= (page == MIDI_MODIFY_SPLIT) ? CTRL_G_MODIFY_SPLIT : CTRL_G_MODIFY_CHANGE;

            int vel  = (int)save_get(MIDI_MODIFY_VELOCITY_TYPE);
            mask |= (vel == MIDI_MODIFY_FIXED_VEL) ? CTRL_G_MODIFY_VEL_FIXED : CTRL_G_MODIFY_VEL_CHANGED;
        } break;

        default:
            mask |= CTRL_G_TEMPO;
            break;
    }

    return mask;
}

void ctrl_build_active_fields(uint32_t active_groups, CtrlActiveList *out)
{
    out->count = 0;
    for (uint16_t f = 0; f < SAVE_FIELD_COUNT && out->count < MENU_ACTIVE_LIST_CAP; ++f) {
        uint32_t gm = menu_controls[f].groups;
        if ((gm & active_groups) == 0) continue;
        out->fields_idx[out->count++] = f;
    }
}

uint8_t ctrl_row_count(const CtrlActiveList *list)
{
    uint8_t rows = 0;
    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];
        rows += is_bits_item(f) ? 16 : 1;
    }
    return rows;
}

void ctrl_toggle_row(const CtrlActiveList *list, uint8_t row_index)
{
    uint8_t row = 0;
    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];
        if (is_bits_item(f)) {
            if (row_index >= row && row_index < (uint8_t)(row + 16)) {
                uint8_t bit = (uint8_t)(row_index - row);
                update_channel_filter(f, bit);
                return;
            }
            row = (uint8_t)(row + 16);
        } else {
            if (row == row_index) {
                const menu_controls_t mt = menu_controls[f];
                if (mt.handler) mt.handler(f, mt.handler_arg);
                return;
            }
            row++;
        }
    }
}

// -------------------------
// UI state & navigation (from original _menu_ui.c)
// -------------------------

extern TIM_HandleTypeDef htim3;

typedef struct {
    uint8_t current_menu;
    uint8_t old_menu;
} ui_state_t;

static volatile uint8_t ui_state_busy = 0;
static ui_state_t ui_state = {0};

// Persisted select per menu page
static uint8_t s_menu_selects[UI_STATE_FIELD_COUNT] = {0};
static uint8_t s_prev_selects[UI_STATE_FIELD_COUNT] = {0};

// Per-frame list of active fields (indices)
#ifndef ACTIVE_LIST_CAP
#define ACTIVE_LIST_CAP 64
#endif
static uint16_t s_active_list[ACTIVE_LIST_CAP];
static uint8_t  s_active_count = 0;

// Field-change tracking
uint32_t s_field_change_bits[CHANGE_BITS_WORDS] = {0};

static inline uint8_t test_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return 0;
    return (s_field_change_bits[f >> 5] >> (f & 31)) & 1u;
}

static inline void clear_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return;
    s_field_change_bits[f >> 5] &= ~(1u << (f & 31));
}

void save_mark_all_changed(void) {
    for (int i = 0; i < CHANGE_BITS_WORDS; ++i) s_field_change_bits[i] = 0xFFFFFFFFu;
}

void toggle_underline_items(ui_group_t group, uint8_t index)
{
    const uint32_t mask = ctrl_active_groups_from_ui_group(group);
    CtrlActiveList list;
    ctrl_build_active_fields(mask, &list);
    ctrl_toggle_row(&list, index);
}

uint8_t build_select_states(ui_group_t group,
                            uint8_t current_select,
                            uint8_t *states,
                            uint8_t states_cap)
{
    const uint32_t mask = ctrl_active_groups_from_ui_group(group);
    CtrlActiveList list;
    ctrl_build_active_fields(mask, &list);

    const uint8_t rows = ctrl_row_count(&list);
    if (states && states_cap) {
        for (uint8_t i = 0; i < states_cap; ++i) states[i] = 0;
        if (rows && current_select < states_cap) states[current_select] = 1;
    }
    return rows;
}

void menu_nav_begin(ui_group_t group)
{
    s_active_count = 0;
    const uint32_t mask = ctrl_active_groups_from_ui_group(group);
    CtrlActiveList list;
    ctrl_build_active_fields(mask, &list);
    for (uint8_t i = 0; i < list.count && i < ACTIVE_LIST_CAP; ++i) {
        s_active_list[i] = list.fields_idx[i];
    }
    s_active_count = list.count;
}

uint8_t menu_nav_end(ui_state_field_t field, uint8_t current_select)
{
    const uint8_t old_select = (field < UI_STATE_FIELD_COUNT) ? s_prev_selects[field] : 0;
    const uint8_t sel_changed = (field < UI_STATE_FIELD_COUNT) && (old_select != current_select);

    uint8_t data_changed = 0;
    for (uint8_t i = 0; i < s_active_count; ++i) {
        save_field_t f = (save_field_t)s_active_list[i];
        if (test_field_changed(f)) { data_changed = 1; break; }
    }

    if (field < UI_STATE_FIELD_COUNT) {
        s_menu_selects[field] = current_select;
        ui_state_modify(field, UI_MODIFY_SET, current_select);
    }

    for (uint8_t i = 0; i < s_active_count; ++i) {
        clear_field_changed((save_field_t)s_active_list[i]);
    }

    return (sel_changed || data_changed);
}

void menu_nav_reset(ui_state_field_t field, uint8_t value)
{
    if (field >= UI_STATE_FIELD_COUNT) return;
    s_menu_selects[field] = value;
    ui_state_modify(field, UI_MODIFY_SET, value);
}

uint8_t menu_nav_get_select(ui_state_field_t field) {
    return (field < UI_STATE_FIELD_COUNT) ? s_menu_selects[field] : 0;
}

void menu_nav_update_select(ui_state_field_t field, ui_group_t group)
{
    const int8_t step = encoder_read_step(&htim3);
    const uint8_t rows = build_select_states(group, 0, NULL, 0);

    uint8_t sel_prev = s_menu_selects[field];
    s_prev_selects[field] = sel_prev;

    if (rows == 0) { s_menu_selects[field] = 0; return; }
    if (sel_prev >= rows) sel_prev = (uint8_t)(rows - 1);
    if (step == 0) return;

    int32_t v = (int32_t)sel_prev + (int32_t)step;
    int32_t m = v % rows; if (m < 0) m += rows;
    s_menu_selects[field] = (uint8_t)m;
}

static int ui_state_try_lock(void) {
    if (ui_state_busy) return 0;
    ui_state_busy = 1;
    return 1;
}

static void ui_state_unlock(void) { ui_state_busy = 0; }

uint8_t ui_state_get(ui_state_field_t field) {
    if (!ui_state_try_lock()) return UI_STATE_BUSY;

    uint8_t value = 0;
    switch (field) {
        case UI_CURRENT_MENU: value = ui_state.current_menu; break;
        case UI_OLD_MENU:     value = ui_state.old_menu;     break;
        default:              value = 0;                     break;
    }
    ui_state_unlock();
    return value;
}

static uint8_t ui_state_set(ui_state_field_t field, uint8_t value) {
    if (!ui_state_try_lock()) return 0;

    switch (field) {
        case UI_CURRENT_MENU: ui_state.current_menu = value; break;
        case UI_OLD_MENU:     ui_state.old_menu = value;     break;
        default: break;
    }
    ui_state_unlock();
    return 1;
}

static uint8_t ui_state_increment(ui_state_field_t field) {
    uint8_t value = ui_state_get(field);
    if (value == UI_STATE_BUSY) return UI_STATE_BUSY;

    switch (field) {
        case UI_CURRENT_MENU:
            value = (uint8_t)((value + 1u) % AMOUNT_OF_MENUS);
            break;
        case UI_OLD_MENU:
            return 1; // unchanged
        default:
            return 1; // selects handled elsewhere
    }
    return ui_state_set(field, value);
}

uint8_t ui_state_modify(ui_state_field_t field, ui_modify_op_t op, uint8_t value_if_set) {
    switch(op) {
        case UI_MODIFY_INCREMENT: return ui_state_increment(field);
        case UI_MODIFY_SET:       return ui_state_set(field, value_if_set);
    }
    return 0;
}
