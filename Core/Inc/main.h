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

struct midi_tempo_data_struct{
	uint32_t current_tempo;
	uint32_t currently_sending;
};

struct save_struct{
	struct midi_tempo_data_struct midi_tempo_data;
	uint32_t check_data_validity;
};


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


#define MIDI_TEMPO 0
#define MIDI_MODIFY 1
#define SETTINGS 2

#define FLASH_SECTOR7_ADDR  ((uint32_t)0x08060000)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
