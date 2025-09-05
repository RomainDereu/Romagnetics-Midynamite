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

/* ---------- helpers ---------- */

/* number of interactive rows derived from UI_GROUP_SETTINGS */
static inline uint8_t settings_row_count(void) {
    return build_select_states(UI_GROUP_SETTINGS, /*current_select=*/0, /*states=*/NULL, /*cap=*/0);
}

/* total selectable items = interactive rows + 1 "About" row */
static inline uint8_t settings_total_items(void) {
    return (uint8_t)(settings_row_count() + 1);
}

/* ---------- section renderers ---------- */

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
    write_underline_68_2(message->choices.off_on[save_get(SETTINGS_CHANNEL_FILTER)], 80, LINE_3_VERT, select_states[idx++]);
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

        write_underline_68(label, x, y, underline);
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
    const uint8_t rows  = settings_row_count();
    const uint8_t total = (uint8_t)(rows + 1); // +1 About
    uint8_t current_select = menu_nav_get_select(UI_SETTINGS_SELECT);
    if (current_select >= total) current_select = (uint8_t)(total - 1);

    // Build underline map only for interactive rows
    uint8_t select_states[rows ? rows : 1];
    if (rows) {
        (void)build_select_states(UI_GROUP_SETTINGS, current_select, select_states, rows);
    }

    screen_driver_Fill(Black);

    if (current_select < rows) {
        // Decide which section to render based on the selected index mapping
        // These ranges mirror your previous screen grouping:
        if (current_select >= (SETTINGS_FIRST_GLOBAL1 - SETTINGS_START_MENU) &&
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
            // Fallback if grouping changes
            screen_update_global_settings1(select_states);
        }
    } else {
        // “About” row
        screen_update_settings_about();
    }

    draw_line(0, LINE_4_VERT, 127, LINE_4_VERT);
    write_68(message->save_instruction, TEXT_LEFT_START, BOTTOM_LINE_VERT);
    screen_driver_UpdateScreen();
}

/* ---------- save sequence ---------- */

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

/* ---------- controller ---------- */

void settings_update_menu(void)
{
    // Begin frame: track only active settings fields
    menu_nav_begin(UI_GROUP_SETTINGS);

    const uint8_t rows  = settings_row_count();
    const uint8_t total = (uint8_t)(rows + 1); // +1 About
    uint8_t current_select = menu_nav_update_and_get(
        UI_SETTINGS_SELECT,
        /*min=*/0,
        /*max=*/(uint8_t)(total - 1),
        /*step=*/1,
        /*wrap=*/WRAP
    );

    // Drive the selected row only if it’s an interactive settings item
    if (current_select < rows) {
        toggle_underline_items(UI_GROUP_SETTINGS, current_select);
    }

    // Save on Btn1
    if (debounce_button(GPIOB, Btn1_Pin, NULL, 10)) {
        saving_settings_ui();
    }

    // End frame: if selection changed or any tracked field changed, repaint
    if (menu_nav_end(UI_SETTINGS_SELECT, UI_GROUP_SETTINGS, current_select)) {
        threads_display_notify(FLAG_SETTINGS);
    }
}
