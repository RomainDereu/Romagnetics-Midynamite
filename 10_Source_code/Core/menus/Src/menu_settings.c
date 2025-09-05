/*
 * menu_settings.c
 *
 *  Created on: Jun 25, 2025
 *      Author: Romain Dereu
 */
#include <string.h>
#include <stdio.h>

#include "menu_settings.h"

#include "memory_ui_state.h"
#include "memory_main.h"
#include "screen_driver.h"

// under_here_header_checks
#include "menu.h"
#include "utils.h"
#include "text.h"
#include "threads.h"

#define SETTINGS_FIRST_GLOBAL1  SETTINGS_START_MENU
#define SETTINGS_LAST_GLOBAL1   SETTINGS_BRIGHTNESS

#define SETTINGS_FIRST_GLOBAL2  SETTINGS_MIDI_THRU
#define SETTINGS_LAST_GLOBAL2   SETTINGS_CHANNEL_FILTER

#define SETTINGS_FIRST_FILTER   SETTINGS_FILTERED_CHANNELS

extern const Message *message;


static void saving_settings_ui(void){
    write_68(message->saving, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    screen_driver_UpdateScreen();

    save_struct s;
    s.midi_tempo_data     = save_snapshot_tempo();
    s.midi_modify_data    = save_snapshot_modify();
    s.midi_transpose_data = save_snapshot_transpose();
    s.settings_data       = save_snapshot_settings();
    s.check_data_validity = DATA_VALIDITY_CHECKSUM;

    (void)store_settings(&s);

    write_68(message->saved, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    screen_driver_UpdateScreen();
    osDelay(1000);
    write_68(message->save_instruction, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    screen_driver_UpdateScreen();
}



void settings_update_menu(void)
{
    ui_group_t group = UI_GROUP_SETTINGS;
    menu_nav_begin(group);
    uint8_t current_select = update_select(UI_SETTINGS_SELECT, group, /*tail_extra=*/0, /*mult=*/1, WRAP);

    if (debounce_button(GPIOB, Btn1_Pin, NULL, 10)) {
        saving_settings_ui();
    }

    toggle_underline_items(group, current_select);

    if (menu_nav_end(UI_SETTINGS_SELECT, group, current_select)) {
        threads_display_notify(FLAG_SETTINGS);
    }
}




static void screen_update_global_settings1(uint8_t *select_states){
    menu_display(message->global_settings_1);
    uint8_t idx = (uint8_t)(SETTINGS_FIRST_GLOBAL1 - SETTINGS_START_MENU);

    // Start Menu
    write_68(message->start_menu, TEXT_LEFT_START, LINE_1_VERT);
    const char *start_menu_options[] = {
        message->tempo, message->modify, message->transpose, message->settings,
    };
    write_underline_68(start_menu_options[save_get(SETTINGS_START_MENU)], 70, LINE_1_VERT, select_states[idx++]);

    // Send to USB
    write_68(message->usb_midi, TEXT_LEFT_START, LINE_2_VERT);
    write_underline_68(message->choices.usb_receive_send[save_get(SETTINGS_SEND_USB)], 70, LINE_2_VERT, select_states[idx++]);

    // Contrast
    write_68(message->contrast, TEXT_LEFT_START, LINE_3_VERT);
    uint8_t brightness_val = save_get(SETTINGS_BRIGHTNESS);
    if (brightness_val > 9) brightness_val = 9;
    write_underline_68(message->contrast_levels[brightness_val], 70, LINE_3_VERT, select_states[idx++]);
}

static void screen_update_global_settings2(uint8_t *select_states){
    menu_display(message->global_settings_2);
    uint8_t idx = (uint8_t)(SETTINGS_FIRST_GLOBAL2 - SETTINGS_START_MENU);

    // MIDI THRU
    write_68(message->MIDI_Thru, TEXT_LEFT_START, LINE_1_VERT);
    write_underline_68(message->choices.off_on[save_get(SETTINGS_MIDI_THRU)], 80, LINE_1_VERT, select_states[idx++]);

    // USB THRU
    write_68(message->USB_Thru, TEXT_LEFT_START, LINE_2_VERT);
    write_underline_68(message->choices.off_on[save_get(SETTINGS_USB_THRU)], 80, LINE_2_VERT, select_states[idx++]);

    // MIDI filter enable
    write_68(message->MIDI_Filter, TEXT_LEFT_START, LINE_3_VERT);
    write_underline_68(message->choices.off_on[save_get(SETTINGS_CHANNEL_FILTER)], 80, LINE_3_VERT, select_states[idx++]);
}

static void screen_update_midi_filter(uint8_t *select_states)
{
    menu_display(message->MIDI_Filter);

    write_68(message->X_equals_ignore_channel, TEXT_LEFT_START, LINE_1_VERT);

    uint32_t mask = save_get_u32(SETTINGS_FILTERED_CHANNELS);
    for (uint8_t i = 0; i < 16; i++) {
        const char *label = (mask & ((uint32_t)1 << i)) ? "X" : message->one_to_sixteen[i];

        uint8_t x = (uint8_t)(5 + 10 * (i % 8));
        uint8_t y = (i < 8) ? LINE_2_VERT : LINE_3_VERT;

        uint8_t base_idx  = (uint8_t)(SETTINGS_FIRST_FILTER - SETTINGS_START_MENU);
        uint8_t underline = select_states[base_idx + i];

        write_underline_68_2(label, x, y, underline);
    }
}

static void screen_update_settings_about(void){
    menu_display(message->about);
    write_68(message->about_brand,   TEXT_LEFT_START, LINE_1_VERT);
    write_68(message->about_product, TEXT_LEFT_START, LINE_2_VERT);
    write_68(message->about_version, TEXT_LEFT_START, LINE_3_VERT);
}

/* ---------- top-level screen paint ---------- */

void screen_update_settings(void)
{
    // rows now includes ABOUT because SETTINGS_ABOUT is in UI_GROUP_SETTINGS
    const uint8_t rows = build_select_states(UI_GROUP_SETTINGS, /*current_select=*/0, /*states=*/NULL, /*cap=*/0);
    uint8_t current_select = ui_state_get(UI_SETTINGS_SELECT);
    if (rows == 0) current_select = 0;
    else if (current_select >= rows) current_select = (uint8_t)(rows - 1);

    uint8_t select_states[rows ? rows : 1];
    if (rows) {
        (void)build_select_states(UI_GROUP_SETTINGS, current_select, select_states, rows);
    }

    screen_driver_Fill(Black);

    // index of ABOUT within the settings rows
    const uint8_t about_idx = (uint8_t)(SETTINGS_ABOUT - SETTINGS_START_MENU);

    if (rows && current_select == about_idx) {
        // ABOUT row (now a real item)
        screen_update_settings_about();
    }
    else if (current_select >= (SETTINGS_FIRST_GLOBAL1 - SETTINGS_START_MENU) &&
             current_select <= (SETTINGS_LAST_GLOBAL1   - SETTINGS_START_MENU)) {
        screen_update_global_settings1(select_states);
    }
    else if (current_select >= (SETTINGS_FIRST_GLOBAL2 - SETTINGS_START_MENU) &&
             current_select <= (SETTINGS_LAST_GLOBAL2   - SETTINGS_START_MENU)) {
        screen_update_global_settings2(select_states);
    }
    else if (current_select >= (SETTINGS_FIRST_FILTER - SETTINGS_START_MENU) &&
             current_select <  (SETTINGS_FIRST_FILTER - SETTINGS_START_MENU) + 16) {
        screen_update_midi_filter(select_states);
    }
    else {
        // fallback if grouping shifts
        screen_update_global_settings1(select_states);
    }

    draw_line(0, LINE_4_VERT, 127, LINE_4_VERT);
    write_68(message->save_instruction, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    screen_driver_UpdateScreen();
}
