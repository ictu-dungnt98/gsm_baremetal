#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>
#include "stm32f4xx_gpio.h"

typedef struct {
    uint16_t pin;
    GPIO_TypeDef* port;
} stm32_led_t;

void led_init(void);
void led_task(void);

#endif  //_LED_H_
