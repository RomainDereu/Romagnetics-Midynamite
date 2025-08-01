#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// Only define it if the real one hasn't already.
#ifndef GPIO_TypeDef
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
} GPIO_TypeDef;
#endif

#endif 