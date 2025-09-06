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
    [SETTINGS_ABOUT]                      = {    0,   0,      NO_WRAP,  0,    no_update      ,  1,      UI_GROUP_SETTINGS },


    [SAVE_DATA_VALIDITY]                  = {    0,   0xFFFFFFFF, NO_WRAP, DATA_VALIDITY_CHECKSUM, no_update, 0, UI_GROUP_NONE },
};


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

