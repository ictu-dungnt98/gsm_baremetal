#include "hardware.h"

#include <string.h>

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "app_debug.h"


void wdt_init(void)
{
    /* Enable the LSI oscillator */
    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
    }

    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/256 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);

    /* Set counter reload value to obtain 5000ms IWDG TimeOut.
         Counter Reload Value = 37000/256*5000 = 722.65
        */
    IWDG_SetReload(1000);  // 5000ms

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG(the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

uint32_t tick_ms;
void SysTick_Handler(void) { tick_ms++; }

void systick_config(void)
{
    /* Setup SysTick Timer for 1 msec interrupts */
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1);
    }

    NVIC_SetPriority(SysTick_IRQn, 0x00);
}

void rcc_config(void)
{
    RCC_DeInit();
	SystemCoreClockUpdate();
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

uint32_t sys_get_tick_ms(void) { return tick_ms; }

void gsm_gpio_init(void)
{
    static bool is_gsm_gpio_init = false;
    if (is_gsm_gpio_init) {
        return;
    }
    is_gsm_gpio_init = true;

    GPIO_InitTypeDef GPIO_InitStructure;

    /* Power Control */
    GPIO_InitStructure.GPIO_Pin = GSM_PWKEY_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GSM_PWKEY_GPIO, &GPIO_InitStructure);

    /* Status */
    GPIO_InitStructure.GPIO_Pin = GSM_STATUS_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GSM_STATUS_GPIO, &GPIO_InitStructure);

    GSM_PWKEY_HIGH();
}

bool gsm_power_off(void)
{
    static uint8_t pwr_off_step = 0;
    static uint32_t last_time_ms = 0;
    static uint32_t time_wait_ms = 0;

    if (sys_get_tick_ms() - last_time_ms < time_wait_ms)
        return false;

    switch (pwr_off_step) {
        case 0: { /* turn off to reset gsm power */
            GSM_PWKEY_LOW();
            time_wait_ms = 2500;
            pwr_off_step ++;
        } break;

        case 1: {
            GSM_PWKEY_HIGH();
            time_wait_ms = 3000;
            pwr_off_step++;
        } break;

        case 2: { /* wait pin to low */
            time_wait_ms = 100;

            if (GSM_READ_STATUS() == 0) { 
                DEBUG_INFO("status down %d\r\n", GSM_READ_STATUS());
                pwr_off_step = 0;
                time_wait_ms = 0;
                return true;
            }
        } break;

        default:
            break;
    }

    last_time_ms = sys_get_tick_ms();
    
    return false;
}

bool gsm_power_on(void)
{
    static uint8_t pwr_on_step = 0;
    static uint32_t last_time_ms = 0;
    static uint32_t time_wait_ms = 0;

    if (sys_get_tick_ms() - last_time_ms < time_wait_ms)
        return false;

    DEBUG_INFO("pwr_on_step %d. STT %d\r\n", pwr_on_step, GSM_READ_STATUS());

    switch (pwr_on_step) {
        case 0: {  /* power-on gsm */
            GSM_PWKEY_LOW();
            time_wait_ms = 1000;
            pwr_on_step ++;
        } break;

        case 1: {
            GSM_PWKEY_HIGH();
            time_wait_ms = 5000;
            pwr_on_step++;

        } break;

        case 2: { 
            if (GSM_READ_STATUS()) {
                pwr_on_step = 0;
                time_wait_ms = 0;
                return true;
            }
        } break;

        default:
            break;
    }

    
    last_time_ms = sys_get_tick_ms();

    return false;
}

bool gsm_power_reset(void)
{
    static uint8_t reset_pwr_step = 0;
    bool ret = false;

    switch (reset_pwr_step) {
        case 0: { /* turn off to reset gsm power */
            if (gsm_power_off())
                reset_pwr_step ++;
        } break;

        case 1: {
            ret =  gsm_power_on();
            if (ret == true) {
                reset_pwr_step = 0;
            }
        } break;

        default:
            break;
    }

    return ret;
}
