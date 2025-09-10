/*
 * _menu_ui.c
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#include "main.h"
#include "_menu_ui.h"
#include "_menu_controller.h"
#include "memory_main.h"
#include "screen_driver.h"
#include "utils.h"
#include "text.h"

extern const Message *message;



void menu_display(const char * menu_message){
	draw_line(0, 10, 127, 10);
	write_68(menu_message, TEXT_LEFT_START, 0);
}


void menu_change_check(){
	 static uint8_t button_pressed = 0;
	  if(debounce_button(GPIOB, Btn4_Pin, &button_pressed, 50)){
		  ui_state_modify(UI_CURRENT_MENU, UI_MODIFY_INCREMENT, 0);
	  }
}

static void draw_text(const char *s, int16_t x, int16_t y, ui_font_t font) {
    if (!s) return;
    switch (font) {
        case UI_FONT_6x8:   write_68(s, x, y); break;
        case UI_FONT_11x18: write_1118(s, x, y); break;
        case UI_FONT_16x24: break;
    }
}

static void draw_text_ul(const char *s, int16_t x, int16_t y, ui_font_t font, uint8_t ul) {
	if (!s) return;
    switch (font) {
        case UI_FONT_6x8:   write_underline_68(s, x, y, ul); break;
        case UI_FONT_11x18: write_underline_1118(s, x, y, ul); break;
        case UI_FONT_16x24: write_underline_1624(s, x, y, ul); break;
    }
}


static inline uint32_t ui_safe_index(save_field_t f, int32_t v)
{
    const save_limits_t lim = save_limits[f];
    const uint32_t span = (uint32_t)(lim.max - lim.min);   // inclusive span, e.g. 160 for -80..+80
    uint32_t idx = (uint32_t)(v - lim.min);                // normalize so min maps to 0
    if (idx > span) idx = span;                            // clamp to max valid index
    return idx;
}

static inline void draw_item_row(const ui_element *e)
{
    const save_field_t f = (save_field_t)e->save_item;
    const uint32_t idx = ui_safe_index(f, save_get(f));    // save_get already clamps to limits
    const char *const *table = (const char *const *)e->text;
    draw_text_ul(table[idx], e->x, e->y, e->font, ui_is_field_selected(f) ? 1 : 0);
}



void menu_ui_render(const ui_element *elems, size_t count) {
    if (!elems || count == 0) return;

    for (size_t i = 0; i < count; ++i) {
        const ui_element *e = &elems[i];
        switch (e->type) {
            case UI_ELEM_TEXT:
                draw_text(e->text, e->x, e->y, e->font);
                break;

            case UI_ELEM_ITEM:
                // Arrays you pass are already mode-filtered; just draw.
                draw_item_row(e);
                break;

            default: break;
        }
    }
}


void menu_ui_draw_text(const char *s, int16_t x, int16_t y, ui_font_t font) {
    draw_text(s, x, y, font);
}

void menu_ui_draw_text_ul(const char *s, int16_t x, int16_t y, ui_font_t font, uint8_t underline) {
    draw_text_ul(s, x, y, font, underline);
}

void menu_ui_draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    draw_line(x1, y1, x2, y2);
}

void menu_ui_draw_title(const char *title) {
    menu_display(title);
}


