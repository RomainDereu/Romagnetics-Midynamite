/*
 * menu_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include "_menu_controller.h"
#include "memory_main.h"
#include "_menu_ui.h"
#include "menu_modify.h"
#include "screen_driver.h"
#include "text.h"
#include "threads.h"
#include "utils.h" // Needed for text placement

// -------------------------
// Controller loop
// -------------------------
void midi_modify_update_menu(void)
{
    menu_nav_begin_and_update(UI_MIDI_MODIFY_SELECT);

    if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
        // Decide which selector we just toggled based on row count
        const uint8_t count = (uint8_t)build_select_states(UI_GROUP_MODIFY_BOTH, 0, NULL, 0);
        const uint8_t last  = (uint8_t)(count - 1u);

        const save_field_t target =
            (menu_nav_get_select(UI_MIDI_MODIFY_SELECT) < last)
            ? MIDI_MODIFY_CHANGE_OR_SPLIT
            : MIDI_MODIFY_VELOCITY_TYPE;

        save_modify_u8(target, SAVE_MODIFY_INCREMENT, 0);
        menu_nav_reset(UI_MIDI_MODIFY_SELECT, 0);
        threads_display_notify(FLAG_MODIFY);
        return;
    }

    (void)menu_nav_end_auto(UI_MIDI_MODIFY_SELECT);
}

// -------------------------
// Render
// -------------------------
void screen_update_midi_modify(void)
{
    // Keep selection/active-list up-to-date
    menu_nav_begin_and_update(UI_MIDI_MODIFY_SELECT);

    screen_driver_Fill(Black);
    menu_display(message->midi_modify);
    draw_line(0, LINE_4_VERT, 127, LINE_4_VERT);
    midi_display_on_off(save_get(MIDI_MODIFY_CURRENTLY_SENDING), LINE_4_VERT);

     const ui_element elems[] = {
        // ---------- CHANGE page ----------
        { UI_ELEM_TEXT, 0, message->send_1_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT, CTRL_G_MODIFY_CHANGE },
        { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1, (const char*)message->choices.midi_channels, UI_FONT_6x8, 50, LINE_1_VERT, CTRL_G_MODIFY_CHANGE },

        { UI_ELEM_TEXT, 0, message->send_2_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT, CTRL_G_MODIFY_CHANGE },
        { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2, (const char*)message->choices.midi_channels, UI_FONT_6x8, 50, LINE_2_VERT, CTRL_G_MODIFY_CHANGE },

        { UI_ELEM_TEXT, 0, message->output_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT, CTRL_G_MODIFY_CHANGE },
        { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_OUT, (const char*)message->choices.midi_outs_split, UI_FONT_6x8, 50, LINE_3_VERT, CTRL_G_MODIFY_CHANGE },

        // ---------- SPLIT page ----------
        { UI_ELEM_TEXT, 0, message->low_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT, CTRL_G_MODIFY_SPLIT },
        { UI_ELEM_ITEM, MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1, (const char*)message->zero_to_sixteen, UI_FONT_6x8, 30, LINE_1_VERT, CTRL_G_MODIFY_SPLIT },

        { UI_ELEM_TEXT, 0, message->high_sem, UI_FONT_6x8, 45, LINE_1_VERT, CTRL_G_MODIFY_SPLIT },
        { UI_ELEM_ITEM, MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2, (const char*)message->zero_to_sixteen, UI_FONT_6x8, 80, LINE_1_VERT, CTRL_G_MODIFY_SPLIT },

        { UI_ELEM_TEXT, 0, message->split, UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT, CTRL_G_MODIFY_SPLIT },
        { UI_ELEM_ITEM, MIDI_MODIFY_SPLIT_NOTE, (const char*)message->midi_note_names, UI_FONT_6x8, 40, LINE_2_VERT, CTRL_G_MODIFY_SPLIT },

        { UI_ELEM_TEXT, 0, message->output_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT, CTRL_G_MODIFY_SPLIT },
        { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_OUT, (const char*)message->choices.midi_outs_split, UI_FONT_6x8, 50, LINE_3_VERT, CTRL_G_MODIFY_SPLIT },

        // ---------- VELOCITY (CHANGED) ----------
        { UI_ELEM_TEXT, 0, message->change_velocity, UI_FONT_6x8, TEXT_LEFT_START, BOTTOM_LINE_VERT, CTRL_G_MODIFY_VEL_CHANGED },
        { UI_ELEM_ITEM, MIDI_MODIFY_VELOCITY_PLUS_MINUS, (const char*)message->numbers_neg80_to_pos80, UI_FONT_6x8, 100, BOTTOM_LINE_VERT, CTRL_G_MODIFY_VEL_CHANGED },

        // ---------- VELOCITY (FIXED) ----------
        { UI_ELEM_TEXT, 0, message->fixed_velocity, UI_FONT_6x8, TEXT_LEFT_START, BOTTOM_LINE_VERT, CTRL_G_MODIFY_VEL_FIXED },
        { UI_ELEM_ITEM, MIDI_MODIFY_VELOCITY_ABSOLUTE, (const char*)message->numbers_0_to_300, UI_FONT_6x8, 100, BOTTOM_LINE_VERT, CTRL_G_MODIFY_VEL_FIXED },
    };


    menu_ui_render(elems, (uint8_t)(sizeof(elems)/sizeof(elems[0])));
    screen_driver_UpdateScreen();
    (void)menu_nav_end_auto(UI_MIDI_MODIFY_SELECT);
}
