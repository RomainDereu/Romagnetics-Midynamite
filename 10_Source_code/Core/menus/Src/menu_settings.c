/*
 * menu_settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */
#include "_menu_controller.h" //CTRL_G
#include "_menu_ui.h"
#include "menus.h"
#include "screen_driver.h"
#include "text.h"


void screen_update_settings(void)
{
    screen_driver_Fill(Black);

    // Decide title by active sub-group(s)
    const uint32_t active = ui_active_groups();
    const char *title = message->about;  // default
    if (active & (1u << (CTRL_G_SETTINGS_GLOBAL1 - 1)))      title = message->global_settings_1;
    else if (active & (1u << (CTRL_G_SETTINGS_GLOBAL2 - 1))) title = message->global_settings_2;
    else if (active & (1u << (CTRL_G_SETTINGS_FILTER  - 1))) title = message->MIDI_Filter;

    menu_display(title);

    // Flat, group-gated UI elements (everything except the 16-bit filter grid)
    const ui_element elems[] = {
        // -------- GLOBAL 1 --------
        { UI_ELEM_TEXT, 0,                    message->start_menu,              UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT, CTRL_G_SETTINGS_GLOBAL1 },
        { UI_ELEM_ITEM, SETTINGS_START_MENU, (const char*)message->choices.menu_list,         UI_FONT_6x8, 70,            LINE_1_VERT, CTRL_G_SETTINGS_GLOBAL1 },

        { UI_ELEM_TEXT, 0,                    message->usb_midi,                UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT, CTRL_G_SETTINGS_GLOBAL1 },
        { UI_ELEM_ITEM, SETTINGS_SEND_USB,   (const char*)message->choices.usb_receive_send, UI_FONT_6x8, 70,            LINE_2_VERT, CTRL_G_SETTINGS_GLOBAL1 },

        { UI_ELEM_TEXT, 0,                    message->contrast,                UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT, CTRL_G_SETTINGS_GLOBAL1 },
        { UI_ELEM_ITEM, SETTINGS_BRIGHTNESS, (const char*)message->contrast_levels,           UI_FONT_6x8, 70,            LINE_3_VERT, CTRL_G_SETTINGS_GLOBAL1 },

        // -------- GLOBAL 2 --------
        { UI_ELEM_TEXT, 0,                    message->MIDI_Thru,               UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT, CTRL_G_SETTINGS_GLOBAL2 },
        { UI_ELEM_ITEM, SETTINGS_MIDI_THRU,  (const char*)message->choices.off_on,            UI_FONT_6x8, 80,            LINE_1_VERT, CTRL_G_SETTINGS_GLOBAL2 },

        { UI_ELEM_TEXT, 0,                    message->USB_Thru,                UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT, CTRL_G_SETTINGS_GLOBAL2 },
        { UI_ELEM_ITEM, SETTINGS_USB_THRU,   (const char*)message->choices.off_on,            UI_FONT_6x8, 80,            LINE_2_VERT, CTRL_G_SETTINGS_GLOBAL2 },

        { UI_ELEM_TEXT, 0,                    message->MIDI_Filter,             UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT, CTRL_G_SETTINGS_GLOBAL2 },
        { UI_ELEM_ITEM, SETTINGS_CHANNEL_FILTER, (const char*)message->choices.off_on,        UI_FONT_6x8, 80,            LINE_3_VERT, CTRL_G_SETTINGS_GLOBAL2 },

        // -------- ABOUT (text-only) --------
        { UI_ELEM_TEXT, 0, message->X_equals_ignore_channel, UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT, CTRL_G_SETTINGS_FILTER },


        // -------- ABOUT (text-only) --------
        { UI_ELEM_TEXT, 0, message->about_brand,   UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT, CTRL_G_SETTINGS_ABOUT },
        { UI_ELEM_TEXT, 0, message->about_product, UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT, CTRL_G_SETTINGS_ABOUT },
        { UI_ELEM_TEXT, 0, message->about_version, UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT, CTRL_G_SETTINGS_ABOUT },



    };

    // -------- FILTER GRID (custom draw, but only when its group is active) --------
    filter_controller();

    // Footer (can be CTRL_G_SETTINGS_BOTH if you prefer to gate it too)
    draw_line(0, LINE_4_VERT, 127, LINE_4_VERT);
    write_68(message->save_instruction, TEXT_LEFT_START, BOTTOM_LINE_VERT);

    // Draw the flat, group-gated elements
    menu_ui_render(elems, sizeof(elems)/sizeof(elems[0]));
}
