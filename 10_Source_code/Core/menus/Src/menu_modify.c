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
#include "midi_transform.h"
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
        const uint8_t last = (uint8_t)(build_select_states(UI_GROUP_MODIFY_BOTH, 0, NULL, 0) - 1u);
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
    // Ensure correct root group / active mask / selection for this frame
    menu_nav_begin_and_update(UI_MIDI_MODIFY_SELECT);

    screen_driver_Fill(Black);
    menu_display(message->midi_modify);

    // Top section divider
    draw_line(0, LINE_4_VERT, 127, LINE_4_VERT);

    // Top: CHANGE vs SPLIT
    uint8_t mode = (uint8_t)save_get(MIDI_MODIFY_CHANGE_OR_SPLIT);

    if (mode == MIDI_MODIFY_CHANGE) {
        const ui_element elems_change[] = {
            { UI_ELEM_TEXT, 0                                  , message->send_1_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1, (const char*)message->choices.midi_channels, UI_FONT_6x8, 50, LINE_1_VERT },

            { UI_ELEM_TEXT, 0                                  , message->send_2_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2, (const char*)message->choices.midi_channels, UI_FONT_6x8, 50, LINE_2_VERT },

            { UI_ELEM_TEXT, 0                                  , message->output_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_OUT, (const char*)message->choices.midi_outs_split, UI_FONT_6x8, 50, LINE_3_VERT },
        };
        menu_ui_render(elems_change, sizeof(elems_change)/sizeof(elems_change[0]));
    } else {
        const ui_element elems_split[] = {
            { UI_ELEM_TEXT, 0                                  , message->low_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_1_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1, (const char*)message->numbers_0_to_300, UI_FONT_6x8, 30, LINE_1_VERT },

            { UI_ELEM_TEXT, 0                                  , message->high_sem, UI_FONT_6x8, 45, LINE_1_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2, (const char*)message->numbers_0_to_300, UI_FONT_6x8, 80, LINE_1_VERT },

            { UI_ELEM_TEXT, 0                                  , message->split, UI_FONT_6x8, TEXT_LEFT_START, LINE_2_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_SPLIT_NOTE, (const char*)message->midi_note_names, UI_FONT_6x8, 40, LINE_2_VERT },

            { UI_ELEM_TEXT, 0                                  , message->output_sem, UI_FONT_6x8, TEXT_LEFT_START, LINE_3_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_SEND_TO_MIDI_OUT, (const char*)message->choices.midi_outs_split, UI_FONT_6x8, 50, LINE_3_VERT },
        };
        menu_ui_render(elems_split, sizeof(elems_split)/sizeof(elems_split[0]));
    }

    // Divider
    draw_line(0, LINE_4_VERT, 127, LINE_4_VERT);

    // Bottom: Velocity row (dynamic)
    uint8_t vel_type = (uint8_t)save_get(MIDI_MODIFY_VELOCITY_TYPE);
    if (vel_type == MIDI_MODIFY_CHANGED_VEL) {
        const ui_element elems_vel_changed[] = {
            { UI_ELEM_TEXT, 0                              , message->change_velocity, UI_FONT_6x8, TEXT_LEFT_START, BOTTOM_LINE_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_VELOCITY_PLUS_MINUS, (const char*)message->numbers_neg80_to_pos80, UI_FONT_6x8, 100, BOTTOM_LINE_VERT },
        };
        menu_ui_render(elems_vel_changed, sizeof(elems_vel_changed)/sizeof(elems_vel_changed[0]));
    } else {
        const ui_element elems_vel_fixed[] = {
            { UI_ELEM_TEXT, 0                            , message->fixed_velocity, UI_FONT_6x8, TEXT_LEFT_START, BOTTOM_LINE_VERT },
            { UI_ELEM_ITEM, MIDI_MODIFY_VELOCITY_ABSOLUTE, (const char*)message->numbers_0_to_300, UI_FONT_6x8, 100, BOTTOM_LINE_VERT },
        };
        menu_ui_render(elems_vel_fixed, sizeof(elems_vel_fixed)/sizeof(elems_vel_fixed[0]));
    }


    // On/Off (status-only, drawn outside elems so it never underlines)
    midi_display_on_off(save_get(MIDI_MODIFY_CURRENTLY_SENDING), LINE_4_VERT);

    screen_driver_UpdateScreen();

    (void)menu_nav_end_auto(UI_MIDI_MODIFY_SELECT);
}
