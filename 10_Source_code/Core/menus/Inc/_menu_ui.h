/*
 * _menu_ui.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MENU_INC_MENU_UI_H_
#define MENU_INC_MENU_UI_H_


#include <stdint.h>
#include <stddef.h>

#define LINE_0 0
#define LINE_1 15
#define LINE_2 25
#define LINE_3 35
#define LINE_4 45
#define BOTTOM_LINE LINE_4 + 3

#define TXT_LEFT 5



// Fonts you already use via write_* functions
typedef enum {
    UI_6x8,
    UI_11x18,
    UI_16x24
} ui_font_t;

// What to render
typedef enum {
    ELEM_TEXT,
	ELEM_ITEM,
} ui_elem_type_t;

#define UI_CHOICE(tbl) ((const char*)(tbl))
#define UI_TEXT_NUM    ((const char*)-1)

typedef struct {
    ui_elem_type_t type;
    uint8_t save_item;
    const char *text;
    ui_font_t font;
    int16_t x;
    int16_t y;
    uint8_t ctrl_group_id;
} ui_element;



// ---------------------
// API
// ---------------------
void menu_display(const char * menu_message);
void midi_display_on_off(uint8_t on_or_off, uint8_t bottom_line);

void menu_change_check(void);


// Render an array of elements in order.
void menu_ui_render(const ui_element *elems, size_t count);

// Convenience helpers (optional)
void menu_ui_draw_text(const char *s, int16_t x, int16_t y, ui_font_t font);
void menu_ui_draw_text_ul(const char *s, int16_t x, int16_t y, ui_font_t font, uint8_t underline);
void menu_ui_draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
void menu_ui_draw_title(const char *title);

void filter_controller_ui(uint32_t mask, uint8_t base_idx, uint8_t sel);



#endif /* MENU_INC_MENU_UI_H_ */
