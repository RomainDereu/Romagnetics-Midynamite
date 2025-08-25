/*
 * main.c
 *
 *      Author: Romain Dereu
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "memory_ui_state.h"
#include "menu.h"
#include "midi_tempo.h"
#include "midi_modify.h"
#include "memory_main.h"
#include "settings.h"
#include "utils.h"

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* Definitions for midi_core */
osThreadId_t midi_coreHandle;
const osThreadAttr_t midi_core_attributes = {
  .name = "midi_core",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for medium_tasks */
osThreadId_t medium_tasksHandle;
const osThreadAttr_t medium_tasks_attributes = {
  .name = "medium_tasks",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for display_update */
osThreadId_t display_updateHandle;
const osThreadAttr_t display_update_attributes = {
  .name = "display_update",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* USER CODE BEGIN PV */
//Romagnetics code
//midi receive
midi_modify_circular_buffer midi_modify_buff = {0};
uint8_t midi_uart_rx_byte;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
void MidiCore(void *argument);
void MediumTasks(void *argument);
void DisplayUpdate(void *argument);


int main(void)
{

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  screen_driver_Init();


  save_load_from_flash();
  uint8_t brightness = (uint8_t)save_get(SAVE_SETTINGS_BRIGHTNESS);
  const uint8_t contrast_values[10] =
      {0x39,0x53,0x6D,0x87,0xA1,0xBB,0xD5,0xEF,0xF9,0xFF};
  uint8_t new_contrast = contrast_values[brightness];
  screen_driver_SetContrast(new_contrast);


  if(save_get(SAVE_MIDI_TEMPO_CURRENTLY_SENDING) == 1){
	  mt_start_stop(&htim2);
  }

  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

  __HAL_TIM_SET_COUNTER(&htim3, ENCODER_CENTER);
  __HAL_TIM_SET_COUNTER(&htim4, ENCODER_CENTER);


  HAL_UART_Receive_IT(&huart2, &midi_uart_rx_byte, 1);



  osKernelInitialize();


  midi_coreHandle = osThreadNew(MidiCore, NULL, &midi_core_attributes);
  medium_tasksHandle = osThreadNew(MediumTasks, NULL, &medium_tasks_attributes);
  display_updateHandle = osThreadNew(DisplayUpdate, NULL, &display_update_attributes);
  osKernelStart();
}



void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}


static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 600-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4160;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}


static void MX_TIM3_Init(void)
{

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 10;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}


static void MX_TIM4_Init(void)
{

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 10;
  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }


}


static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 31250;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_HalfDuplex_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }

}


static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 31250;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

}


static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : Btn3_Pin Btn4_Pin Btn1_Pin Btn2_Pin */
  GPIO_InitStruct.Pin = Btn3_Pin|Btn4_Pin|Btn1_Pin|Btn2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_I2C2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){

  	if(huart->Instance == USART2){
  		midi_buffer_push(midi_uart_rx_byte);
  		HAL_UART_Receive_IT(&huart2, &midi_uart_rx_byte, 1);
  	}

}


void MidiCore(void *argument)
{

  MX_USB_DEVICE_Init();
  for(;;)
  {
  calculate_incoming_midi();
  osDelay(5);
  }
}



void MediumTasks(void *argument)
{

	ui_state_modify(UI_CURRENT_MENU, UI_MODIFY_SET, save_get(SAVE_SETTINGS_START_MENU));
	ui_state_modify(UI_OLD_MENU, UI_MODIFY_SET,99);


  for(;;)
  {
	menu_change_check();


	uint8_t current_menu = ui_state_get(UI_CURRENT_MENU);

	switch(current_menu) {
		case MIDI_TEMPO:
			midi_tempo_update_menu(&htim3, &htim4, display_updateHandle);
			break;

		case MIDI_MODIFY:
			midi_modify_update_menu(&htim3, &htim4, display_updateHandle);
			break;

		case MIDI_TRANSPOSE:
			midi_transpose_update_menu(&htim3, &htim4, display_updateHandle);
			break;

		case SETTINGS:
			settings_update_menu(&htim3, &htim4, display_updateHandle);
			break;

		default:
			break;
	}

	current_menu = ui_state_get(UI_CURRENT_MENU);
	ui_state_modify(UI_OLD_MENU, UI_MODIFY_SET,current_menu);




	static uint8_t OldBtn3State = 1;
	if(debounce_button(GPIOB, Btn3_Pin, &OldBtn3State, 50)){
		uint8_t current_menu = ui_state_get(UI_CURRENT_MENU);
		 switch (current_menu) {
		 	case MIDI_TEMPO:
		 		save_modify_u8(SAVE_MIDI_TEMPO_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
		 		mt_start_stop(&htim2);
		 		osThreadFlagsSet(display_updateHandle, FLAG_TEMPO);
		 		break;

		 	case MIDI_MODIFY:
		 		save_modify_u8(SAVE_MIDI_MODIFY_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
		 		osThreadFlagsSet(display_updateHandle, FLAG_MODIFY);
		 		break;

		 	case MIDI_TRANSPOSE:
		 		save_modify_u8(SAVE_TRANSPOSE_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
		 		osThreadFlagsSet(display_updateHandle, FLAG_TRANSPOSE);
		 		break;

		 	default:
		 		break;
		 }
	}


	// Check for panic button (both buttons held down)
	panic_midi(&huart1, &huart2, GPIOB, Btn1_Pin, Btn2_Pin);

	//Let other tasks update
	osDelay(10);

  }
  /* USER CODE END MediumTasks */
}



void DisplayUpdate(void *argument)
{
  for(;;)
  {
	  uint32_t displayFlags = osThreadFlagsWait(0x0F, osFlagsWaitAny, osWaitForever);

	  uint8_t current_menu = ui_state_get(UI_CURRENT_MENU);
	  switch (current_menu) {
	  	case MIDI_TEMPO:
	  		if (displayFlags & FLAG_TEMPO) {
	  			screen_update_midi_tempo();
	  		}
	  		break;

	  	case MIDI_MODIFY:
	  		if (displayFlags & FLAG_MODIFY) {
	  			screen_update_midi_modify();
	  		}
	  		break;

	  	case MIDI_TRANSPOSE:
	  		if (displayFlags & FLAG_TRANSPOSE) {
	  			screen_update_midi_transpose();
	  		}
	  		break;

	  	case SETTINGS:
	  		if (displayFlags & FLAG_SETTINGS) {
	  			screen_update_settings();
	  		}
	  		break;

	  	default:
	  		break;
	  }
     osDelay(30);
  }
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  if (htim->Instance == TIM2) {
	  uint8_t send_to_out     = save_get(SAVE_MIDI_TEMPO_SEND_TO_OUT);
	  uint32_t tempo_click_rate = save_get_u32(SAVE_MIDI_TEMPO_CLICK_RATE);
	  send_midi_tempo_out(tempo_click_rate, send_to_out);
  }
}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{


}
#endif /* USE_FULL_ASSERT */
