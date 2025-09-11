/*
 * _menu_controller.c
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 *
 */
#include "main.h"
#include "_menu_controller.h"
#include "_menu_ui.h"
#include "memory_main.h"
#include "threads.h"
#include "utils.h" //For the definitions of value updating functions

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
static uint16_t s_active_list[MENU_ACTIVE_LIST_CAP];
static uint8_t  s_active_count = 0;
static ui_group_t s_current_root_group = 0;


// Field-change tracking
uint32_t s_field_change_bits[CHANGE_BITS_WORDS] = {0};


// -------------------------
// Controller (from original _menu_controller.c)
// -------------------------

const menu_controls_t menu_controls[SAVE_FIELD_COUNT] = {
                                             //wrap,   handler,    handler_arg, group
    [MIDI_TEMPO_CURRENT_TEMPO]           = { NO_WRAP, update_value,         10, CTRL_G_TEMPO },
    [MIDI_TEMPO_CURRENTLY_SENDING]       = {   WRAP,  no_update,             0, CTRL_G_TEMPO },
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

    [MIDI_MODIFY_CURRENTLY_SENDING]      = {   WRAP,  no_update,             0, CTRL_G_MODIFY_BOTH },


    [MIDI_TRANSPOSE_TRANSPOSE_TYPE]      = {   WRAP,  no_update,             0, 0 }, // selector only
    [MIDI_TRANSPOSE_MIDI_SHIFT_VALUE]    = { NO_WRAP, update_value,         12, CTRL_G_TRANSPOSE_SHIFT },

	[MIDI_TRANSPOSE_BASE_NOTE]           = { NO_WRAP, update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_INTERVAL]            = { NO_WRAP, update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_TRANSPOSE_SCALE]     = {   WRAP,  update_value,          1, CTRL_G_TRANSPOSE_SCALED },

	[MIDI_TRANSPOSE_SEND_ORIGINAL]       = {   WRAP,  update_value,          1, CTRL_G_TRANSPOSE_BOTH },
    [MIDI_TRANSPOSE_CURRENTLY_SENDING]   = {   WRAP,  no_update,             0, 0 },


    [SETTINGS_START_MENU]                = {   WRAP,  update_value,          1, CTRL_G_SETTINGS_GLOBAL1  },
    [SETTINGS_SEND_USB]                  = {   WRAP,  update_value,          1, CTRL_G_SETTINGS_GLOBAL1  },
    [SETTINGS_BRIGHTNESS]                = { NO_WRAP, update_contrast,       1, CTRL_G_SETTINGS_GLOBAL1  },

    [SETTINGS_MIDI_THRU]                 = {   WRAP,  update_value,          1, CTRL_G_SETTINGS_GLOBAL2  },
    [SETTINGS_USB_THRU]                  = {   WRAP,  update_value,          1, CTRL_G_SETTINGS_GLOBAL2  },
    [SETTINGS_CHANNEL_FILTER]            = {   WRAP,  update_value,          1, CTRL_G_SETTINGS_GLOBAL2  },

    [SETTINGS_FILTERED_CHANNELS]         = {   WRAP,  update_channel_filter, 1, CTRL_G_SETTINGS_FILTER  }, // 16-bit strip
};

typedef struct {
    CtrlActiveList tempo_item_list;
    CtrlActiveList modify_item_list;
    CtrlActiveList transpose_item_list;
    CtrlActiveList settings_item_list;
} MenuActiveLists;

static MenuActiveLists s_menu_lists;





static ui_group_t root_group(ui_group_t g) {
    switch (g) {
        case UI_GROUP_TEMPO:
            return UI_GROUP_TEMPO;

        // Modify: any sub-group maps to BOTH
        case UI_GROUP_MODIFY:
        case UI_GROUP_MODIFY_CHANGE:
        case UI_GROUP_MODIFY_SPLIT:
        case UI_GROUP_MODIFY_BOTH:
        case UI_GROUP_MODIFY_VEL_CHANGED:
        case UI_GROUP_MODIFY_VEL_FIXED:
            return UI_GROUP_MODIFY_BOTH;

		// Transpose: any sub-group maps to BOTH
		case UI_GROUP_TRANSPOSE_SHIFT:
		case UI_GROUP_TRANSPOSE_SCALED:
		case UI_GROUP_TRANSPOSE_BOTH:
			return UI_GROUP_TRANSPOSE_BOTH;

        case UI_GROUP_SETTINGS:
            return UI_GROUP_SETTINGS;

        default:
            return UI_GROUP_TEMPO;
    }
}


// Map a select field to its display flag (for notify)
static inline uint32_t display_flag_from_field(ui_state_field_t field) {
    switch (field) {
        case UI_MIDI_TEMPO_SELECT:     return FLAG_TEMPO;
        case UI_MIDI_MODIFY_SELECT:    return FLAG_MODIFY;
        case UI_MIDI_TRANSPOSE_SELECT: return FLAG_TRANSPOSE;
        case UI_SETTINGS_SELECT:       return FLAG_SETTINGS;
        default:                       return FLAG_TEMPO;
    }
}

static CtrlActiveList* list_for_root(ui_group_t root) {
    switch (root) {
        case UI_GROUP_TEMPO:            return &s_menu_lists.tempo_item_list;
        case UI_GROUP_SETTINGS:         return &s_menu_lists.settings_item_list;
        case UI_GROUP_TRANSPOSE_BOTH:   return &s_menu_lists.transpose_item_list;
        case UI_GROUP_MODIFY_BOTH:      return &s_menu_lists.modify_item_list;
        default:                        return &s_menu_lists.tempo_item_list;
    }
}






static inline uint32_t flag_from_id(uint32_t id) {
    return (id >= 1 && id <= 31) ? (1u << (id - 1)) : 0u;
}


static inline uint8_t is_bits_item(save_field_t f) {
    return menu_controls[f].handler == update_channel_filter;
}


static uint32_t ctrl_active_groups_from_ui_group(ui_group_t requested)
{
    uint32_t mask = 0;

    switch (requested) {
        case UI_GROUP_TEMPO:
            mask |= flag_from_id(CTRL_G_TEMPO);
            break;

        case UI_GROUP_MODIFY_BOTH: {
            mask |= flag_from_id(CTRL_G_MODIFY_BOTH);

            int page = (int)save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
            if (page == MIDI_MODIFY_SPLIT) {
                mask |= flag_from_id(CTRL_G_MODIFY_SPLIT);
            } else { // default/fallback to CHANGE
                mask |= flag_from_id(CTRL_G_MODIFY_CHANGE);
            }

            int vel  = (int)save_get(MIDI_MODIFY_VELOCITY_TYPE);
            if (vel == MIDI_MODIFY_FIXED_VEL) {
                mask |= flag_from_id(CTRL_G_MODIFY_VEL_FIXED);
            } else { // default/fallback to CHANGED
                mask |= flag_from_id(CTRL_G_MODIFY_VEL_CHANGED);
            }
        } break;

        case UI_GROUP_TRANSPOSE_BOTH: {
            mask |= flag_from_id(CTRL_G_TRANSPOSE_BOTH);
            int t = (int)save_get(MIDI_TRANSPOSE_TRANSPOSE_TYPE);
            if (t == MIDI_TRANSPOSE_SHIFT) {
                mask |= flag_from_id(CTRL_G_TRANSPOSE_SHIFT);
            } else { // default/fallback to SCALED
                mask |= flag_from_id(CTRL_G_TRANSPOSE_SCALED);
            }
        } break;

        case UI_GROUP_SETTINGS: {
            // Optional: keep ALL for headers/dividers if you use it
            mask |= flag_from_id(CTRL_G_SETTINGS_ALL);

            // Count interactive rows per section (ABOUT has no fields → not counted)
            uint8_t rows_g1 = 0, rows_g2 = 0, rows_f = 0;
            for (uint16_t f = 0; f < SAVE_FIELD_COUNT; ++f) {
                const menu_controls_t mt = menu_controls[f];
                const uint8_t adv = is_bits_item((save_field_t)f) ? 16u : 1u;
                if      (mt.groups == CTRL_G_SETTINGS_GLOBAL1) rows_g1 = (uint8_t)(rows_g1 + adv);
                else if (mt.groups == CTRL_G_SETTINGS_GLOBAL2) rows_g2 = (uint8_t)(rows_g2 + adv);
                else if (mt.groups == CTRL_G_SETTINGS_FILTER)  rows_f  = (uint8_t)(rows_f  + adv);
            }

            const uint8_t sel = s_menu_selects[UI_SETTINGS_SELECT];
            const uint8_t rows_total = (uint8_t)(rows_g1 + rows_g2 + rows_f);
            // menu_nav_update_select adds +1, so the last index is ABOUT
            if (sel < rows_g1) {
                mask |= flag_from_id(CTRL_G_SETTINGS_GLOBAL1);
            } else if (sel < (uint8_t)(rows_g1 + rows_g2)) {
                mask |= flag_from_id(CTRL_G_SETTINGS_GLOBAL2);
            } else if (sel < rows_total) {
                mask |= flag_from_id(CTRL_G_SETTINGS_FILTER);
            } else {
                mask |= flag_from_id(CTRL_G_SETTINGS_ABOUT);  // only when selected
            }
        } break;



        default:
            mask |= flag_from_id(CTRL_G_TEMPO);
            break;
    }

    return mask;
}




static void ctrl_build_active_fields(uint32_t active_groups, CtrlActiveList *out)
{
    out->count = 0;
    for (uint16_t f = 0; f < SAVE_FIELD_COUNT && out->count < MENU_ACTIVE_LIST_CAP; ++f) {
        const menu_controls_t mt = menu_controls[f];

        uint32_t gm = flag_from_id(mt.groups);
        if ((gm & active_groups) == 0) continue;

        if (mt.handler == no_update && mt.groups != CTRL_G_SETTINGS_ABOUT) continue;

        out->fields_idx[out->count++] = f;
    }
}




static void rebuild_list_for_group(ui_group_t group) {
    ui_group_t root = root_group(group);
    uint32_t mask;

    if (root == UI_GROUP_SETTINGS) {
        // NAVIGATION spans all sections, including ABOUT
        mask =  flag_from_id(CTRL_G_SETTINGS_GLOBAL1)
              | flag_from_id(CTRL_G_SETTINGS_GLOBAL2)
              | flag_from_id(CTRL_G_SETTINGS_FILTER)
              | flag_from_id(CTRL_G_SETTINGS_ABOUT);   // <-- add this
    } else {
        mask = ctrl_active_groups_from_ui_group(root);
    }

    CtrlActiveList* out = list_for_root(root);
    ctrl_build_active_fields(mask, out);
}






// Map a select field to its root UI group (so callers don't pass group explicitly)
static inline ui_group_t group_from_select_field(ui_state_field_t field) {
    switch (field) {
        case UI_MIDI_TEMPO_SELECT:     return UI_GROUP_TEMPO;
        case UI_MIDI_MODIFY_SELECT:    return UI_GROUP_MODIFY_BOTH;
        case UI_MIDI_TRANSPOSE_SELECT: return UI_GROUP_TRANSPOSE_BOTH;
        case UI_SETTINGS_SELECT:       return UI_GROUP_SETTINGS;
        default:                       return UI_GROUP_TEMPO;
    }
}

static const CtrlActiveList* get_list_for_group(ui_group_t group) {
    return list_for_root(root_group(group));
}




static uint8_t ctrl_row_count(const CtrlActiveList *list)
{
    uint8_t rows = 0;
    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];
        rows += is_bits_item(f) ? 16 : 1;
    }
    return rows;
}


static void menu_nav_update_select(ui_state_field_t field, ui_group_t group)
{
    const int8_t step = encoder_read_step(&htim3);
    const CtrlActiveList* list = get_list_for_group(group);
    uint8_t rows = ctrl_row_count(list);

    if (group == UI_GROUP_SETTINGS) {
        rows = (uint8_t)(rows + 1);  // +1 virtual row for ABOUT
    }

    uint8_t sel_prev = s_menu_selects[field];
    s_prev_selects[field] = sel_prev;

    if (step == 0) return;

    if (rows == 0) { s_menu_selects[field] = 0; return; }
    if (sel_prev >= rows) sel_prev = (uint8_t)(rows - 1);

    int32_t v = (int32_t)sel_prev + (int32_t)step;
    int32_t m = v % rows; if (m < 0) m += rows;
    s_menu_selects[field] = (uint8_t)m;
}



// + paste this near other UI helpers

// Replace select_group_for_field_mask(...) with this:
static inline ui_group_t select_group_for_field_id(uint32_t id) {
    switch (id) {
        case CTRL_G_TEMPO:     return UI_GROUP_TEMPO;

        case CTRL_G_MODIFY_CHANGE:
        case CTRL_G_MODIFY_SPLIT:
        case CTRL_G_MODIFY_BOTH:
        case CTRL_G_MODIFY_VEL_CHANGED:
        case CTRL_G_MODIFY_VEL_FIXED:
            return UI_GROUP_MODIFY_BOTH;

        case CTRL_G_TRANSPOSE_SHIFT:
        case CTRL_G_TRANSPOSE_SCALED:
        case CTRL_G_TRANSPOSE_BOTH:
            return UI_GROUP_TRANSPOSE_BOTH;

        case CTRL_G_SETTINGS_GLOBAL1:
        case CTRL_G_SETTINGS_GLOBAL2:
        case CTRL_G_SETTINGS_FILTER:
        case CTRL_G_SETTINGS_ALL:
        case CTRL_G_SETTINGS_ABOUT:
            return UI_GROUP_SETTINGS;


        default:
            return UI_GROUP_TEMPO;
    }
}

static inline ui_state_field_t select_field_for_group(ui_group_t g) {
    switch (g) {
        case UI_GROUP_TEMPO:          return UI_MIDI_TEMPO_SELECT;
        case UI_GROUP_SETTINGS:       return UI_SETTINGS_SELECT;
        case UI_GROUP_TRANSPOSE_BOTH: return UI_MIDI_TRANSPOSE_SELECT;
        case UI_GROUP_MODIFY_BOTH:    return UI_MIDI_MODIFY_SELECT;
        default:                      return UI_MIDI_TEMPO_SELECT;
    }
}



uint8_t menu_nav_get_select(ui_state_field_t field) {
    return (field < UI_STATE_FIELD_COUNT) ? s_menu_selects[field] : 0;
}


static void menu_nav_begin(ui_group_t group)
{
    s_active_count = 0;

    // remember root for this page/frame
    s_current_root_group = root_group(group);

    rebuild_list_for_group(group);
    const CtrlActiveList* list = get_list_for_group(group);
    for (uint8_t i = 0; i < list->count && i < MENU_ACTIVE_LIST_CAP; ++i) {
        s_active_list[i] = list->fields_idx[i];
    }
    s_active_count = list->count;
}

uint32_t ui_active_groups(void) {
    return ctrl_active_groups_from_ui_group(s_current_root_group);
}

// Begin + update select (single entry point)
void menu_nav_begin_and_update(ui_state_field_t field) {
    ui_group_t g = group_from_select_field(field);
    menu_nav_begin(g);
    menu_nav_update_select(field, g);
}


typedef struct {
    ui_group_t   root;
    uint8_t      row;       // selected row index on the page
    save_field_t field;     // selected SAVE field (or INVALID)
    uint8_t      is_bits;   // 1 if this row is a bit of a 16-bit strip
    uint8_t      bit;       // which bit (0..15) if is_bits=1, else 0xFF
    uint32_t     gid;       // menu_controls[field].groups (0 if INVALID)
} NavSel;

/* Find the currently selected item (field/bit) for a given page select. */
static inline NavSel nav_selection(ui_state_field_t sel_field) {
    NavSel s = {0};
    s.root = group_from_select_field(sel_field);
    s.row  = menu_nav_get_select(sel_field);
    s.bit  = 0xFF;
    s.field = SAVE_FIELD_INVALID;

    const CtrlActiveList *list = get_list_for_group(s.root);
    uint8_t row = 0;

    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];

        if (is_bits_item(f)) {
            if (s.row >= row && s.row < (uint8_t)(row + 16u)) {
                s.field   = f;
                s.is_bits = 1;
                s.bit     = (uint8_t)(s.row - row);
                s.gid     = menu_controls[f].groups;
                return s;
            }
            row = (uint8_t)(row + 16u);
        } else {
            if (s.row == row) {
                s.field   = f;
                s.is_bits = 0;
                s.gid     = menu_controls[f].groups;
                return s;
            }
            row++;
        }
    }
    // nothing matched (empty page, etc.)
    return s;
}

/* For press-to-toggle-page logic: map the selection to its page selector. */
static inline save_field_t selector_for_press(const NavSel *s) {
    if (s->field == SAVE_FIELD_INVALID) return SAVE_FIELD_INVALID;

    if (s->root == UI_GROUP_TRANSPOSE_BOTH)
        return MIDI_TRANSPOSE_TRANSPOSE_TYPE;

    if (s->root == UI_GROUP_MODIFY_BOTH) {
        if (s->gid == CTRL_G_MODIFY_VEL_CHANGED || s->gid == CTRL_G_MODIFY_VEL_FIXED)
            return MIDI_MODIFY_VELOCITY_TYPE;
        return MIDI_MODIFY_CHANGE_OR_SPLIT;
    }
    return SAVE_FIELD_INVALID;
}



void select_press_menu_change(ui_state_field_t sel_field) {
    const NavSel s   = nav_selection(sel_field);
    const save_field_t tgt = selector_for_press(&s);
    if (tgt == SAVE_FIELD_INVALID) return;
    save_modify_u8(tgt, SAVE_MODIFY_INCREMENT, 0);
    s_menu_selects[sel_field] = 0;
    rebuild_list_for_group(group_from_select_field(sel_field));
}



/* Apply the row’s action (used for underline “calc step” and similar). */
static inline void nav_apply_selection(const NavSel *s) {
    if (s->field == SAVE_FIELD_INVALID) return;
    if (s->is_bits) {
        update_channel_filter(s->field, s->bit);
    } else {
        const menu_controls_t mt = menu_controls[s->field];
        if (mt.handler) mt.handler(s->field, mt.handler_arg);
    }
}

/* Tiny adapter to toggle the currently selected row. */
static inline void toggle_selected_row(ui_state_field_t sel_field) {
    NavSel s = nav_selection(sel_field);
    nav_apply_selection(&s);
}

/* “Is this field selected?” simplified to a single compare. */
uint8_t ui_is_field_selected(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return 0;
    const ui_group_t  root      = select_group_for_field_id(menu_controls[f].groups);
    const ui_state_field_t page = select_field_for_group(root);
    const NavSel s = nav_selection(page);
    return (s.field == f) ? 1u : 0u;
}

static inline uint8_t test_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return 0;
    return (s_field_change_bits[f >> 5] >> (f & 31)) & 1u;
}

static inline void clear_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return;
    s_field_change_bits[f >> 5] &= ~(1u << (f & 31));
}


static inline uint8_t any_field_changed(void) {
    for (int i = 0; i < CHANGE_BITS_WORDS; ++i) {
        if (s_field_change_bits[i]) return 1;
    }
    return 0;
}
static inline void clear_all_field_changed(void) {
    for (int i = 0; i < CHANGE_BITS_WORDS; ++i) {
        s_field_change_bits[i] = 0;
    }
}

static uint8_t has_menu_changed(ui_state_field_t field, uint8_t current_select)
{
    const uint8_t old_select  = (field < UI_STATE_FIELD_COUNT) ? s_prev_selects[field] : 0;
    const uint8_t sel_changed = (field < UI_STATE_FIELD_COUNT) && (old_select != current_select);

    const uint8_t data_changed = any_field_changed();

    if (field < UI_STATE_FIELD_COUNT) {
        s_menu_selects[field] = current_select;
        ui_state_modify(field, UI_MODIFY_SET, current_select);
    }

    if (data_changed) {
        clear_all_field_changed();   // consume all pending field changes
    }

    return (sel_changed || data_changed);
}






// -------------------------
// UI state & navigation (from original _menu_ui.c)
// -------------------------



void save_mark_all_changed(void) {
    for (int i = 0; i < CHANGE_BITS_WORDS; ++i) s_field_change_bits[i] = 0xFFFFFFFFu;
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


// ---------------------
// Settings: 16-channel filter drawer
// ---------------------
void filter_controller(void) {
    const uint32_t active = ui_active_groups();
    if ((active & (1u << (CTRL_G_SETTINGS_FILTER - 1))) == 0) return;

    const uint32_t mask     = (uint32_t)save_get(SETTINGS_FILTERED_CHANNELS);
    const uint8_t  base_idx = (uint8_t)(SETTINGS_FILTERED_CHANNELS - SETTINGS_START_MENU);
    const uint8_t  sel      = menu_nav_get_select(SETTINGS);

    filter_controller_ui(mask, base_idx, sel);
}

static uint8_t menu_nav_end_auto(ui_state_field_t field) {
    toggle_selected_row(field);

    const uint8_t sel = menu_nav_get_select(field);
    const uint8_t changed = has_menu_changed(field, sel);

    if (changed) {
        threads_display_notify(display_flag_from_field(field));
    }
    return changed;
}

static uint8_t handle_menu_toggle(GPIO_TypeDef *port,
                           uint16_t pin1,
                           uint16_t pin2)
{
    static uint8_t prev_state = 1;
    uint8_t s1 = HAL_GPIO_ReadPin(port, pin1);
    uint8_t s2 = HAL_GPIO_ReadPin(port, pin2);

    // detect a fresh press of pin1 while pin2 is held
    if (s1 == 0 && prev_state == 1 && s2 == 1) {
        osDelay(100);
        if (HAL_GPIO_ReadPin(port, pin1) == 0 &&
            HAL_GPIO_ReadPin(port, pin2) == 1)
        {
            prev_state = 0;
            return 1;
        }
    }

    prev_state = s1;
    return 0;
}



void update_menu(menu_list_t menu)
{
    ui_state_field_t field;

    // Pick the UI_* select directly
    switch (menu) {
        case MIDI_TEMPO:     field = UI_MIDI_TEMPO_SELECT;     break;
        case MIDI_MODIFY:    field = UI_MIDI_MODIFY_SELECT;    break;
        case MIDI_TRANSPOSE: field = UI_MIDI_TRANSPOSE_SELECT; break;
        case SETTINGS:       field = UI_SETTINGS_SELECT;       break;
        default:             field = UI_MIDI_TEMPO_SELECT;     break;
    }

    menu_nav_begin_and_update(field);

    // Per-page actions
    switch (menu) {
        case MIDI_TEMPO: {
            const uint32_t bpm = save_get(MIDI_TEMPO_CURRENT_TEMPO);
            save_modify_u32(MIDI_TEMPO_TEMPO_CLICK_RATE, SAVE_MODIFY_SET, 6000000u / (bpm * 24u));
        } break;

        case MIDI_TRANSPOSE:
        case MIDI_MODIFY: {
            if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
                // Your impl resets select to 0 after page toggle
                select_press_menu_change(field);
            }
        } break;

        case SETTINGS: {
            saving_settings_ui();
        } break;

        default: break;
    }

    (void)menu_nav_end_auto(field);
}


