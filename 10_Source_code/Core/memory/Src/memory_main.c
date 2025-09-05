/*
 * memory_main.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Romain Dereu
 */

#include <string.h>

#include "memory_main.h"
#include "memory_ui_state.h"
#include "stm32f4xx_hal.h"
#include "utils.h"



// Expose for tests
const menu_items_parameters_t menu_items_parameters[SAVE_FIELD_COUNT] = {

    //                                         min    max     wrap     def    handler       handler_arg   ui_group
    [MIDI_TEMPO_CURRENT_TEMPO]            = {   20,   300,    NO_WRAP, 120,   update_value   , 10,      UI_GROUP_TEMPO },
    [MIDI_TEMPO_TEMPO_CLICK_RATE]         = {    1,   50000,  NO_WRAP,  24,   no_update      ,  0,      UI_GROUP_NONE },
    [MIDI_TEMPO_CURRENTLY_SENDING]        = {    0,   1,      WRAP,      0,   no_update      ,  0,      UI_GROUP_NONE },
    [MIDI_TEMPO_SEND_TO_MIDI_OUT]         = {    0,   2,      WRAP,      0,   update_value   ,  1,      UI_GROUP_TEMPO },


    [MIDI_MODIFY_CHANGE_OR_SPLIT]         = {    0,   1,      WRAP,      1,   no_update      ,  0,      UI_GROUP_NONE },
    [MIDI_MODIFY_VELOCITY_TYPE]           = {    0,   1,      WRAP,      0,   no_update      ,  0,      UI_GROUP_NONE },

    [MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1]  = {    1,   16,     NO_WRAP,   1,   update_value   ,  1,      UI_GROUP_MODIFY_CHANGE },
    [MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2]  = {    0,   16,     NO_WRAP,   0,   update_value   ,  1,      UI_GROUP_MODIFY_CHANGE },
    [MIDI_MODIFY_SEND_TO_MIDI_OUT]        = {    0,   3,      WRAP,      0,   update_value   ,  1,      UI_GROUP_MODIFY_BOTH },

	[MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1]    = {    1,   16,     NO_WRAP,   1,   update_value   ,  1,      UI_GROUP_MODIFY_SPLIT },
    [MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2]    = {    1,   16,     NO_WRAP,   2,   update_value   ,  1,      UI_GROUP_MODIFY_SPLIT },
	[MIDI_MODIFY_SPLIT_NOTE]              = {    0,   127,    NO_WRAP,  60,   update_value   ,  12,     UI_GROUP_MODIFY_SPLIT },

	[MIDI_MODIFY_VELOCITY_PLUS_MINUS]     = { -127,   127,    NO_WRAP,   0,   update_value   ,  10,     UI_GROUP_MODIFY_VEL_CHANGED },
    [MIDI_MODIFY_VELOCITY_ABSOLUTE]       = {    0,   127,    NO_WRAP,  64,   update_value   ,  10,     UI_GROUP_MODIFY_VEL_FIXED },

	[MIDI_MODIFY_CURRENTLY_SENDING]       = {    0,   1,      WRAP,      0,   no_update      ,  0,      UI_GROUP_NONE },


	[MIDI_TRANSPOSE_TRANSPOSE_TYPE]       = {    0,   1,      WRAP,      0,   no_update      ,  0,      UI_GROUP_NONE },
	[MIDI_TRANSPOSE_MIDI_SHIFT_VALUE]     = { -127, 127,      NO_WRAP,   0,   update_value   , 12,      UI_GROUP_TRANSPOSE_SHIFT },
    [MIDI_TRANSPOSE_BASE_NOTE]            = {    0,   11,     NO_WRAP,   0,   update_value   ,  1,      UI_GROUP_TRANSPOSE_SCALED  },
    [MIDI_TRANSPOSE_INTERVAL]             = {    0,   9,      NO_WRAP,   0,   update_value   ,  1,      UI_GROUP_TRANSPOSE_SCALED  },
    [MIDI_TRANSPOSE_TRANSPOSE_SCALE]      = {    0,   6,      WRAP,      0,   update_value   ,  1,      UI_GROUP_TRANSPOSE_SCALED  },
    [MIDI_TRANSPOSE_SEND_ORIGINAL]        = {    0,   1,      WRAP,      0,   update_value   ,  1,      UI_GROUP_TRANSPOSE_BOTH },
    [MIDI_TRANSPOSE_CURRENTLY_SENDING]    = {    0,   1,      WRAP,      0,   no_update      ,  0,      UI_GROUP_NONE },


    [SETTINGS_START_MENU]                 = {    0,   3,      WRAP,      0,   update_value   ,  1,      UI_GROUP_SETTINGS },
    [SETTINGS_SEND_USB]                   = {    0,   1,      WRAP,      0,   update_value   ,  1,      UI_GROUP_SETTINGS },
    [SETTINGS_BRIGHTNESS]                 = {    0,   9,      NO_WRAP,   0,   update_contrast,  1,      UI_GROUP_SETTINGS },
    [SETTINGS_MIDI_THRU]                  = {    0,   1,      WRAP,      0,   update_value   ,  1,      UI_GROUP_SETTINGS },
    [SETTINGS_USB_THRU]                   = {    0,   1,      WRAP,      0,   update_value   ,  1,      UI_GROUP_SETTINGS },
    [SETTINGS_CHANNEL_FILTER]             = {    0,   1,      WRAP,      0,   update_value   ,  1,      UI_GROUP_SETTINGS },
    [SETTINGS_FILTERED_CHANNELS]          = {    0,   0x0000FFFF, WRAP,  0,   update_channel_filter   ,  1,      UI_GROUP_SETTINGS },


    [SAVE_DATA_VALIDITY]                  = {    0,   0xFFFFFFFF, NO_WRAP, DATA_VALIDITY_CHECKSUM, no_update, 0, UI_GROUP_NONE },
};

// ---------------------
// UI functions
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

// Router: which family to use based on the incoming group
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

// Helper: an item is a 16-bit virtual strip if its handler is update_channel_filter
extern void update_channel_filter(save_field_t field, uint8_t bit_index);
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

// -------- Rewritten build_select_states (generic) --------
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

// -------- Rewritten toggle_underline_items (generic) --------
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






// ---------------------
// Runtime save copy
// ---------------------

static volatile uint8_t save_busy = 0;

// ---------------------
// Lock helpers
// ---------------------
static int save_try_lock(void) {
    if (save_busy) return 0;
    save_busy = 1;
    return 1;
}
static void save_unlock(void) { save_busy = 0; }

// (Optional) bounded retry for writers; cheap and unit-test friendly
static int save_lock_with_retries(void) {
    for (int i = 0; i < 5; i++) {
        if (save_try_lock()) return 1;
        // short, bounded spin
        volatile int spin = 200;
        while (spin--) { /* no-op */ }
    }
    return 0;
}


// ---------------------
// Generic getters (BUSY-safe)
// ---------------------
// Small bounded wait helper used by getters (non-blocking feel)
static inline void save_busy_wait_short(void) {
    // ~ a few hundred no-ops; tweak if needed
    volatile int spin = 200;
    while (spin--) { /* no-op */ }
}

static inline int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}



// ---- Field-change tracking (one bit per save_field_t) ----
#define CHANGE_BITS_WORDS (((SAVE_FIELD_COUNT) + 31) / 32)

static uint32_t s_field_change_bits[CHANGE_BITS_WORDS] = {0};

static inline void mark_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return;
    s_field_change_bits[f >> 5] |= (1u << (f & 31));
}

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


// Persisted select per menu page
static uint8_t s_menu_selects[UI_STATE_FIELD_COUNT] = {0};

// Per-frame list of active fields (indices)
#ifndef ACTIVE_LIST_CAP
#define ACTIVE_LIST_CAP 64  // more than enough for your visible rows
#endif
static uint16_t s_active_list[ACTIVE_LIST_CAP];
static uint8_t  s_active_count = 0;

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



uint8_t menu_nav_end(ui_state_field_t field, ui_group_t group, uint8_t current_select)
{
    const uint8_t old_select = (field < UI_STATE_FIELD_COUNT) ? s_menu_selects[field] : 0;
    const uint8_t sel_changed = (field < UI_STATE_FIELD_COUNT) && (old_select != current_select);

    uint8_t data_changed = 0;
    for (uint8_t i = 0; i < s_active_count; ++i) {
        save_field_t f = (save_field_t)s_active_list[i];
        if (test_field_changed(f)) { data_changed = 1; break; }
    }

    // Persist new select
    if (field < UI_STATE_FIELD_COUNT) {
        s_menu_selects[field] = current_select;
        // Mirror into UI state so painters can read it
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
}

int32_t save_get_u32(save_field_t field) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    int32_t *p = u32_fields[field];
    if (!p) return 0;

    for (int i = 0; i < 5; ++i) {
        if (!save_busy) break;
        save_busy_wait_short();
    }

    int32_t v = *p;
    const menu_items_parameters_t lim = menu_items_parameters[field];
    return clamp_i32(v, lim.min, lim.max);
}

uint8_t save_get(save_field_t field) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    uint8_t *p = u8_fields[field];
    if (!p) return 0;

    for (int i = 0; i < 5; ++i) {
        if (!save_busy) break;
        save_busy_wait_short();
    }

    int32_t v = (int32_t)(*p);
    const menu_items_parameters_t lim = menu_items_parameters[field];
    return (uint8_t)clamp_i32(v, lim.min, lim.max);
}





// ---------------------
// Increment / set (u32 / u8)
// ---------------------
uint8_t save_modify_u32(save_field_t field, save_modify_op_t op, uint32_t value_if_set) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    if (!u32_fields[field]) return 0;
    if (!save_lock_with_retries()) return 0;

    const menu_items_parameters_t lim = menu_items_parameters[field];
    int32_t old_v = *u32_fields[field];
    int32_t v = old_v;

    switch (op) {
        case SAVE_MODIFY_INCREMENT: v += 1; break;
        case SAVE_MODIFY_SET:       v  = (int32_t)value_if_set; break;
        default: save_unlock(); return 0;
    }

    v = wrap_or_clamp_i32(v, lim.min, lim.max, lim.wrap);

    if (v != old_v) {
        *u32_fields[field] = v;
        mark_field_changed(field);
    }

    save_unlock();
    return 1;
}

uint8_t save_modify_u8(save_field_t field, save_modify_op_t op, uint8_t value_if_set) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    if (!u8_fields[field]) return 0;
    if (!save_lock_with_retries()) return 0;

    const menu_items_parameters_t lim = menu_items_parameters[field];
    uint8_t old_v = *u8_fields[field];
    int32_t v = (int32_t)old_v;

    switch (op) {
        case SAVE_MODIFY_SET: {
            int32_t desired = (int32_t)value_if_set;
            if (!lim.wrap) {
                if (desired > lim.max && desired >= 128) v = lim.min;
                else if (desired < lim.min)             v = lim.min;
                else                                     v = desired;
            } else {
                v = desired;
            }
        } break;

        case SAVE_MODIFY_INCREMENT:
            v += 1;
            break;

        default:
            save_unlock();
            return 0;
    }

    v = wrap_or_clamp_i32(v, lim.min, lim.max, lim.wrap);
    uint8_t new_v = (uint8_t)v;

    if (new_v != old_v) {
        *u8_fields[field] = new_v;
        mark_field_changed(field);
    }

    save_unlock();
    return 1;
}

