/*
 * _menu_ui.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#include "cmsis_os.h" //osDelay
#include "main.h"
#include "_menu_ui.h"
#include "_menu_controller.h"
#include "memory_main.h"
#include "screen_driver.h"
#include "utils.h"
#include "text.h"


static void draw_text(const char *s, int16_t x, int16_t y, ui_font_t font) {
    if (!s) return;
    switch (font) {
        case UI_6x8:   write_68(s, x, y); break;
        case UI_6x8_2: break;
        case UI_11x18: write_1118(s, x, y); break;
        case UI_16x24: break;
    }
}

static void draw_text_ul(const char *s, int16_t x, int16_t y, ui_font_t font, uint8_t ul) {
	if (!s) return;
    switch (font) {
        case UI_6x8:   write_underline_68(s, x, y, ul); break;
        case UI_6x8_2: write_underline_68_2(s, x, y, ul); break;
        case UI_11x18: write_underline_1118(s, x, y, ul); break;
        case UI_16x24: write_underline_1624(s, x, y, ul); break;
    }
}


static inline void draw_item_row(const ui_element *e)
{
    const save_field_t  f   = (save_field_t)e->save_item;
    const save_limits_t lim = save_limits[f];

    // save_get() is already clamped to [min..max]
    const int32_t v = save_get(f);

    // Compute table index
    uint32_t idx;
    if (lim.min < 0) {idx = (uint32_t)(v - lim.min);}
    else {idx = (uint32_t)v;}

    // Clamp to table range implied by limits
    const uint32_t last = (lim.min < 0)
                        ? (uint32_t)(lim.max - lim.min)
                        : (uint32_t) lim.max;
    if (idx > last) idx = last;

    const char *const *table = (const char *const *)e->text;
    draw_text_ul(table[idx], e->x, e->y, e->font, ui_is_field_selected(f) ? 1 : 0);
}


static inline uint8_t elem_is_visible(const ui_element *e, uint32_t active_groups_mask)
{
    if (e->ctrl_group_id == 0) return 1;
    uint8_t id = e->ctrl_group_id;
    if (id < 1 || id > 31) return 0;
    uint32_t bit = (1u << (id - 1));
    return (active_groups_mask & bit) ? 1u : 0u;
}

// -------------------------
// Individual menu drawing
// -------------------------

//Helpers

static void midi_display_on_off(uint8_t on_or_off, uint8_t bottom_line){
	draw_line(92, 10, 92, bottom_line);
	uint8_t text_position = bottom_line/2;
    const char *text_print = message->off_on[on_or_off];
	write_1118(text_print, 95, text_position);
}

//Individual menus

static void ui_tempo(menu_list_t field) {

  //Vertical line  right of BPM
  screen_driver_Line(64, 10, 64, 64, White);
  //Horizontal line above On / Off
  screen_driver_Line(0, 40, 64, 40, White);

}

static void ui_modify(menu_list_t field)    {

	midi_display_on_off(save_get(MODIFY_SENDING), LINE_4);
	//Bottom line above velocity
	draw_line(0, LINE_4, 127, LINE_4);

}

static void ui_transpose(menu_list_t field) {
	midi_display_on_off(save_get(TRANSPOSE_SENDING), 63);
}

static void ui_settings(menu_list_t field)  {
	saving_settings_ui();
	//Bottom line above save text
	draw_line(0, LINE_4, 127, LINE_4);
}

// One small tick per page (keeps update_menu tiny)
typedef void (*individual_menu_ui)(menu_list_t field);

static const individual_menu_ui ind_menu_ui[AMOUNT_OF_MENUS] = {
  [MIDI_TEMPO]     = ui_tempo,
  [MIDI_MODIFY]    = ui_modify,
  [MIDI_TRANSPOSE] = ui_transpose,
  [SETTINGS]       = ui_settings,
};





void menu_ui_render(menu_list_t menu, const ui_element *elems, size_t count) {

    const uint32_t active = ui_active_groups();

	screen_driver_Fill(Black);

    for (size_t i = 0; i < count; ++i) {
        const ui_element *e = &elems[i];

        // New: skip elements not in the active groups
        if (!elem_is_visible(e, active)) continue;

        switch (e->type) {
        case ELEM_TEXT:   draw_text(e->text, e->x, e->y, e->font); break;
        case ELEM_ITEM:   draw_item_row(e);                        break;
        case ELEM_16CH:   menu_ui_draw_16ch(e);                          break;
        default: break;
        }
    }

    //Separation line on top common to all menus
    draw_line(0, 10, 127, 10);

    ind_menu_ui[menu](menu);

    screen_driver_UpdateScreen();
}



void menu_ui_draw_text(const char *s, int16_t x, int16_t y, ui_font_t font) {
    draw_text(s, x, y, font);
}

void menu_ui_draw_text_ul(const char *s, int16_t x, int16_t y, ui_font_t font, uint8_t underline) {
    draw_text_ul(s, x, y, font, underline);
}





void menu_ui_draw_16ch(const ui_element *e) {
    const save_field_t f    = (save_field_t)e->save_item;
    const uint32_t     mask = (uint32_t)save_get(f);
    const int8_t       selb = ui_selected_bit(f);   // -1 if not selected

    // Use the element's text as the "on" glyph, defaulting to "X"
    const char *on_label = e->text ? e->text : "X";

    const int16_t base_x = (int16_t)e->x;
    const int16_t y1     = (int16_t)e->y;          // first row at LINE_*
    const int16_t y2     = (int16_t)(y1 + 10);     // second row exactly +10 px

    for (uint8_t i = 0; i < 16; ++i) {
        const char  *label = (mask & (1u << i)) ? on_label
                                                : message->one_to_sixteen_one_char[i];
        const int16_t x    = (int16_t)(base_x + 10 * (i % 8));
        const int16_t y    = (i < 8) ? y1 : y2;
        const uint8_t ul   = (selb == (int8_t)i) ? 1u : 0u;

        // Uses e->font (e.g., UI_6x8_2) to pick the correct underline writer
        draw_text_ul(label, x, y, e->font, ul);
    }
}


/* ---------------------------
 * Menu helpers
 * --------------------------- */



void menu_change_check(){
	 static uint8_t button_pressed = 0;
	  if(debounce_button(GPIOB, Btn4_Pin, &button_pressed, 50)){
		  ui_state_modify(CURRENT_MENU, UI_MODIFY_INCREMENT, 0);
	  }
}


/* ---------------------------
 * Saving helper
 * --------------------------- */

void saving_settings_ui(void){
    if (debounce_button(GPIOB, Btn1_Pin, NULL, 10)) {
		write_68(message->saving, TXT_LEFT, B_LINE);
		screen_driver_UpdateScreen();

		store_settings();

		write_68(message->saved, TXT_LEFT, B_LINE);
		screen_driver_UpdateScreen();
		osDelay(1000);
		write_68(message->save_instruction, TXT_LEFT, B_LINE);
		screen_driver_UpdateScreen();
    }
}




//Control functions using UI elements

void update_contrast_ui() {
    screen_driver_UpdateContrast();
}
