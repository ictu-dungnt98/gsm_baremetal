/******************************************************************************
 * @file    	m_gsm_hardwareLayer.c
 * @author
 * @version 	V1.0.0
 * @date    	10/11/2014
 * @brief
 ******************************************************************************/

/******************************************************************************
                                   INCLUDES
 ******************************************************************************/
#include <string.h>

#include "app_debug.h"
#include "gsm.h"
#include "hardware.h"
#include <time.h>

/* porting hardware layer */
extern void uart_init(void);
extern void uart_send_byte(char data);
extern void uart_send_string(char *data, uint16_t length);

// Buffer for ppp
static gsm_modem_buffer_t m_gsm_modem_buffer;

// Buffer for AT command
static gsm_hardware_atc_t m_gsm_atc;

static volatile bool m_new_uart_data = false;
static char m_gsm_imei[16] = {0};
static char m_sim_imei[16];
static char m_nw_operator[32];
static char m_sim_ccid[21];
static uint8_t m_signal_strength = 0;
static struct tm m_time_local;

/* global variables */
uint32_t m_poll_interval = 5;
uint32_t current_response_length;
uint32_t expect_compare_length;
uint8_t *p_compare_end_str;
uint8_t *uart_rx_pointer = m_gsm_atc.atc.recv_buff.buffer;
uint8_t *expect_end_str;

struct tm* gsm_get_local_time(void)
{
    return &m_time_local;
}

void gsm_set_local_time(struct tm *time)
{
    memcpy(&m_time_local, time, sizeof(struct tm));
}

uint8_t gsm_get_signal_strength(void)
{
    return m_signal_strength;
}

void gsm_set_signal_strength(uint8_t signal_strength)
{
    m_signal_strength = signal_strength;
}

char *gsm_get_sim_imei(void) { return m_sim_imei; }
void gsm_set_sim_imei(char *imei)
{
    memcpy(m_sim_imei, imei, 15);
    m_sim_imei[15] = 0;
}

char *gsm_get_sim_ccid(void) { return m_sim_ccid; }

char *gsm_get_module_imei(void) { return m_gsm_imei; }
void gsm_set_module_imei(char *imei)
{
    memcpy(m_gsm_imei, imei, 15);
    m_gsm_imei[15] = 0;
}

char *gsm_get_network_operator(void) { return m_nw_operator; }
void gsm_set_network_operator(char *nw_operator)
{
    snprintf(m_nw_operator, sizeof(m_nw_operator) - 1, "%s", nw_operator);
    m_nw_operator[sizeof(m_nw_operator) - 1] = 0;
}

void gsm_init_hw(void)
{
    // Init gpio pin
    gsm_gpio_init();
    // Enable GSM USART
    uart_init();

    // Init gsm data layer
    gsm_data_layer_initialize();

    // Change gsm first state to RESET
    gsm_change_state(GSM_STATE_RESET);

    gsm_change_hw_polling_interval(50); /* ms */
}

#define RESET_MODULE 0
#define POWER_ON_MODULE 1
void gsm_power_on_sequence(void)
{
    static uint8_t step = 0;
    static uint8_t power_mode = 0;
    // DEBUG_INFO("GSM hard reset step %d\r\n", step);

    switch (step) {
        case 0: {
            /* check what to do (reset or power on) */
            if (GSM_READ_STATUS())
                power_mode = RESET_MODULE;
            else {
                power_mode = POWER_ON_MODULE;
            }

            step++;
        } break;

        case 1: {
            if (power_mode == RESET_MODULE) {
                if (gsm_power_reset()) {
                    step = 0;
                    gsm_change_state(GSM_STATE_POWER_ON);
                }
            } else if (power_mode == POWER_ON_MODULE) {
                if (gsm_power_on()) {
                    step = 0;
                    gsm_change_state(GSM_STATE_POWER_ON);
                }
            }
        } break;

        default:
            break;
    }
}

void gsm_change_hw_polling_interval(uint32_t ms) { m_poll_interval = ms; }

void gsm_hw_uart_send_raw(uint8_t *raw, uint32_t length)
{
    /* send data to uart */
    uart_send_string((char *)raw, length);
}

void gsm_hw_layer_reset_rx_buffer(void)
{
    memset(m_gsm_atc.atc.recv_buff.buffer, 0, sizeof(((gsm_atc_buffer_t *)0)->buffer));
    m_gsm_atc.atc.recv_buff.index = 0;
    m_gsm_atc.atc.retry_count_atc = 0;
}

void gsm_hw_send_at_cmd(char *cmd, char *expect_resp,
                        char *expected_response_at_the_end_of_response, uint32_t timeout_ms,
                        uint8_t retry_count, gsm_send_at_cb_t callback)
{
    if (strlen(cmd) < 64) {
        DEBUG_INFO("ATC: %s", cmd);
    }

    if (timeout_ms == 0 || callback == NULL) {
        gsm_hw_uart_send_raw((uint8_t *)cmd, strlen(m_gsm_atc.atc.cmd));
        DEBUG_INFO("Do not wait repond\n");
        return;
    }

    m_gsm_atc.atc.cmd = cmd;
    m_gsm_atc.atc.expect_resp_from_atc = expect_resp;
    m_gsm_atc.atc.expected_response_at_the_end = expected_response_at_the_end_of_response;
    m_gsm_atc.atc.retry_count_atc = retry_count;
    m_gsm_atc.atc.send_at_callback = callback;
    m_gsm_atc.atc.timeout_atc_ms = timeout_ms;
    m_gsm_atc.atc.current_timeout_atc_ms = sys_get_tick_ms();
    memset(m_gsm_atc.atc.recv_buff.buffer, 0, sizeof(((gsm_atc_buffer_t *)0)->buffer));
    m_gsm_atc.atc.recv_buff.index = 0;

    gsm_hw_uart_send_raw((uint8_t *)cmd, strlen(cmd));
}

void gsm_hw_layer_uart_fill_rx(uint8_t *data, uint32_t length)
{
    if (length) {
        m_new_uart_data = true;
        /* Device do not enter AT mode =>> bypass data into PPP stack */
        if (gsm_is_in_ppp_mode()) {
            for (uint32_t i = 0; i < length; i++) {
                m_gsm_modem_buffer.buffer[m_gsm_modem_buffer.idx_in++] = data[i];
                if (m_gsm_modem_buffer.idx_in >= GSM_PPP_MODEM_BUFFER_SIZE) {
                    m_gsm_modem_buffer.idx_in = 0;
                }
                m_gsm_modem_buffer.buffer[m_gsm_modem_buffer.idx_in] = 0;
            }
        } else {
            for (uint32_t i = 0; i < length; i++) {
                m_gsm_atc.atc.recv_buff.buffer[m_gsm_atc.atc.recv_buff.index++] = data[i];
                if (m_gsm_atc.atc.recv_buff.index >= sizeof(((gsm_atc_buffer_t *)0)->buffer)) {
                    DEBUG_ERROR("GSM ATC RX overflow\r\n");
                    m_gsm_atc.atc.recv_buff.index = 0;
                    m_gsm_atc.atc.recv_buff.buffer[0] = 0;
                    return;
                }
            }
        }
    }
}

uint32_t gsm_hardware_layer_copy_ppp_buffer(uint8_t *data, uint32_t len)
{
    uint32_t i = 0;
    for (i = 0; i < len; i++) {
        if (m_gsm_modem_buffer.idx_out == m_gsm_modem_buffer.idx_in) {
            return i;  // No more memory
        }

        data[i] = m_gsm_modem_buffer.buffer[m_gsm_modem_buffer.idx_out];

        m_gsm_modem_buffer.idx_out++;
        if (m_gsm_modem_buffer.idx_out == GSM_PPP_MODEM_BUFFER_SIZE)
            m_gsm_modem_buffer.idx_out = 0;
    }
    return i;
}

void gsm_hw_layer_loop(void)
{
    static uint32_t m_last_poll = 0;
    if ((sys_get_tick_ms() - m_last_poll) < m_poll_interval) {
        return;
    }
    m_last_poll = sys_get_tick_ms();

    bool ret_now = true;

    // If device is in AT mode

    if (m_gsm_atc.atc.timeout_atc_ms) {
        __disable_irq();
        if (m_new_uart_data) {
            m_new_uart_data = false;
            ret_now = false;
        }
        __enable_irq();

        if (ret_now == false) {
            // DEBUG_PRINTF("CMD: %s", m_gsm_atc.atc.cmd);
            DEBUG_PRINTF("Result: %s", m_gsm_atc.atc.recv_buff.buffer);
        }

        if (ret_now == false && (strstr((char *)(m_gsm_atc.atc.recv_buff.buffer),
                                        m_gsm_atc.atc.expect_resp_from_atc))) {
            bool do_cb = true;

            if (m_gsm_atc.atc.expected_response_at_the_end &&
                strlen(m_gsm_atc.atc.expected_response_at_the_end)) {
                // DEBUG_PRINTF("Expected end %s\r\n", m_gsm_atc.atc.expected_response_at_the_end);
                current_response_length = m_gsm_atc.atc.recv_buff.index;
                expect_compare_length = strlen(m_gsm_atc.atc.expected_response_at_the_end);

                if (expect_compare_length < current_response_length) {
                    p_compare_end_str =
                        &m_gsm_atc.atc.recv_buff
                             .buffer[current_response_length - expect_compare_length];
                    if (!memcmp(p_compare_end_str, m_gsm_atc.atc.expected_response_at_the_end,
                                expect_compare_length)) {
                        do_cb = true;
                    } else {
                        do_cb = false;
                        // For debug only
                        expect_end_str = (uint8_t *)m_gsm_atc.atc.expected_response_at_the_end;
                        uart_rx_pointer = m_gsm_atc.atc.recv_buff.buffer;
                    }
                } else {
                    do_cb = false;
                }
            }

            if (do_cb) {
                /* make sure null character at the end */
                m_gsm_atc.atc.recv_buff.buffer[m_gsm_atc.atc.recv_buff.index] = 0;
                m_gsm_atc.atc.timeout_atc_ms = 0;

                if (m_gsm_atc.atc.send_at_callback) {
                    m_gsm_atc.atc.send_at_callback(GSM_EVENT_OK, m_gsm_atc.atc.recv_buff.buffer);
                }

                // Clear RX buffer
                gsm_hw_layer_reset_rx_buffer();
            }
        } else if (ret_now == false) {
            char *p = strstr((char *)(m_gsm_atc.atc.recv_buff.buffer), "CME ERROR: ");
            if (p && strstr(p, "\r\n")) {
                DEBUG_VERBOSE("%s", p);

                m_gsm_atc.atc.timeout_atc_ms = 0;
                if (m_gsm_atc.atc.send_at_callback) {
                    m_gsm_atc.atc.send_at_callback(GSM_EVENT_ERROR, m_gsm_atc.atc.recv_buff.buffer);
                }
            }
        }
    } else {
        /*  */
    }

    /* timeout */
    uint32_t diff = sys_get_tick_ms() - m_gsm_atc.atc.current_timeout_atc_ms;
    if (m_gsm_atc.atc.timeout_atc_ms && diff >= m_gsm_atc.atc.timeout_atc_ms) {
        if (m_gsm_atc.atc.retry_count_atc) {
            m_gsm_atc.atc.retry_count_atc--;
        }

        if (m_gsm_atc.atc.retry_count_atc == 0) {
            /* send timeout event to callback */
            m_gsm_atc.atc.timeout_atc_ms = 0;
            if (m_gsm_atc.atc.send_at_callback != NULL) {
                m_gsm_atc.atc.send_at_callback(GSM_EVENT_TIMEOUT, NULL);
            }
            m_gsm_atc.atc.recv_buff.index = 0;
            m_gsm_atc.atc.recv_buff.buffer[m_gsm_atc.atc.recv_buff.index] = 0;
        } else {
            /* retry send command */
            DEBUG_INFO("Resend ATC: %sExpect %s\r\n", m_gsm_atc.atc.cmd,
                       m_gsm_atc.atc.expect_resp_from_atc);
            m_gsm_atc.atc.current_timeout_atc_ms = sys_get_tick_ms();
            gsm_hw_uart_send_raw((uint8_t *)m_gsm_atc.atc.cmd, strlen(m_gsm_atc.atc.cmd));
        }
    }

    m_last_poll = sys_get_tick_ms();
}
