#ifndef __screen_driver_FONTS_H__
#define __screen_driver_FONTS_H__

#include "screen_driver.h"

#ifdef screen_driver_INCLUDE_FONT_6x8
extern const screen_driver_Font_t Font_6x8;
#endif
#ifdef screen_driver_INCLUDE_FONT_7x10
extern const screen_driver_Font_t Font_7x10;
#endif
#ifdef screen_driver_INCLUDE_FONT_11x18
extern const screen_driver_Font_t Font_11x18;
#endif
#ifdef screen_driver_INCLUDE_FONT_16x26
extern const screen_driver_Font_t Font_16x26;
#endif
#ifdef screen_driver_INCLUDE_FONT_16x24
extern const screen_driver_Font_t Font_16x24;
#endif
#ifdef screen_driver_INCLUDE_FONT_16x15
/** Generated Roboto Thin 15 
 * @copyright Google https://github.com/googlefonts/roboto
 * @license This font is licensed under the Apache License, Version 2.0.
*/
extern const screen_driver_Font_t Font_16x15;
#endif

#endif // __screen_driver_FONTS_H__
