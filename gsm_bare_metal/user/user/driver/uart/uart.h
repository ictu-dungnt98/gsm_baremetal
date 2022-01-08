#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

void uart_init(void);
void uart_send_byte(char data);
void uart_send_string(char *data, uint16_t length);

#endif
