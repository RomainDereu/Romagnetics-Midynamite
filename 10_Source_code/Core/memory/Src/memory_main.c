/*
 * memory_main.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Romain Dereu
 */

#include "memory_main.h"
#include <string.h>
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

// ---------------------
// Field pointers
// ---------------------
static int32_t* u32_fields[SAVE_FIELD_COUNT] = {0};
static uint8_t*  u8_fields[SAVE_FIELD_COUNT] = {0};

// ---------------------
// Limits table
// ---------------------
typedef struct { int32_t min; int32_t max; uint8_t wrap; } save_field_limits_t;

static const save_field_limits_t save_limits[SAVE_FIELD_COUNT] = {
    [SAVE_MIDI_TEMPO_CURRENT]        = {20,300,0},
    [SAVE_MIDI_TEMPO_CLICK_RATE]     = {1,1000,0},
    [SAVE_MIDI_TEMPO_CURRENTLY_SENDING] = {0,1,0},
    [SAVE_MIDI_TEMPO_SEND_TO_OUT]    = {0,1,0},

    [SAVE_MIDI_MODIFY_CHANGE_OR_SPLIT] = {0,1,0},
    [SAVE_MIDI_MODIFY_VELOCITY_TYPE]   = {0,2,0},
    [SAVE_MIDI_MODIFY_SEND_TO_OUT]     = {0,1,0},
    [SAVE_MIDI_MODIFY_CHANNEL_1]       = {0,15,0},
    [SAVE_MIDI_MODIFY_CHANNEL_2]       = {0,15,0},
    [SAVE_MIDI_MODIFY_SPLIT_NOTE]      = {0,127,0},
    [SAVE_MIDI_MODIFY_SPLIT_CH1]       = {0,15,0},
    [SAVE_MIDI_MODIFY_SPLIT_CH2]       = {0,15,0},
    [SAVE_MIDI_MODIFY_VELOCITY_PLUS_MINUS] = {-127,127,0},
    [SAVE_MIDI_MODIFY_VELOCITY_ABS]    = {0,127,0},
    [SAVE_MIDI_MODIFY_CURRENTLY_SENDING]= {0,1,0},

    [SAVE_TRANSPOSE_TYPE]              = {0,3,0},
    [SAVE_TRANSPOSE_SHIFT_VALUE]       = {-127,127,0},
    [SAVE_TRANSPOSE_SEND_ORIGINAL]     = {0,1,0},
    [SAVE_TRANSPOSE_BASE_NOTE]         = {0,127,0},
    [SAVE_TRANSPOSE_INTERVAL]          = {0,11,1},
    [SAVE_TRANSPOSE_SCALE]             = {0,11,1},
    [SAVE_TRANSPOSE_CURRENTLY_SENDING] = {0,1,0},

    [SAVE_SETTINGS_START_MENU]         = {0,3,0},
    [SAVE_SETTINGS_SEND_USB]           = {0,1,0},
    [SAVE_SETTINGS_BRIGHTNESS]         = {0,100,0},
    [SAVE_SETTINGS_CHANNEL_FILTER]     = {0,15,0},
    [SAVE_SETTINGS_MIDI_THRU]          = {0,1,0},
    [SAVE_SETTINGS_USB_THRU]           = {0,1,0},
    [SAVE_SETTINGS_FILTERED_CHANNELS]  = {0, (int32_t)0xFFFF, 0},

    [SAVE_DATA_VALIDITY]               = {0,0xFFFFFFFF,0}
};

// ---------------------
// Initialize pointer arrays
// ---------------------
static void save_init_field_pointers(void) {
    u32_fields[SAVE_MIDI_TEMPO_CURRENT]        = &save_data.midi_tempo_data.current_tempo;
    u32_fields[SAVE_MIDI_TEMPO_CLICK_RATE]     = &save_data.midi_tempo_data.tempo_click_rate;
    u32_fields[SAVE_MIDI_MODIFY_VELOCITY_PLUS_MINUS] = &save_data.midi_modify_data.velocity_plus_minus;
    u32_fields[SAVE_TRANSPOSE_SHIFT_VALUE]     = &save_data.midi_transpose_data.midi_shift_value;
    u32_fields[SAVE_SETTINGS_FILTERED_CHANNELS] = (int32_t*)&save_data.settings_data.filtered_channels;
    u32_fields[SAVE_DATA_VALIDITY]             = (int32_t*)&save_data.check_data_validity;

    u8_fields[SAVE_MIDI_TEMPO_CURRENTLY_SENDING] = &save_data.midi_tempo_data.currently_sending;
    u8_fields[SAVE_MIDI_TEMPO_SEND_TO_OUT]       = &save_data.midi_tempo_data.send_to_midi_out;

    u8_fields[SAVE_MIDI_MODIFY_CHANGE_OR_SPLIT] = &save_data.midi_modify_data.change_or_split;
    u8_fields[SAVE_MIDI_MODIFY_VELOCITY_TYPE]   = &save_data.midi_modify_data.velocity_type;
    u8_fields[SAVE_MIDI_MODIFY_SEND_TO_OUT]     = &save_data.midi_modify_data.send_to_midi_out;
    u8_fields[SAVE_MIDI_MODIFY_CHANNEL_1]       = &save_data.midi_modify_data.send_to_midi_channel_1;
    u8_fields[SAVE_MIDI_MODIFY_CHANNEL_2]       = &save_data.midi_modify_data.send_to_midi_channel_2;
    u8_fields[SAVE_MIDI_MODIFY_SPLIT_NOTE]      = &save_data.midi_modify_data.split_note;
    u8_fields[SAVE_MIDI_MODIFY_SPLIT_CH1]       = &save_data.midi_modify_data.split_midi_channel_1;
    u8_fields[SAVE_MIDI_MODIFY_SPLIT_CH2]       = &save_data.midi_modify_data.split_midi_channel_2;
    u8_fields[SAVE_MIDI_MODIFY_VELOCITY_ABS]    = &save_data.midi_modify_data.velocity_absolute;
    u8_fields[SAVE_MIDI_MODIFY_CURRENTLY_SENDING]= &save_data.midi_modify_data.currently_sending;

    u8_fields[SAVE_TRANSPOSE_TYPE]             = &save_data.midi_transpose_data.transpose_type;
    u8_fields[SAVE_TRANSPOSE_SEND_ORIGINAL]   = &save_data.midi_transpose_data.send_original;
    u8_fields[SAVE_TRANSPOSE_BASE_NOTE]       = &save_data.midi_transpose_data.transpose_base_note;
    u8_fields[SAVE_TRANSPOSE_INTERVAL]        = &save_data.midi_transpose_data.transpose_interval;
    u8_fields[SAVE_TRANSPOSE_SCALE]           = &save_data.midi_transpose_data.transpose_scale;
    u8_fields[SAVE_TRANSPOSE_CURRENTLY_SENDING]= &save_data.midi_transpose_data.currently_sending;

    u8_fields[SAVE_SETTINGS_START_MENU]       = &save_data.settings_data.start_menu;
    u8_fields[SAVE_SETTINGS_SEND_USB]         = &save_data.settings_data.send_to_usb;
    u8_fields[SAVE_SETTINGS_BRIGHTNESS]       = &save_data.settings_data.brightness;
    u8_fields[SAVE_SETTINGS_CHANNEL_FILTER]   = &save_data.settings_data.channel_filter;
    u8_fields[SAVE_SETTINGS_MIDI_THRU]        = &save_data.settings_data.midi_thru;
    u8_fields[SAVE_SETTINGS_USB_THRU]         = &save_data.settings_data.usb_thru;
}

// ---------------------
// Generic getters
// ---------------------
uint32_t save_get_u32(save_field_t field) {
    if (!save_try_lock()) return SAVE_STATE_BUSY;
    uint32_t val = 0;
    if (u32_fields[field]) val = (uint32_t)(*u32_fields[field]);
    save_unlock();
    return val;
}

uint8_t save_get_u8(save_field_t field) {
    if (!save_try_lock()) return SAVE_STATE_BUSY;
    uint8_t val = 0;
    if (u8_fields[field]) val = *u8_fields[field];
    save_unlock();
    return val;
}

// ---------------------
// Setters with boundaries
// ---------------------
static uint8_t save_set_u32(save_field_t field, uint32_t value) {
    if (!save_try_lock()) return 0;
    int32_t val_signed = (int32_t)value;
    if (val_signed < save_limits[field].min) val_signed = save_limits[field].min;
    if (val_signed > save_limits[field].max) {
        if (save_limits[field].wrap) val_signed = save_limits[field].min;
        else val_signed = save_limits[field].max;
    }
    if (u32_fields[field]) *u32_fields[field] = val_signed;
    save_unlock();
    return 1;
}

static uint8_t save_set_u8(save_field_t field, uint8_t value) {
    if (!save_try_lock()) return 0;
    if (value < save_limits[field].min) value = save_limits[field].min;
    if (value > save_limits[field].max) {
        if (save_limits[field].wrap) value = save_limits[field].min;
        else value = save_limits[field].max;
    }
    if (u8_fields[field]) *u8_fields[field] = value;
    save_unlock();
    return 1;
}

// ---------------------
// Increment / set
// ---------------------
uint8_t save_modify_u32(save_field_t field, save_modify_op_t op, uint32_t value_if_set) {
    switch(op) {
        case SAVE_MODIFY_INCREMENT: return save_set_u32(field, save_get_u32(field)+1);
        case SAVE_MODIFY_SET:       return save_set_u32(field, value_if_set);
        default: return 0;
    }
}

uint8_t save_modify_u8(save_field_t field, save_modify_op_t op, uint8_t value_if_set) {
    switch(op) {
        case SAVE_MODIFY_INCREMENT: return save_set_u8(field, save_get_u8(field)+1);
        case SAVE_MODIFY_SET:       return save_set_u8(field, value_if_set);
        default: return 0;
    }
}

// ---------------------
// Load / store from flash
// ---------------------
HAL_StatusTypeDef store_settings(save_struct *data) {
    HAL_StatusTypeDef status;
    uint32_t error_status = 0;

    // Make sure data pointer is valid
    if (!data) return HAL_ERROR;

    // Unlock flash
    HAL_FLASH_Unlock();

    // Clear flash flags
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
                           FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    // Prepare sector erase
    FLASH_EraseInitTypeDef flash_erase_struct = {0};
    flash_erase_struct.TypeErase = FLASH_TYPEERASE_SECTORS;
    flash_erase_struct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    flash_erase_struct.Sector = FLASH_SECTOR_7;
    flash_erase_struct.NbSectors = 1;

    // Erase sector
    status = HAL_FLASHEx_Erase(&flash_erase_struct, &error_status);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return status;
    }

    // Program flash, 32-bit word by word
    uint32_t *data_ptr = (uint32_t*)data;
    uint32_t words = (sizeof(save_struct) + 3) / 4; // round up

    for (uint32_t i = 0; i < words; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                   FLASH_SECTOR7_ADDR + i * 4,
                                   data_ptr[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return status;
        }
    }

    // Lock flash
    HAL_FLASH_Lock();
    return HAL_OK;
}


static save_struct* read_setting_memory(void) {
    return (save_struct*)FLASH_SECTOR7_ADDR;
}


void save_load_from_flash(void) {
    // Get pointer directly to flash memory
    save_struct* flash_ptr = read_setting_memory();

    // Copy from flash to RAM if checksum is valid, else load defaults
    if (flash_ptr->check_data_validity == DATA_VALIDITY_CHECKSUM) {
        save_data = *flash_ptr;  // copy flash contents to RAM
    } else {
        save_data = make_default_settings();
    }

    // Initialize field pointers for runtime access
    save_init_field_pointers();
}

// ---------------------
// Original helpers
// ---------------------
save_struct make_default_settings(void) {
    save_struct s = {0};

    // ---------------------
    // MIDI Tempo Data
    // ---------------------
    s.midi_tempo_data.current_tempo = 120;
    s.midi_tempo_data.tempo_click_rate = 24;
    s.midi_tempo_data.currently_sending = 0;
    s.midi_tempo_data.send_to_midi_out = 0;

    // ---------------------
    // MIDI Modify Data
    // ---------------------
    s.midi_modify_data.change_or_split = 0;
    s.midi_modify_data.velocity_type = 0;
    s.midi_modify_data.send_to_midi_out = 0;
    s.midi_modify_data.send_to_midi_channel_1 = 0;
    s.midi_modify_data.send_to_midi_channel_2 = 0;
    s.midi_modify_data.split_note = 60;
    s.midi_modify_data.split_midi_channel_1 = 0;
    s.midi_modify_data.split_midi_channel_2 = 0;
    s.midi_modify_data.velocity_plus_minus = 0;
    s.midi_modify_data.velocity_absolute = 64;
    s.midi_modify_data.currently_sending = 0;

    // ---------------------
    // MIDI Transpose Data
    // ---------------------
    s.midi_transpose_data.transpose_type = 0;
    s.midi_transpose_data.midi_shift_value = 0;
    s.midi_transpose_data.send_original = 0;
    s.midi_transpose_data.transpose_base_note = 60;
    s.midi_transpose_data.transpose_interval = 0;
    s.midi_transpose_data.transpose_scale = 0;
    s.midi_transpose_data.currently_sending = 0;

    // ---------------------
    // Settings Data
    // ---------------------
    s.settings_data.start_menu = 0;
    s.settings_data.send_to_usb = 0;
    s.settings_data.brightness = 100;
    s.settings_data.channel_filter = 0;
    s.settings_data.midi_thru = 0;
    s.settings_data.usb_thru = 0;
    s.settings_data.filtered_channels = 0xFFFF;


    s.check_data_validity = DATA_VALIDITY_CHECKSUM;

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
