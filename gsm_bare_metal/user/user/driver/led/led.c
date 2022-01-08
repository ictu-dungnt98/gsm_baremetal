#include "led.h"
#include "hardware.h"

#include <string.h>

#include "app_debug.h"
#include "app_led.h"

static os_led_control_t *led_1;
static os_led_control_t *led_2;

stm32_led_t arr_leds[MAXIMUM_LED_CONTROL] = {
    {.pin = LED_BLUE_STATEMACHINE_PIN, .port = LED_BLUE_STATEMACHINE_PORT},
    {.pin = LED_GREEN_NETWORK_PIN, .port = LED_GREEN_NETWORK_PORT},
};

static void led_write(uint8_t pin, uint8_t state)
{
    // DEBUG_INFO("[%s] pin %d, state %d\r\n", __FUNCTION__, pin, state);
    if (state == 1) {
        GPIO_SetBits(arr_leds[pin].port, arr_leds[pin].pin);
    } else {
        GPIO_ResetBits(arr_leds[pin].port, arr_leds[pin].pin);
    }
}

static void led_config(uint8_t pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // DEBUG_INFO("[%s] pin %d\r\n", __FUNCTION__, pin);

    /* Led */
    memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = arr_leds[pin].pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(arr_leds[pin].port, &GPIO_InitStructure);

    led_write(pin, 0);
}

void led_init(void)
{
    app_led_init(led_write, led_config);

    led_1 = insert_led_handle_blink_forervel(1000, 1000);
    if (led_1 == NULL) {
        DEBUG_ERROR("[%s] insert_led_handle_blink_forervel led_1 failed\r\n", __FUNCTION__);
    }

    led_2 = insert_led_handle_blink_forervel(500, 500);
    if (led_2 == NULL) {
        DEBUG_ERROR("[%s] insert_led_handle_blink_forervel led_2 failed\r\n", __FUNCTION__);
    }
}

void led_task(void)
{
    static uint32_t last_time_handle = 0;
    if (sys_get_tick_ms() - last_time_handle < TIME_MS_TASK_SYNC_LED)
        return;

    last_time_handle = sys_get_tick_ms();

    app_led_handle(NULL);
}
