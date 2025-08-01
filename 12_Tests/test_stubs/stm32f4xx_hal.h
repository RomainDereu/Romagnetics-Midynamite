#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- HAL status type (matching STM32) ---
typedef enum {
    HAL_OK      = 0x00U,
    HAL_ERROR   = 0x01U,
    HAL_BUSY    = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

// --- Fake STM32 HAL handle types with mock state ---
#define UART_TX_BUFFER_SIZE 1024

typedef struct {
    int dummy; // placeholder to match signature

    // Mock state:
    uint8_t tx_buffer[UART_TX_BUFFER_SIZE];
    size_t tx_len;

    // Behavior control:
    int force_error;        // if nonzero, HAL_UART_Transmit returns HAL_ERROR
    int force_timeout;      // if nonzero, returns HAL_TIMEOUT
    int simulate_delay_ms;  // (optional) pretend to delay - no real sleep here unless you add it

    // Optional callback invoked on each transmit:
    void (*on_transmit)(uint8_t *data, size_t len, void *user_ctx);
    void *on_transmit_ctx;

} UART_HandleTypeDef;

typedef struct { int dummy; } TIM_HandleTypeDef;

// --- Helpers to reset / inspect stub state ---
static inline void mock_uart_reset(UART_HandleTypeDef *huart) {
    if (!huart) return;
    huart->tx_len = 0;
    huart->force_error = 0;
    huart->force_timeout = 0;
    huart->simulate_delay_ms = 0;
    huart->on_transmit = NULL;
    huart->on_transmit_ctx = NULL;
}

static inline void mock_uart_set_callback(UART_HandleTypeDef *huart,
                                          void (*cb)(uint8_t*, size_t, void*),
                                          void *ctx) {
    if (!huart) return;
    huart->on_transmit = cb;
    huart->on_transmit_ctx = ctx;
}

// --- Stubbed HAL functions ---
static inline HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, int len, int timeout) {
    (void)timeout;

    if (!huart || !pData || len <= 0) return HAL_ERROR;

    if (huart->force_error) {
        return HAL_ERROR;
    }
    if (huart->force_timeout) {
        return HAL_TIMEOUT;
    }

    // Cap to buffer size
    int to_copy = len;
    if ((size_t)to_copy + huart->tx_len > UART_TX_BUFFER_SIZE) {
        to_copy = (int)(UART_TX_BUFFER_SIZE - huart->tx_len);
    }

    memcpy(&huart->tx_buffer[huart->tx_len], pData, to_copy);
    huart->tx_len += to_copy;

    // Optional callback for tests
    if (huart->on_transmit) {
        huart->on_transmit(huart->tx_buffer + huart->tx_len - to_copy, to_copy, huart->on_transmit_ctx);
    }

    // For visibility in test logs
    printf("[HAL_UART_Transmit] ");
    for (int i = 0; i < to_copy; ++i) {
        printf("%02X ", pData[i]);
    }
    printf("\n");

    return HAL_OK;
}

// Convenience to retrieve pointer/length from handle
static inline const uint8_t* mock_uart_get_tx_data(const UART_HandleTypeDef *huart, size_t *out_len) {
    if (out_len) {
        *out_len = huart ? huart->tx_len : 0;
    }
    return huart ? huart->tx_buffer : NULL;
}

static inline void send_usb_midi_message(uint8_t *msg, uint8_t length) {
    (void)msg; (void)length;
    // no-op for tests
}

#endif // STM32F4XX_HAL_H
