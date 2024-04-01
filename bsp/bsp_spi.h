#ifndef __BSP_SPI_H
#define	__BSP_SPI_H

#include "stm32f4xx.h"
#include "sysconfig.h"

/*
	串行Flash型号为 SST25VF020B (80MHz-2Mbit)
	PD10 = CS
	PB10 = SCK
	PB14 = MISO
	PB15 = MOSI

	STM32硬件SPI接口 = SPI2
	SPI1的时钟源是84M, SPI2 & 3的时钟源是42M
*/
#define SPI_FLASH_SPI		SPI2

#define ENABLE_SPI_RCC() 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE)

#define SPI_BAUD			SPI_BaudRatePrescaler_8		/* 选择8分频时, SCK时钟 = 5.25M */

/* 片选GPIO端口  */
#define SF_CS_GPIO			GPIOB
#define SF_CS_PIN			GPIO_Pin_12
/* 片选口线置低选中  */
#define SPI_FLASH_CS_LOW	(GPIOB->BSRRH = SF_CS_PIN)
/* 片选口线置高不选中 */
#define SPI_FLASH_CS_HIGH	(GPIOB->BSRRL = SF_CS_PIN)




void bsp_InitSPI2(void);
void bsp_InitSPI1(void);
/***********************************************************************************************
* Function		: SPI_SendRcvByte
* Description	: 读写spi数据
* Input			:
* Output		:
* Note(s)		: 
* Contributor	:
***********************************************************************************************/
INT8U SPI1_SendRcvByte( INT8U Data, INT8U *statu);

#endif /* __LED_H */

