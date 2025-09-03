/*
 * memory_ui_state.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */
#include "memory_ui_state.h"


typedef struct {
    uint8_t midi_tempo_current_select;
    uint8_t midi_modify_current_select;
    uint8_t midi_transpose_current_select;
    uint8_t settings_current_select;
    uint8_t current_menu;
    uint8_t old_menu;
} ui_state_t;

static ui_state_t ui_state = {0};


static int ui_state_try_lock(void) {
    if (ui_state_busy) {
        return 0; // already in use
    }
    ui_state_busy = 1;
    return 1;
}

static void ui_state_unlock(void) {
    ui_state_busy = 0;
}



uint8_t ui_state_get(ui_state_field_t field) {
    if (!ui_state_try_lock()) {
        return UI_STATE_BUSY; // busy
    }

    uint8_t value = 0;

    switch (field) {
        case UI_MIDI_TEMPO_SELECT:      value = ui_state.midi_tempo_current_select; break;
        case UI_MIDI_MODIFY_SELECT:     value = ui_state.midi_modify_current_select; break;
        case UI_MIDI_TRANSPOSE_SELECT:  value = ui_state.midi_transpose_current_select; break;
        case UI_SETTINGS_SELECT:        value = ui_state.settings_current_select; break;
        case UI_CURRENT_MENU:           value = ui_state.current_menu; break;
        case UI_OLD_MENU:               value = ui_state.old_menu; break;
    }

    ui_state_unlock();
    return value;
}



uint8_t ui_state_set(ui_state_field_t field, uint8_t value) {
    if (!ui_state_try_lock()) {
        return 0; // busy
    }

    switch (field) {
        case UI_MIDI_TEMPO_SELECT:
            ui_state.midi_tempo_current_select = value; break;
        case UI_MIDI_MODIFY_SELECT:
            ui_state.midi_modify_current_select = value; break;
        case UI_MIDI_TRANSPOSE_SELECT:
            ui_state.midi_transpose_current_select = value; break;
        case UI_SETTINGS_SELECT:
            ui_state.settings_current_select = value; break;
        case UI_CURRENT_MENU:
            ui_state.current_menu = value; break;
        case UI_OLD_MENU:
            ui_state.old_menu = value; break;
    }

    ui_state_unlock();
    return 1;
}


static uint8_t ui_state_increment(ui_state_field_t field) {
    uint8_t value = ui_state_get(field);
    if (value == UI_STATE_BUSY) {
        return UI_STATE_BUSY;
    }

    value++;

    if (value > ui_limits[field].max) {
        if (ui_limits[field].wrap) {
            value = ui_limits[field].min; // wrap
        } else {
            value = ui_limits[field].max; // clamp
        }
    }

    return ui_state_set(field, value);
}

uint8_t ui_state_modify(ui_state_field_t field, ui_modify_op_t op, uint8_t value_if_set) {
    switch(op) {
        case UI_MODIFY_INCREMENT:
            return ui_state_increment(field);
        case UI_MODIFY_SET:
            return ui_state_set(field, value_if_set);
    }
    return 0; // unknown op
}


void    ui_state_note_select_changed(void);
uint8_t ui_state_toggle_select_changed(void);

