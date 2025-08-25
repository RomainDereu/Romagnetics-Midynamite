/*
 * main.h
 *
 *      Author: Romain Dereu
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"


#define MIDI_MODIFY_BUFFER_SIZE 256

typedef struct {
    uint8_t data[MIDI_MODIFY_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} midi_modify_circular_buffer;


#define MIDI_IN_PORTS_NUM   0x01
#define MIDI_OUT_PORTS_NUM  0x01

void Error_Handler(void);



#define Btn3_Pin GPIO_PIN_12
#define Btn3_GPIO_Port GPIOB
#define Btn4_Pin GPIO_PIN_13
#define Btn4_GPIO_Port GPIOB
#define Btn1_Pin GPIO_PIN_14
#define Btn1_GPIO_Port GPIOB
#define Btn2_Pin GPIO_PIN_15
#define Btn2_GPIO_Port GPIOB



#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
