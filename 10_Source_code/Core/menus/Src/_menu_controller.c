/*
 * _menu_controller.c
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 *
 *  ui_group_t removed. Visibility/selection are driven by ctrl_group_id_t + memory state.
 */
#include "cmsis_os.h" //For osDelay
#include "main.h"
#include "_menu_controller.h"
#include "_menu_ui.h"
#include "menus.h"
#include "memory_main.h"
#include "stm32f4xx_hal.h"   // HAL types (TIM, GPIO)
#include "threads.h"

#define SETTINGS_ROWS_G1      3u
#define SETTINGS_ROWS_G2      3u
#define SETTINGS_ROWS_FILTER 16u

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

static uint8_t s_menu_selects[AMOUNT_OF_MENUS] = {0};
static uint8_t s_prev_selects[AMOUNT_OF_MENUS] = {0};

uint32_t s_field_change_bits[CHANGE_BITS_WORDS] = {0};


// -------------------------
// Encoder & value helpers
// -------------------------
static int8_t encoder_read_step(TIM_HandleTypeDef *timer) {
    int32_t delta = __HAL_TIM_GET_COUNTER(timer) - ENCODER_CENTER;
    if (delta <= -ENCODER_THRESHOLD) { __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER); return -1; }
    if (delta >=  ENCODER_THRESHOLD) { __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER); return +1; }
    return 0; // no step
}

STATIC_PRODUCTION  void no_update(save_field_t field, uint8_t arg) {
    (void)field; (void)arg;
}

//Displayed but not updated
STATIC_PRODUCTION  void shadow_select(save_field_t field, uint8_t arg) {
    (void)field; (void)arg;
}

STATIC_PRODUCTION  void update_value(save_field_t field, uint8_t multiplier)
{
    TIM_HandleTypeDef *timer = &htim4;

    uint8_t active_mult = 1;
    if (multiplier != 1) {
        uint8_t Btn2State = HAL_GPIO_ReadPin(GPIOB, Btn2_Pin);
        active_mult = (Btn2State == 0) ? multiplier : 1;
    }

    int8_t step = encoder_read_step(timer);
    if (step == 0) return;

    const int32_t delta = (int32_t)step * (int32_t)active_mult;

    // Get current, add delta (no wrap here)
    int32_t cur = (int32_t)save_get(field);
    int32_t next = cur + delta;

    // Let save layer apply the (single) wrap/clamp via SET
    if (u32_fields[field]) {
        (void)save_modify_u32(field, SAVE_MODIFY_SET, (uint32_t)next);
    } else {
        (void)save_modify_u8(field,  SAVE_MODIFY_SET, (uint8_t)next);
    }
}

STATIC_PRODUCTION void update_contrast(save_field_t f, uint8_t step) {
    update_value(f, step);
    update_contrast_ui();
}

STATIC_PRODUCTION void update_channel_filter(save_field_t field, uint8_t bit_index)
{
    if (bit_index > 15) return;

    TIM_HandleTypeDef *timer4 = &htim4;
    int8_t step = encoder_read_step(timer4);
    if (step == 0) return;

    uint32_t mask = (uint32_t)save_get(field);
    mask ^= (1UL << bit_index);  // toggle exactly this bit
    (void)save_modify_u32(field, SAVE_MODIFY_SET, mask);
}


// -------------------------
// Controller table
// -------------------------
const menu_controls_t menu_controls[SAVE_FIELD_COUNT] = {
    //                                wrap     handler             handler_arg   group
    [TEMPO_CURRENT_TEMPO]        = { NO_WRAP, update_value,            10,      CTRL_TEMPO_ALL },
    [TEMPO_CURRENTLY_SENDING]    = {   WRAP,  no_update,                0,      CTRL_TEMPO_ALL },
    [TEMPO_SEND_TO_MIDI_OUT]     = {   WRAP,  update_value,             1,      CTRL_TEMPO_ALL },

    [MODIFY_CHANGE_OR_SPLIT]     = {   WRAP,  no_update,                0,      0 },
    [MODIFY_VELOCITY_TYPE]       = {   WRAP,  no_update,                0,      0 },

    [MODIFY_SEND_TO_MIDI_CH1]    = { NO_WRAP, update_value,             1,      CTRL_MODIFY_CHANGE },
    [MODIFY_SEND_TO_MIDI_CH2]    = { NO_WRAP, update_value,             1,      CTRL_MODIFY_CHANGE },

    [MODIFY_SPLIT_MIDI_CH1]      = { NO_WRAP, update_value,             1,      CTRL_MODIFY_SPLIT },
    [MODIFY_SPLIT_MIDI_CH2]      = { NO_WRAP, update_value,             1,      CTRL_MODIFY_SPLIT },
    [MODIFY_SPLIT_NOTE]          = { NO_WRAP, update_value,            12,      CTRL_MODIFY_SPLIT },

    [MODIFY_SEND_TO_MIDI_OUT]    = {   WRAP,  update_value,             1,      CTRL_MODIFY_ALL },

    [MODIFY_VEL_PLUS_MINUS]      = { NO_WRAP, update_value,            10,      CTRL_MODIFY_VEL_CHANGED },
    [MODIFY_VEL_ABSOLUTE]        = { NO_WRAP, update_value,            10,      CTRL_MODIFY_VEL_FIXED },

    [MODIFY_SENDING]             = {   WRAP,  no_update,                0,      CTRL_MODIFY_ALL },

    [TRANSPOSE_TRANSPOSE_TYPE]   = {   WRAP,  no_update,                0,      0 },
    [TRANSPOSE_MIDI_SHIFT_VALUE] = { NO_WRAP, update_value,            12,      CTRL_TRANSPOSE_SHIFT },

    [TRANSPOSE_BASE_NOTE]        = { NO_WRAP, update_value,             1,      CTRL_TRANSPOSE_SCALED },
    [TRANSPOSE_INTERVAL]         = { NO_WRAP, update_value,             1,      CTRL_TRANSPOSE_SCALED },
    [TRANSPOSE_TRANSPOSE_SCALE]  = {   WRAP,  update_value,             1,      CTRL_TRANSPOSE_SCALED },

    [TRANSPOSE_SEND_ORIGINAL]    = {   WRAP,  update_value,             1,      CTRL_TRANSPOSE_ALL },
    [TRANSPOSE_SENDING]          = {   WRAP,  no_update,                0,      0 },

    [SETTINGS_START_MENU]        = {   WRAP,  update_value,             1,      CTRL_SETTINGS_GLOBAL1 },
    [SETTINGS_SEND_USB]          = {   WRAP,  update_value,             1,      CTRL_SETTINGS_GLOBAL1 },
    [SETTINGS_BRIGHTNESS]        = { NO_WRAP, update_contrast,          1,      CTRL_SETTINGS_GLOBAL1 },

    [SETTINGS_MIDI_THRU]         = {   WRAP,  update_value,             1,      CTRL_SETTINGS_GLOBAL2 },
    [SETTINGS_USB_THRU]          = {   WRAP,  update_value,             1,      CTRL_SETTINGS_GLOBAL2 },
    [SETTINGS_CHANNEL_FILTER]    = {   WRAP,  update_value,             1,      CTRL_SETTINGS_GLOBAL2 },

    [SETTINGS_FILTERED_CH]       = {   WRAP,  update_channel_filter,    1,      CTRL_SETTINGS_FILTER },

    [SETTINGS_ABOUT]             = { NO_WRAP, shadow_select,            0,      CTRL_SETTINGS_ABOUT },
};


// -------------------------
// Per-menu active lists
// -------------------------
typedef struct {
    CtrlActiveList tempo_item_list;
    CtrlActiveList modify_item_list;
    CtrlActiveList transpose_item_list;
    CtrlActiveList settings_item_list;
} MenuActiveLists;

static MenuActiveLists s_menu_lists;

// --- Menu <-> Root group mapping (ui_group_t removed) ---
static const ctrl_group_id_t kMenuToRoot[AMOUNT_OF_MENUS] = {
  [MENU_TEMPO]     = CTRL_TEMPO_ALL,
  [MENU_MODIFY]    = CTRL_MODIFY_ALL,
  [MENU_TRANSPOSE] = CTRL_TRANSPOSE_ALL,
  [MENU_SETTINGS]  = CTRL_SETTINGS_ALL,
};

// Map a ctrl_group_id_t (any subgroup) to its owning menu
static inline menu_list_t page_for_ctrl_id(uint32_t id) {
  if (id == CTRL_TEMPO_ALL)                                      return MENU_TEMPO;
  if (id >= CTRL_MODIFY_CHANGE   && id <= CTRL_MODIFY_VEL_FIXED)  return MENU_MODIFY;
  if (id >= CTRL_TRANSPOSE_SHIFT && id <= CTRL_TRANSPOSE_ALL)     return MENU_TRANSPOSE;
  if (id >= CTRL_SETTINGS_GLOBAL1&& id <= CTRL_SETTINGS_ALWAYS)   return MENU_SETTINGS; // include ALWAYS
  return MENU_TEMPO; // safe default
}

// Compact “ctrl_group_id_t -> root ctrl group”
static inline ctrl_group_id_t root_for_ctrl_id(uint32_t id) {
  if (id == CTRL_TEMPO_ALL)                                      return CTRL_TEMPO_ALL;
  if (id >= CTRL_MODIFY_CHANGE   && id <= CTRL_MODIFY_VEL_FIXED)  return CTRL_MODIFY_ALL;
  if (id >= CTRL_TRANSPOSE_SHIFT && id <= CTRL_TRANSPOSE_ALL)     return CTRL_TRANSPOSE_ALL;
  if (id >= CTRL_SETTINGS_GLOBAL1&& id <= CTRL_SETTINGS_ALWAYS)   return CTRL_SETTINGS_ALL; // include ALWAYS
  return CTRL_TEMPO_ALL; // safe default
}

// Collapse any subgroup to its root *_ALL group
static ctrl_group_id_t root_ctrl_group(ctrl_group_id_t g) {
    switch (g) {
        case CTRL_TEMPO_ALL:
            return CTRL_TEMPO_ALL;

        // Modify: any sub-group maps to ALL
        case CTRL_MODIFY_CHANGE:
        case CTRL_MODIFY_SPLIT:
        case CTRL_MODIFY_ALL:
        case CTRL_MODIFY_VEL_CHANGED:
        case CTRL_MODIFY_VEL_FIXED:
            return CTRL_MODIFY_ALL;

        // Transpose: any sub-group maps to ALL
        case CTRL_TRANSPOSE_SHIFT:
        case CTRL_TRANSPOSE_SCALED:
        case CTRL_TRANSPOSE_ALL:
            return CTRL_TRANSPOSE_ALL;

        case CTRL_SETTINGS_GLOBAL1:
        case CTRL_SETTINGS_GLOBAL2:
        case CTRL_SETTINGS_FILTER:
        case CTRL_SETTINGS_ABOUT:
        case CTRL_SETTINGS_ALWAYS:
        case CTRL_SETTINGS_ALL:
            return CTRL_SETTINGS_ALL;

        default:
            return CTRL_TEMPO_ALL;
    }
}

static CtrlActiveList* list_for_root(ctrl_group_id_t root) {
    switch (root) {
        case CTRL_TEMPO_ALL:       return &s_menu_lists.tempo_item_list;
        case CTRL_SETTINGS_ALL:    return &s_menu_lists.settings_item_list;
        case CTRL_TRANSPOSE_ALL:   return &s_menu_lists.transpose_item_list;
        case CTRL_MODIFY_ALL:      return &s_menu_lists.modify_item_list;
        default:                   return &s_menu_lists.tempo_item_list;
    }
}

static inline uint32_t bit(ctrl_group_id_t id) { return (1u << (id - 1)); }
static inline uint32_t flag_from_id(uint32_t id) { return (id >= 1 && id <= 31) ? (1u << (id - 1)) : 0u; }

static inline uint8_t is_bits_item(save_field_t f) {
    return menu_controls[f].handler == update_channel_filter;
}


// ==========================================================
//    Minimal generic "selector -> group" engine
// ==========================================================
typedef uint8_t (*compute_case_fn)(void);  // returns active case index

typedef struct {
    save_field_t           selector;   // SAVE_FIELD_INVALID => virtual selector
    uint8_t                cases;      // number of cases
    const ctrl_group_id_t *groups;     // [cases] maps case index -> ctrl group
    compute_case_fn        compute;    // if selector==INVALID, compute() must be set
} ctrl_switch_t;

typedef struct {
    ctrl_group_id_t        root;       // *_ALL
    const ctrl_switch_t   *switches;
    uint8_t                switch_count;
    uint8_t                include_always; // e.g. Settings wants CTRL_SETTINGS_ALWAYS
} ctrl_root_spec_t;

// ---- Compute functions for cases ----
static uint8_t mod_case_change_split(void) {
    return (save_get(MODIFY_CHANGE_OR_SPLIT) == MIDI_MODIFY_SPLIT) ? 1 : 0; // 0:CHANGE, 1:SPLIT
}
static uint8_t mod_case_vel(void) {
    return (save_get(MODIFY_VELOCITY_TYPE) == MIDI_MODIFY_FIXED_VEL) ? 1 : 0; // 0:CHANGED, 1:FIXED
}
static uint8_t transp_case_type(void) {
    return (save_get(TRANSPOSE_TRANSPOSE_TYPE) == MIDI_TRANSPOSE_SCALED) ? 1 : 0; // 0:SHIFT, 1:SCALED
}
static uint8_t settings_case_page(void) {
    uint8_t sel = s_menu_selects[MENU_SETTINGS];
    const uint8_t t0 = SETTINGS_ROWS_G1;
    const uint8_t t1 = (uint8_t)(t0 + SETTINGS_ROWS_G2);
    const uint8_t t2 = (uint8_t)(t1 + SETTINGS_ROWS_FILTER);
    if (sel < t0) return 0;         // GLOBAL1
    if (sel < t1) return 1;         // GLOBAL2
    if (sel < t2) return 2;         // FILTER
    return 3;                        // ABOUT
}

// ---- Group arrays per selector ----
static const ctrl_group_id_t kModChangeGroups[] = { CTRL_MODIFY_CHANGE,   CTRL_MODIFY_SPLIT  };
static const ctrl_group_id_t kModVelGroups[]    = { CTRL_MODIFY_VEL_CHANGED, CTRL_MODIFY_VEL_FIXED };
static const ctrl_group_id_t kTranspGroups[]    = { CTRL_TRANSPOSE_SHIFT, CTRL_TRANSPOSE_SCALED };
static const ctrl_group_id_t kSettingsGroups[]  = { CTRL_SETTINGS_GLOBAL1, CTRL_SETTINGS_GLOBAL2,
                                                    CTRL_SETTINGS_FILTER,  CTRL_SETTINGS_ABOUT };

// ---- Switch lists per root ----
static const ctrl_switch_t kSwitchesModify[] = {
    { MODIFY_CHANGE_OR_SPLIT, 2, kModChangeGroups, mod_case_change_split },
    { MODIFY_VELOCITY_TYPE,   2, kModVelGroups,    mod_case_vel          },
};
static const ctrl_switch_t kSwitchesTranspose[] = {
    { TRANSPOSE_TRANSPOSE_TYPE, 2, kTranspGroups, transp_case_type },
};
static const ctrl_switch_t kSwitchesSettings[] = {
    { SAVE_FIELD_INVALID, 4, kSettingsGroups, settings_case_page }, // virtual selector
};

// ---- Root specs ----
static const ctrl_root_spec_t kRootSpecs[] = {
    { CTRL_TEMPO_ALL,     NULL,                 0, 0 },
    { CTRL_MODIFY_ALL,    kSwitchesModify,      (uint8_t)(sizeof(kSwitchesModify)/sizeof(kSwitchesModify[0])), 0 },
    { CTRL_TRANSPOSE_ALL, kSwitchesTranspose,   (uint8_t)(sizeof(kSwitchesTranspose)/sizeof(kSwitchesTranspose[0])), 0 },
    { CTRL_SETTINGS_ALL,  kSwitchesSettings,    (uint8_t)(sizeof(kSwitchesSettings)/sizeof(kSwitchesSettings[0])), 1 }, // include ALWAYS
};

static const ctrl_root_spec_t* find_spec(ctrl_group_id_t root) {
    for (size_t i = 0; i < sizeof(kRootSpecs)/sizeof(kRootSpecs[0]); ++i)
        if (kRootSpecs[i].root == root) return &kRootSpecs[i];
    return NULL;
}

static uint32_t ctrl_active_mask_for_root(ctrl_group_id_t root)
{
    uint32_t mask = bit(root);

    const ctrl_root_spec_t *spec = find_spec(root);
    if (!spec) return mask;

    if (spec->include_always) mask |= bit(CTRL_SETTINGS_ALWAYS);

    for (uint8_t i = 0; i < spec->switch_count; ++i) {
        const ctrl_switch_t *sw = &spec->switches[i];
        uint8_t idx = sw->compute ? sw->compute() : 0;
        if (idx >= sw->cases) idx = 0;
        mask |= bit(sw->groups[idx]);
    }
    return mask;
}


// -------------------------
// Build the active fields list (ABOUT is real row; inert rows skipped)
// -------------------------
static void ctrl_build_active_fields(uint32_t active_groups, CtrlActiveList *out)
{
    uint8_t count = 0;

    for (uint16_t f = 0; f < SAVE_FIELD_COUNT && count < MENU_ACTIVE_LIST_CAP; ++f) {
        const menu_controls_t mt = menu_controls[f];

        // Quick reject: group not active
        const uint32_t gm = flag_from_id(mt.groups);
        if ((gm & active_groups) == 0) continue;

        // Skip inert rows
        if (mt.handler == no_update) continue;

        out->fields_idx[count++] = f;
    }

    out->count = count;
}

// Rebuild on demand. For SETTINGS, build a union list so navigation spans the whole page.
static const CtrlActiveList* get_list_for_group(ctrl_group_id_t group)
{
    const ctrl_group_id_t root = root_ctrl_group(group);

    uint32_t mask;
    if (root == CTRL_SETTINGS_ALL) {
        mask =   flag_from_id(CTRL_SETTINGS_GLOBAL1)
               | flag_from_id(CTRL_SETTINGS_GLOBAL2)
               | flag_from_id(CTRL_SETTINGS_FILTER)
               | flag_from_id(CTRL_SETTINGS_ABOUT)
               | flag_from_id(CTRL_SETTINGS_ALWAYS);
    } else {
        mask = ctrl_active_mask_for_root(root);
    }

    CtrlActiveList *dst = list_for_root(root);
    ctrl_build_active_fields(mask, dst);
    return dst;
}


// -------------------------
// Row counting (bit strips expand to 16)
// -------------------------
static uint8_t ctrl_row_count(const CtrlActiveList *list)
{
    uint8_t rows = 0;
    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];
        rows += is_bits_item(f) ? 16 : 1;
    }
    return rows;
}


// -------------------------
// Navigation/select (menu-scoped)
// -------------------------
static void menu_nav_update_select(menu_list_t field, ctrl_group_id_t group)
{
    const int8_t step = encoder_read_step(&htim3);

    // Snapshot previous selection so has_menu_changed() can compare even when step==0.
    uint8_t sel = (field < AMOUNT_OF_MENUS) ? s_menu_selects[field] : 0;
    if (field < AMOUNT_OF_MENUS) s_prev_selects[field] = sel;

    if (step == 0) return;

    const CtrlActiveList* list = get_list_for_group(group);
    uint8_t rows = ctrl_row_count(list);

    if (rows == 0) { if (field < AMOUNT_OF_MENUS) s_menu_selects[field] = 0; return; }
    if (sel >= rows) sel = (uint8_t)(rows - 1);

    // Wrap without slow modulo on negatives
    int16_t v = (int16_t)sel + (int16_t)step;
    while (v < 0)       v += rows;
    while (v >= rows)   v -= rows;

    if (field < AMOUNT_OF_MENUS) s_menu_selects[field] = (uint8_t)v;
}

uint8_t menu_nav_get_select(menu_list_t field) {
    return (field < AMOUNT_OF_MENUS) ? s_menu_selects[field] : 0;
}

uint32_t ui_active_groups(void) {
    uint8_t m = ui_state_get(CURRENT_MENU);
    if (m == 0xFF || m >= AMOUNT_OF_MENUS) m = MENU_SETTINGS;
    const ctrl_group_id_t root = kMenuToRoot[m];
    return ctrl_active_mask_for_root(root);
}


// -------------------------
// Begin + update select
// -------------------------
void menu_nav_begin_and_update(menu_list_t field) {
    if (field >= AMOUNT_OF_MENUS) return;
    const ctrl_group_id_t g = kMenuToRoot[field];
    // No explicit caching; lists are rebuilt on demand
    menu_nav_update_select(field, g);
}


// -------------------------
// Selection resolution (field/bit for current row)
// -------------------------
typedef struct {
    ctrl_group_id_t root;
    uint8_t      row;       // selected row index on the page
    save_field_t field;     // selected SAVE field (or INVALID)
    uint8_t      is_bits;   // 1 if this row is a bit of a 16-bit strip
    uint8_t      bit;       // which bit (0..15) if is_bits=1, else 0xFF
    uint32_t     gid;       // menu_controls[field].groups (0 if INVALID)
} NavSel;

static inline NavSel nav_selection(menu_list_t sel_field)
{
    NavSel s = (NavSel){0};
    if (sel_field >= AMOUNT_OF_MENUS) return s;

    s.root  = kMenuToRoot[sel_field];
    s.row   = menu_nav_get_select(sel_field);
    s.bit   = 0xFF;
    s.field = SAVE_FIELD_INVALID;

    const CtrlActiveList *list = get_list_for_group(s.root);
    uint8_t row_cursor = 0;

    for (uint8_t i = 0; i < list->count; ++i) {
        const save_field_t f = (save_field_t)list->fields_idx[i];
        const uint8_t span  = is_bits_item(f) ? 16u : 1u;

        if (s.row < (uint8_t)(row_cursor + span)) {
            s.field   = f;
            s.is_bits = (span == 16u);
            s.bit     = s.is_bits ? (uint8_t)(s.row - row_cursor) : 0xFF;
            s.gid     = menu_controls[f].groups;
            return s;
        }
        row_cursor = (uint8_t)(row_cursor + span);
    }
    return s;
}


// -------------------------
// Unified selector cycling (Modify/Transpose only)
// -------------------------
static save_field_t selector_to_cycle_for_root(ctrl_group_id_t root, uint32_t gid)
{
    const ctrl_root_spec_t *spec = find_spec(root);
    if (!spec) return SAVE_FIELD_INVALID;

    for (uint8_t i = 0; i < spec->switch_count; ++i) {
        const ctrl_switch_t *sw = &spec->switches[i];
        if (sw->selector == SAVE_FIELD_INVALID) continue; // virtual selector: not cyclable
        for (uint8_t k = 0; k < sw->cases; ++k) {
            if (sw->groups[k] == gid) return sw->selector;
        }
    }
    return SAVE_FIELD_INVALID;
}

void select_press_menu_change(menu_list_t sel_field) {
    const NavSel s   = nav_selection(sel_field);
    if (s.field == SAVE_FIELD_INVALID) return;

    // Only Modify/Transpose have cyclable selectors; Settings/Tempo are inert on press
    if (s.root == CTRL_SETTINGS_ALL || s.root == CTRL_TEMPO_ALL) return;

    const save_field_t tgt = selector_to_cycle_for_root(s.root, s.gid);
    if (tgt == SAVE_FIELD_INVALID) return;

    save_modify_u8(tgt, SAVE_MODIFY_INCREMENT, 0);
    if (sel_field < AMOUNT_OF_MENUS) s_menu_selects[sel_field] = 0;
    // No rebuild needed; next calls will rebuild on demand
}


// -------------------------
// Apply/toggle selected row
// -------------------------
static inline void nav_apply_selection(const NavSel *s)
{
    if (s->field == SAVE_FIELD_INVALID) return;

    if (s->is_bits) {
        update_channel_filter(s->field, s->bit);
        return;
    }

    const menu_controls_t mt = menu_controls[s->field];
    if (mt.handler) mt.handler(s->field, mt.handler_arg);
}

static inline void toggle_selected_row(menu_list_t sel_field)
{
    const NavSel s = nav_selection(sel_field);
    nav_apply_selection(&s);
}


// -------------------------
// Selection queries
// -------------------------
int8_t ui_selected_bit(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return -1;
    const ctrl_group_id_t  root = root_for_ctrl_id(menu_controls[f].groups);
    const menu_list_t page = page_for_ctrl_id(root);
    const NavSel s = nav_selection(page);
    if (s.field == f && s.is_bits) return (int8_t)s.bit;
    return -1;
}

uint8_t ui_is_field_selected(save_field_t f)
{
    if ((unsigned)f >= SAVE_FIELD_COUNT) return 0;
    const ctrl_group_id_t  root = root_for_ctrl_id(menu_controls[f].groups);
    const menu_list_t page = page_for_ctrl_id(root);
    const NavSel s = nav_selection(page);
    return (s.field == f) ? 1u : 0u;
}


// -------------------------
// Change-bit tracking
// -------------------------
static inline uint8_t test_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return 0;
    return (s_field_change_bits[f >> 5] >> (f & 31)) & 1u;
}

static inline void clear_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return;
    s_field_change_bits[f >> 5] &= ~(1u << (f & 31));
}

static inline uint8_t any_field_changed(void)
{
    for (int i = 0; i < CHANGE_BITS_WORDS; ++i)
        if (s_field_change_bits[i]) return 1;
    return 0;
}

static inline void clear_all_field_changed(void)
{
    for (int i = 0; i < CHANGE_BITS_WORDS; ++i)
        s_field_change_bits[i] = 0;
}

void save_mark_all_changed(void) {
    for (int i = 0; i < CHANGE_BITS_WORDS; ++i) s_field_change_bits[i] = 0xFFFFFFFFu;
}

static uint8_t has_menu_changed(menu_list_t field, uint8_t current_select)
{
    const uint8_t old_select  = (field < AMOUNT_OF_MENUS) ? s_prev_selects[field] : 0;
    const uint8_t sel_changed = (field < AMOUNT_OF_MENUS) && (old_select != current_select);
    const uint8_t data_changed = any_field_changed();

    if (field < AMOUNT_OF_MENUS) {
        s_menu_selects[field] = current_select;
    }

    if (data_changed) clear_all_field_changed();

    return (uint8_t)(sel_changed | data_changed);
}


// -------------------------
// End-of-frame
// -------------------------
static uint8_t menu_nav_end_auto(menu_list_t field)
{
    toggle_selected_row(field);

    const uint8_t sel = menu_nav_get_select(field);
    const uint8_t changed = has_menu_changed(field, sel);

    if (changed) threads_display_notify(flag_for_menu(field));
    return changed;
}


// -------------------------
// Update flow per menu
// -------------------------
void update_menu(menu_list_t menu)
{
  if (menu >= AMOUNT_OF_MENUS) menu = MENU_SETTINGS;
  const menu_list_t field = menu;

  menu_nav_begin_and_update(field);
  cont_update_menu(field);
  (void)menu_nav_end_auto(field);
}
