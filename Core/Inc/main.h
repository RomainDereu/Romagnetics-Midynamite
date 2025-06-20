/* USER CODE BEGIN Header */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */




typedef struct {
	uint32_t current_tempo;
	uint32_t currently_sending;
	uint8_t send_channels;
}midi_tempo_data_struct;

typedef struct {
	midi_tempo_data_struct midi_tempo_data;
	uint32_t check_data_validity;
}save_struct;


/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Btn3_Pin GPIO_PIN_12
#define Btn3_GPIO_Port GPIOB
#define Btn4_Pin GPIO_PIN_13
#define Btn4_GPIO_Port GPIOB
#define Btn1_Pin GPIO_PIN_14
#define Btn1_GPIO_Port GPIOB
#define Btn2_Pin GPIO_PIN_15
#define Btn2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define ENCODER_CENTER     32768
#define ENCODER_THRESHOLD  4


#define MIDI_TEMPO 0
#define MIDI_MODIFY 1
#define SETTINGS 2

#define MIDI_OUT_1 1
#define MIDI_OUT_2 2
#define MIDI_OUT_1_2 3
#define MIDI_OUT_USB 4
#define MIDI_OUT_USB_1 5
#define MIDI_OUT_USB_2 6
#define MIDI_OUT_USB_1_2 7


#define FLASH_SECTOR7_ADDR  ((uint32_t)0x08060000)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
