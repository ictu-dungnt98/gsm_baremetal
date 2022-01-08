#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <stdbool.h>
#include "stm32f4xx_gpio.h"

#include "led.h"
#include "button.h"
#include "uart.h"
#include "app_flash.h"


#define TURN_ON 1
#define TURN_OFF 0

#define HW_MAX_CHANNEL_SUPPORT 4

/* Define Led indicate status (Pair, reset default) */
#define LED_GREEN_NETWORK_PIN       GPIO_Pin_9
#define LED_GREEN_NETWORK_PORT      GPIOB
#define LED_BLUE_STATEMACHINE_PIN   GPIO_Pin_4
#define LED_BLUE_STATEMACHINE_PORT  GPIOB

/* Define SENSOR INPUT PIN */
#define TIME_READ_SENSOR_PIN    31
#define SENSOR_PIN              GPIO_Pin_4
#define SENSOR_PORT             GPIOA
#define SENSOR_READ_PIN()       GPIO_ReadInputDataBit(SENSOR_PORT, SENSOR_PIN)

/* Define Button on board */
#define BTN_1_PIN GPIO_Pin_0
#define BTN_1_PORT GPIOA
#define BTN_1_READ_PIN() GPIO_ReadInputDataBit(BTN_1_PORT, BTN_1_PIN)

#define BTN_2_PIN GPIO_Pin_1
#define BTN_2_PORT GPIOA
#define BTN_2_READ_PIN() GPIO_ReadInputDataBit(BTN_2_PORT, BTN_2_PIN)

#define BTN_3_PIN GPIO_Pin_0
#define BTN_3_PORT GPIOB
#define BTN_3_READ_PIN() GPIO_ReadInputDataBit(BTN_3_PORT, BTN_3_PIN)

#define BTN_4_PIN GPIO_Pin_1
#define BTN_4_PORT GPIOB
#define BTN_4_READ_PIN() GPIO_ReadInputDataBit(BTN_4_PORT, BTN_4_PIN)

/* Define Switch Pin */
#define SW_1_PORT GPIOB
#define SW_1_PIN GPIO_Pin_5
#define SWITCH_1_READ_PIN() GPIO_ReadInputDataBit(SW_1_PORT, SW_1_PIN)

#define SW_2_PORT GPIOB
#define SW_2_PIN GPIO_Pin_7
#define SWITCH_2_READ_PIN() GPIO_ReadInputDataBit(SW_2_PORT, SW_2_PIN)

#define SW_3_PORT GPIOB
#define SW_3_PIN GPIO_Pin_6
#define SWITCH_3_READ_PIN() GPIO_ReadInputDataBit(SW_3_PORT, SW_3_PIN)

#define SW_4_PORT GPIOB
#define SW_4_PIN GPIO_Pin_8
#define SWITCH_4_READ_PIN() GPIO_ReadInputDataBit(SW_4_PORT, SW_4_PIN)

/* VBAT */
#define OS_GSM_VBAT_PORT GPIOB
#define OS_GSM_VBAT_PIN GPIO_Pin_3

/* Define PC UART pin */
#define OS_PC_UART_TX_GPIO_PIN GPIO_Pin_2
#define OS_PC_UART_RX_GPIO_PIN GPIO_Pin_3
#define OS_PC_UART_GPIO_PORT GPIOA
#define OS_PC_UART_INSTANCE USART2

/* Define GSM Uart Pin */
#define GSM_USART_Tx_Pin GPIO_Pin_9
#define GSM_USART_Rx_Pin GPIO_Pin_10
#define GSM_USART_GPIO GPIOA
#define GSM_USART_PORT USART1

#define GSM_STATUS_Pin GPIO_Pin_12
#define GSM_STATUS_GPIO GPIOA
#define GSM_READ_STATUS() GPIO_ReadInputDataBit(GSM_STATUS_GPIO, GSM_STATUS_Pin)

#define GSM_PWKEY_Pin GPIO_Pin_8
#define GSM_PWKEY_GPIO GPIOA

#define GSM_PWKEY_LOW() GPIO_SetBits(GSM_PWKEY_GPIO, GSM_PWKEY_Pin)
#define GSM_PWKEY_HIGH() GPIO_ResetBits(GSM_PWKEY_GPIO, GSM_PWKEY_Pin)

void rcc_config(void);
void systick_config(void);
void wdt_init(void);

uint32_t sys_get_tick_ms(void);

void gsm_gpio_init(void);
bool gsm_power_on(void);
bool gsm_power_off(void);
bool gsm_power_reset(void);

#endif
