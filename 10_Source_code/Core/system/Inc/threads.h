#ifndef THREADS_H_
#define THREADS_H_


#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct {
  TIM_HandleTypeDef *htim2; // tempo timer (start/stop)
  TIM_HandleTypeDef *htim3; // encoder A
  TIM_HandleTypeDef *htim4; // encoder B
} threads_ctx_t;

// Start all application threads (display, midi_core, medium_tasks)
void threads_start(const threads_ctx_t *ctx);

// Convenience (optional): notify display thread and/or fetch its handle
void        threads_display_notify(uint32_t flags);
osThreadId_t threads_display_handle(void);


#endif /* THREADS_H_ */
