/*
 * _menu_controller.c
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 *
 *  Data-driven: logic is menu/group-agnostic.
 */
#include "cmsis_os.h" //For osDelay
#include "main.h"
#include "_menu_controller.h"
#include "_menu_ui.h"
#include "menus.h"
#include "memory_main.h"
#include "stm32f4xx_hal.h"   // HAL types (TIM, GPIO)
#include "threads.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

static uint8_t s_menu_selects[AMOUNT_OF_MENUS] = {0};
static uint8_t s_prev_selects[AMOUNT_OF_MENUS] = {0};

uint32_t s_field_change_bits[CHANGE_BITS_WORDS] = {0};


// -------------------------
// Encoder & value helpers (logic-agnostic)
// -------------------------
static int8_t encoder_read_step(TIM_HandleTypeDef *timer) {
    int32_t delta = __HAL_TIM_GET_COUNTER(timer) - ENCODER_CENTER;
    if (delta <= -ENCODER_THRESHOLD) { __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER); return -1; }
    if (delta >=  ENCODER_THRESHOLD) { __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER); return +1; }
    return 0; // no step
}

static void no_update(save_field_t field, uint8_t arg) { (void)field; (void)arg; }
static void shadow_select(save_field_t field, uint8_t arg) { (void)field; (void)arg; }

static void update_value(save_field_t field, uint8_t multiplier)
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
    int32_t cur = (int32_t)save_get(field);
    int32_t next = cur + delta;

    if (u32_fields[field]) { (void)save_modify_u32(field, SAVE_MODIFY_SET, (uint32_t)next); }
    else                  { (void)save_modify_u8 (field, SAVE_MODIFY_SET, (uint8_t) next); }
}

void update_contrast(save_field_t f, uint8_t step) {
    update_value(f, step);
    update_contrast_ui();
}

static void update_channel_filter(save_field_t field, uint8_t bit_index)
{
    if (bit_index > 15) return;
    int8_t step = encoder_read_step(&htim4);
    if (step == 0) return;
    uint32_t mask = (uint32_t)save_get(field);
    mask ^= (1UL << bit_index);
    (void)save_modify_u32(field, SAVE_MODIFY_SET, mask);
}


// -------------------------
// Controller table (DATA)
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
// Root/page mapping (DATA)
// -------------------------
static const ctrl_group_id_t kRoots[] = {
    CTRL_TEMPO_ALL, CTRL_MODIFY_ALL, CTRL_TRANSPOSE_ALL, CTRL_SETTINGS_ALL
};

static const menu_list_t kRootToMenu[] = {
    MENU_TEMPO, MENU_MODIFY, MENU_TRANSPOSE, MENU_SETTINGS
};


// -------------------------
// Utility (pure logic, data-agnostic)
// -------------------------
static inline uint32_t bit(ctrl_group_id_t id) { return (1u << (id - 1)); }
static inline uint32_t flag_from_id(uint32_t id) { return (id >= 1 && id <= 31) ? (1u << (id - 1)) : 0u; }

static inline uint8_t is_bits_item(save_field_t f) {
    return menu_controls[f].handler == update_channel_filter;
}

static int8_t root_index(ctrl_group_id_t root) {
    for (int8_t i = 0; i < (int)(sizeof kRoots / sizeof kRoots[0]); ++i)
        if (kRoots[i] == root) return i;
    return -1;
}


// ==========================================================
//   Selectors: save-based vs position-based (DATA + logic)
//   (Defined BEFORE any code that uses them.)
// ==========================================================
typedef uint8_t (*selector_compute_fn_t)(void);

typedef enum {
    SEL_SAVE_BASED = 0,
    SEL_POSITION_BASED
} selector_kind_t;

typedef struct selector_def_s {
    selector_kind_t   kind;
    ctrl_group_id_t   root;         // contributes to this *_ALL
    uint8_t           cases;        // number of subgroup options
    const ctrl_group_id_t *groups;  // array[cases]
    // save-based:
    save_field_t      field;        // SAVE_FIELD_INVALID if none
    selector_compute_fn_t compute;  // optional override for index
    uint8_t           cycle_on_press; // 1 => cyclable on press
    // position-based:
    uint8_t           page;         // menu_list_t index whose row drives selection
} selector_def_t;

// --- generic helpers for selectors ---
static uint8_t idx_from_save(save_field_t f, uint8_t cases) {
    int32_t v = (f != SAVE_FIELD_INVALID) ? save_get(f) : 0;
    if (v < 0) v = 0;
    uint8_t idx = (uint8_t)v;
    return (idx < cases) ? idx : 0;
}

// Build union list of all fields that belong to any group in `groups[]`.
static void build_union_for_groups(const ctrl_group_id_t *groups, uint8_t n_groups, CtrlActiveList *out) {
    uint8_t count = 0;
    for (uint16_t f = 0; f < SAVE_FIELD_COUNT && count < MENU_ACTIVE_LIST_CAP; ++f) {
        const menu_controls_t mt = menu_controls[f];
        if (mt.handler == no_update) continue;   // skip inert rows
        for (uint8_t g = 0; g < n_groups; ++g) {
            if (mt.groups == groups[g]) {
                out->fields_idx[count++] = f;
                break;
            }
        }
    }
    out->count = count;
}

static uint8_t rows_for_list(const CtrlActiveList *list) {
    uint8_t rows = 0;
    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];
        rows += is_bits_item(f) ? 16 : 1;
    }
    return rows;
}

// ----- save-based computes (data) -----
static uint8_t sel_mod_change_split(void) { return (save_get(MODIFY_CHANGE_OR_SPLIT) == MIDI_MODIFY_SPLIT) ? 1 : 0; }
static uint8_t sel_mod_vel_type(void)    { return (save_get(MODIFY_VELOCITY_TYPE)   == MIDI_MODIFY_FIXED_VEL) ? 1 : 0; }
static uint8_t sel_transpose_type(void)  { return (save_get(TRANSPOSE_TRANSPOSE_TYPE)== MIDI_TRANSPOSE_SCALED) ? 1 : 0; }
static uint8_t sel_fixed0(void)          { return 0; } // for single-case selectors

// -------------------------
// Selector table (DATA only)
// -------------------------
static const selector_def_t kSelectors[] = {
    // MODIFY
    { SEL_SAVE_BASED,     CTRL_MODIFY_ALL,    2,
      (const ctrl_group_id_t[]){ CTRL_MODIFY_CHANGE, CTRL_MODIFY_SPLIT },
      MODIFY_CHANGE_OR_SPLIT,  sel_mod_change_split, 1, /*page*/ 0 },

    { SEL_SAVE_BASED,     CTRL_MODIFY_ALL,    2,
      (const ctrl_group_id_t[]){ CTRL_MODIFY_VEL_CHANGED, CTRL_MODIFY_VEL_FIXED },
      MODIFY_VELOCITY_TYPE,    sel_mod_vel_type,     1, /*page*/ 0 },

    // TRANSPOSE
    { SEL_SAVE_BASED,     CTRL_TRANSPOSE_ALL, 2,
      (const ctrl_group_id_t[]){ CTRL_TRANSPOSE_SHIFT, CTRL_TRANSPOSE_SCALED },
      TRANSPOSE_TRANSPOSE_TYPE, sel_transpose_type,   1, /*page*/ 0 },

    // SETTINGS (ALWAYS fixed + page-position selector)
    { SEL_SAVE_BASED,     CTRL_SETTINGS_ALL,  1,
      (const ctrl_group_id_t[]){ CTRL_SETTINGS_ALWAYS },
      SAVE_FIELD_INVALID,       sel_fixed0,           0, /*page*/ 0 },

    { SEL_POSITION_BASED, CTRL_SETTINGS_ALL,  4,
      (const ctrl_group_id_t[]){ CTRL_SETTINGS_GLOBAL1, CTRL_SETTINGS_GLOBAL2, CTRL_SETTINGS_FILTER, CTRL_SETTINGS_ABOUT },
      SAVE_FIELD_INVALID,       NULL,                  0, /*page*/ MENU_SETTINGS },
};

static const size_t kSelectorsCount = sizeof(kSelectors)/sizeof(kSelectors[0]);

// helpers to find pos/selectors for a root
static const selector_def_t* first_pos_selector_for_root(ctrl_group_id_t root) {
    for (size_t i = 0; i < kSelectorsCount; ++i)
        if (kSelectors[i].root == root && kSelectors[i].kind == SEL_POSITION_BASED) return &kSelectors[i];
    return NULL;
}

// compute index for a position-based selector from its page row (page & groups come from DATA)
static uint8_t idx_from_position_selector(const selector_def_t *sel) {
    CtrlActiveList u = {0};
    build_union_for_groups(sel->groups, sel->cases, &u);

    const uint8_t sel_row = (sel->page < AMOUNT_OF_MENUS) ? s_menu_selects[sel->page] : 0;
    uint8_t cursor = 0;

    for (uint8_t i = 0; i < u.count; ++i) {
        const save_field_t f = (save_field_t)u.fields_idx[i];
        const uint8_t span  = is_bits_item(f) ? 16u : 1u;

        if (sel_row < (uint8_t)(cursor + span)) {
            const ctrl_group_id_t gid = (ctrl_group_id_t)menu_controls[f].groups;
            for (uint8_t k = 0; k < sel->cases; ++k)
                if (sel->groups[k] == gid) return k;
            return 0;
        }
        cursor = (uint8_t)(cursor + span);
    }
    return 0;
}


// -------------------------
// Roots/pages (pure logic using DATA)
// -------------------------
static ctrl_group_id_t root_for_ctrl_id(uint32_t id) {
    // If id is itself a root, return it
    for (size_t i = 0; i < sizeof(kRoots)/sizeof(kRoots[0]); ++i)
        if ((uint32_t)kRoots[i] == id) return (ctrl_group_id_t)id;

    // Otherwise derive from selector DATA
    for (size_t i = 0; i < kSelectorsCount; ++i) {
        const selector_def_t *sel = &kSelectors[i];
        for (uint8_t k = 0; k < sel->cases; ++k)
            if (sel->groups[k] == id) return sel->root;
    }
    // Fallback: first root
    return kRoots[0];
}

static menu_list_t page_for_ctrl_id(uint32_t id) {
    ctrl_group_id_t r = root_for_ctrl_id(id);
    int8_t idx = root_index(r);
    return (idx >= 0) ? kRootToMenu[idx] : kRootToMenu[0];
}

static inline ctrl_group_id_t root_for_page(menu_list_t page) {
    for (size_t i = 0; i < sizeof kRootToMenu / sizeof kRootToMenu[0]; ++i)
        if (kRootToMenu[i] == page) return kRoots[i];
    return kRoots[0];
}


// -------------------------
// Active lists cache per root (logic uses these only)
// -------------------------
typedef struct {
    CtrlActiveList tempo_item_list;
    CtrlActiveList modify_item_list;
    CtrlActiveList transpose_item_list;
    CtrlActiveList settings_item_list;
} MenuActiveLists;

static MenuActiveLists s_menu_lists;

static CtrlActiveList* list_for_root(ctrl_group_id_t root) {
    // map by index, not by names
    int8_t idx = root_index(root);
    switch (idx) {
        case 0: return &s_menu_lists.tempo_item_list;
        case 1: return &s_menu_lists.modify_item_list;
        case 2: return &s_menu_lists.transpose_item_list;
        case 3: return &s_menu_lists.settings_item_list;
        default: return &s_menu_lists.tempo_item_list;
    }
}


// -------------------------
// Active-mask from selector DATA (pure logic)
// -------------------------
static uint32_t ctrl_active_mask_for_root(ctrl_group_id_t root)
{
    uint32_t mask = bit(root);

    for (size_t i = 0; i < kSelectorsCount; ++i) {
        const selector_def_t *sel = &kSelectors[i];
        if (sel->root != root) continue;

        uint8_t idx = 0;
        if (sel->kind == SEL_SAVE_BASED) {
            idx = sel->compute ? sel->compute() : idx_from_save(sel->field, sel->cases);
        } else { // SEL_POSITION_BASED
            idx = idx_from_position_selector(sel);
        }
        if (idx >= sel->cases) idx = 0;
        mask |= bit(sel->groups[idx]);
    }
    return mask;
}


// -------------------------
// Build active list (pure logic)
// -------------------------
static void ctrl_build_active_fields(uint32_t active_groups, CtrlActiveList *out)
{
    uint8_t count = 0;
    for (uint16_t f = 0; f < SAVE_FIELD_COUNT && count < MENU_ACTIVE_LIST_CAP; ++f) {
        const menu_controls_t mt = menu_controls[f];
        const uint32_t gm = flag_from_id(mt.groups);
        if ((gm & active_groups) == 0) continue;
        if (mt.handler == no_update) continue;
        out->fields_idx[count++] = f;
    }
    out->count = count;
}

static const CtrlActiveList* get_list_for_group(ctrl_group_id_t group)
{
    const ctrl_group_id_t root = root_for_ctrl_id(group);
    const uint32_t mask = ctrl_active_mask_for_root(root);
    CtrlActiveList *dst = list_for_root(root);
    ctrl_build_active_fields(mask, dst);
    return dst;
}


// -------------------------
// Row counting (pure logic)
// -------------------------
static uint8_t ctrl_row_count(const CtrlActiveList *list)
{
    return rows_for_list(list);
}


// -------------------------
// Navigation/select (pure logic)
// -------------------------
static void menu_nav_update_select(menu_list_t page, ctrl_group_id_t any_group_of_page_root)
{
    const int8_t step = encoder_read_step(&htim3);

    uint8_t sel = (page < AMOUNT_OF_MENUS) ? s_menu_selects[page] : 0;
    if (page < AMOUNT_OF_MENUS) s_prev_selects[page] = sel;
    if (step == 0) return;

    const ctrl_group_id_t root = root_for_ctrl_id(any_group_of_page_root);

    // If this root has a position-based selector, navigation spans union of its groups
    const selector_def_t *pos_sel = first_pos_selector_for_root(root);

    uint8_t rows = 0;
    if (pos_sel) {
        CtrlActiveList u = {0};
        build_union_for_groups(pos_sel->groups, pos_sel->cases, &u);
        rows = rows_for_list(&u);
    } else {
        const CtrlActiveList* list = get_list_for_group(root);
        rows = ctrl_row_count(list);
    }

    if (rows == 0) { if (page < AMOUNT_OF_MENUS) s_menu_selects[page] = 0; return; }
    if (sel >= rows) sel = (uint8_t)(rows - 1);

    int16_t v = (int16_t)sel + (int16_t)step;
    while (v < 0)     v += rows;
    while (v >= rows) v -= rows;

    if (page < AMOUNT_OF_MENUS) s_menu_selects[page] = (uint8_t)v;
}

uint8_t menu_nav_get_select(menu_list_t page) {
    return (page < AMOUNT_OF_MENUS) ? s_menu_selects[page] : 0;
}

uint32_t ui_active_groups(void) {
    uint8_t m = ui_state_get(CURRENT_MENU);
    if (m >= AMOUNT_OF_MENUS) m = 0;
    return ctrl_active_mask_for_root(root_for_page((menu_list_t)m));
}

void menu_nav_begin_and_update(menu_list_t page) {
    if (page >= AMOUNT_OF_MENUS) return;
    const ctrl_group_id_t r = root_for_page(page);
    menu_nav_update_select(page, r);
}


// -------------------------
// Selection resolution (pure logic)
// -------------------------
typedef struct {
    ctrl_group_id_t root;
    uint8_t      row;
    save_field_t field;
    uint8_t      is_bits;
    uint8_t      bit;
    uint32_t     gid;
} NavSel;

static inline NavSel nav_selection(menu_list_t page)
{
    NavSel s = (NavSel){0};
    if (page >= AMOUNT_OF_MENUS) return s;

    const ctrl_group_id_t root = root_for_page(page);
    s.root  = root;
    s.row   = menu_nav_get_select(page);
    s.bit   = 0xFF;
    s.field = SAVE_FIELD_INVALID;

    const selector_def_t *pos_sel = first_pos_selector_for_root(root);
    if (pos_sel) {
        CtrlActiveList u = {0};
        build_union_for_groups(pos_sel->groups, pos_sel->cases, &u);

        uint8_t cursor = 0;
        for (uint8_t i = 0; i < u.count; ++i) {
            const save_field_t f = (save_field_t)u.fields_idx[i];
            const uint8_t span  = is_bits_item(f) ? 16u : 1u;

            if (s.row < (uint8_t)(cursor + span)) {
                s.field   = f;
                s.is_bits = (span == 16u);
                s.bit     = s.is_bits ? (uint8_t)(s.row - cursor) : 0xFF;
                s.gid     = menu_controls[f].groups;
                return s;
            }
            cursor = (uint8_t)(cursor + span);
        }
        return s;
    }

    // save-based roots: use active list
    const CtrlActiveList *list = get_list_for_group(root);
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
// Press-to-cycle (pure logic; data decides if/what cycles)
// -------------------------
static const selector_def_t* save_selector_owning_gid(ctrl_group_id_t root, uint32_t gid)
{
    for (size_t i = 0; i < kSelectorsCount; ++i) {
        const selector_def_t *sel = &kSelectors[i];
        if (sel->root != root) continue;
        if (sel->kind != SEL_SAVE_BASED) continue;
        for (uint8_t k = 0; k < sel->cases; ++k)
            if (sel->groups[k] == gid) return sel;
    }
    return NULL;
}

void select_press_menu_change(menu_list_t page) {
    const NavSel s = nav_selection(page);
    if (s.field == SAVE_FIELD_INVALID) return;

    const selector_def_t *owner = save_selector_owning_gid(s.root, s.gid);
    if (!owner || !owner->cycle_on_press) return;

    if (owner->field != SAVE_FIELD_INVALID) {
        save_modify_u8(owner->field, SAVE_MODIFY_INCREMENT, 0);
        if (page < AMOUNT_OF_MENUS) s_menu_selects[page] = 0;
    }
}


// -------------------------
// Apply/toggle selected row (pure logic)
// -------------------------
static inline void nav_apply_selection(const NavSel *s)
{
    if (s->field == SAVE_FIELD_INVALID) return;
    if (s->is_bits) { update_channel_filter(s->field, s->bit); return; }
    const menu_controls_t mt = menu_controls[s->field];
    if (mt.handler) mt.handler(s->field, mt.handler_arg);
}

static inline void toggle_selected_row(menu_list_t page)
{
    const NavSel s = nav_selection(page);
    nav_apply_selection(&s);
}


// -------------------------
// Selection queries (pure logic)
// -------------------------
int8_t ui_selected_bit(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return -1;
    const menu_list_t page = page_for_ctrl_id(menu_controls[f].groups);
    const NavSel s = nav_selection(page);
    return (s.field == f && s.is_bits) ? (int8_t)s.bit : -1;
}

uint8_t ui_is_field_selected(save_field_t f)
{
    if ((unsigned)f >= SAVE_FIELD_COUNT) return 0;
    const menu_list_t page = page_for_ctrl_id(menu_controls[f].groups);
    const NavSel s = nav_selection(page);
    return (s.field == f) ? 1u : 0u;
}


// -------------------------
// Change-bit tracking (unchanged)
// -------------------------
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

static uint8_t has_menu_changed(menu_list_t page, uint8_t current_select)
{
    const uint8_t old_select  = (page < AMOUNT_OF_MENUS) ? s_prev_selects[page] : 0;
    const uint8_t sel_changed = (page < AMOUNT_OF_MENUS) && (old_select != current_select);
    const uint8_t data_changed = any_field_changed();

    if (page < AMOUNT_OF_MENUS) s_menu_selects[page] = current_select;
    if (data_changed) clear_all_field_changed();

    return (uint8_t)(sel_changed | data_changed);
}


// -------------------------
// End-of-frame + update flow (pure logic)
// -------------------------
static uint8_t menu_nav_end_auto(menu_list_t page)
{
    toggle_selected_row(page);
    const uint8_t sel = menu_nav_get_select(page);
    const uint8_t changed = has_menu_changed(page, sel);
    if (changed) threads_display_notify(flag_for_menu(page));
    return changed;
}

void update_menu(menu_list_t menu_page)
{
    if (menu_page >= AMOUNT_OF_MENUS) menu_page = 0;

    // Drive nav state from data (no menu-specific logic)
    menu_nav_begin_and_update(menu_page);

    cont_update_menu(menu_page);

    (void)menu_nav_end_auto(menu_page);
}
