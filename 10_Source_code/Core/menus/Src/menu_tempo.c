/*
 * menu_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */
#include "_menu_controller.h" //CTRL_G + enum
#include "_menu_ui.h"
#include "menus.h"
#include "text.h"


const ui_element tempo_ui_elements[] = {
    //        type        save_item                  text            font      x    y    ctrl_group_id
    { ELEM_TEXT ,  0,                     TEXT_(send_midi_tempo),  UI_6x8,   TXT_LEFT, LINE_0, CTRL_TEMPO_ALL },
    { ELEM_TEXT ,  0,                     TEXT_(target),           UI_6x8,   TXT_LEFT, LINE_1, CTRL_TEMPO_ALL },
    { ELEM_ITEM ,  TEMPO_SEND_TO_MIDI_OUT, TEXT_(midi_outs),       UI_6x8,   TXT_LEFT, LINE_2, CTRL_TEMPO_ALL },
    { ELEM_ITEM ,  TEMPO_CURRENTLY_SENDING,TEXT_(off_on),          UI_11x18, 15,      42     , CTRL_TEMPO_ALL },
    { ELEM_ITEM ,  TEMPO_CURRENT_TEMPO,    TEXT_(zer_to_300),      UI_16x24, 80,      20     , CTRL_TEMPO_ALL },
    { ELEM_TEXT ,  0,                     TEXT_(bpm),              UI_6x8,   80,      48     , CTRL_TEMPO_ALL },
};

const size_t tempo_ui_count = sizeof(tempo_ui_elements) / sizeof(tempo_ui_elements[0]);
