/**
  ******************************************************************************
  * @file    bsp_debug_usart.c
  * @author  fire
  * @version V1.0
  * @date    
  * @brief   重现c库printf函数到usart端口
  ******************************************************************************
  ******************************************************************************
  */ 
#include "bsp_debug_usart.h"
#include "string.h"
//#include <stdio.h>

void Debug_USART_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
		
  RCC_AHB1PeriphClockCmd( DEBUG_USART_RX_GPIO_CLK|DEBUG_USART_TX_GPIO_CLK, ENABLE);

  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
  
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(DEBUG_USART_RX_GPIO_PORT,DEBUG_USART_RX_SOURCE, DEBUG_USART_RX_AF);

  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(DEBUG_USART_TX_GPIO_PORT,DEBUG_USART_TX_SOURCE,DEBUG_USART_TX_AF);

  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_PIN  ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_PIN;
  GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
			
  /* USART1 mode config */
  USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(DEBUG_USART, &USART_InitStructure); 
  USART_Cmd(DEBUG_USART, ENABLE);
	
	USART_ClearFlag(DEBUG_USART, USART_FLAG_TC);
}

///重定向c库函数printf到USART1
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到USART1 */
		USART_SendData(DEBUG_USART, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}
typedef struct
{
	uint8_t data;		//数字
	uint8_t asicc;		//对应的ASICC数值
}_CHAR_TABLE;
_CHAR_TABLE chartable[16] = 
{
	{0, 	0x30},
	{1, 	0x31},
	{2, 	0x32},
	{3, 	0x33},
	{4, 	0x34},
	{5, 	0x35},
	{6, 	0x36},
	{7, 	0x37},
	{8, 	0x38},
	{9, 	0x39},
	{10, 	0x41},
	{11, 	0x42},
	{12, 	0x43},
	{13, 	0x44},
	{14, 	0x45},
	{15, 	0x46},
};
//打印字符串
static	uint8_t printf_recvbuf[501] = {0x30};
static	uint8_t printf_sendbuf[501] = {0x30};
void printfchar_NETrecv(uint8_t * pdata,uint16_t len)
{

	uint8_t count;
	uint8_t i;
	uint8_t data1_4;
	uint8_t data5_8;
	memset(printf_recvbuf,0x30,sizeof(printf_recvbuf));
	if(len > 250)
	{
		return;
	}
	for(count = 0;count < len;count++)
	{
		data5_8 = (pdata[count] & 0xf0) >> 4;
		data1_4 = pdata[count] & 0x0f;
		for(i = 0;i < 16;i++)
		{
			if(data1_4 == chartable[i].data)
			{
				printf_recvbuf[count*i] = chartable[i].asicc;
			}
			if(data5_8 == chartable[i].data)
			{
				printf_recvbuf[2*count +1] = chartable[i].asicc;
			}
		}
	
	}
	printf_recvbuf[2*len] = '\0';
	printf("4G recv [%s]",printf_recvbuf);    //打印接收到的数据
	
}

void printfchar_NETsend(uint8_t * pdata,uint16_t len)
{
	uint8_t count;
	uint8_t i;
	uint8_t data1_4;
	uint8_t data5_8;
	memset(printf_sendbuf,0x30,sizeof(printf_sendbuf));
	if(len > 250)
	{
		return;
	}
	for(count = 0;count < len;count++)
	{
		data5_8 = (pdata[count] & 0xf0) >> 4;
		data1_4 = pdata[count] & 0x0f;
		for(i = 0;i < 16;i++)
		{
			if(data1_4 == chartable[i].data)
			{
				printf_sendbuf[2*count] = chartable[i].asicc;
			}
			if(data5_8 == chartable[i].data)
			{
				printf_sendbuf[2*count +1] = chartable[i].asicc;
			}
		}
	
	}
	printf_sendbuf[2*len] = '\0';
	printf("4G Send [%s]",printf_sendbuf);    //打印接收到的数据
	
}
///重定向c库函数scanf到USART1
//int fgetc(FILE *f)
//{
//		/* 等待串口1输入数据 */
//		while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_RXNE) == RESET);

//		return (int)USART_ReceiveData(DEBUG_USART);
//}
/*********************************************END OF FILE**********************/
