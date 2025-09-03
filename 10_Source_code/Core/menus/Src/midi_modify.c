/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "memory_ui_state.h"
#include "memory_main.h"


//under_here_header_checks
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "threads.h"
#include "utils.h"

extern const Message * message;


// midi modify menu
void midi_modify_update_menu(void)
{
    midi_modify_data_struct old_modify_data = save_snapshot_modify();
    static uint8_t old_select = 0;

    uint8_t current_select = ui_state_get(UI_MIDI_MODIFY_SELECT);
    uint8_t mode = save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
    ui_group_t group = (mode == MIDI_MODIFY_CHANGE) ? UI_GROUP_MODIFY_CHANGE
                                                    : UI_GROUP_MODIFY_SPLIT;

    // Derive count from the table/rank (no hardcoded 4/5 needed elsewhere)
    uint8_t count = build_select_states(group, current_select, NULL, 0);

    update_select(&current_select, 0, count - 1, 1, WRAP);
    ui_state_modify(UI_MIDI_MODIFY_SELECT, UI_MODIFY_SET, current_select);

    // Mode toggle: last row toggles velocity type, other rows toggle change/split
    if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
        if (current_select < count - 1) {
            save_modify_u8(MIDI_MODIFY_CHANGE_OR_SPLIT, SAVE_MODIFY_INCREMENT, 0);
        } else {
            save_modify_u8(MIDI_MODIFY_VELOCITY_TYPE, SAVE_MODIFY_INCREMENT, 0);
        }
        ui_state_modify(UI_MIDI_MODIFY_SELECT, UI_MODIFY_SET, 0);

        // Recompute group if mode changed
        mode = save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
        group = (mode == MIDI_MODIFY_CHANGE) ? UI_GROUP_MODIFY_CHANGE
                                             : UI_GROUP_MODIFY_SPLIT;
    }

    // Drive the selected row via the table handler (step sizes come from menu_items_parameters)
    toggle_underline_items(group, current_select);

    midi_modify_data_struct new_modify_data = save_snapshot_modify();
    if (menu_check_for_updates(&old_modify_data,
                               &new_modify_data, sizeof new_modify_data,
                               &current_select, &old_select)) {
        threads_display_notify(FLAG_MODIFY);
    }
    old_select = current_select;
}


// -------- Display helpers (read from save_*, no struct param) --------

static void screen_update_channel_change(uint8_t * select_states){
    screen_driver_SetCursor_WriteString(message->send_1_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    const char *ch1 = message->choices.midi_channels[ save_get(MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1) ];
    screen_driver_underline_WriteString(ch1, Font_6x8 , White, 50, LINE_1_VERT, select_states[0]);

    screen_driver_SetCursor_WriteString(message->send_2_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);
    const char *ch2 = message->choices.midi_channels[ save_get(MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2) ];
    screen_driver_underline_WriteString(ch2, Font_6x8 , White, 50, LINE_2_VERT, select_states[1]);

    screen_driver_SetCursor_WriteString(message->output_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_3_VERT);
    const char *out = message->choices.midi_outs[ save_get(MIDI_MODIFY_SEND_TO_MIDI_OUT) ];
    screen_driver_underline_WriteString(out, Font_6x8 , White, 50, LINE_3_VERT, select_states[2]);
}

static void screen_update_channel_split(uint8_t * select_states){
    screen_driver_SetCursor_WriteString(message->low_sem,  Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    uint8_t low = save_get(MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1);
    static char low_txt[6];  sprintf(low_txt, "%u", low);
    screen_driver_underline_WriteString(low_txt, Font_6x8, White, 30, LINE_1_VERT, select_states[0]);

    screen_driver_SetCursor_WriteString(message->high_sem, Font_6x8 , White, 45, LINE_1_VERT);
    uint8_t high = save_get(MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2);
    static char high_txt[6]; sprintf(high_txt, "%u", high);
    screen_driver_underline_WriteString(high_txt, Font_6x8, White, 80, LINE_1_VERT, select_states[1]);

    screen_driver_SetCursor_WriteString(message->split, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
    uint8_t split_note = save_get(MIDI_MODIFY_SPLIT_NOTE);
    const char *note_to_write = message->midi_note_names[split_note];
    screen_driver_underline_WriteString(note_to_write, Font_6x8, White, 40, LINE_2_VERT, select_states[2]);

    screen_driver_SetCursor_WriteString(message->output_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_3_VERT);
    const char *out = message->choices.midi_outs[ save_get(MIDI_MODIFY_SEND_TO_MIDI_OUT) ];
    screen_driver_underline_WriteString(out, Font_6x8 , White, 50, LINE_3_VERT, select_states[3]);
}

static void screen_update_velocity_change(uint8_t * select_states){
    screen_driver_SetCursor_WriteString(message->change_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    uint8_t row = (save_get(MIDI_MODIFY_CHANGE_OR_SPLIT) == MIDI_MODIFY_CHANGE) ? 3 : 4;
    int8_t delta = (int8_t)(int32_t)save_get_u32(MIDI_MODIFY_VELOCITY_PLUS_MINUS);
    static char txt[6]; sprintf(txt, "%+d", delta);
    screen_driver_underline_WriteString(txt, Font_6x8, White, 100, BOTTOM_LINE_VERT, select_states[row]);
}

static void screen_update_velocity_fixed(uint8_t * select_states){
    screen_driver_SetCursor_WriteString(message->fixed_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    uint8_t row = (save_get(MIDI_MODIFY_CHANGE_OR_SPLIT) == MIDI_MODIFY_CHANGE) ? 3 : 4;
    uint8_t v = save_get(MIDI_MODIFY_VELOCITY_ABSOLUTE);
    static char txt[6]; sprintf(txt, "%u", v);
    screen_driver_underline_WriteString(txt, Font_6x8, White, 100, LINE_4_VERT+3, select_states[row]);
}

void screen_update_midi_modify(void)
{
    static uint8_t select_states[5] = {0};

    uint8_t mode = save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
    ui_group_t group = (mode == MIDI_MODIFY_CHANGE) ? UI_GROUP_MODIFY_CHANGE
                                                    : UI_GROUP_MODIFY_SPLIT;

    uint8_t current_select = ui_state_get(UI_MIDI_MODIFY_SELECT);
    (void)build_select_states(group, current_select, select_states, 5);

    screen_driver_Fill(Black);
    menu_display(&Font_6x8, message->midi_modify);

    // Top: Channel rows
    if (mode == MIDI_MODIFY_CHANGE){
        screen_update_channel_change(select_states);
    } else {
        screen_update_channel_split(select_states);
    }

    screen_driver_Line(0, LINE_4_VERT, 127, LINE_4_VERT, White);

    // Bottom: Velocity row (dynamic)
    uint8_t vel_type = save_get(MIDI_MODIFY_VELOCITY_TYPE);
    if (vel_type == MIDI_MODIFY_CHANGED_VEL){
        screen_update_velocity_change(select_states);
    } else {
        screen_update_velocity_fixed(select_states);
    }

    // On/Off
    uint8_t onoff = save_get(MIDI_MODIFY_CURRENTLY_SENDING);
    midi_display_on_off(onoff, LINE_4_VERT);

    screen_driver_UpdateScreen();
}

