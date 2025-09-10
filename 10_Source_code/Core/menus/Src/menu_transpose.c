/*
 * menu_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Romain Dereu
 */
#include "_menu_controller.h"
#include "memory_main.h"
#include "_menu_ui.h"
#include "midi_transform.h"
#include "screen_driver.h"
#include "text.h"
#include "threads.h"
#include "utils.h" //Needed for text placement


void midi_transpose_update_menu(void)
{
    menu_nav_begin_and_update(UI_MIDI_TRANSPOSE_SELECT);

    if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
        save_modify_u8(MIDI_TRANSPOSE_TRANSPOSE_TYPE, SAVE_MODIFY_INCREMENT, 0);
        menu_nav_reset(UI_MIDI_TRANSPOSE_SELECT, 0);
        threads_display_notify(FLAG_TRANSPOSE);
        return;
    }

    (void)menu_nav_end_auto(UI_MIDI_TRANSPOSE_SELECT);
}


void screen_update_midi_transpose(void)
{
    // Keep active mask/selection in sync for this frame
    menu_nav_begin_and_update(UI_MIDI_TRANSPOSE_SELECT);

    screen_driver_Fill(Black);
    menu_display(message->midi_transpose);

    const uint8_t type = (uint8_t)save_get(MIDI_TRANSPOSE_TRANSPOSE_TYPE);

    if (type == MIDI_TRANSPOSE_SHIFT) {
        // SHIFT page
        const ui_element elems_shift[] = {
            { UI_ELEM_TEXT, 0, message->shift_by, UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT },
            // Use numbers_neg80_to_pos80 but offset the base so index 0 maps to -36
            { UI_ELEM_ITEM, MIDI_TRANSPOSE_MIDI_SHIFT_VALUE,
              (const char*)(message->numbers_neg80_to_pos80 + (80 - 36)),
              UI_FONT_6x8, 65, LINE_1_VERT },

            { UI_ELEM_TEXT, 0, message->semitones, UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT },

            { UI_ELEM_TEXT, 0, message->send_base, UI_FONT_6x8, TEXT_LEFT_START, LINE_4_VERT },
            { UI_ELEM_ITEM, MIDI_TRANSPOSE_SEND_ORIGINAL,
              (const char*)message->choices.no_yes,
              UI_FONT_6x8, 65, LINE_4_VERT },
        };
        menu_ui_render(elems_shift, sizeof(elems_shift)/sizeof(elems_shift[0]));
    } else {
        // SCALED page
        const ui_element elems_scaled[] = {
            { UI_ELEM_TEXT, 0, message->root_note, UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT },
            { UI_ELEM_ITEM, MIDI_TRANSPOSE_BASE_NOTE,
              (const char*)message->twelve_notes_names,
              UI_FONT_6x8, 62, LINE_1_VERT },

            { UI_ELEM_TEXT, 0, message->interval, UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT },
            { UI_ELEM_ITEM, MIDI_TRANSPOSE_INTERVAL,
              (const char*)message->choices.intervals,
              UI_FONT_6x8, 55, LINE_2_VERT },

            { UI_ELEM_TEXT, 0, message->scale, UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT },
            { UI_ELEM_ITEM, MIDI_TRANSPOSE_TRANSPOSE_SCALE,
              (const char*)message->choices.scales,
              UI_FONT_6x8, 40, LINE_3_VERT },

            { UI_ELEM_TEXT, 0, message->send_base, UI_FONT_6x8, TEXT_LEFT_START, LINE_4_VERT },
            { UI_ELEM_ITEM, MIDI_TRANSPOSE_SEND_ORIGINAL,
              (const char*)message->choices.no_yes,
              UI_FONT_6x8, 65, LINE_4_VERT },
        };
        menu_ui_render(elems_scaled, sizeof(elems_scaled)/sizeof(elems_scaled[0]));
    }

    midi_display_on_off(save_get(MIDI_TRANSPOSE_CURRENTLY_SENDING), 63);

    screen_driver_UpdateScreen();

    (void)menu_nav_end_auto(UI_MIDI_TRANSPOSE_SELECT);
}
