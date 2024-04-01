#ifndef __BSP_SPI_H
#define	__BSP_SPI_H

#include "stm32f4xx.h"
#include "sysconfig.h"

/*
	����Flash�ͺ�Ϊ SST25VF020B (80MHz-2Mbit)
	PD10 = CS
	PB10 = SCK
	PB14 = MISO
	PB15 = MOSI

	STM32Ӳ��SPI�ӿ� = SPI2
	SPI1��ʱ��Դ��84M, SPI2 & 3��ʱ��Դ��42M
*/
#define SPI_FLASH_SPI		SPI2

#define ENABLE_SPI_RCC() 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE)

#define SPI_BAUD			SPI_BaudRatePrescaler_8		/* ѡ��8��Ƶʱ, SCKʱ�� = 5.25M */

/* ƬѡGPIO�˿�  */
#define SF_CS_GPIO			GPIOB
#define SF_CS_PIN			GPIO_Pin_12
/* Ƭѡ�����õ�ѡ��  */
#define SPI_FLASH_CS_LOW	(GPIOB->BSRRH = SF_CS_PIN)
/* Ƭѡ�����ø߲�ѡ�� */
#define SPI_FLASH_CS_HIGH	(GPIOB->BSRRL = SF_CS_PIN)




void bsp_InitSPI2(void);
void bsp_InitSPI1(void);
/***********************************************************************************************
* Function		: SPI_SendRcvByte
* Description	: ��дspi����
* Input			:
* Output		:
* Note(s)		: 
* Contributor	:
***********************************************************************************************/
INT8U SPI1_SendRcvByte( INT8U Data, INT8U *statu);

#endif /* __LED_H */

