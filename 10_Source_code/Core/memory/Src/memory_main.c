/*
 * memory_main.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Romain Dereu
 */

#include <string.h>

#include "memory_main.h"
#include "stm32f4xx_hal.h"
#include "utils.h"


// ---------------------
// Runtime save copy
// ---------------------
static save_struct save_data;
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
// Field pointers
// ---------------------
static int32_t* u32_fields[SAVE_FIELD_COUNT] = {0};
static uint8_t*  u8_fields[SAVE_FIELD_COUNT] = {0};



// ---------------------
// Limits table
// ---------------------


#ifdef UNIT_TEST
// Expose for tests
const menu_items_parameters_t menu_items_parameters[SAVE_FIELD_COUNT] = {
#else
// Keep private in production
static const menu_items_parameters_t menu_items_parameters[SAVE_FIELD_COUNT] = {
#endif
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
// Initialize pointer arrays
// ---------------------
static void save_init_field_pointers(void) {
    // u32 fields
    u32_fields[MIDI_TEMPO_CURRENT_TEMPO]              = &save_data.midi_tempo_data.current_tempo;
    u32_fields[MIDI_TEMPO_TEMPO_CLICK_RATE]           = &save_data.midi_tempo_data.tempo_click_rate;
    u32_fields[MIDI_MODIFY_VELOCITY_PLUS_MINUS]       = &save_data.midi_modify_data.velocity_plus_minus;
    u32_fields[MIDI_TRANSPOSE_MIDI_SHIFT_VALUE]       = &save_data.midi_transpose_data.midi_shift_value;
    u32_fields[SETTINGS_FILTERED_CHANNELS]            = &save_data.settings_data.filtered_channels;
    u32_fields[SAVE_DATA_VALIDITY]                    = (int32_t*)&save_data.check_data_validity;

    // u8 fields
    u8_fields[MIDI_TEMPO_CURRENTLY_SENDING]           = &save_data.midi_tempo_data.currently_sending;
    u8_fields[MIDI_TEMPO_SEND_TO_MIDI_OUT]            = &save_data.midi_tempo_data.send_to_midi_out;

    u8_fields[MIDI_MODIFY_CHANGE_OR_SPLIT]            = &save_data.midi_modify_data.change_or_split;
    u8_fields[MIDI_MODIFY_VELOCITY_TYPE]              = &save_data.midi_modify_data.velocity_type;
    u8_fields[MIDI_MODIFY_SEND_TO_MIDI_OUT]           = &save_data.midi_modify_data.send_to_midi_out;
    u8_fields[MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1]     = &save_data.midi_modify_data.send_to_midi_channel_1;
    u8_fields[MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2]     = &save_data.midi_modify_data.send_to_midi_channel_2;
    u8_fields[MIDI_MODIFY_SPLIT_NOTE]                 = &save_data.midi_modify_data.split_note;
    u8_fields[MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1]       = &save_data.midi_modify_data.split_midi_channel_1;
    u8_fields[MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2]       = &save_data.midi_modify_data.split_midi_channel_2;
    u8_fields[MIDI_MODIFY_VELOCITY_ABSOLUTE]          = &save_data.midi_modify_data.velocity_absolute;
    u8_fields[MIDI_MODIFY_CURRENTLY_SENDING]          = &save_data.midi_modify_data.currently_sending;

    u8_fields[MIDI_TRANSPOSE_TRANSPOSE_TYPE]          = &save_data.midi_transpose_data.transpose_type;
    u8_fields[MIDI_TRANSPOSE_BASE_NOTE]               = &save_data.midi_transpose_data.transpose_base_note;
    u8_fields[MIDI_TRANSPOSE_INTERVAL]                = &save_data.midi_transpose_data.transpose_interval;
    u8_fields[MIDI_TRANSPOSE_TRANSPOSE_SCALE]         = &save_data.midi_transpose_data.transpose_scale;
    u8_fields[MIDI_TRANSPOSE_SEND_ORIGINAL]           = &save_data.midi_transpose_data.send_original;
    u8_fields[MIDI_TRANSPOSE_CURRENTLY_SENDING]       = &save_data.midi_transpose_data.currently_sending;

    u8_fields[SETTINGS_START_MENU]                    = &save_data.settings_data.start_menu;
    u8_fields[SETTINGS_SEND_USB]                      = &save_data.settings_data.send_to_usb;
    u8_fields[SETTINGS_BRIGHTNESS]                    = &save_data.settings_data.brightness;
    u8_fields[SETTINGS_CHANNEL_FILTER]                = &save_data.settings_data.channel_filter;
    u8_fields[SETTINGS_MIDI_THRU]                     = &save_data.settings_data.midi_thru;
    u8_fields[SETTINGS_USB_THRU]                      = &save_data.settings_data.usb_thru;
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
    int32_t v = *u32_fields[field];

    switch (op) {
        case SAVE_MODIFY_INCREMENT: v += 1; break;
        case SAVE_MODIFY_SET:       v  = (int32_t)value_if_set; break;
        default: save_unlock(); return 0;
    }

    v = wrap_or_clamp_i32(v, lim.min, lim.max, lim.wrap);
    *u32_fields[field] = v;

    save_unlock();
    return 1;
}

uint8_t save_modify_u8(save_field_t field, save_modify_op_t op, uint8_t value_if_set) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    if (!u8_fields[field]) return 0;
    if (!save_lock_with_retries()) return 0;

    const menu_items_parameters_t lim = menu_items_parameters[field];
    int32_t v = (int32_t)(*u8_fields[field]);

    switch (op) {
        case SAVE_MODIFY_SET: {
            int32_t desired = (int32_t)value_if_set;

            if (!lim.wrap) {
                if (desired > lim.max && desired >= 128) {v = lim.min;}
                else if (desired < lim.min) {v = lim.min;}
                else {v = desired;}
            }
            else {v = desired;}
            break;
        }

        case SAVE_MODIFY_INCREMENT:
            v += 1;
            break;

        default:
            save_unlock();
            return 0;
    }

    v = wrap_or_clamp_i32(v, lim.min, lim.max, lim.wrap);
    *u8_fields[field] = (uint8_t)v;

    save_unlock();
    return 1;
}


// ---------------------
// Load / store from flash
// ---------------------
HAL_StatusTypeDef store_settings(save_struct *data) {
    if (!data) return HAL_ERROR;

    HAL_StatusTypeDef status;
    uint32_t error_status = 0;

    // Unlock flash
    HAL_FLASH_Unlock();

    // Clear flash flags
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
                           FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    // Erase sector 7
    FLASH_EraseInitTypeDef flash_erase_struct = {0};
    flash_erase_struct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    flash_erase_struct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    flash_erase_struct.Sector       = FLASH_SECTOR_7;
    flash_erase_struct.NbSectors    = 1;

    status = HAL_FLASHEx_Erase(&flash_erase_struct, &error_status);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return status;
    }

    // Program flash word-by-word
    const uint32_t *data_ptr = (const uint32_t*)data;
    uint32_t words = (sizeof(save_struct) + 3u) / 4u;

    for (uint32_t i = 0; i < words; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                   FLASH_SECTOR7_ADDR + i * 4u,
                                   data_ptr[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return status;
        }
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}

static save_struct* read_setting_memory(void) {
    return (save_struct*)FLASH_SECTOR7_ADDR;
}

void save_load_from_flash(void) {
    save_struct* flash_ptr = read_setting_memory();

    if (flash_ptr->check_data_validity == DATA_VALIDITY_CHECKSUM) {
        save_data = *flash_ptr;  // copy from flash
    } else {
        save_data = make_default_settings();
    }

    save_init_field_pointers();
}

// ---------------------
// Original helpers
// ---------------------


static void save_set_field_default(save_struct *s, save_field_t f) {
    int32_t d = menu_items_parameters[f].def;
    switch (f) {
        // --- midi_tempo_data ---
        case MIDI_TEMPO_CURRENT_TEMPO:            s->midi_tempo_data.current_tempo = d; break;
        case MIDI_TEMPO_TEMPO_CLICK_RATE:         s->midi_tempo_data.tempo_click_rate = d; break;
        case MIDI_TEMPO_CURRENTLY_SENDING:        s->midi_tempo_data.currently_sending = (uint8_t)d; break;
        case MIDI_TEMPO_SEND_TO_MIDI_OUT:         s->midi_tempo_data.send_to_midi_out = (uint8_t)d; break;

        // --- midi_modify_data ---
        case MIDI_MODIFY_CHANGE_OR_SPLIT:         s->midi_modify_data.change_or_split = (uint8_t)d; break;
        case MIDI_MODIFY_VELOCITY_TYPE:           s->midi_modify_data.velocity_type = (uint8_t)d; break;
        case MIDI_MODIFY_SEND_TO_MIDI_OUT:        s->midi_modify_data.send_to_midi_out = (uint8_t)d; break;
        case MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1:  s->midi_modify_data.send_to_midi_channel_1 = (uint8_t)d; break;
        case MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2:  s->midi_modify_data.send_to_midi_channel_2 = (uint8_t)d; break;
        case MIDI_MODIFY_SPLIT_NOTE:              s->midi_modify_data.split_note = (uint8_t)d; break;
        case MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1:    s->midi_modify_data.split_midi_channel_1 = (uint8_t)d; break;
        case MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2:    s->midi_modify_data.split_midi_channel_2 = (uint8_t)d; break;
        case MIDI_MODIFY_VELOCITY_PLUS_MINUS:     s->midi_modify_data.velocity_plus_minus = d; break;
        case MIDI_MODIFY_VELOCITY_ABSOLUTE:       s->midi_modify_data.velocity_absolute = (uint8_t)d; break;
        case MIDI_MODIFY_CURRENTLY_SENDING:       s->midi_modify_data.currently_sending = (uint8_t)d; break;

        // --- midi_transpose_data ---
        case MIDI_TRANSPOSE_TRANSPOSE_TYPE:       s->midi_transpose_data.transpose_type = (uint8_t)d; break;
        case MIDI_TRANSPOSE_MIDI_SHIFT_VALUE:     s->midi_transpose_data.midi_shift_value = d; break;
        case MIDI_TRANSPOSE_BASE_NOTE:            s->midi_transpose_data.transpose_base_note = (uint8_t)d; break;
        case MIDI_TRANSPOSE_INTERVAL:             s->midi_transpose_data.transpose_interval = (uint8_t)d; break;
        case MIDI_TRANSPOSE_TRANSPOSE_SCALE:      s->midi_transpose_data.transpose_scale = (uint8_t)d; break;
        case MIDI_TRANSPOSE_SEND_ORIGINAL:        s->midi_transpose_data.send_original = (uint8_t)d; break;
        case MIDI_TRANSPOSE_CURRENTLY_SENDING:    s->midi_transpose_data.currently_sending = (uint8_t)d; break;

        // --- settings_data ---
        case SETTINGS_START_MENU:                 s->settings_data.start_menu = (uint8_t)d; break;
        case SETTINGS_SEND_USB:                   s->settings_data.send_to_usb = (uint8_t)d; break;
        case SETTINGS_BRIGHTNESS:                 s->settings_data.brightness = (uint8_t)d; break;
        case SETTINGS_MIDI_THRU:                  s->settings_data.midi_thru = (uint8_t)d; break;
        case SETTINGS_USB_THRU:                   s->settings_data.usb_thru = (uint8_t)d; break;
        case SETTINGS_CHANNEL_FILTER:             s->settings_data.channel_filter = (uint8_t)d; break;
        case SETTINGS_FILTERED_CHANNELS:          s->settings_data.filtered_channels = d; break;

        // --- checksum ---
        case SAVE_DATA_VALIDITY:                  s->check_data_validity = (uint32_t)d; break;

        default: break;
    }
}



save_struct make_default_settings(void) {
    save_struct s;
    memset(&s, 0, sizeof(s));
    for (int f = 0; f < SAVE_FIELD_COUNT; ++f) {
        save_set_field_default(&s, (save_field_t)f);
    }
    return s;
}



save_struct creating_save(midi_tempo_data_struct * midi_tempo_data_to_save,
                          midi_modify_data_struct * midi_modify_data_to_save,
                          midi_transpose_data_struct * midi_transpose_data_to_save,
                          settings_data_struct *settings_data_to_save)
{
    save_struct this_save;
    this_save.midi_tempo_data      = *midi_tempo_data_to_save;
    this_save.midi_modify_data     = *midi_modify_data_to_save;
    this_save.midi_transpose_data  = *midi_transpose_data_to_save;
    this_save.settings_data        = *settings_data_to_save;
    this_save.check_data_validity  = DATA_VALIDITY_CHECKSUM;
    return this_save;
}

// ---------------------
// Snapshot helpers
// ---------------------
midi_tempo_data_struct save_snapshot_tempo(void) {
    midi_tempo_data_struct copy;
    memcpy(&copy, &save_data.midi_tempo_data, sizeof(copy));
    return copy;
}

midi_modify_data_struct save_snapshot_modify(void) {
    midi_modify_data_struct copy;
    memcpy(&copy, &save_data.midi_modify_data, sizeof(copy));
    return copy;
}

midi_transpose_data_struct save_snapshot_transpose(void) {
    midi_transpose_data_struct copy;
    memcpy(&copy, &save_data.midi_transpose_data, sizeof(copy));
    return copy;
}

settings_data_struct save_snapshot_settings(void) {
    settings_data_struct copy;
    memcpy(&copy, &save_data.settings_data, sizeof(copy));
    return copy;
}

#ifdef UNIT_TEST
void memory_init_defaults(void) {
    save_data = make_default_settings();
    save_init_field_pointers();
}

void memory_overwrite_modify(const midi_modify_data_struct *src) {
    if (!src) return;
    if (!save_lock_with_retries()) return;
    save_data.midi_modify_data = *src;
    save_unlock();
}

void memory_set_midi_thru(uint8_t v) {
    if (!save_lock_with_retries()) return;
    save_data.settings_data.midi_thru = v ? 1 : 0;
    save_unlock();
}
#endif

