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
	//Modified in the tempo menu
	int32_t current_tempo;
	int32_t tempo_click_rate;
	uint8_t currently_sending;
	uint8_t send_to_midi_out;
}midi_tempo_data_struct;


typedef struct {
	//Modified in the settings
	uint8_t change_or_split;
	uint8_t velocity_type;
	uint8_t send_to_midi_out;

	//Modified in the menu
	uint8_t send_to_midi_channel_1;
	uint8_t send_to_midi_channel_2;


	uint8_t split_note;
	uint8_t split_midi_channel_1;
	uint8_t split_midi_channel_2;

	int32_t velocity_plus_minus;
	uint8_t velocity_absolute;

	uint8_t currently_sending;


}midi_modify_data_struct;

typedef struct {
	uint8_t transpose_type;

	int32_t midi_shift_value;
	uint8_t send_original;

	uint8_t transpose_base_note;
	uint8_t transpose_interval;
	uint8_t transpose_scale;

	uint8_t currently_sending;

}midi_transpose_data_struct;


typedef struct {
	uint8_t start_menu;
	uint8_t send_to_usb;
	uint8_t brightness;
	uint8_t channel_filter;

	uint8_t midi_thru;
	uint8_t usb_thru;
	uint16_t filtered_channels;

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

typedef struct {
    uint8_t midi_tempo_current_select;
    uint8_t settings_current_select;
    uint8_t current_menu;
} ui_state_t;



/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define MIDI_IN_PORTS_NUM   0x01
#define MIDI_OUT_PORTS_NUM  0x01
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

#define DATA_VALIDITY_CHECKSUM 17491
#define FLASH_SECTOR7_ADDR  ((uint32_t)0x08060000)

//Menu list
typedef enum {
    MIDI_TEMPO = 0,
    MIDI_MODIFY,
    MIDI_TRANSPOSE,
    SETTINGS,
    AMOUNT_OF_MENUS
} menu_list_t;


typedef enum {
	MIDI_OUT_1,
	MIDI_OUT_2,
	MIDI_OUT_1_2,
	MIDI_OUT_SPLIT
} midi_outs_t;

//midi_modify_data
typedef enum {
    MIDI_MODIFY_CHANGE = 0,
    MIDI_MODIFY_SPLIT
} midi_modify_change_split_t;

//velocity_type
typedef enum {
    MIDI_MODIFY_CHANGED_VEL = 0,
    MIDI_MODIFY_FIXED_VEL
} midi_modify_velocity_t;

//midi_transpose_data
typedef enum {
    MIDI_TRANSPOSE_SHIFT = 0,
    MIDI_TRANSPOSE_SCALED
} midi_transpose_types_t;

typedef enum {
    IONIAN = 0,
    DORIAN,
    PHRYGIAN,
    LYDIAN,
    MIXOLYDIAN,
    AEOLIAN,
    LOCRIAN,
    AMOUNT_OF_MODES
} modes_t;

typedef enum {
    OCTAVE_DOWN = 0,
    SIXTH_DOWN,
    FIFTH_DOWN,
    FOURTH_DOWN,
    THIRD_DOWN,
    THIRD_UP,
    FOURTH_UP,
    FIFTH_UP,
    SIXTH_UP,
    OCTAVE_UP
} intervals_t;


//USB
typedef enum {
    USB_MIDI_OFF,
	USB_MIDI_SEND,
} midi_mode_t;


//Sending
typedef enum {
    NOT_SENDING,
	SENDING,
} sending_t;

//Sending
typedef enum {
    FALSE,
	TRUE,
} boolean_t;


typedef enum {
    FLAG_TEMPO      = (1 << 0),
    FLAG_MODIFY     = (1 << 1),
    FLAG_TRANSPOSE  = (1 << 2),
    FLAG_SETTINGS   = (1 << 3)
} DisplayFlags_t;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
