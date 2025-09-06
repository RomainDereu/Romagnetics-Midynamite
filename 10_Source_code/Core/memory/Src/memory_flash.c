/*
 * memory_main.c
 *
 *  Created on: Sep 05, 2025
 *      Author: Romain Dereu
 */

#include <string.h>

#include "memory_main.h"
#include "memory_ui_state.h"

#include "stm32f4xx_hal.h"
#include "utils.h"


// ---------------------
// MIDI Tempo Data
// ---------------------
typedef struct {
    int32_t current_tempo;
    int32_t tempo_click_rate;
    uint8_t currently_sending;
    uint8_t send_to_midi_out;
} midi_tempo_data_struct;

// ---------------------
// MIDI Modify Data
// ---------------------
typedef struct {
    uint8_t change_or_split;
    uint8_t velocity_type;
    uint8_t send_to_midi_out;
    uint8_t send_to_midi_channel_1;
    uint8_t send_to_midi_channel_2;
    uint8_t split_note;
    uint8_t split_midi_channel_1;
    uint8_t split_midi_channel_2;
    int32_t velocity_plus_minus;
    uint8_t velocity_absolute;
    uint8_t currently_sending;
} midi_modify_data_struct;

// ---------------------
// MIDI Transpose Data
// ---------------------
typedef struct {
    uint8_t transpose_type;
    int32_t midi_shift_value;
    uint8_t send_original;
    uint8_t transpose_base_note;
    uint8_t transpose_interval;
    uint8_t transpose_scale;
    uint8_t currently_sending;
} midi_transpose_data_struct;

// ---------------------
// Settings Data
// ---------------------
typedef struct {
    uint8_t  start_menu;
    uint8_t  send_to_usb;
    uint8_t  brightness;
    uint8_t  channel_filter;
    uint8_t  midi_thru;
    uint8_t  usb_thru;
    int32_t filtered_channels;
} settings_data_struct;




typedef struct {
    midi_tempo_data_struct     midi_tempo_data;
    midi_modify_data_struct    midi_modify_data;
    midi_transpose_data_struct midi_transpose_data;
    settings_data_struct       settings_data;
    uint32_t                   check_data_validity;
} save_struct;



static save_struct save_data;


// ---------------------
// Field pointers
// ---------------------

int32_t* u32_fields[SAVE_FIELD_COUNT] = {0};
uint8_t*  u8_fields[SAVE_FIELD_COUNT] = {0};


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
// Load / store from flash
// ---------------------
HAL_StatusTypeDef store_settings(void)
{
    save_struct local = save_data;
    local.check_data_validity = DATA_VALIDITY_CHECKSUM;

    HAL_StatusTypeDef status;
    uint32_t error_status = 0;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
                           FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    FLASH_EraseInitTypeDef flash_erase_struct = {0};
    flash_erase_struct.TypeErase    = FLASH_TYPEERASE_SECTORS;
    flash_erase_struct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    flash_erase_struct.Sector       = FLASH_SECTOR_7;
    flash_erase_struct.NbSectors    = 1;

    status = HAL_FLASHEx_Erase(&flash_erase_struct, &error_status);
    if (status != HAL_OK) { HAL_FLASH_Lock(); return status; }

    const uint32_t *p = (const uint32_t*)&local;
    uint32_t words = (sizeof(save_struct) + 3u) / 4u;

    for (uint32_t i = 0; i < words; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                   FLASH_SECTOR7_ADDR + i * 4u,
                                   p[i]);
        if (status != HAL_OK) { HAL_FLASH_Lock(); return status; }
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}


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





static save_struct make_default_settings(void) {
    save_struct s;
    memset(&s, 0, sizeof(s));
    for (int f = 0; f < SAVE_FIELD_COUNT; ++f) {
        save_set_field_default(&s, (save_field_t)f);
    }
    return s;
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
    save_mark_all_changed();
}






#ifdef UNIT_TEST
void memory_init_defaults(void) {
    save_data = make_default_settings();
    save_init_field_pointers();
    save_mark_all_changed();
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


