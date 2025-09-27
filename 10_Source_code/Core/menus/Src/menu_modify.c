/*
 * menu_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include "memory_main.h"
#include "_menu_ui.h"
#include "menus.h"
#include "text.h"

void cont_update_modify(menu_list_t field) {
	toggle_subpage(field);
}


void ui_update_modify(void)
{
    const ui_element elems[] = {
        // type      save_item                text                        font    x       y           ctrl_group_id
        { ELEM_TEXT , 0,                       TEXT_(midi_modify),        UI_6x8, TXT_LEFT, LINE_0,      CTRL_MODIFY_ALL },

        { ELEM_TEXT , 0,                       TEXT_(send_1_sem),         UI_6x8, TXT_LEFT, LINE_1,      CTRL_MODIFY_CHANGE },
        { ELEM_ITEM , MODIFY_SEND_TO_MIDI_CH1,  TEXT_(midi_channels),      UI_6x8, 50,      LINE_1,      CTRL_MODIFY_CHANGE },

        { ELEM_TEXT , 0,                       TEXT_(send_2_sem),         UI_6x8, TXT_LEFT, LINE_2,      CTRL_MODIFY_CHANGE },
        { ELEM_ITEM , MODIFY_SEND_TO_MIDI_CH2,  TEXT_(midi_channels),      UI_6x8, 50,      LINE_2,      CTRL_MODIFY_CHANGE },

        { ELEM_TEXT , 0,                       TEXT_(output_sem),         UI_6x8, TXT_LEFT, LINE_3,      CTRL_MODIFY_CHANGE },
        { ELEM_ITEM , MODIFY_SEND_TO_MIDI_OUT,  TEXT_(midi_outs_split),    UI_6x8, 50,      LINE_3,      CTRL_MODIFY_CHANGE },

        // ---------- SPLIT page ----------
        { ELEM_TEXT , 0,                       TEXT_(low_sem),            UI_6x8, TXT_LEFT, LINE_1,      CTRL_MODIFY_SPLIT },
        { ELEM_ITEM , MODIFY_SPLIT_MIDI_CH1,    TEXT_(zero_to_sixteen),    UI_6x8, 30,      LINE_1,      CTRL_MODIFY_SPLIT },

        { ELEM_TEXT , 0,                       TEXT_(high_sem),           UI_6x8, 45,      LINE_1,      CTRL_MODIFY_SPLIT },
        { ELEM_ITEM , MODIFY_SPLIT_MIDI_CH2,    TEXT_(zero_to_sixteen),    UI_6x8, 80,      LINE_1,      CTRL_MODIFY_SPLIT },

        { ELEM_TEXT , 0,                       TEXT_(split),              UI_6x8, TXT_LEFT, LINE_2,      CTRL_MODIFY_SPLIT },
        { ELEM_ITEM , MODIFY_SPLIT_NOTE,        TEXT_(midi_note_names),    UI_6x8, 40,      LINE_2,      CTRL_MODIFY_SPLIT },

        { ELEM_TEXT , 0,                       TEXT_(output_sem),         UI_6x8, TXT_LEFT, LINE_3,      CTRL_MODIFY_SPLIT },
        { ELEM_ITEM , MODIFY_SEND_TO_MIDI_OUT,  TEXT_(midi_outs_split),    UI_6x8, 50,      LINE_3,      CTRL_MODIFY_SPLIT },

        // ---------- VELOCITY (CHANGED) ----------
        { ELEM_TEXT , 0,                       TEXT_(change_velocity),    UI_6x8, TXT_LEFT, B_LINE, CTRL_MODIFY_VEL_CHANGED },
        { ELEM_ITEM , MODIFY_VEL_PLUS_MINUS,    TEXT_(neg_pos_80),         UI_6x8, 100,     B_LINE, CTRL_MODIFY_VEL_CHANGED },

        // ---------- VELOCITY (FIXED) ----------
        { ELEM_TEXT , 0,                       TEXT_(fixed_velocity),     UI_6x8, TXT_LEFT, B_LINE, CTRL_MODIFY_VEL_FIXED },
        { ELEM_ITEM , MODIFY_VEL_ABSOLUTE,      TEXT_(zer_to_300),         UI_6x8, 100,     B_LINE, CTRL_MODIFY_VEL_FIXED },
    };
    menu_ui_render(MENU_MODIFY, elems, (uint8_t)(sizeof(elems)/sizeof(elems[0])));
}


void ui_code_modify()    {

	midi_display_on_off(save_get(MODIFY_SENDING), LINE_4);
	//Bottom line above velocity
	draw_line(0, LINE_4, 127, LINE_4);

}
