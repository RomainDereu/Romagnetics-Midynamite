/*
 * menu_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "memory_ui_state.h"
#include "memory_main.h"

#include "midi_transform.h"

#include "screen_driver.h"

//under_here_header_checks
#include "menu.h"


#include "text.h"
#include "threads.h"
#include "utils.h"



void midi_transpose_update_menu(void)
{
    ui_group_t group = UI_GROUP_TRANSPOSE_BOTH; // family root
    menu_nav_begin(group);
    menu_nav_update_select(UI_MIDI_TRANSPOSE_SELECT, group);
    uint8_t current_select = menu_nav_get_select(UI_MIDI_TRANSPOSE_SELECT);

    if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
        save_modify_u8(MIDI_TRANSPOSE_TRANSPOSE_TYPE, SAVE_MODIFY_INCREMENT, 0);
        menu_nav_reset(UI_MIDI_TRANSPOSE_SELECT, 0);
        threads_display_notify(FLAG_TRANSPOSE);
        return;
    }

    toggle_underline_items(group, current_select);
    if (menu_nav_end(UI_MIDI_TRANSPOSE_SELECT, group, current_select)) {
        threads_display_notify(FLAG_TRANSPOSE);
    }
}



void screen_update_midi_transpose(void)
{
    // Build underline states for the *current* selection on the active page
    uint8_t type = save_get(MIDI_TRANSPOSE_TRANSPOSE_TYPE);
    ui_group_t group = (type == MIDI_TRANSPOSE_SHIFT)
                 ? UI_GROUP_TRANSPOSE_SHIFT
                 : UI_GROUP_TRANSPOSE_SCALED;

    uint8_t current_select = menu_nav_get_select(UI_MIDI_TRANSPOSE_SELECT);

    // enough for the worst-case page (SCALED has 4 rows; SHIFT has 2)
    uint8_t select_states[5] = {0};
    (void)build_select_states(group, current_select, select_states, sizeof select_states);

    screen_driver_Fill(Black);
    menu_display(message->midi_transpose);

    if (type == MIDI_TRANSPOSE_SHIFT) {
        // SHIFT page
        write_68(message->shift_by, TEXT_LEFT_START, LINE_1_VERT);
        char modify_value[6];
        int8_t plus_minus_i8 = (int8_t)(int32_t)save_get_u32(MIDI_TRANSPOSE_MIDI_SHIFT_VALUE);
        sprintf(modify_value, "%+d", plus_minus_i8);
        write_underline_68(modify_value, 65, LINE_1_VERT, select_states[0]);

        write_68(message->semitones, TEXT_LEFT_START, LINE_2_VERT);

        write_68(message->send_base, TEXT_LEFT_START, LINE_4_VERT);
        const char *send_base = message->choices.no_yes[ save_get(MIDI_TRANSPOSE_SEND_ORIGINAL) ];
        write_underline_68(send_base, 65, LINE_4_VERT, select_states[1]);
    } else {
        // SCALED page
        write_68(message->root_note, TEXT_LEFT_START, LINE_1_VERT);
        const char *base_note = message->twelve_notes_names[ save_get(MIDI_TRANSPOSE_BASE_NOTE) ];
        write_underline_68(base_note, 62, LINE_1_VERT, select_states[0]);

        write_68(message->interval, TEXT_LEFT_START, LINE_2_VERT);
        const char *interval = message->choices.intervals[ save_get(MIDI_TRANSPOSE_INTERVAL) ];
        write_underline_68(interval, 55, LINE_2_VERT, select_states[1]);

        write_68(message->scale, TEXT_LEFT_START, LINE_3_VERT);
        const char *scale = message->choices.scales[ save_get(MIDI_TRANSPOSE_TRANSPOSE_SCALE) ];
        write_underline_68(scale, 40, LINE_3_VERT, select_states[2]);

        write_68(message->send_base, TEXT_LEFT_START, LINE_4_VERT);
        const char *send_base = message->choices.no_yes[ save_get(MIDI_TRANSPOSE_SEND_ORIGINAL) ];
        write_underline_68(send_base, 65, LINE_4_VERT, select_states[3]);
    }

    midi_display_on_off(save_get(MIDI_TRANSPOSE_CURRENTLY_SENDING), 63);

    screen_driver_UpdateScreen();
}


