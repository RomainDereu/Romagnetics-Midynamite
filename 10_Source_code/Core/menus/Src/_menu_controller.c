/*
 * _menu_controller.c
 *
 *  Created on: Sep 8, 2025
 *      Author: Astaa
 */

#include "_menu_controller.h"
#include "memory_main.h"
#include "utils.h"

const menu_controls_t menu_controls[SAVE_FIELD_COUNT] = {
                                             //wrap,   handler,    handler_arg, group
    [MIDI_TEMPO_CURRENT_TEMPO]           = { NO_WRAP, update_value,         10, CTRL_G_TEMPO },
    [MIDI_TEMPO_CURRENTLY_SENDING]       = {   WRAP,  no_update,             0, 0 },
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

    [MIDI_MODIFY_CURRENTLY_SENDING]      = {   WRAP,  no_update,             0, 0 },

    [MIDI_TRANSPOSE_TRANSPOSE_TYPE]      = {   WRAP,  no_update,             0, 0 }, // selector only
    [MIDI_TRANSPOSE_MIDI_SHIFT_VALUE]    = { NO_WRAP, update_value,         12, CTRL_G_TRANSPOSE_SHIFT },
    [MIDI_TRANSPOSE_BASE_NOTE]           = { NO_WRAP, update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_INTERVAL]            = { NO_WRAP, update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_TRANSPOSE_SCALE]     = {   WRAP,  update_value,          1, CTRL_G_TRANSPOSE_SCALED },
    [MIDI_TRANSPOSE_SEND_ORIGINAL]       = {   WRAP,  update_value,          1, CTRL_G_TRANSPOSE_BOTH },
    [MIDI_TRANSPOSE_CURRENTLY_SENDING]   = {   WRAP,  no_update,             0, 0 },

    [SETTINGS_START_MENU]                = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_SEND_USB]                  = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_BRIGHTNESS]                = { NO_WRAP, update_contrast,       1, CTRL_G_SETTINGS },
    [SETTINGS_MIDI_THRU]                 = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_USB_THRU]                  = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_CHANNEL_FILTER]            = {   WRAP,  update_value,          1, CTRL_G_SETTINGS },
    [SETTINGS_FILTERED_CHANNELS]         = {   WRAP,  update_channel_filter, 1, CTRL_G_SETTINGS }, // 16-bit strip
};


static inline uint8_t is_bits_item(save_field_t f) {
    return menu_controls[f].handler == update_channel_filter;
}

// ---------- Active groups based on requested UI group and selectors ----------
uint32_t ctrl_active_groups_from_ui_group(ui_group_t requested)
{
    uint32_t mask = 0;

    switch (requested) {
        case UI_GROUP_TEMPO:
            mask |= CTRL_G_TEMPO;
            break;

        case UI_GROUP_SETTINGS:
            mask |= CTRL_G_SETTINGS;
            break;

        case UI_GROUP_TRANSPOSE_SHIFT:
        case UI_GROUP_TRANSPOSE_SCALED:
        case UI_GROUP_TRANSPOSE_BOTH: {
            mask |= CTRL_G_TRANSPOSE_BOTH;
            int v = (int)save_get(MIDI_TRANSPOSE_TRANSPOSE_TYPE);
            if (v <= MIDI_TRANSPOSE_SHIFT) mask |= CTRL_G_TRANSPOSE_SHIFT;
            else                           mask |= CTRL_G_TRANSPOSE_SCALED;
        } break;

        case UI_GROUP_MODIFY_CHANGE:
        case UI_GROUP_MODIFY_SPLIT:
        case UI_GROUP_MODIFY_BOTH:
        case UI_GROUP_MODIFY_VEL_CHANGED:
        case UI_GROUP_MODIFY_VEL_FIXED: {
            mask |= CTRL_G_MODIFY_BOTH;
            int page = (int)save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
            mask |= (page == MIDI_MODIFY_SPLIT) ? CTRL_G_MODIFY_SPLIT : CTRL_G_MODIFY_CHANGE;

            int vel  = (int)save_get(MIDI_MODIFY_VELOCITY_TYPE);
            mask |= (vel == MIDI_MODIFY_FIXED_VEL) ? CTRL_G_MODIFY_VEL_FIXED : CTRL_G_MODIFY_VEL_CHANGED;
        } break;

        default:
            // Fallback to tempo to avoid empty pages
            mask |= CTRL_G_TEMPO;
            break;
    }

    return mask;
}

// ---------- Build active field list ----------
void ctrl_build_active_fields(uint32_t active_groups, CtrlActiveList *out)
{
    out->count = 0;
    for (uint16_t f = 0; f < SAVE_FIELD_COUNT && out->count < MENU_ACTIVE_LIST_CAP; ++f) {
        uint32_t gm = menu_controls[f].groups;
        if ((gm & active_groups) == 0) continue;
        out->fields_idx[out->count++] = f;
    }
}

uint8_t ctrl_row_count(const CtrlActiveList *list)
{
    uint8_t rows = 0;
    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];
        rows += is_bits_item(f) ? 16 : 1;
    }
    return rows;
}

void ctrl_toggle_row(const CtrlActiveList *list, uint8_t row_index)
{
    uint8_t row = 0;
    for (uint8_t i = 0; i < list->count; ++i) {
        save_field_t f = (save_field_t)list->fields_idx[i];
        if (is_bits_item(f)) {
            if (row_index >= row && row_index < (uint8_t)(row + 16)) {
                uint8_t bit = (uint8_t)(row_index - row);
                update_channel_filter(f, bit);
                return;
            }
            row = (uint8_t)(row + 16);
        } else {
            if (row == row_index) {
                const menu_controls_t mt = menu_controls[f];
                if (mt.handler) mt.handler(f, mt.handler_arg);
                return;
            }
            row++;
        }
    }
}
