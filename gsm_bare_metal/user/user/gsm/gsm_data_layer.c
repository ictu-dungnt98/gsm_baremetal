#include "gsm.h"
#include "gsm_utilities.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "app_debug.h"
#include "hardware.h"
#include "gsm_atc_config.h"

#define UNLOCK_BAND 0
#define CUSD_ENABLE 1

/* porting hardware layer */
extern void gsm_power_on_sequence(void);

gsm_manager_t m_gsm_manager;

void gsm_data_layer_initialize(void)
{
    memset(&m_gsm_manager, 0, sizeof(m_gsm_manager));
}

bool gsm_data_layer_is_module_sleeping(void)
{
    if (m_gsm_manager.state == GSM_STATE_SLEEP) {
        return 1;
    }
    return 0;
}

void gsm_change_state(gsm_state_t new_state)
{
    if (new_state == GSM_STATE_OK)  // Command state -> Data state in PPP mode
    {
        m_gsm_manager.gsm_ready = 2;
    }

    DEBUG_INFO("Change GSM state to: ");
    switch ((uint8_t)new_state) {
        case GSM_STATE_OK:
            DEBUG_RAW("OK\r\n");
            break;

        case GSM_STATE_RESET:
            DEBUG_RAW("RESET\r\n");
            gsm_hw_layer_reset_rx_buffer();
            break;

        case GSM_STATE_POWER_ON:
            DEBUG_RAW("POWERON\r\n");
            gsm_hw_layer_reset_rx_buffer();  // Reset USART RX buffer
            break;

        case GSM_STATE_INTERNET_ATTACKED:
            DEBUG_RAW("INTERNET ATTACKED\r\n");
            gsm_hw_layer_reset_rx_buffer();  // Reset USART RX buffer
            break;

        default:
            break;
    }

    m_gsm_manager.state = new_state;
}

void gsm_state_machine_polling(void)
{
    static uint32_t m_last_tick = 0;
    uint32_t now = sys_get_tick_ms();
    if (now - m_last_tick >= (uint32_t)100)  // Poll every 1 second
    {
        m_last_tick = now;
        /* GSM state machine */
        switch (m_gsm_manager.state) {
            case GSM_STATE_RESET: /* Hard Reset */
                m_gsm_manager.gsm_ready = 0;
                m_gsm_manager.step = 0;
                gsm_power_on_sequence();
                break;

            case GSM_STATE_POWER_ON:
                if (m_gsm_manager.step == 0) {
                    m_gsm_manager.step = 1;
                    gsm_hw_send_at_cmd("AT\r\n", "OK\r\n", "", 1000, 30, gsm_config_module);
                }
                break;

            case GSM_STATE_INTERNET_ATTACKED:
                m_gsm_manager.gsm_ready = 2;
                break;

            case GSM_STATE_OK:
                break;

            default:
                DEBUG_ERROR("Unhandled case %u\r\n", m_gsm_manager.state);
                break;
        }
    }
}

void gsm_mnr_task(void *arg)
{
    gsm_hw_layer_loop(); /* uart rx process */
    gsm_state_machine_polling();
}
