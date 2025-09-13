/*
 * menu_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Romain Dereu
 */
#include "_menu_controller.h" //CTRL_G + enum
#include "_menu_ui.h"
#include "menus.h"
#include "text.h"

const ui_element transpose_ui_elements[] = {
	// type      save_item                 text                              font    x        y     ctrl_group_id
	{ ELEM_TEXT , 0,                          TEXT_(midi_transpose),       UI_6x8, TXT_LEFT, LINE_0, CTRL_TRANSPOSE_ALL },

	// ---------- SHIFT page ----------
	{ ELEM_TEXT , 0,                          TEXT_(shift_by),             UI_6x8, TXT_LEFT, LINE_1, CTRL_TRANSPOSE_SHIFT },
	{ ELEM_ITEM , TRANSPOSE_MIDI_SHIFT_VALUE, TEXT_(neg_pos_80 + (80-36)), UI_6x8, 65,       LINE_1, CTRL_TRANSPOSE_SHIFT },
	{ ELEM_TEXT , 0,                          TEXT_(semitones),            UI_6x8, TXT_LEFT, LINE_2, CTRL_TRANSPOSE_SHIFT },

	// ---------- SCALED page ----------
	{ ELEM_TEXT , 0,                          TEXT_(root_note),            UI_6x8, TXT_LEFT, LINE_1, CTRL_TRANSPOSE_SCALED },
	{ ELEM_ITEM , TRANSPOSE_BASE_NOTE,         TEXT_(twelve_notes_names),  UI_6x8, 62,       LINE_1, CTRL_TRANSPOSE_SCALED },
	{ ELEM_TEXT , 0,                          TEXT_(interval),             UI_6x8, TXT_LEFT, LINE_2, CTRL_TRANSPOSE_SCALED },
	{ ELEM_ITEM , TRANSPOSE_INTERVAL,          TEXT_(intervals),           UI_6x8, 55,       LINE_2, CTRL_TRANSPOSE_SCALED },
	{ ELEM_TEXT , 0,                          TEXT_(scale),                UI_6x8, TXT_LEFT, LINE_3, CTRL_TRANSPOSE_SCALED },
	{ ELEM_ITEM , TRANSPOSE_TRANSPOSE_SCALE,   TEXT_(scales),              UI_6x8, 40,       LINE_3, CTRL_TRANSPOSE_SCALED },

	// ---------- COMMON (both pages) ----------
	{ ELEM_TEXT , 0,                          TEXT_(send_base),            UI_6x8, TXT_LEFT, LINE_4, CTRL_TRANSPOSE_ALL },
	{ ELEM_ITEM , TRANSPOSE_SEND_ORIGINAL,     TEXT_(no_yes),              UI_6x8, 65,       LINE_4, CTRL_TRANSPOSE_ALL },
};

const size_t transpose_ui_count = sizeof(transpose_ui_elements) / sizeof(tempo_transpose_ui_elementsui_elements[0]);
