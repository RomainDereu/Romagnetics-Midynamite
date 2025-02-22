/**
 * Private configuration file for the screen_driver library.
 * This example is configured for STM32F0, I2C and including all fonts.
 */

#ifndef __screen_driver_CONF_H__
#define __screen_driver_CONF_H__
#include "main.h"

// Choose a microcontroller family
//#define STM32F0
//#define STM32F1
#define STM32F4
//#define STM32L0
//#define STM32L1
//#define STM32L4
//#define STM32F3
//#define STM32H7
//#define STM32F7
//#define STM32G0

// Choose a bus
//#define screen_driver_USE_I2C
#define screen_driver_USE_SPI

// I2C Configuration
// #define screen_driver_I2C_PORT        hi2c1
// #define screen_driver_I2C_ADDR        (0x3C << 1)

// SPI Configuration
#define screen_driver_SPI_PORT        hspi1
#define screen_driver_CS_Port         OLED_CS_GPIO_Port
#define screen_driver_CS_Pin          OLED_CS_Pin
#define screen_driver_DC_Port         OLED_DC_GPIO_Port
#define screen_driver_DC_Pin          OLED_DC_Pin
#define screen_driver_Reset_Port      OLED_Res_GPIO_Port
#define screen_driver_Reset_Pin       OLED_Res_Pin

// Mirror the screen if needed
//#define screen_driver_MIRROR_VERT
//#define screen_driver_MIRROR_HORIZ

// Set inverse color if needed
// # define screen_driver_INVERSE_COLOR

// Include only needed fonts
#define screen_driver_INCLUDE_FONT_6x8
#define screen_driver_INCLUDE_FONT_7x10
#define screen_driver_INCLUDE_FONT_11x18
#define screen_driver_INCLUDE_FONT_16x26
#define screen_driver_INCLUDE_FONT_16x24
#define screen_driver_INCLUDE_FONT_16x15

// The width of the screen can be set using this
// define. The default value is 128.
#define screen_driver_WIDTH           128

// If your screen horizontal axis does not start
// in column 0 you can use this define to
// adjust the horizontal offset
// #define screen_driver_X_OFFSET

// The height can be changed as well if necessary.
// It can be 32, 64 or 128. The default value is 64.
#define screen_driver_HEIGHT          128

#endif /* __screen_driver_CONF_H__ */
