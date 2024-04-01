#ifndef __DEBUG_USART_H
#define	__DEBUG_USART_H

#include "stm32f4xx.h"
#include <stdio.h>


//#define USART1_DR_Base  0x40013804		// 0x40013800 + 0x04 = 0x40013804
//#define SENDBUFF_SIZE   5000

#define DEBUG_USART                             USART2
#define DEBUG_USART_CLK                         RCC_APB1Periph_USART2
#define DEBUG_USART_BAUDRATE                    115200

#define DEBUG_USART_RX_GPIO_PORT                GPIOD
#define DEBUG_USART_RX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define DEBUG_USART_RX_PIN                      GPIO_Pin_6
#define DEBUG_USART_RX_AF                       GPIO_AF_USART2
#define DEBUG_USART_RX_SOURCE                   GPIO_PinSource6

#define DEBUG_USART_TX_GPIO_PORT                GPIOD
#define DEBUG_USART_TX_GPIO_CLK                 RCC_AHB1Periph_GPIOD
#define DEBUG_USART_TX_PIN                      GPIO_Pin_5
#define DEBUG_USART_TX_AF                       GPIO_AF_USART2
#define DEBUG_USART_TX_SOURCE                   GPIO_PinSource5


void Debug_USART_Config(void);
void printfchar_NETrecv(uint8_t * pdata,uint16_t len);
void printfchar_NETsend(uint8_t * pdata,uint16_t len);
//int fputc(int ch, FILE *f);

#endif /* __USART1_H */
