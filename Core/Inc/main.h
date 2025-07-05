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
	uint8_t currently_sending;
	uint8_t send_channels;
}midi_tempo_data_struct;


typedef struct {
	uint8_t change_or_split;
	uint8_t velocity_type;

	uint8_t send_to_midi_channel;
	uint8_t send_to_midi_out;

	uint8_t split_note;
	uint8_t split_midi_channel_1;
	uint8_t split_midi_channel_2;

	int32_t velocity_plus_minus;
	uint8_t velocity_absolute;

}midi_modify_data_struct;

typedef struct {
	uint8_t transpose_type;

	int8_t midi_shift_value;

	uint8_t transpose_base_note;
	uint8_t transpose_scale;
	uint8_t transpose_interval;

	uint8_t send_channels;


}midi_transpose_data_struct;


typedef struct {
	uint8_t start_menu;
	uint8_t brightness;
}settings_data_struct;


typedef struct {
	midi_tempo_data_struct midi_tempo_data;
	midi_modify_data_struct midi_modify_data;
	midi_transpose_data_struct midi_transpose_data;
	settings_data_struct settings_data;
	uint32_t check_data_validity;
}save_struct;

#define MIDI_MODIFY_BUFFER_SIZE 256

typedef struct {
    uint8_t data[MIDI_MODIFY_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} midi_modify_circular_buffer;


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

#define DATA_VALIDITY_CHECKSUM 13751

//Menu list
#define MIDI_TEMPO 0
#define MIDI_MODIFY 1
#define SETTINGS 2

#define AMOUNT_OF_MENUS 3


//midi_tempo_data
#define MIDI_OUT_1 1
#define MIDI_OUT_2 2
#define MIDI_OUT_1_2 3
#define MIDI_OUT_USB 4
#define MIDI_OUT_USB_1 5
#define MIDI_OUT_USB_2 6
#define MIDI_OUT_USB_1_2 7

//midi_modify_data
#define MIDI_MODIFY_CHANGE 0
#define MIDI_MODIFY_SPLIT 1

//velocity_type
#define MIDI_MODIFY_CHANGED_VEL 0
#define MIDI_MODIFY_FIXED_VEL 1

//midi_transpose_data
#define MIDI_TRANSPOSE_SHIFT 0
#define MIDI_TRANSPOSE_SCALED 1

#define IONIAN 0
#define DORIAN 1
#define PHRYGIAN 2
#define LYDIAN 3
#define MIXOLYDIAN 4
#define AEOLIAN 5
#define LOCRIAN 6
#define AMOUNT_OF_MODES 7

#define SIXTH_DOWN 0
#define FIFTH_DOWN 1
#define FOURTH_DOWN 2
#define THIRD_DOWN 3
#define THIRD_UP 4
#define FOURTH_UP 5
#define FIFTH_UP 5
#define SIXTH_UP 5

#define FLASH_SECTOR7_ADDR  ((uint32_t)0x08060000)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
