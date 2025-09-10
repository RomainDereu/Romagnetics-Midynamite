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
    menu_display(message->send_midi_tempo);
	screen_driver_Line(64, 10, 64, 64, White);
	screen_driver_Line(0, 40, 64, 40, White);




    ui_element elems[] = {
    { UI_ELEM_UNDERL, MIDI_TEMPO_CURRENT_TEMPO, (const char*)message->numbers_0_to_300, UI_FONT_16x24, 80,  20, select_states[0] },
    { UI_ELEM_TEXT  , 0                        ,message->bpm             , UI_FONT_6x8,   80,               48, 0 },
    { UI_ELEM_TEXT  , 0                        ,message->target          , UI_FONT_6x8,   TEXT_LEFT_START , 15, 0 },
    { UI_ELEM_UNDERL, MIDI_TEMPO_SEND_TO_MIDI_OUT, (const char*)message->choices.midi_outs, UI_FONT_6x8, TEXT_LEFT_START, 25, select_states[1] },
    { UI_ELEM_SWITCH, MIDI_TEMPO_CURRENTLY_SENDING,(const char*)message->choices.off_on, UI_FONT_11x18, 15, 42, 0 },
    };



    menu_ui_render(elems, sizeof(elems) / sizeof(elems[0]));

    screen_driver_UpdateScreen();
}
