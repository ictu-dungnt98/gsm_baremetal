#include <string.h>
#include "stm32f4xx_usart.h"
#include "hardware.h"

/* Interrupt for gsm uart */
extern void gsm_hw_layer_uart_fill_rx(uint8_t *data, uint32_t length);
void USART1_IRQHandler(void)
{
    uint8_t temp;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        /* Obtain a byte from the RX buffer register */
        temp = (uint8_t)(USART1->DR & (uint16_t)0x01FF);

        gsm_hw_layer_uart_fill_rx(&temp, 1);

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void uart_send_byte(char data)
{
    while (USART_GetFlagStatus(GSM_USART_PORT, USART_FLAG_TXE) == RESET);
    USART_SendData(GSM_USART_PORT, (uint16_t)data);
}

void uart_send_string(char *data, uint16_t length)
{
    uint16_t m_index = 0;
    if (data == NULL)
        return;

    while (length--) {
        uart_send_byte(data[m_index++]);
    }
}

/* UART_1 for Sim700c
 * PA9 - Tx
 * PA10 - Rx
 * Baudrate: 115200
 */
void uart_init(void)
{
    static bool is_gsm_uart_init = false;
    if (is_gsm_uart_init) {
        return;
    }
    is_gsm_uart_init = true;

    GPIO_InitTypeDef usartPin;
    USART_InitTypeDef usartGSM;
    memset(&usartPin, 0, sizeof(usartPin));
    memset(&usartGSM, 0, sizeof(usartGSM));

    usartPin.GPIO_Pin = GSM_USART_Tx_Pin | GSM_USART_Rx_Pin;
    usartPin.GPIO_Mode = GPIO_Mode_AF;
    usartPin.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &usartPin);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART2);

    usartGSM.USART_BaudRate = 115200;
    usartGSM.USART_WordLength = USART_WordLength_8b;
    usartGSM.USART_StopBits = USART_StopBits_1;
    usartGSM.USART_Parity = USART_Parity_No;
    usartGSM.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usartGSM.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usartGSM);

    /* NVIC configuration */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable USART */
    USART_Cmd(USART1, ENABLE);
    USART_ITConfig(GSM_USART_PORT, USART_IT_TXE, DISABLE);
    USART_ITConfig(GSM_USART_PORT, USART_IT_RXNE, ENABLE);
}
