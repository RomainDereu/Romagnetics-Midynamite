/*
 * _menu_ui.h
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MENU_INC_MENU_UI_H_
#define MENU_INC_MENU_UI_H_

#include <stdint.h>
#include <stddef.h>

#include "_menu_controller.h" // for menu_list_t and save enums

// Y positions
#define LINE_0 0
#define LINE_1 15
#define LINE_2 25
#define LINE_3 35
#define LINE_4 45
#define B_LINE (LINE_4 + 3)

#define TXT_LEFT 5

// Fonts used by write_* functions
typedef enum {
    UI_6x8,
    UI_6x8_2,   // for MIDI 16 channels grid
    UI_11x18,
    UI_16x24
} ui_font_t;

// Element kinds
typedef enum {
    ELEM_TEXT,
    ELEM_ITEM,
    ELEM_16CH,
} ui_elem_type_t;

#define UI_CHOICE(tbl) ((const char*)(tbl))
#define UI_TEXT_NUM    ((const char*)-1)



// ---------------------
// API
// ---------------------
#ifndef UNIT_TEST

void initialize_screen(void);
void menu_ui_render(menu_list_t menu, const ui_element *elems, size_t count);
void menu_change_check(void);
void update_contrast_ui(void);

#else // UNIT_TEST

// No-op stubs for tests to avoid pulling display deps
static inline void initialize_screen(void) {}

static inline void menu_ui_render(menu_list_t menu,
                                  const ui_element *elems,
                                  size_t count)
{
    (void)menu; (void)elems; (void)count;
}

static inline void menu_change_check(void) {}
static inline void update_contrast_ui(void) {}

#endif // UNIT_TEST

#endif // MENU_INC_MENU_UI_H_
