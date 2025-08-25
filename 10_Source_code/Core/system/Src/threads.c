#include "threads.h"

#include "usb_device.h"
#include "memory_ui_state.h"
#include "memory_main.h"
#include "menu.h"
#include "midi_tempo.h"
#include "midi_modify.h"
#include "settings.h"
#include "utils.h"

// -------------------------
// Internal state
// -------------------------
static osThreadId_t s_display_handle     = NULL;
static osThreadId_t s_midi_core_handle   = NULL;
static osThreadId_t s_medium_tasks_handle= NULL;
static threads_ctx_t s_ctx;

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
static void DisplayUpdateThread(void *argument)
{
  (void)argument;
  for (;;) {
    uint32_t displayFlags = osThreadFlagsWait(DISPLAY_FLAG_MASK, osFlagsWaitAny, osWaitForever);

    uint8_t current_menu = ui_state_get(UI_CURRENT_MENU);
    switch (current_menu) {
      case MIDI_TEMPO:
        if (displayFlags & FLAG_TEMPO)     screen_update_midi_tempo();
        break;
      case MIDI_MODIFY:
        if (displayFlags & FLAG_MODIFY)    screen_update_midi_modify();
        break;
      case MIDI_TRANSPOSE:
        if (displayFlags & FLAG_TRANSPOSE) screen_update_midi_transpose();
        break;
      case SETTINGS:
        if (displayFlags & FLAG_SETTINGS)  screen_update_settings();
        break;
      default:
        break;
    }
    osDelay(30);
  }
}

// -------------------------
// MIDI core thread
// -------------------------
static void MidiCoreThread(void *argument)
{
  (void)argument;
  MX_USB_DEVICE_Init(); // same place you had it
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
  ui_state_modify(UI_CURRENT_MENU, UI_MODIFY_SET, save_get(SAVE_SETTINGS_START_MENU));
  ui_state_modify(UI_OLD_MENU, UI_MODIFY_SET, 99);

  for (;;) {
    menu_change_check();

    uint8_t current_menu = ui_state_get(UI_CURRENT_MENU);
    switch (current_menu) {
      case MIDI_TEMPO:
        midi_tempo_update_menu(s_ctx.htim3, s_ctx.htim4, threads_display_handle());
        break;
      case MIDI_MODIFY:
        midi_modify_update_menu(s_ctx.htim3, s_ctx.htim4, threads_display_handle());
        break;
      case MIDI_TRANSPOSE:
        midi_transpose_update_menu(s_ctx.htim3, s_ctx.htim4, threads_display_handle());
        break;
      case SETTINGS:
        settings_update_menu(s_ctx.htim3, s_ctx.htim4, threads_display_handle());
        break;
      default:
        break;
    }

    current_menu = ui_state_get(UI_CURRENT_MENU);
    ui_state_modify(UI_OLD_MENU, UI_MODIFY_SET, current_menu);

    // Btn3 toggles "currently sending" depending on menu
    static uint8_t OldBtn3State = 1;
    if (debounce_button(GPIOB, Btn3_Pin, &OldBtn3State, 50)) {
      switch (ui_state_get(UI_CURRENT_MENU)) {
        case MIDI_TEMPO:
          save_modify_u8(SAVE_MIDI_TEMPO_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
          mt_start_stop(s_ctx.htim2);
          threads_display_notify(FLAG_TEMPO);
          break;
        case MIDI_MODIFY:
          save_modify_u8(SAVE_MIDI_MODIFY_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
          threads_display_notify(FLAG_MODIFY);
          break;
        case MIDI_TRANSPOSE:
          save_modify_u8(SAVE_TRANSPOSE_CURRENTLY_SENDING, SAVE_MODIFY_INCREMENT, 0);
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
void threads_start(const threads_ctx_t *ctx)
{
  s_ctx = *ctx; // copy pointers

  // Start display first so others can signal it
  s_display_handle      = osThreadNew(DisplayUpdateThread, NULL, &s_display_attrs);
  s_midi_core_handle    = osThreadNew(MidiCoreThread,       NULL, &s_midi_core_attrs);
  s_medium_tasks_handle = osThreadNew(MediumTasksThread,    NULL, &s_medium_tasks_attrs);
}

void threads_display_notify(uint32_t flags)
{
  if (s_display_handle) osThreadFlagsSet(s_display_handle, flags);
}

osThreadId_t threads_display_handle(void)
{
  return s_display_handle;
}
