/*
 * menu_settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */
#include "_menu_controller.h" //CTRL_G + enum
#include "_menu_ui.h"
#include "menus.h"
#include "text.h"

void screen_update_settings(void)
{
    const ui_element elems[] = {
        // type      save_item                 text                         font    x        y     ctrl_group_id
        // -------- GLOBAL 1 --------
        { ELEM_TEXT , 0,                       TEXT_(global_settings_1),   UI_6x8, TXT_LEFT, LINE_0, CTRL_SETTINGS_GLOBAL1 },
        { ELEM_TEXT , 0,                       TEXT_(start_menu),          UI_6x8, TXT_LEFT, LINE_1, CTRL_SETTINGS_GLOBAL1 },
        { ELEM_ITEM , SETTINGS_START_MENU,     TEXT_(menu_list),           UI_6x8, 70,       LINE_1, CTRL_SETTINGS_GLOBAL1 },

        { ELEM_TEXT , 0,                       TEXT_(usb_midi),            UI_6x8, TXT_LEFT, LINE_2, CTRL_SETTINGS_GLOBAL1 },
        { ELEM_ITEM , SETTINGS_SEND_USB,       TEXT_(usb_receive_send),    UI_6x8, 70,       LINE_2, CTRL_SETTINGS_GLOBAL1 },

        { ELEM_TEXT , 0,                       TEXT_(contrast),            UI_6x8, TXT_LEFT, LINE_3, CTRL_SETTINGS_GLOBAL1 },
        { ELEM_ITEM , SETTINGS_BRIGHTNESS,     TEXT_(contrast_levels),     UI_6x8, 70,       LINE_3, CTRL_SETTINGS_GLOBAL1 },
        // -------- GLOBAL 2 --------
        { ELEM_TEXT , 0,                       TEXT_(global_settings_2),   UI_6x8, TXT_LEFT, LINE_0, CTRL_SETTINGS_GLOBAL2 },
        { ELEM_TEXT , 0,                       TEXT_(MIDI_Thru),           UI_6x8, TXT_LEFT, LINE_1, CTRL_SETTINGS_GLOBAL2 },
        { ELEM_ITEM , SETTINGS_MIDI_THRU,      TEXT_(off_on),              UI_6x8, 80,       LINE_1, CTRL_SETTINGS_GLOBAL2 },

        { ELEM_TEXT , 0,                       TEXT_(USB_Thru),            UI_6x8, TXT_LEFT, LINE_2, CTRL_SETTINGS_GLOBAL2 },
        { ELEM_ITEM , SETTINGS_USB_THRU,       TEXT_(off_on),              UI_6x8, 80,       LINE_2, CTRL_SETTINGS_GLOBAL2 },

        { ELEM_TEXT , 0,                       TEXT_(MIDI_Filter),         UI_6x8, TXT_LEFT, LINE_3, CTRL_SETTINGS_GLOBAL2 },
        { ELEM_ITEM , SETTINGS_CHANNEL_FILTER, TEXT_(off_on),              UI_6x8, 80,       LINE_3, CTRL_SETTINGS_GLOBAL2 },
        // -------- Filters --------
        { ELEM_TEXT , 0,                       TEXT_(MIDI_Filter),             UI_6x8, TXT_LEFT, LINE_0, CTRL_SETTINGS_FILTER },
        { ELEM_TEXT , 0,                       TEXT_(X_equals_ignore_channel), UI_6x8, TXT_LEFT, LINE_1, CTRL_SETTINGS_FILTER },
        { ELEM_16CH , SETTINGS_FILTERED_CH,    "X",                        UI_6x8_2,TXT_LEFT, LINE_2, CTRL_SETTINGS_FILTER },
        // -------- ABOUT--------
        { ELEM_TEXT , 0,                       TEXT_(about),               UI_6x8,  TXT_LEFT, LINE_0, CTRL_SETTINGS_ABOUT },
        { ELEM_TEXT , 0,                       TEXT_(about_brand),         UI_6x8,  TXT_LEFT, LINE_1, CTRL_SETTINGS_ABOUT },
        { ELEM_TEXT , 0,                       TEXT_(about_product),       UI_6x8,  TXT_LEFT, LINE_2, CTRL_SETTINGS_ABOUT },
        { ELEM_TEXT , 0,                       TEXT_(about_version),       UI_6x8, TXT_LEFT, LINE_3, CTRL_SETTINGS_ABOUT },
        // -------- Bottom part (always on) --------
        { ELEM_TEXT , 0,                       TEXT_(save_instruction),    UI_6x8, TXT_LEFT, B_LINE, CTRL_SETTINGS_ALWAYS },
    };
    menu_ui_render(SETTINGS, elems, sizeof(elems)/sizeof(elems[0]));
}



void ui_code_settings()  {
	saving_settings_ui();
	//Bottom line above save text
	draw_line(0, LINE_4, 127, LINE_4);
}

