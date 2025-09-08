/*
 * memory_main.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Romain Dereu
 */

#include <string.h>

#include "memory_main.h"
#include "memory_ui_state.h"
#include "menu_controller.h"
#include "stm32f4xx_hal.h"
#include "utils.h"



// Expose for tests
const save_limits_t save_limits[SAVE_FIELD_COUNT] = {
		                                //min,   max,  default
    [MIDI_TEMPO_CURRENT_TEMPO]         = {  20,   300, 120 },
    [MIDI_TEMPO_TEMPO_CLICK_RATE]      = {   1, 50000,  24 },
    [MIDI_TEMPO_CURRENTLY_SENDING]     = {   0,     1,   0 },
    [MIDI_TEMPO_SEND_TO_MIDI_OUT]      = {   0,     2,   0 },

    [MIDI_MODIFY_CHANGE_OR_SPLIT]      = {   0,     1,   1 },
    [MIDI_MODIFY_VELOCITY_TYPE]        = {   0,     1,   0 },

    [MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1] = { 1,   16,   1 },
    [MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2] = { 0,   16,   0 },

    [MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1] = {  1,    16,   1 },
    [MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2] = {  1,    16,   2 },
    [MIDI_MODIFY_SPLIT_NOTE]           = {  0,   127,  60 },

    [MIDI_MODIFY_SEND_TO_MIDI_OUT]     = {  0,     3,   0 },

    [MIDI_MODIFY_VELOCITY_PLUS_MINUS]  = { -127, 127,   0 },
    [MIDI_MODIFY_VELOCITY_ABSOLUTE]    = {   0,  127,  64 },

    [MIDI_MODIFY_CURRENTLY_SENDING]    = {   0,     1,   0 },

    [MIDI_TRANSPOSE_TRANSPOSE_TYPE]    = {   0,     1,   0 },
    [MIDI_TRANSPOSE_MIDI_SHIFT_VALUE]  = { -127,  127,  0 },
    [MIDI_TRANSPOSE_BASE_NOTE]         = {   0,    11,  0 },
    [MIDI_TRANSPOSE_INTERVAL]          = {   0,     9,  0 },
    [MIDI_TRANSPOSE_TRANSPOSE_SCALE]   = {   0,     6,  0 },
    [MIDI_TRANSPOSE_SEND_ORIGINAL]     = {   0,     1,  0 },
    [MIDI_TRANSPOSE_CURRENTLY_SENDING] = {   0,     1,  0 },

    [SETTINGS_START_MENU]              = {   0,     3,  0 },
    [SETTINGS_SEND_USB]                = {   0,     1,  0 },
    [SETTINGS_BRIGHTNESS]              = {   0,     9,  6 },
    [SETTINGS_MIDI_THRU]               = {   0,     1,  0 },
    [SETTINGS_USB_THRU]                = {   0,     1,  0 },
    [SETTINGS_CHANNEL_FILTER]          = {   0,     1,  0 },
    [SETTINGS_FILTERED_CHANNELS]       = {   0, 0x0000FFFF, 0 },
    [SETTINGS_ABOUT]                   = {   0,     0,  0 },

    [SAVE_DATA_VALIDITY]               = {   0, 0xFFFFFFFF, DATA_VALIDITY_CHECKSUM },
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

void save_unlock(void) { save_busy = 0; }

// (Optional) bounded retry for writers; cheap and unit-test friendly
uint8_t save_lock_with_retries(void) {
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
    const save_limits_t   lim = save_limits[field];
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
    const save_limits_t   lim = save_limits[field];
    return (uint8_t)clamp_i32(v, lim.min, lim.max);
}





// ---------------------
// Increment / set (u32 / u8)
// ---------------------
static void mark_field_changed(save_field_t f) {
    if ((unsigned)f >= SAVE_FIELD_COUNT) return;
    s_field_change_bits[f >> 5] |= (1u << (f & 31));
}



uint8_t save_toggle(save_field_t field) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    if (!u8_fields[field]) return 0;
    if (!save_lock_with_retries()) return 0;

    uint8_t old_v = *u8_fields[field];
    uint8_t new_v = (old_v == 0) ? 1 : 0;

    if (new_v != old_v) {
        *u8_fields[field] = new_v;
        mark_field_changed(field);
    }

    save_unlock();
    return new_v;
}


uint8_t save_modify_u32(save_field_t field, save_modify_op_t op, uint32_t value_if_set) {
    if (field < 0 || field >= SAVE_FIELD_COUNT) return 0;
    if (!u32_fields[field]) return 0;
    if (!save_lock_with_retries()) return 0;

    const save_limits_t   lim = save_limits[field];
    const menu_controls_t mt  = menu_controls[field];
    int32_t old_v = *u32_fields[field];
    int32_t v = old_v;

    switch (op) {
        case SAVE_MODIFY_INCREMENT: v += 1; break;
        case SAVE_MODIFY_SET:       v  = (int32_t)value_if_set; break;
        default: save_unlock(); return 0;
    }

    v = wrap_or_clamp_i32(v, lim.min, lim.max, mt.wrap);

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

    const save_limits_t   lim = save_limits[field];
    const menu_controls_t mt  = menu_controls[field];

    uint8_t old_v = *u8_fields[field];
    int32_t v = (int32_t)old_v;

    switch (op) {
        case SAVE_MODIFY_SET: {
            int32_t desired = (int32_t)value_if_set;
            if (!mt.wrap) {
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

    v = wrap_or_clamp_i32(v, lim.min, lim.max, mt.wrap);
    uint8_t new_v = (uint8_t)v;

    if (new_v != old_v) {
        *u8_fields[field] = new_v;
        mark_field_changed(field);
    }

    save_unlock();
    return 1;
}
