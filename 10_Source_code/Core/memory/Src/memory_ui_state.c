/*
 * memory_ui_state.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */
#include "memory_ui_state.h"
#include "utils.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;


typedef struct {
    uint8_t current_menu;
    uint8_t old_menu;
} ui_state_t;

static ui_state_t ui_state = {0};

// Persisted select per menu page
static uint8_t s_menu_selects[UI_STATE_FIELD_COUNT] = {0};
static uint8_t s_prev_selects[UI_STATE_FIELD_COUNT] = {0};


// Per-frame list of active fields (indices)
#ifndef ACTIVE_LIST_CAP
#define ACTIVE_LIST_CAP 64  // more than enough for your visible rows
#endif
static uint16_t s_active_list[ACTIVE_LIST_CAP];
static uint8_t  s_active_count = 0;


// ---------------------
// UI functions
// ---------------------

// ---- Field-change tracking (one bit per save_field_t) ----
#define CHANGE_BITS_WORDS (((SAVE_FIELD_COUNT) + 31) / 32)

static uint32_t s_field_change_bits[CHANGE_BITS_WORDS] = {0};

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


void mark_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return;
    s_field_change_bits[f >> 5] |= (1u << (f & 31));
}



// Family definitions
static const ui_group_t kTempoAlways[] = { UI_GROUP_TEMPO };
static const MenuDef kMenuTempo = { kTempoAlways, 1, NULL, 0 };

static const ui_group_t kSettingsAlways[] = { UI_GROUP_SETTINGS };
static const MenuDef kMenuSettings = { kSettingsAlways, 1, NULL, 0 };

// Transpose: one selector (SHIFT vs SCALED) + BOTH
static const ui_group_t kTransposeAlways[] = { UI_GROUP_TRANSPOSE_BOTH };
static const ui_group_t kTransposeOpts[]   = { UI_GROUP_TRANSPOSE_SHIFT, UI_GROUP_TRANSPOSE_SCALED };
static const VariantSelector kTransposeSel[] = {
    { MIDI_TRANSPOSE_TRANSPOSE_TYPE, kTransposeOpts, 2 },
};
static const MenuDef kMenuTranspose = { kTransposeAlways, 1, kTransposeSel, 1 };

// Modify: two selectors (CHANGE/SPLIT) and (VEL_CHANGED/VEL_FIXED) + BOTH
static const ui_group_t kModifyAlways[] = { UI_GROUP_MODIFY_BOTH };
static const ui_group_t kModifyPageOpts[] = { UI_GROUP_MODIFY_CHANGE, UI_GROUP_MODIFY_SPLIT };
static const ui_group_t kModifyVelOpts[]  = { UI_GROUP_MODIFY_VEL_CHANGED, UI_GROUP_MODIFY_VEL_FIXED };
static const VariantSelector kModifySel[] = {
    { MIDI_MODIFY_CHANGE_OR_SPLIT, kModifyPageOpts, 2 },
    { MIDI_MODIFY_VELOCITY_TYPE,   kModifyVelOpts,  2 },
};
static const MenuDef kMenuModify = { kModifyAlways, 1, kModifySel, 2 };



// Helper: an item is a 16-bit virtual strip if its handler is update_channel_filter
static inline uint8_t is_bits_item(save_field_t f) {
    return menu_items_parameters[f].handler == update_channel_filter;
}

// Build active group set for the family, combining "always" groups and variant-selected groups
static uint8_t build_active_groups(const MenuDef *def, ui_group_t *out, uint8_t cap) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < def->always_count && n < cap; ++i) out[n++] = def->always_groups[i];

    for (uint8_t s = 0; s < def->selector_count; ++s) {
        int v = (int)save_get(def->selectors[s].key);
        if (v < 0) v = 0;
        if (v >= def->selectors[s].option_count) v = def->selectors[s].option_count - 1;
        if (n < cap) out[n++] = def->selectors[s].options[v];
    }
    return n;
}

static inline uint8_t group_is_active(ui_group_t g, const ui_group_t *act, uint8_t n) {
    if (g == UI_GROUP_NONE) return 0; // do NOT render uncategorized items
    for (uint8_t i = 0; i < n; ++i) if (act[i] == g) return 1;
    return 0;
}

static inline const MenuDef* menu_def_for(ui_group_t group) {
    switch (group) {
        case UI_GROUP_TEMPO:             return &kMenuTempo;
        case UI_GROUP_SETTINGS:          return &kMenuSettings;

        case UI_GROUP_TRANSPOSE_SHIFT:
        case UI_GROUP_TRANSPOSE_SCALED:
        case UI_GROUP_TRANSPOSE_BOTH:    return &kMenuTranspose;

        case UI_GROUP_MODIFY_CHANGE:
        case UI_GROUP_MODIFY_SPLIT:
        case UI_GROUP_MODIFY_BOTH:
        case UI_GROUP_MODIFY_VEL_CHANGED:
        case UI_GROUP_MODIFY_VEL_FIXED:  return &kMenuModify;

        default:                         return &kMenuTempo;
    }
}


void toggle_underline_items(ui_group_t group, uint8_t index)
{
    const MenuDef *def = menu_def_for(group);

    ui_group_t active[8];
    uint8_t act_n = build_active_groups(def, active, 8);

    uint8_t row = 0;

    for (int f = 0; f < SAVE_FIELD_COUNT; ++f) {
        const menu_items_parameters_t *p = &menu_items_parameters[f];
        if (!group_is_active(p->ui_group, active, act_n)) continue;

        if (is_bits_item((save_field_t)f)) {
            if (index >= row && index < (uint8_t)(row + 16)) {
                uint8_t bit = (uint8_t)(index - row);
                update_channel_filter((save_field_t)f, bit);
                return;
            }
            row = (uint8_t)(row + 16);
        } else {
            if (row == index) {
                if (p->handler) p->handler((save_field_t)f, p->handler_arg);
                return;
            }
            row++;
        }
    }
}



uint8_t build_select_states(ui_group_t group,
                            uint8_t current_select,
                            uint8_t *states,
                            uint8_t states_cap)
{
    const MenuDef *def = menu_def_for(group);

    ui_group_t active[8];
    uint8_t act_n = build_active_groups(def, active, 8);

    uint8_t rows = 0;
    if (states && states_cap) {
        for (uint8_t i = 0; i < states_cap; ++i) states[i] = 0;
    }

    for (int f = 0; f < SAVE_FIELD_COUNT; ++f) {
        const menu_items_parameters_t *p = &menu_items_parameters[f];
        if (!group_is_active(p->ui_group, active, act_n)) continue;

        if (is_bits_item((save_field_t)f)) {
            for (uint8_t b = 0; b < 16; ++b) {
                if (states && rows == current_select && rows < states_cap) states[rows] = 1;
                rows++;
            }
        } else {
            if (states && rows == current_select && rows < states_cap) states[rows] = 1;
            rows++;
        }
    }
    return rows;
}


void menu_nav_begin(ui_group_t group)
{
    s_active_count = 0;

    const MenuDef *def = menu_def_for(group);
    ui_group_t active[8];
    const uint8_t act_n = build_active_groups(def, active, 8);

    for (uint16_t f = 0; f < SAVE_FIELD_COUNT && s_active_count < ACTIVE_LIST_CAP; ++f) {
        const menu_items_parameters_t *p = &menu_items_parameters[f];
        if (!group_is_active(p->ui_group, active, act_n)) continue;
        s_active_list[s_active_count++] = f;
    }
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
    if (value == UI_STATE_BUSY) {
        return UI_STATE_BUSY;
    }

    value++;

    if (value > ui_limits[field].max) {
        if (ui_limits[field].wrap) {
            value = ui_limits[field].min; // wrap
        } else {
            value = ui_limits[field].max; // clamp
        }
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


void    ui_state_note_select_changed(void);
uint8_t ui_state_toggle_select_changed(void);

