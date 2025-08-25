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
static volatile uint8_t save_busy = 0;   // single definition

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
typedef struct {
    int32_t min;
    int32_t max;
    uint8_t wrap;   // 0 = clamp, 1 = wrap
    int32_t def;    // DEFAULT value for this field
} save_field_limits_t;

static const save_field_limits_t save_limits[SAVE_FIELD_COUNT] = {
    [SAVE_MIDI_TEMPO_CURRENT]            = {  20,       300, 0, 120},
    [SAVE_MIDI_TEMPO_CLICK_RATE]         = {   1,      1000, 0,  24},
    [SAVE_MIDI_TEMPO_CURRENTLY_SENDING]  = {   0,         1, 0,   0},
    [SAVE_MIDI_TEMPO_SEND_TO_OUT]        = {   0,         1, 1,   0},

    [SAVE_MIDI_MODIFY_CHANGE_OR_SPLIT]   = {   0,         1, 1,   0},
    [SAVE_MIDI_MODIFY_VELOCITY_TYPE]     = {   0,         1, 1,   0},
    [SAVE_MIDI_MODIFY_SEND_TO_OUT]       = {   0,         3, 1,   0},
    [SAVE_MIDI_MODIFY_CHANNEL_1]         = {   0,        15, 0,   0},
    [SAVE_MIDI_MODIFY_CHANNEL_2]         = {   0,        15, 0,   0},
    [SAVE_MIDI_MODIFY_SPLIT_NOTE]        = {   0,       127, 0,  60},
    [SAVE_MIDI_MODIFY_SPLIT_CH1]         = {   0,        15, 0,   0},
    [SAVE_MIDI_MODIFY_SPLIT_CH2]         = {   0,        15, 0,   0},
    [SAVE_MIDI_MODIFY_VELOCITY_PLUS_MINUS] = {-127,      127, 0,   0},
    [SAVE_MIDI_MODIFY_VELOCITY_ABS]      = {   0,       127, 0,  64},
    [SAVE_MIDI_MODIFY_CURRENTLY_SENDING] = {   0,         1, 0,   0},

    [SAVE_TRANSPOSE_TYPE]                = {   0,         1, 1,   0},
    [SAVE_TRANSPOSE_SHIFT_VALUE]         = { -127,      127, 0,   0},
    [SAVE_TRANSPOSE_SEND_ORIGINAL]       = {   0,         1, 1,   0},
    [SAVE_TRANSPOSE_BASE_NOTE]           = {   0,        11, 0,   0},
    [SAVE_TRANSPOSE_INTERVAL]            = {   0,         9, 0,   0},
    [SAVE_TRANSPOSE_SCALE]               = {   0,         6, 1,   0},
    [SAVE_TRANSPOSE_CURRENTLY_SENDING]   = {   0,         1, 0,   0},

    [SAVE_SETTINGS_START_MENU]           = {   0,         3, 1,   0},
    [SAVE_SETTINGS_SEND_USB]             = {   0,         1, 1,   0},
    [SAVE_SETTINGS_BRIGHTNESS]           = {   0,         9, 0,   9},
    [SAVE_SETTINGS_CHANNEL_FILTER]       = {   0,        15, 1,   0},
    [SAVE_SETTINGS_MIDI_THRU]            = {   0,         1, 1,   0},
    [SAVE_SETTINGS_USB_THRU]             = {   0,         1, 1,   0},
    [SAVE_SETTINGS_FILTERED_CHANNELS]    = {   0,  0x0000FFFF, 1, 0x0000FFFF},

    [SAVE_DATA_VALIDITY]                 = {   0,  0xFFFFFFFF, 0, (int32_t)DATA_VALIDITY_CHECKSUM}
};



// ---------------------
// Initialize pointer arrays
// ---------------------
static void save_init_field_pointers(void) {
    // u32 fields
    u32_fields[SAVE_MIDI_TEMPO_CURRENT]              = &save_data.midi_tempo_data.current_tempo;
    u32_fields[SAVE_MIDI_TEMPO_CLICK_RATE]           = &save_data.midi_tempo_data.tempo_click_rate;
    u32_fields[SAVE_MIDI_MODIFY_VELOCITY_PLUS_MINUS] = &save_data.midi_modify_data.velocity_plus_minus;
    u32_fields[SAVE_TRANSPOSE_SHIFT_VALUE]           = &save_data.midi_transpose_data.midi_shift_value;
    u32_fields[SAVE_SETTINGS_FILTERED_CHANNELS]      = (int32_t*)&save_data.settings_data.filtered_channels;
    u32_fields[SAVE_DATA_VALIDITY]                   = (int32_t*)&save_data.check_data_validity;

    // u8 fields
    u8_fields[SAVE_MIDI_TEMPO_CURRENTLY_SENDING]   = &save_data.midi_tempo_data.currently_sending;
    u8_fields[SAVE_MIDI_TEMPO_SEND_TO_OUT]         = &save_data.midi_tempo_data.send_to_midi_out;

    u8_fields[SAVE_MIDI_MODIFY_CHANGE_OR_SPLIT]    = &save_data.midi_modify_data.change_or_split;
    u8_fields[SAVE_MIDI_MODIFY_VELOCITY_TYPE]      = &save_data.midi_modify_data.velocity_type;
    u8_fields[SAVE_MIDI_MODIFY_SEND_TO_OUT]        = &save_data.midi_modify_data.send_to_midi_out;
    u8_fields[SAVE_MIDI_MODIFY_CHANNEL_1]          = &save_data.midi_modify_data.send_to_midi_channel_1;
    u8_fields[SAVE_MIDI_MODIFY_CHANNEL_2]          = &save_data.midi_modify_data.send_to_midi_channel_2;
    u8_fields[SAVE_MIDI_MODIFY_SPLIT_NOTE]         = &save_data.midi_modify_data.split_note;
    u8_fields[SAVE_MIDI_MODIFY_SPLIT_CH1]          = &save_data.midi_modify_data.split_midi_channel_1;
    u8_fields[SAVE_MIDI_MODIFY_SPLIT_CH2]          = &save_data.midi_modify_data.split_midi_channel_2;
    u8_fields[SAVE_MIDI_MODIFY_VELOCITY_ABS]       = &save_data.midi_modify_data.velocity_absolute;
    u8_fields[SAVE_MIDI_MODIFY_CURRENTLY_SENDING]  = &save_data.midi_modify_data.currently_sending;

    u8_fields[SAVE_TRANSPOSE_TYPE]                 = &save_data.midi_transpose_data.transpose_type;
    u8_fields[SAVE_TRANSPOSE_SEND_ORIGINAL]        = &save_data.midi_transpose_data.send_original;
    u8_fields[SAVE_TRANSPOSE_BASE_NOTE]            = &save_data.midi_transpose_data.transpose_base_note;
    u8_fields[SAVE_TRANSPOSE_INTERVAL]             = &save_data.midi_transpose_data.transpose_interval;
    u8_fields[SAVE_TRANSPOSE_SCALE]                = &save_data.midi_transpose_data.transpose_scale;
    u8_fields[SAVE_TRANSPOSE_CURRENTLY_SENDING]    = &save_data.midi_transpose_data.currently_sending;

    u8_fields[SAVE_SETTINGS_START_MENU]            = &save_data.settings_data.start_menu;
    u8_fields[SAVE_SETTINGS_SEND_USB]              = &save_data.settings_data.send_to_usb;
    u8_fields[SAVE_SETTINGS_BRIGHTNESS]            = &save_data.settings_data.brightness;
    u8_fields[SAVE_SETTINGS_CHANNEL_FILTER]        = &save_data.settings_data.channel_filter;
    u8_fields[SAVE_SETTINGS_MIDI_THRU]             = &save_data.settings_data.midi_thru;
    u8_fields[SAVE_SETTINGS_USB_THRU]              = &save_data.settings_data.usb_thru;
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

int32_t save_get_u32(save_field_t field) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    int32_t *p = u32_fields[field];
    if (!p) return 0;

    // Try a few quick waits for ongoing write to finish
    for (int i = 0; i < 5; ++i) {
        if (!save_busy) return *p;     // fast path
        save_busy_wait_short();        // short backoff
    }

    // Best-effort read (32-bit aligned writes on Cortex-M are atomic)
    return *p;
}

uint8_t save_get(save_field_t field) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    uint8_t *p = u8_fields[field];
    if (!p) return 0;

    for (int i = 0; i < 5; ++i) {
        if (!save_busy) return *p;
        save_busy_wait_short();
    }

    // Best-effort read (8-bit writes are atomic on Cortex-M)
    return *p;
}




// ---------------------
// Increment / set (u32 / u8)
// ---------------------
uint8_t save_modify_u32(save_field_t field, save_modify_op_t op, uint32_t value_if_set) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    if (!u32_fields[field]) return 0;
    if (!save_lock_with_retries()) return 0;

    const save_field_limits_t lim = save_limits[field];
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

    const save_field_limits_t lim = save_limits[field];
    int32_t v = (int32_t)(*u8_fields[field]);

    switch (op) {
        case SAVE_MODIFY_INCREMENT: v += 1; break;
        case SAVE_MODIFY_SET:       v  = (int32_t)value_if_set; break;
        default: save_unlock(); return 0;
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
    int32_t d = save_limits[f].def;
    switch (f) {
        // --- midi_tempo_data ---
        case SAVE_MIDI_TEMPO_CURRENT:             s->midi_tempo_data.current_tempo = d; break;
        case SAVE_MIDI_TEMPO_CLICK_RATE:          s->midi_tempo_data.tempo_click_rate = d; break;
        case SAVE_MIDI_TEMPO_CURRENTLY_SENDING:   s->midi_tempo_data.currently_sending = (uint8_t)d; break;
        case SAVE_MIDI_TEMPO_SEND_TO_OUT:         s->midi_tempo_data.send_to_midi_out = (uint8_t)d; break;

        // --- midi_modify_data ---
        case SAVE_MIDI_MODIFY_CHANGE_OR_SPLIT:    s->midi_modify_data.change_or_split = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_VELOCITY_TYPE:      s->midi_modify_data.velocity_type = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_SEND_TO_OUT:        s->midi_modify_data.send_to_midi_out = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_CHANNEL_1:          s->midi_modify_data.send_to_midi_channel_1 = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_CHANNEL_2:          s->midi_modify_data.send_to_midi_channel_2 = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_SPLIT_NOTE:         s->midi_modify_data.split_note = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_SPLIT_CH1:          s->midi_modify_data.split_midi_channel_1 = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_SPLIT_CH2:          s->midi_modify_data.split_midi_channel_2 = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_VELOCITY_PLUS_MINUS:s->midi_modify_data.velocity_plus_minus = d; break;
        case SAVE_MIDI_MODIFY_VELOCITY_ABS:       s->midi_modify_data.velocity_absolute = (uint8_t)d; break;
        case SAVE_MIDI_MODIFY_CURRENTLY_SENDING:  s->midi_modify_data.currently_sending = (uint8_t)d; break;

        // --- midi_transpose_data ---
        case SAVE_TRANSPOSE_TYPE:                 s->midi_transpose_data.transpose_type = (uint8_t)d; break;
        case SAVE_TRANSPOSE_SHIFT_VALUE:          s->midi_transpose_data.midi_shift_value = d; break;
        case SAVE_TRANSPOSE_SEND_ORIGINAL:        s->midi_transpose_data.send_original = (uint8_t)d; break;
        case SAVE_TRANSPOSE_BASE_NOTE:            s->midi_transpose_data.transpose_base_note = (uint8_t)d; break;
        case SAVE_TRANSPOSE_INTERVAL:             s->midi_transpose_data.transpose_interval = (uint8_t)d; break;
        case SAVE_TRANSPOSE_SCALE:                s->midi_transpose_data.transpose_scale = (uint8_t)d; break;
        case SAVE_TRANSPOSE_CURRENTLY_SENDING:    s->midi_transpose_data.currently_sending = (uint8_t)d; break;

        // --- settings_data ---
        case SAVE_SETTINGS_START_MENU:            s->settings_data.start_menu = (uint8_t)d; break;
        case SAVE_SETTINGS_SEND_USB:              s->settings_data.send_to_usb = (uint8_t)d; break;
        case SAVE_SETTINGS_BRIGHTNESS:            s->settings_data.brightness = (uint8_t)d; break;
        case SAVE_SETTINGS_CHANNEL_FILTER:        s->settings_data.channel_filter = (uint8_t)d; break;
        case SAVE_SETTINGS_MIDI_THRU:             s->settings_data.midi_thru = (uint8_t)d; break;
        case SAVE_SETTINGS_USB_THRU:              s->settings_data.usb_thru = (uint8_t)d; break;
        case SAVE_SETTINGS_FILTERED_CHANNELS:     s->settings_data.filtered_channels = (uint16_t)d; break;

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
