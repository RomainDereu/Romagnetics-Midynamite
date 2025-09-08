/*
 * menu_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>

#include "_menu_ui.h"
#include "memory_main.h"
#include "menu.h"
#include "menu_modify.h"
#include "midi_transform.h"
#include "screen_driver.h"
#include "text.h"
#include "threads.h"
#include "utils.h" //Needed for text placement


// midi modify menu
void midi_modify_update_menu(void)
{
    uint8_t mode = save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
    ui_group_t group = (mode == MIDI_MODIFY_CHANGE) ? UI_GROUP_MODIFY_CHANGE : UI_GROUP_MODIFY_SPLIT;
    menu_nav_begin(group);
    menu_nav_update_select(UI_MIDI_MODIFY_SELECT, group);
    uint8_t current_select = menu_nav_get_select(UI_MIDI_MODIFY_SELECT);

    if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
        // unchanged…
        uint8_t count = build_select_states(group, 0, NULL, 0);
        if (current_select < (uint8_t)(count - 1)) {
            save_modify_u8(MIDI_MODIFY_CHANGE_OR_SPLIT, SAVE_MODIFY_INCREMENT, 0);
        } else {
            save_modify_u8(MIDI_MODIFY_VELOCITY_TYPE, SAVE_MODIFY_INCREMENT, 0);
        }
        menu_nav_reset(UI_MIDI_MODIFY_SELECT, 0);
        threads_display_notify(FLAG_MODIFY);
        return;
    }

    toggle_underline_items(group, current_select);
    if (menu_nav_end(UI_MIDI_MODIFY_SELECT, current_select)) {
        threads_display_notify(FLAG_MODIFY);
    }
}






// -------- Display helpers (read from save_*, no struct param) --------

static void screen_update_channel_change(uint8_t * select_states){
	write_68(message->send_1_sem, TEXT_LEFT_START, LINE_1_VERT);
    const char *ch1 = message->choices.midi_channels[ save_get(MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1) ];
    write_underline_68(ch1, 50, LINE_1_VERT, select_states[0]);

    write_68(message->send_2_sem, TEXT_LEFT_START, LINE_2_VERT);
    const char *ch2 = message->choices.midi_channels[ save_get(MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2) ];
    write_underline_68(ch2, 50, LINE_2_VERT, select_states[1]);

    write_68(message->output_sem, TEXT_LEFT_START, LINE_3_VERT);
    const char *out = message->choices.midi_outs[ save_get(MIDI_MODIFY_SEND_TO_MIDI_OUT) ];
    write_underline_68(out, 50, LINE_3_VERT, select_states[2]);
}

static void screen_update_channel_split(uint8_t * select_states){
	write_68(message->low_sem, TEXT_LEFT_START, LINE_1_VERT);
    uint8_t low = save_get(MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1);
    static char low_txt[6];  sprintf(low_txt, "%u", low);
    write_underline_68(low_txt, 30, LINE_1_VERT, select_states[0]);

    write_68(message->high_sem, 45, LINE_1_VERT);
    uint8_t high = save_get(MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2);
    static char high_txt[6]; sprintf(high_txt, "%u", high);
    write_underline_68(high_txt, 80, LINE_1_VERT, select_states[1]);

    write_68(message->split, TEXT_LEFT_START, LINE_2_VERT);
    uint8_t split_note = save_get(MIDI_MODIFY_SPLIT_NOTE);
    const char *note_to_write = message->midi_note_names[split_note];
    write_underline_68(note_to_write, 40, LINE_2_VERT, select_states[2]);

    write_68(message->output_sem, TEXT_LEFT_START, LINE_3_VERT);
    const char *out = message->choices.midi_outs[ save_get(MIDI_MODIFY_SEND_TO_MIDI_OUT) ];
    write_underline_68(out, 50, LINE_3_VERT, select_states[3]);
}

static void screen_update_velocity_change(uint8_t * select_states){
	write_68(message->change_velocity, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    uint8_t row = (save_get(MIDI_MODIFY_CHANGE_OR_SPLIT) == MIDI_MODIFY_CHANGE) ? 3 : 4;
    int8_t delta = (int8_t)(int32_t)save_get_u32(MIDI_MODIFY_VELOCITY_PLUS_MINUS);
    static char txt[6]; sprintf(txt, "%+d", delta);
    write_underline_68(txt, 100, BOTTOM_LINE_VERT, select_states[row]);
}

static void screen_update_velocity_fixed(uint8_t * select_states){
	write_68(message->fixed_velocity, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    uint8_t row = (save_get(MIDI_MODIFY_CHANGE_OR_SPLIT) == MIDI_MODIFY_CHANGE) ? 3 : 4;
    uint8_t v = save_get(MIDI_MODIFY_VELOCITY_ABSOLUTE);
    static char txt[6]; sprintf(txt, "%u", v);
    write_underline_68(txt, 100, LINE_4_VERT+3, select_states[row]);
}

void screen_update_midi_modify(void)
{
    static uint8_t select_states[5] = {0};

    uint8_t mode = save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);
    ui_group_t group = (mode == MIDI_MODIFY_CHANGE) ? UI_GROUP_MODIFY_CHANGE
                                                    : UI_GROUP_MODIFY_SPLIT;

    uint8_t current_select = menu_nav_get_select(UI_MIDI_MODIFY_SELECT);
    (void)build_select_states(group, current_select, select_states, 5);

    screen_driver_Fill(Black);
    menu_display(message->midi_modify);

    // Top: Channel rows
    if (mode == MIDI_MODIFY_CHANGE){
        screen_update_channel_change(select_states);
    } else {
        screen_update_channel_split(select_states);
    }

    draw_line(0, LINE_4_VERT, 127, LINE_4_VERT);

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

