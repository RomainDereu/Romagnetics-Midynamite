#ifndef __screen_driver_H__
#define __screen_driver_H__

typedef struct { int dummy; } FontDef;

static inline void screen_driver_Fill(int c) { (void)c; }
static inline void screen_driver_SetCursor_WriteString(const char* str, FontDef font, int color, int x, int y)
{ (void)str; (void)font; (void)color; (void)x; (void)y; }
static inline void screen_driver_UpdateScreen(void) {}

#endif // __screen_driver_H__