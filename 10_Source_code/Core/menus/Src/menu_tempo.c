/*
 * menu_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include "_menu_controller.h"
#include "_menu_ui.h"
#include "menus.h"
#include "midi_tempo.h"
#include "screen_driver.h"
#include "text.h"


void screen_update_midi_tempo(void)
{
    screen_driver_Fill(Black);
    menu_display(message->send_midi_tempo);

    // static decorations (not group-gated; always OK)
    screen_driver_Line(64, 10, 64, 64, White);
    screen_driver_Line(0, 40, 64, 40, White);

    const ui_element elems[] = {
        //        type        save_item                  text                 font      x        y    ctrl_group_id
        { ELEM_TEXT ,  0,                     TEXT_(target),           UI_6x8,   TXT_LEFT, 15, CTRL_TEMPO },
        { ELEM_ITEM ,  TEMPO_SEND_TO_MIDI_OUT, TEXT_(midi_outs),       UI_6x8,   TXT_LEFT, 25, CTRL_TEMPO },
        { ELEM_ITEM ,  TEMPO_CURRENTLY_SENDING,TEXT_(off_on),          UI_11x18, 15,      42, CTRL_TEMPO },
        { ELEM_ITEM ,  TEMPO_CURRENT_TEMPO,    TEXT_(zer_to_300),      UI_16x24, 80,      20, CTRL_TEMPO },
        { ELEM_TEXT ,  0,                     TEXT_(bpm),              UI_6x8,   80,      48, CTRL_TEMPO },
    };

    menu_ui_render(elems, sizeof(elems) / sizeof(elems[0]));
}
