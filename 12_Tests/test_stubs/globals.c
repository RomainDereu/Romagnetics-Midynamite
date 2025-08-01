#include "../10_Source_code/Core/Inc/midi_modify.h"
#include "../10_Source_code/Core/Inc/settings.h"

// Definitions (must match typedefs in main.h). 
midi_modify_data_struct midi_modify_data = {0};
midi_transpose_data_struct midi_transpose_data = {0};
settings_data_struct settings_data = {0};
midi_modify_circular_buffer midi_modify_buff = {0};

// Dummy UART handles if referenced globally
UART_HandleTypeDef huart1 = {0};
UART_HandleTypeDef huart2 = {0};