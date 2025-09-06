#include "main.h"
#include "memory_ui_state.h"
#include "memory_main.h"
#include "menu.h" //menu change check

#include "menu_tempo.h"
#include "menu_modify.h"
#include "menu_settings.h"

#include "midi_tempo.h"
#include "midi_transform.h"

#include "stm32f4xx_hal.h"  // HAL types
#include "threads.h"
#include "usb_device.h"
#include "utils.h"

// Timers used inside threads (owned/initialized by CubeMX in main.c)
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

// -------------------------
// Internal state
// -------------------------
static osThreadId_t s_display_handle      = NULL;
static osThreadId_t s_midi_core_handle    = NULL;
static osThreadId_t s_medium_tasks_handle = NULL;

static osEventFlagsId_t s_display_flags;
static const osEventFlagsAttr_t s_flags_attrs = { .name = "display_flags" };

#ifndef DISPLAY_FLAG_MASK
#define DISPLAY_FLAG_MASK (FLAG_TEMPO | FLAG_MODIFY | FLAG_TRANSPOSE | FLAG_SETTINGS)
#endif

// -------------------------
// Thread attributes
// -------------------------
static const osThreadAttr_t s_display_attrs = {
  .name       = "display_update",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t)osPriorityBelowNormal,
};
static const osThreadAttr_t s_midi_core_attrs = {
  .name       = "midi_core",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t)osPriorityRealtime,
};
static const osThreadAttr_t s_medium_tasks_attrs = {
  .name       = "medium_tasks",
  .stack_size = 128 * 4,
  .priority   = (osPriority_t)osPriorityAboveNormal,
};

// -------------------------
// Display thread
// -------------------------
static void DisplayUpdateThread(void *arg) {
  (void)arg;
  for (;;) {
    uint32_t f = osEventFlagsWait(s_display_flags, DISPLAY_FLAG_MASK,
                                  osFlagsWaitAny, osWaitForever);
    uint8_t current = ui_state_get(UI_CURRENT_MENU);
    if ((f & FLAG_TEMPO)     && current == MIDI_TEMPO)     screen_update_midi_tempo();
    if ((f & FLAG_MODIFY)    && current == MIDI_MODIFY)    screen_update_midi_modify();
    if ((f & FLAG_TRANSPOSE) && current == MIDI_TRANSPOSE) screen_update_midi_transpose();
    if ((f & FLAG_SETTINGS)  && current == SETTINGS)       screen_update_settings();
    osDelay(30);
  }
}

static void screen_refresh(void) {
    switch (ui_state_get(UI_CURRENT_MENU)) {
      case MIDI_TEMPO:     threads_display_notify(FLAG_TEMPO);     break;
      case MIDI_MODIFY:    threads_display_notify(FLAG_MODIFY);    break;
      case MIDI_TRANSPOSE: threads_display_notify(FLAG_TRANSPOSE); break;
      case SETTINGS:       threads_display_notify(FLAG_SETTINGS); break;
      default: break;
    }
}

// -------------------------
// MIDI core thread
// -------------------------
static void MidiCoreThread(void *argument)
{
  (void)argument;
  MX_USB_DEVICE_Init();
  for (;;) {
    calculate_incoming_midi();
    osDelay(5);
  }
}

// -------------------------
// Medium tasks thread
// -------------------------
static void MediumTasksThread(void *argument)
{
  (void)argument;

  // Initial menu draw trigger
  ui_state_modify(UI_CURRENT_MENU, UI_MODIFY_SET, save_get(SETTINGS_START_MENU));
  ui_state_modify(UI_OLD_MENU, UI_MODIFY_SET, 99);
  screen_refresh();

  switch (ui_state_get(UI_CURRENT_MENU)) {
    case MIDI_TEMPO:     threads_display_notify(FLAG_TEMPO); break;
    case MIDI_MODIFY:    threads_display_notify(FLAG_MODIFY); break;
    case MIDI_TRANSPOSE: threads_display_notify(FLAG_TRANSPOSE); break;
    case SETTINGS:       threads_display_notify(FLAG_SETTINGS); break;
  }

  for (;;) {
    menu_change_check();

    uint8_t old_menu    = ui_state_get(UI_OLD_MENU);
    uint8_t current_menu = ui_state_get(UI_CURRENT_MENU);

    if (old_menu != current_menu) {
        screen_refresh();
    }

    switch (current_menu) {
      case MIDI_TEMPO:     midi_tempo_update_menu();     break;
      case MIDI_MODIFY:    midi_modify_update_menu();    break;
      case MIDI_TRANSPOSE: midi_transpose_update_menu(); break;
      case SETTINGS:       settings_update_menu();       break;
      default: break;
    }

    current_menu = ui_state_get(UI_CURRENT_MENU);
    ui_state_modify(UI_OLD_MENU, UI_MODIFY_SET, current_menu);

    // Btn3 toggles "currently sending" depending on menu
    static uint8_t OldBtn3State = 1;
    if (debounce_button(GPIOB, Btn3_Pin, &OldBtn3State, 50)) {
      switch (ui_state_get(UI_CURRENT_MENU)) {
        case MIDI_TEMPO:
          save_modify_u8(MIDI_TEMPO_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
          mt_start_stop(&htim2);
          threads_display_notify(FLAG_TEMPO);
          break;
        case MIDI_MODIFY:
          save_modify_u8(MIDI_MODIFY_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
          threads_display_notify(FLAG_MODIFY);
          break;
        case MIDI_TRANSPOSE:
          save_modify_u8(MIDI_TRANSPOSE_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
          threads_display_notify(FLAG_TRANSPOSE);
          break;
        default:
          break;
      }
    }

    // Panic (both buttons)
    extern UART_HandleTypeDef huart1, huart2;
    panic_midi(&huart1, &huart2, GPIOB, Btn1_Pin, Btn2_Pin);

    osDelay(10);
  }
}

// -------------------------
// Public API
// -------------------------


void threads_start(void) {
  s_display_flags      = osEventFlagsNew(&s_flags_attrs);
  s_display_handle     = osThreadNew(DisplayUpdateThread, NULL, &s_display_attrs);
  s_midi_core_handle   = osThreadNew(MidiCoreThread,       NULL, &s_midi_core_attrs);
  s_medium_tasks_handle= osThreadNew(MediumTasksThread,    NULL, &s_medium_tasks_attrs);
}

void threads_display_notify(uint32_t flags) {
  if (s_display_flags) osEventFlagsSet(s_display_flags, flags);
}

osThreadId_t threads_display_handle(void)
{
  return s_display_handle;
}
