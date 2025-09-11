/*
 * menu_tempo.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include "memory_main.h"
#include "_menu_controller.h"
#include "_menu_ui.h"
#include "menus.h"
#include "midi_tempo.h"
#include "screen_driver.h"
#include "text.h"
#include "threads.h"

void midi_tempo_update_menu(void)
{
    menu_nav_begin_and_update(UI_MIDI_TEMPO_SELECT);

    uint32_t bpm = save_get(MIDI_TEMPO_CURRENT_TEMPO);
    save_modify_u32(MIDI_TEMPO_TEMPO_CLICK_RATE, SAVE_MODIFY_SET, 6000000u / (bpm * 24u));

    (void)menu_nav_end_auto(UI_MIDI_TEMPO_SELECT);
}

void screen_update_midi_tempo(void)
{
    screen_driver_Fill(Black);
    menu_display(message->send_midi_tempo);

    // static decorations (not group-gated; always OK)
    screen_driver_Line(64, 10, 64, 64, White);
    screen_driver_Line(0, 40, 64, 40, White);

    ui_element elems[] = {
        //                type        save_item                     text                                   font          x               y    ctrl_group_id
        { UI_ELEM_TEXT ,  0,                           message->target,                           UI_FONT_6x8,    TEXT_LEFT_START, 15,  CTRL_G_TEMPO },
        { UI_ELEM_ITEM ,  MIDI_TEMPO_SEND_TO_MIDI_OUT, (const char*)message->choices.midi_outs,   UI_FONT_6x8,    TEXT_LEFT_START, 25,  CTRL_G_TEMPO },

        { UI_ELEM_ITEM ,  MIDI_TEMPO_CURRENTLY_SENDING,(const char*)message->choices.off_on,      UI_FONT_11x18,  15,             42,  CTRL_G_TEMPO },

        { UI_ELEM_ITEM ,  MIDI_TEMPO_CURRENT_TEMPO,     (const char*)message->numbers_0_to_300,   UI_FONT_16x24,  80,             20,  CTRL_G_TEMPO },
        { UI_ELEM_TEXT ,  0,                           message->bpm,                              UI_FONT_6x8,    80,             48,  CTRL_G_TEMPO },
    };

    menu_ui_render(elems, sizeof(elems) / sizeof(elems[0]));

    screen_driver_UpdateScreen();

}
