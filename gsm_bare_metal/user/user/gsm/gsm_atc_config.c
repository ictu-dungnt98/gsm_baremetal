#include <stdio.h>
#include <string.h>
#include <time.h>

#include "app_debug.h"
#include "gsm.h"
#include "gsm_utilities.h"
#include "hardware.h"

#define UNLOCK_BAND 0
#define CUSD_ENABLE 1

static char *m_list_command_startup[] = {
    "ATZ\r",
    "ATE0\r",       /* Turn off Echo Mode */
    "AT+CFUN=1\r",  /* Phone Full Functionality */
    "AT+CMEE=2\r",  /* Use Error Result Code: +CME ERROR:<err> */
    "AT+CSCLK=1\r", /* enable clock */
    NULL};

static char *m_list_command_init[] = {
    /* Init Sim properties */
    "AT+CMGD=1,4\r",                  /* Delete all messages */
    "AT+CMGF=1\r",                    /* Set SMS Text Mode */
    "AT+CPMS=\"SM\",\"SM\",\"SM\"\r", /* Select memory storages */
    "AT+CNMI=2,1\r",                  /* Set SMS notification */
    "AT+CSMP=49,167,0,0\r\n",         // CMD_SET_SMS_TEXT_MODE_PARAM
    "AT+CSDH=1\r",                    /* Set SMS notification */
    "AT+CVHU=0\r",
    "AT+COLP=0\r",
    NULL};

static gsm_manager_t m_gsm_manager;

static void gsm_atc_startup(gsm_response_event_t event, void *resp_buffer);
static void check_sim_card(gsm_response_event_t event, void *resp_buffer);
static void gsm_atc_config(gsm_response_event_t event, void *resp_buffer);
static void gsm_attack_internet(gsm_response_event_t event, void *resp_buffer);
static void gsm_query_info(gsm_response_event_t event, void *resp_buffer);

extern void gsm_change_state(gsm_state_t new_state);
extern void gsm_config_module(gsm_response_event_t event, void *resp_buffer);

static void gsm_atc_startup(gsm_response_event_t event, void *resp_buffer)
{
    static uint32_t step = 0;

    static uint32_t retry = 0;
    if (event != GSM_EVENT_OK) {
        if (retry++ > 3) {
            retry = 0;
            step = 0;
            gsm_change_state(GSM_STATE_RESET);
            return;
        }
        gsm_hw_send_at_cmd(m_list_command_startup[step], "OK\r\n", "", 10000, 10, gsm_atc_startup);
        return;
    }

    if (m_list_command_startup[step] != NULL) {
        gsm_hw_send_at_cmd(m_list_command_startup[step], "OK\r\n", "", 10000, 10, gsm_atc_startup);
        step++;
    } else { /* complete, change step */
        step = 0;
        gsm_change_hw_polling_interval(5);
        gsm_hw_send_at_cmd("AT\r\n", "OK\r\n", "", 2000, 1, check_sim_card);
    }

    retry = 0;
}

static void check_sim_card(gsm_response_event_t event, void *resp_buffer)
{
    static uint32_t step = 0;
    static uint32_t retry = 0;
    char *imei_buffer;

    switch (step) {
        case 0: { /* get module IMEI */
            gsm_hw_send_at_cmd("AT+CGSN\r\n", "OK\r\n", "", 1000, 5, check_sim_card);
            step++;
        } break;

        case 1: { /* get module IMEI */
            imei_buffer = gsm_get_module_imei();

            if (strlen(imei_buffer) < 15) {
                gsm_utilities_get_imei(resp_buffer, (uint8_t *)imei_buffer, 16);
                DEBUG_INFO("Get GSM IMEI: %s\r\n", gsm_get_module_imei());

                if (strlen(gsm_get_module_imei()) < 15) {
                    DEBUG_INFO("IMEI's invalid! Maybe gsm module error.\r\n");
                    goto sim_card_not_insert;
                }
            }

            /* get sim IMEI */
            gsm_hw_send_at_cmd("AT+CIMI\r\n", "OK\r\n", "", 1000, 5, check_sim_card);
            step++;
        } break;

        case 2: { /* get sim IMEI */
            imei_buffer = gsm_get_sim_imei();
            gsm_utilities_get_imei(resp_buffer, (uint8_t *)imei_buffer, 16);
            DEBUG_INFO("Get SIM IMEI: %s\r\n", gsm_get_sim_imei());

            if (strlen(gsm_get_sim_imei()) < 15) {
                DEBUG_ERROR("SIM's not inserted!\r\n");

                if (retry < 3) {
                    retry++;
                    gsm_hw_send_at_cmd("AT+CIMI\r\n", "", "OK\r\n", 1000, 5, check_sim_card);
                } else {
                    goto sim_card_not_insert;
                }
            } else {
                // Get SIM CCID
                gsm_hw_send_at_cmd("AT+CICCID\r\n", "+ICCID", "OK\r\n", 1000, 3, check_sim_card);
                gsm_change_hw_polling_interval(5);
                step++;
            }
        } break;

        case 3: {  // Get SIM CCID
            uint8_t *ccid_buffer = (uint8_t *)gsm_get_sim_ccid();
            if (strlen((char *)ccid_buffer) < 10) {
                gsm_utilities_get_sim_ccid(resp_buffer, ccid_buffer, 20);
            }
            DEBUG_INFO("SIM CCID: %s\r\n", ccid_buffer);

            if (strlen((char *)ccid_buffer) > 10) {
                /* complete, change step */
                gsm_hw_send_at_cmd("AT+CPIN?\r\n", "READY\r\n", "", 3000, 3, gsm_atc_config);
                step = 0;
            } else {
                if (retry < 3) {
                    retry++;
                    gsm_hw_send_at_cmd("AT+CICCID\r\n", "+ICCID", "OK\r\n", 1000, 3,
                                       check_sim_card);
                } else {
                    goto sim_card_not_insert;
                }
            }
        }

        default:
            break;
    }

    return;

sim_card_not_insert:
    step = 0;
    retry = 0;
    gsm_change_state(GSM_STATE_RESET);
    return;
}

static void gsm_atc_config(gsm_response_event_t event, void *resp_buffer)
{
    static uint32_t step = 0;
    static uint32_t retry = 0;

    if (event == GSM_EVENT_OK) {
        if (m_list_command_init[step] != NULL) {
            gsm_hw_send_at_cmd(m_list_command_init[step], "OK\r\n", "", 10000, 10, gsm_atc_config);
            step++;
        } else {
            /* complete, change step */
            step = 0;
            gsm_change_hw_polling_interval(50);
            gsm_hw_send_at_cmd("AT\r\n", "OK\r\n", "", 2000, 1, gsm_query_info);
        }
    } else {
        if (retry < 3) {
            retry++;
            gsm_hw_send_at_cmd(m_list_command_init[step], "OK\r\n", "", 10000, 10, gsm_atc_config);
        } else {
            step = 0;
            retry = 0;
            gsm_change_state(GSM_STATE_RESET);
        }
    }
}

static void gsm_query_info(gsm_response_event_t event, void *resp_buffer)
{
    static uint32_t step = 0;
    static uint32_t retry = 0;

    switch (step) {
        case 0: { /* get clock from module */
            gsm_hw_send_at_cmd("AT+CCLK?\r\n", "OK\r\n", "", 5000, 5, gsm_query_info);
            step++;
        } break;

        case 1: { /* get clock from module */
            struct tm *time_info = gsm_get_local_time();
            gsm_utilities_parse_timestamp_buffer(resp_buffer, time_info);
            gsm_hw_send_at_cmd("AT+CSQ\r\n", "", "OK\r\n", 3000, 2, gsm_query_info);
            step++;
            break;
        } break;

        case 2: {  // Get 4G signal strength
            if (event != GSM_EVENT_OK) {
                DEBUG_ERROR("Error get signal strength\n");
                step = 0;
                gsm_change_state(GSM_STATE_RESET);
                return;
            }

            uint8_t csq = 99;
            gsm_utilities_get_signal_strength_from_buffer(resp_buffer, &csq);
            DEBUG_INFO("CSQ: %d\r\n", csq);

            if (csq == 99) {  // Invalid CSQ =>> Polling CSQ again
                gsm_change_hw_polling_interval(100);
                gsm_hw_send_at_cmd("AT+CSQ\r\n", "OK\r\n", "", 5000, 3, gsm_query_info);
            } else { /* success */
                gsm_set_signal_strength(csq);
                /* Query Network Operator */
                gsm_hw_send_at_cmd("AT+COPS?\r\n", "OK\r\n", "", 5000, 3, gsm_query_info);
                step++;
            }
        } break;

        case 3: { /* Query Network Operator */
            char *network_operator = gsm_get_network_operator();
            gsm_utilities_get_network_operator(resp_buffer, network_operator, 20);
            DEBUG_INFO("network_operator: %s\r\n", network_operator);
            /* "Viettel" is shortest word */
            if (strlen(gsm_get_network_operator()) >= strlen("Viettel")) {
                gsm_hw_send_at_cmd("AT\r\n", "OK\r\n", "", 5000, 5, gsm_query_info);
                step++;
                return;
            }

            /* retry */
            if (retry < 5) {
                retry++;
                gsm_hw_send_at_cmd("AT+COPS?\r\n", "OK\r\n", "", 5000, 3, gsm_query_info);
            } else {
                step = 0;
                retry = 0;
                gsm_change_state(GSM_STATE_RESET);
            }
        } break;

        case 4: { /* start setup wireless connection */
            gsm_hw_send_at_cmd("AT\r\n", "OK\r\n", "", 3000, 3, gsm_attack_internet);
            step = 0;
        } break;

        default:
            break;
    }
}

static void gsm_attack_internet(gsm_response_event_t event, void *resp_buffer)
{
    static uint32_t step = 0;
    static uint32_t retry = 0;

    switch (step) {
        case 0: { /* network registration */
            gsm_hw_send_at_cmd("AT+CGREG=1\r\n", "OK\r\n", "", 1000, 3, gsm_attack_internet);
            step++;
        } break;

        case 1: { /* Network search mode AUTO */
            gsm_hw_send_at_cmd("AT+CNMP=2\r\n", "OK\r\n", "", 5000, 5, gsm_attack_internet);
            step++;
        } break;

        case 2: { /* attack gprs */
            gsm_hw_send_at_cmd("AT+CGATT=1\r\n", "OK\r\n", "", 5000, 5, gsm_attack_internet);
            gsm_change_hw_polling_interval(5);
            step++;
        } break;

        case 3: { /* set APN */
            gsm_hw_send_at_cmd("AT+CGDCONT=1,\"IP\",\"\"\r\n", "OK\r\n", "", 5000, 5, gsm_attack_internet);
            step++;
        } break;

        case 4: { /* query network status */
            gsm_hw_send_at_cmd("AT+CGREG?\r\n", "OK\r\n", "", 1000, 2, gsm_attack_internet);
            step++;
        } break;

        case 5: { /* module connected internet, change step to online */
            if (event == GSM_EVENT_OK) {
                gsm_hw_send_at_cmd("AT\r\n", "OK\r\n", "", 1000, 3, gsm_config_module);
                step = 0;
                retry = 0;
            } else {
                DEBUG_ERROR("GSM: Query network status fail\r\n");

                if (retry < 3) {
                    retry ++;
                    gsm_hw_send_at_cmd("AT+CGREG?\r\n", "OK\r\n", "", 1000, 2, gsm_attack_internet);
                } else {
                    step = 0;
                    retry = 0;
                    gsm_change_state(GSM_STATE_RESET);
                }
            }
        } break;

        default:
            break;
    }
}

extern gsm_manager_t m_gsm_manager;
void gsm_config_module(gsm_response_event_t event, void *resp_buffer)
{
    switch (m_gsm_manager.step) {
        case 1: {
            if (event != GSM_EVENT_OK) {
                DEBUG_ERROR("Connect modem ERR!\r\n");
                gsm_change_state(GSM_STATE_RESET);
            } else {
                gsm_hw_send_at_cmd("AT\r\n", "OK\r\n", "", 1000, 3, gsm_config_module);
                m_gsm_manager.step++;
            }
        } break;

        case 2: { /* start config module attack internet */
            gsm_atc_startup(event, resp_buffer);
            m_gsm_manager.step++;
        } break;

        case 3: {  // GSM ready, module connected internet
            gsm_change_hw_polling_interval(5);
            gsm_change_state(GSM_STATE_INTERNET_ATTACKED);
            m_gsm_manager.step = 0;
        } break;

        default:
            DEBUG_WARN("GSM unhandled step %u\r\n",  m_gsm_manager.step);
            break;
    }
}
