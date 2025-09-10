/*
 * menu_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include "_menu_controller.h"
#include "memory_main.h"
#include "_menu_ui.h"
#include "menu_tempo.h"
#include "midi_tempo.h"
#include "screen_driver.h"
#include "text.h"
#include "threads.h"
#include "utils.h" //Needed for text placement

void midi_tempo_update_menu(void)
{
    menu_nav_begin_and_update(UI_MIDI_TEMPO_SELECT);

    uint32_t bpm = save_get(MIDI_TEMPO_CURRENT_TEMPO);
    save_modify_u32(MIDI_TEMPO_TEMPO_CLICK_RATE, SAVE_MODIFY_SET, 6000000u / (bpm * 24u));

    (void)menu_nav_end_auto(UI_MIDI_TEMPO_SELECT);
}

void screen_update_midi_tempo(void)
{
    // Current selection for underline map
	uint8_t current_select = menu_nav_get_select(UI_MIDI_TEMPO_SELECT);

    // Build underline states matching current UI_GROUP_TEMPO rows
    uint8_t count = build_select_states(UI_GROUP_TEMPO, current_select, NULL, 0);

    uint8_t select_states[count];
    for (uint8_t i = 0; i < count; ++i) select_states[i] = 0;
    (void)build_select_states(UI_GROUP_TEMPO, current_select, select_states, count);

    screen_driver_Fill(Black);

    // Title + guides
    menu_display(message->send_midi_tempo);



    // Tempo big number
    const char *tempo_print = message->numbers_0_to_300[save_get(MIDI_TEMPO_CURRENT_TEMPO)];
    write_underline_1624(tempo_print, 80, 20, (count > 0) ? select_states[0] : 0);
    write_68(message->bpm, 80, 48);

    // Send target
    write_68(message->target, TEXT_LEFT_START, 15);
    const char *midi_send_out = message->choices.midi_outs[ save_get(MIDI_TEMPO_SEND_TO_MIDI_OUT) ];
    write_underline_68(midi_send_out, TEXT_LEFT_START, 25, (count > 1) ? select_states[1] : 0);

    //Stop/Sending status
    uint8_t currently_sending = save_get(MIDI_TEMPO_CURRENTLY_SENDING);
    write_1118(message->choices.off_on[currently_sending], 15, 42);

    static ui_element elems[] = {
        { UI_ELEM_LINE,  64, 10, 64, 64, NULL, 0, 0 },
        { UI_ELEM_LINE,   0, 40, 64, 40, NULL, 0, 0 },
    };



    menu_ui_render(elems, sizeof(elems) / sizeof(elems[0]));

    screen_driver_UpdateScreen();
}
