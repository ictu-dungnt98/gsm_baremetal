#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <stdint.h>
#include "stm32f4xx_gpio.h"

typedef struct {
	uint16_t pin;
	GPIO_TypeDef* port;
} stm32_button_t;

void button_init(void);
void button_task(void);

#endif
