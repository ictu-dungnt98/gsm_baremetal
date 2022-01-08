#ifndef _LED_DRV_H_
#define _LED_DRV_H_

#include <stdbool.h>
#include <stdint.h>
#include "stm32f4xx_gpio.h"


#define MAXIMUM_LED_CONTROL 2
#define TIME_MS_TASK_SYNC_LED 50  // ms

typedef enum {
    LED_RUNNING = 1,
    LED_STOP = 2,
    LED_SUSPEND = 3
} state_object_led_t;

typedef enum {
    LED_TYPE_ONESHOT = 0,
    LED_TYPE_BLINK_NORMAL = 1,
    LED_TYPE_BLINK_FORERVER = 2,
    LED_TYPE_LED_ON = 3,
    LED_TYPE_LED_OFF = 4
} type_handle_led_t;

typedef struct {
    state_object_led_t current_object_state;
    type_handle_led_t type_handle;

    uint8_t state_turn_on_first_time;
    
    uint16_t count_blink;
    uint32_t on_interval_ms;
    uint32_t off_interval_ms;
    uint32_t current_interval_ms_on;
    uint32_t current_interval_ms_off;
    uint8_t current_state_on_off;
} os_led_control_t;

typedef void (*led_control_cb_t)(uint8_t pin, uint8_t state);
typedef void (*led_cfg_cb_t)(uint8_t pin);

void app_led_init(led_control_cb_t led_write, led_cfg_cb_t led_cfg);
void app_led_handle(void* param);

os_led_control_t* insert_led_handle_oneshot(uint32_t interval_ms);

os_led_control_t* insert_led_handle_blink_normal(uint32_t interval_ms_on,
                                                 uint32_t interval_ms_off,
                                                 uint16_t number_blink);

os_led_control_t* insert_led_handle_blink_forervel(uint32_t interval_ms_on,
                                                   uint32_t interval_ms_off);

#endif  //_OS_APP_LED_H_

