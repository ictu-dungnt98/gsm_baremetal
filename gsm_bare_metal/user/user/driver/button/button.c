#include "button.h"
#include "hardware.h"

#include "app_btn.h"
#include "app_debug.h"

#include <string.h>

stm32_button_t arr_buttons[APP_BTN_MAX_BTN_SUPPORT] = {
    {.pin = BTN_1_PIN, .port = BTN_1_PORT},
    {.pin = BTN_2_PIN, .port = BTN_2_PORT},
    {.pin = BTN_3_PIN, .port = BTN_3_PORT},
    {.pin = BTN_4_PIN, .port = BTN_4_PORT},
};

static void button_config(uint32_t btn_num)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    DEBUG_PRINTF("button_config %d\n", btn_num);

    /* Button */
    memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitStructure));
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Pin = arr_buttons[btn_num].pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(arr_buttons[btn_num].port, &GPIO_InitStructure);
}

static uint32_t button_read(uint32_t btn_num)
{
    uint8_t state = GPIO_ReadInputDataBit(arr_buttons[btn_num].port, arr_buttons[btn_num].pin);
    return (uint32_t)state;
}

void on_button_event_cb(int button_pin, int event, void *data)
{
    static const char *event_name[] = {"pressed", "release", "hold", "hold so long", "double click", "tripple click", "idle", "idle break", "invalid"};

    DEBUG_PRINTF("Button %d, event %s\r\n", button_pin, event_name[event]);

    switch (event) {
        case APP_BTN_EVT_PRESSED:
            break;

        case APP_BTN_EVT_RELEASED:
            break;

        case APP_BTN_EVT_HOLD:
            break;

        case APP_BTN_EVT_DOUBLE_CLICK: {
        } break;

        case APP_BTN_EVT_HOLD_SO_LONG: {
        } break;

        default:
            DEBUG_PRINTF("[%s] Unhandle button event %d\r\n", __FUNCTION__, event);
            break;
    }
}

uint32_t get_tick_ms(void) { return sys_get_tick_ms(); }

void button_init(void)
{
    static app_btn_hw_config_t m_hw_button_initialize_value[APP_BTN_MAX_BTN_SUPPORT];

    for (uint32_t i = 0; i < APP_BTN_MAX_BTN_SUPPORT; i++) {
        m_hw_button_initialize_value[i].idle_level = 1;
        m_hw_button_initialize_value[i].last_state = button_read(i);  // read the very first level at initialization
        m_hw_button_initialize_value[i].pin = i;
    }

    app_btn_config_t conf;
    conf.config = &m_hw_button_initialize_value[0];
    conf.btn_count = APP_BTN_MAX_BTN_SUPPORT;
    conf.get_tick_cb = get_tick_ms;
    conf.btn_initialize = button_config;
    conf.btn_read = button_read;

    app_btn_initialize(&conf);
    app_btn_register_callback(APP_BTN_EVT_PRESSED, on_button_event_cb, NULL);
    app_btn_register_callback(APP_BTN_EVT_RELEASED, on_button_event_cb, NULL);
    app_btn_register_callback(APP_BTN_EVT_TRIPLE_CLICK, on_button_event_cb, NULL);
}

void button_task(void)
{
    static uint32_t last_time_handle = 0;
    if (sys_get_tick_ms() - last_time_handle < 100)
        return;

    last_time_handle = sys_get_tick_ms();

    app_btn_scan(NULL);
}
