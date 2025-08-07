/*
 * test_runner.c
 * Author: Romain Dereu
 */

#ifdef UNIT_TEST

#include "unity.h"
#include "test_midi_logic.h"
#include "test_runner.h"
#include "gpio.h"   
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <time.h>   

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void unity_run_all_tests(void)
{
    printf("[TEST RUN STARTED]\n");
    unsigned base_seed = (unsigned)time(NULL);
    char buf[128];

    //Roro refactor into a different function.
    for (int x = 0; x < 10; ++x) {
        unsigned seed = base_seed + x;
        srand(seed);
        TEST_MESSAGE(buf);
        // Reset shared/test state if needed here, e.g.:
        RUN_TEST(test_velocity_addition);
        RUN_TEST(test_velocity_absolute);

        // Stop early on failure if desired
        if (Unity.TestFailures > 0) break;
    }
    printf("[TEST RUN COMPLETE]\n");
}

int UnityMain(int argc, char **argv, void (*runAllTests)(void))
{
    (void)argc;
    (void)argv;

    const char *suiteName = (argc > 0 && argv[0]) ? argv[0] : "unity";
    UnityBegin(suiteName);
    runAllTests();
    return UnityEnd();
}


int main(int argc, char **argv) {
    return UnityMain(argc, argv, unity_run_all_tests);
}

#endif
