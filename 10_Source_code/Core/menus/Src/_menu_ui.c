/*
 * _menu_ui.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */
#include "_menu_ui.h"
#include "_menu_controller.h"
#include "utils.h"

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


// ---------------------
// UI functions
// ---------------------

// ---- Field-change tracking (one bit per save_field_t) ----

uint32_t s_field_change_bits[CHANGE_BITS_WORDS] = {0};

static inline uint8_t test_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return 0;
    return (s_field_change_bits[f >> 5] >> (f & 31)) & 1u;
}

static inline void clear_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return;
    s_field_change_bits[f >> 5] &= ~(1u << (f & 31));
}

// Useful for big state transitions (load defaults / from flash)
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
    // cache into existing arrays so the rest of your code stays intact
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

    // Persist new select
    if (field < UI_STATE_FIELD_COUNT) {
        s_menu_selects[field] = current_select;
        ui_state_modify(field, UI_MODIFY_SET, current_select);
    }

    // Clear change bits only for the fields we tracked this frame
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

    // compute active row count
    const uint8_t rows = build_select_states(group, 0, NULL, 0);

    // snapshot previous (always) so menu_nav_end can compare reliably
    uint8_t sel_prev = s_menu_selects[field];
    s_prev_selects[field] = sel_prev;

    if (rows == 0) { s_menu_selects[field] = 0; return; }

    // sanitize current
    if (sel_prev >= rows) sel_prev = (uint8_t)(rows - 1);

    // no movement — keep persisted as-is (menu_nav_end will still compare prev vs current)
    if (step == 0) return;

    // apply delta with wrap
    int32_t v = (int32_t)sel_prev + (int32_t)step;
    int32_t m = v % rows; if (m < 0) m += rows;

    // persist new
    s_menu_selects[field] = (uint8_t)m;
}





static int ui_state_try_lock(void) {
    if (ui_state_busy) {
        return 0; // already in use
    }
    ui_state_busy = 1;
    return 1;
}

static void ui_state_unlock(void) {
    ui_state_busy = 0;
}



uint8_t ui_state_get(ui_state_field_t field) {
    if (!ui_state_try_lock()) {
        return UI_STATE_BUSY; // busy
    }

    uint8_t value = 0;

    switch (field) {
        case UI_CURRENT_MENU:           value = ui_state.current_menu; break;
        case UI_OLD_MENU:               value = ui_state.old_menu; break;
        default:                        value = 0;   break;
    }

    ui_state_unlock();
    return value;
}



uint8_t ui_state_set(ui_state_field_t field, uint8_t value) {
    if (!ui_state_try_lock()) {
        return 0; // busy
    }

    switch (field) {
        case UI_CURRENT_MENU: ui_state.current_menu = value; break;
        case UI_OLD_MENU: ui_state.old_menu = value; break;
        default:                        value = 0;   break;
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
            // We never increment OLD_MENU; keep as-is
            return 1; // no change
        default:
            // Select fields are not incremented here anymore (handled by menu_nav_*).
            return 1; // no change
    }
    return ui_state_set(field, value);
}


uint8_t ui_state_modify(ui_state_field_t field, ui_modify_op_t op, uint8_t value_if_set) {
    switch(op) {
        case UI_MODIFY_INCREMENT:
            return ui_state_increment(field);
        case UI_MODIFY_SET:
            return ui_state_set(field, value_if_set);
    }
    return 0; // unknown op
}

