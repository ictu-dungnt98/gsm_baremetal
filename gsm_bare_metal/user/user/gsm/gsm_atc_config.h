#ifndef _GSM_ATC_H_
#define _GSM_ATC_H_

#include "gsm.h"

#define VIETTEL         "Viettel"
#define VIETNAMMOBILE   "Vietnamobile"
#define VINAPHONE       "VinaPhone"
#define MOBILEFONE      "Mobifone"

typedef enum {
	BBC				= 0,		/* Fucking misc operator :v */
	vinaphone 		= 1,
	viettel			= 2,
	mobiphone 		= 3,
	vietnammobie 	= 4,
} operator_t;

void gsm_config_module(gsm_response_event_t event, void *resp_buffer);

void gsm_at_cb_read_sms(gsm_response_event_t event, void *resp_buffer);
void gsm_at_cb_send_sms(gsm_response_event_t event, void *resp_buffer);


#endif