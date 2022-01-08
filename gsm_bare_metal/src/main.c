#include "stm32f4xx.h"
#include <stddef.h>
#include <stdint.h>

#include "led.h"
#include "uart.h"
#include "button.h"
#include "hardware.h"

#include "app_debug.h"
#include "gsm.h"


int main(void)
{
    rcc_config();
    systick_config();
    button_init();
    led_init();

    /* gsm */
    gsm_init_hw();

    while (1) {
        button_task();
        led_task();
        gsm_mnr_task(NULL);
    }
}
