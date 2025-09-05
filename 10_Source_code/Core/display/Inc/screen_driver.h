/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 *
 * https://github.com/afiskon/stm32-screen_driver
 *
 * Further modifications by Romain Dereu to implement inside Midynamite
 *
 *
 */

#ifndef __screen_driver_H__
#define __screen_driver_H__

#include <stddef.h>
#include <stdint.h>
#include <_ansi.h>

_BEGIN_STD_C

#include "screen_driver_conf.h"


#if defined(STM32WB)
#include "stm32wbxx_hal.h"
#elif defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#elif defined(STM32L0)
#include "stm32l0xx_hal.h"
#elif defined(STM32L1)
#include "stm32l1xx_hal.h"
#elif defined(STM32L4)
#include "stm32l4xx_hal.h"
#elif defined(STM32L5)
#include "stm32l5xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#elif defined(STM32F7)
#include "stm32f7xx_hal.h"
#elif defined(STM32G0)
#include "stm32g0xx_hal.h"
#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#else
#error "screen_driver library was tested only on STM32F0, STM32F1, STM32F3, STM32F4, STM32F7, STM32L0, STM32L1, STM32L4, STM32H7, STM32G0, STM32G4, STM32WB MCU families. Please modify screen_driver.h if you know what you are doing. Also please send a pull request if it turns out the library works on other MCU's as well!"
#endif


#ifdef screen_driver_X_OFFSET
#define screen_driver_X_OFFSET_LOWER (screen_driver_X_OFFSET & 0x0F)
#define screen_driver_X_OFFSET_UPPER ((screen_driver_X_OFFSET >> 4) & 0x07)
#else
#define screen_driver_X_OFFSET_LOWER 0
#define screen_driver_X_OFFSET_UPPER 0
#endif

/* vvv I2C config vvv */

#ifndef screen_driver_I2C_PORT
#define screen_driver_I2C_PORT        hi2c1
#endif

#ifndef screen_driver_I2C_ADDR
#define screen_driver_I2C_ADDR        (0x3C << 1)
#endif

/* ^^^ I2C config ^^^ */

/* vvv SPI config vvv */

#ifndef screen_driver_SPI_PORT
#define screen_driver_SPI_PORT        hspi2
#endif

#ifndef screen_driver_CS_Port
#define screen_driver_CS_Port         GPIOB
#endif
#ifndef screen_driver_CS_Pin
#define screen_driver_CS_Pin          GPIO_PIN_12
#endif

#ifndef screen_driver_DC_Port
#define screen_driver_DC_Port         GPIOB
#endif
#ifndef screen_driver_DC_Pin
#define screen_driver_DC_Pin          GPIO_PIN_14
#endif

#ifndef screen_driver_Reset_Port
#define screen_driver_Reset_Port      GPIOA
#endif
#ifndef screen_driver_Reset_Pin
#define screen_driver_Reset_Pin       GPIO_PIN_8
#endif

/* ^^^ SPI config ^^^ */

#if defined(screen_driver_USE_I2C)
extern I2C_HandleTypeDef screen_driver_I2C_PORT;
#elif defined(screen_driver_USE_SPI)
extern SPI_HandleTypeDef screen_driver_SPI_PORT;
#else
#error "You should define screen_driver_USE_SPI or screen_driver_USE_I2C macro!"
#endif

// screen_driver OLED height in pixels
#ifndef screen_driver_HEIGHT
#define screen_driver_HEIGHT          64
#endif

// screen_driver width in pixels
#ifndef screen_driver_WIDTH
#define screen_driver_WIDTH           128
#endif

#ifndef screen_driver_BUFFER_SIZE
#define screen_driver_BUFFER_SIZE   screen_driver_WIDTH * screen_driver_HEIGHT / 8
#endif

// Enumeration for screen colors
typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set. Color depends on OLED
} screen_driver_COLOR;

typedef enum {
    screen_driver_OK = 0x00,
    screen_driver_ERR = 0x01  // Generic error.
} screen_driver_Error_t;

// Struct to store transformations
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Initialized;
    uint8_t DisplayOn;
} screen_driver_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} screen_driver_VERTEX;

/** Font */
typedef struct {
	const uint8_t width;                /**< Font width in pixels */
	const uint8_t height;               /**< Font height in pixels */
	const uint16_t *const data;         /**< Pointer to font data array */
    const uint8_t *const char_width;    /**< Proportional character width in pixels (NULL for monospaced) */
} screen_driver_Font_t;

// Procedure definitions
void screen_driver_Init(void);
void screen_driver_Fill(screen_driver_COLOR color);
void screen_driver_UpdateScreen(void);
void screen_driver_DrawPixel(uint8_t x, uint8_t y, screen_driver_COLOR color);
char screen_driver_WriteChar(char ch, screen_driver_Font_t Font, screen_driver_COLOR color);

void screen_driver_SetCursor(uint8_t x, uint8_t y);

void screen_driver_DrawArc(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, screen_driver_COLOR color);
void screen_driver_DrawArcWithRadiusLine(uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, screen_driver_COLOR color);
void screen_driver_DrawCircle(uint8_t par_x, uint8_t par_y, uint8_t par_r, screen_driver_COLOR color);
void screen_driver_FillCircle(uint8_t par_x,uint8_t par_y,uint8_t par_r,screen_driver_COLOR par_color);
void screen_driver_Polyline(const screen_driver_VERTEX *par_vertex, uint16_t par_size, screen_driver_COLOR color);
void screen_driver_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, screen_driver_COLOR color);
void screen_driver_FillRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, screen_driver_COLOR color);





void write_68(const char* str, uint8_t x_align, uint8_t y_align);
void write_1118(const char* str, uint8_t x_align, uint8_t y_align);

void write_underline_68(const char* str, uint8_t x_align, uint8_t y_align, uint8_t underlined);
void write_underline_68_2(const char* str, uint8_t x_align, uint8_t y_align, uint8_t underlined);
void write_underline_1624(const char* str, uint8_t x_align, uint8_t y_align, uint8_t underlined);

void draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);





/**
 * @brief Invert color of pixels in rectangle (include border)
 * 
 * @param x1 X Coordinate of top left corner
 * @param y1 Y Coordinate of top left corner
 * @param x2 X Coordinate of bottom right corner
 * @param y2 Y Coordinate of bottom right corner
 * @return screen_driver_Error_t status
 */
screen_driver_Error_t screen_driver_InvertRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

void screen_driver_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, screen_driver_COLOR color);

/**
 * @brief Sets the contrast of the display.
 * @param[in] value contrast to set.
 * @note Contrast increases as the value increases.
 * @note RESET = 7Fh.
 */
void screen_driver_SetContrast(const uint8_t value);

void screen_driver_UpdateContrast();

/**
 * @brief Set Display ON/OFF.
 * @param[in] on 0 for OFF, any for ON.
 */
void screen_driver_SetDisplayOn(const uint8_t on);

/**
 * @brief Reads DisplayOn state.
 * @return  0: OFF.
 *          1: ON.
 */
uint8_t screen_driver_GetDisplayOn();

// Low-level procedures
void screen_driver_Reset(void);
void screen_driver_WriteCommand(uint8_t byte);
void screen_driver_WriteData(uint8_t* buffer, size_t buff_size);
screen_driver_Error_t screen_driver_FillBuffer(uint8_t* buf, uint32_t len);

_END_STD_C

#endif // __screen_driver_H__
