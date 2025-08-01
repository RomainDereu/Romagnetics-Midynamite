#ifdef UNIT_TEST

#include "unity.h"
#include "test_runner.h"
#include "gpio.h"   
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <time.h>   

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern void test_velocity_addition(void);

void unity_run_all_tests(void)
{
    printf("[TEST RUN STARTED]\n");

    UNITY_BEGIN();

    unsigned base_seed = (unsigned)time(NULL);
    char buf[128];
    snprintf(buf, sizeof(buf), "base_seed=%u", base_seed);
    TEST_MESSAGE(buf);

    //Roro refactor into a different function.
    for (int x = 0; x < 10; ++x) {
        unsigned seed = base_seed + x;
        srand(seed);
        snprintf(buf, sizeof(buf), "Iteration %d seed=%u", x, seed);
        TEST_MESSAGE(buf);
        // Reset shared/test state if needed here, e.g.:
        mock_uart_reset(&huart1);
        mock_uart_reset(&huart2);
        RUN_TEST(test_velocity_addition);

        // Stop early on failure if desired
        if (Unity.TestFailures > 0) break;
    }

    int result = UNITY_END();
    (void)result;

    printf("[TEST RUN COMPLETE]\n");
}
#endif
