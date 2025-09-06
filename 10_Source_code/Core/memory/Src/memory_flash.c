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
    save_mark_all_changed();
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


