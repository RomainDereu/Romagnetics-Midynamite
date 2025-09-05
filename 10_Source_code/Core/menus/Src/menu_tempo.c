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
    // Snapshot before
    midi_tempo_data_struct old_midi_tempo_data = save_snapshot_tempo();

    // Begin frame for this group
    menu_nav_begin(UI_GROUP_TEMPO);

    // How many interactive rows are active right now
    const uint8_t count = build_select_states(UI_GROUP_TEMPO, /*current_select=*/0, NULL, 0);

    // Selection update (left encoder)
    uint8_t current_select = menu_nav_update_and_get(
        UI_MIDI_TEMPO_SELECT,
        /*min=*/0,
        /*max=*/(uint8_t)(count ? (count - 1) : 0),
        /*step=*/1,
        /*wrap=*/WRAP
    );

    // Drive selected item (right encoder)
    toggle_underline_items(UI_GROUP_TEMPO, current_select);

    // Keep click-rate in sync with tempo
    uint32_t new_tempo = save_get_u32(MIDI_TEMPO_CURRENT_TEMPO);
    save_modify_u32(MIDI_TEMPO_TEMPO_CLICK_RATE, SAVE_MODIFY_SET, 6000000 / (new_tempo * 24));

    // End frame: redraw if selection or any tracked field changed
    if (menu_nav_end(UI_MIDI_TEMPO_SELECT, UI_GROUP_TEMPO, current_select)) {
        threads_display_notify(FLAG_TEMPO);
    } else {
        // (Optional) Preserve your previous compare if you still want the extra guard)
        midi_tempo_data_struct new_midi_tempo_data = save_snapshot_tempo();
        uint8_t dummy_old = current_select, dummy_cur = current_select;
        if (menu_check_for_updates(&old_midi_tempo_data, &new_midi_tempo_data,
                                   sizeof new_midi_tempo_data, &dummy_cur, &dummy_old)) {
            threads_display_notify(FLAG_TEMPO);
        }
    }
}

/* -------------------------
 * Painter
 * ------------------------- */
void screen_update_midi_tempo(void)
{
    uint8_t current_select = menu_nav_get_select(UI_MIDI_TEMPO_SELECT);

    // Build underline map
    uint8_t count = build_select_states(UI_GROUP_TEMPO, current_select, NULL, 0);
    if (count == 0) count = 1; // defensive
    uint8_t select_states[count];
    for (uint8_t i = 0; i < count; ++i) select_states[i] = 0;
    (void)build_select_states(UI_GROUP_TEMPO, current_select, select_states, count);

    screen_driver_Fill(Black);

    // Title + layout guides
    menu_display(message->send_midi_tempo);
    draw_line(64, 10, 64, 64);   // vertical
    draw_line(0, 40, 64, 40);    // horizontal

    // Tempo big number
    char tempo_print[6]; // room for 3 digits + null (or 4 just in case)
    snprintf(tempo_print, sizeof tempo_print, "%lu",
             (unsigned long)save_get_u32(MIDI_TEMPO_CURRENT_TEMPO));
    write_underline_1624(tempo_print, 80, 20, select_states[TEMPO_PRINT]);
    write_68(message->bpm, 80, 48);

    // Send target (array lookup instead of switch)
    write_68(message->target, TEXT_LEFT_START, 15);
    const char *midi_send_out =
        message->choices.midi_outs[ save_get(MIDI_TEMPO_SEND_TO_MIDI_OUT) ];
    write_underline_68(midi_send_out, TEXT_LEFT_START, 25, select_states[MIDI_OUT_PRINT]);

    // On/Off indicator
    midi_display_on_off(save_get(MIDI_TEMPO_CURRENTLY_SENDING), 63);

    screen_driver_UpdateScreen();
}
