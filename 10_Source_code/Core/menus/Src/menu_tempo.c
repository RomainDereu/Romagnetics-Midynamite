/*
 * menu_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory_ui_state.h"
#include "memory_main.h"

#include "menu.h"
#include "menu_tempo.h"
#include "midi_tempo.h"
#include "screen_driver.h"

// under_here_header_checks
#include "text.h"
#include "threads.h"
#include "utils.h"

extern const Message * message;

/* -------------------------
 * Controller
 * ------------------------- */
void midi_tempo_update_menu(void)
{
    ui_group_t group = UI_GROUP_TEMPO; // family root
    menu_nav_begin(group);
    uint8_t current_select = update_select(UI_MIDI_TEMPO_SELECT,     UI_GROUP_TEMPO,          /*mult=*/1, WRAP);


    uint32_t bpm = save_get_u32(MIDI_TEMPO_CURRENT_TEMPO);
    save_modify_u32(MIDI_TEMPO_TEMPO_CLICK_RATE, SAVE_MODIFY_SET, 6000000u / (bpm * 24u));

    toggle_underline_items(group, current_select);
    if (menu_nav_end(UI_MIDI_TEMPO_SELECT, group, current_select)) {
        threads_display_notify(FLAG_TEMPO);
    }
}




/* -------------------------
 * Painter
 * ------------------------- */
void screen_update_midi_tempo(void)
{
    // Current selection for underline map
	uint8_t current_select = update_select(UI_MIDI_TEMPO_SELECT,     UI_GROUP_TEMPO,          /*mult=*/1, WRAP);

    // Build underline states matching current UI_GROUP_TEMPO rows
    uint8_t count = build_select_states(UI_GROUP_TEMPO, current_select, NULL, 0);

    uint8_t select_states[count];
    for (uint8_t i = 0; i < count; ++i) select_states[i] = 0;
    (void)build_select_states(UI_GROUP_TEMPO, current_select, select_states, count);

    screen_driver_Fill(Black);

    // Title + guides
    menu_display(message->send_midi_tempo);
    draw_line(64, 10, 64, 64);   // vertical
    draw_line(0, 40, 64, 40);    // horizontal

    // Tempo big number
    char tempo_print[6];
    snprintf(tempo_print, sizeof tempo_print, "%lu",
             (unsigned long)save_get_u32(MIDI_TEMPO_CURRENT_TEMPO));
    write_underline_1624(tempo_print, 80, 20, select_states[TEMPO_PRINT]);
    write_68(message->bpm, 80, 48);

    // Send target
    write_68(message->target, TEXT_LEFT_START, 15);
    const char *midi_send_out = message->choices.midi_outs[ save_get(MIDI_TEMPO_SEND_TO_MIDI_OUT) ];
    write_underline_68(midi_send_out, TEXT_LEFT_START, 25, select_states[MIDI_OUT_PRINT]);

    //Stop/Sending status
    uint8_t currently_sending = save_get(MIDI_TEMPO_CURRENTLY_SENDING);

    if(currently_sending == 0){
  	  write_1118(message->off, 15, 42);
    }
    else if (currently_sending == 1){
  	  write_1118(message->on, 15, 42);
    }

    screen_driver_UpdateScreen();
}

