/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_rs485.h
* Author			: 
* Date First Issued	: 
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_RS485_H_
#define __BSP_RS485_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/

#include "app_conf.h"
#define    RS485_1_RD_PORT2        GPIOD           
#define    RS485_1_RD_PIN2         GPIO_Pin_2
#define    RS485_RD_1_CLK2         RCC_AHB1Periph_GPIOD

#define    RS485_2_RD_PORT2        GPIOD           
#define    RS485_2_RD_PIN2         GPIO_Pin_7
#define    RS485_RD_2_CLK2         RCC_AHB1Periph_GPIOD


//官总板子
#if(GZ485ban ==1)
#define    RS485_RECV_ENABLE(index)  GPIO_ResetBits(BSP_RS485Table[index].RD.Port, BSP_RS485Table[index].RD.Pin)     //RE=1  //接收使能
#define    RS485_SEND_ENABLE(index)    GPIO_SetBits(BSP_RS485Table[index].RD.Port, BSP_RS485Table[index].RD.Pin)   //RE=0   //发送使能
#else
#define    RS485_SEND_ENABLE(index)  GPIO_ResetBits(BSP_RS485Table[index].RD.Port, BSP_RS485Table[index].RD.Pin)     //RE=1   //接收使能   ////新板子--莫总
#define    RS485_RECV_ENABLE(index)    GPIO_SetBits(BSP_RS485Table[index].RD.Port, BSP_RS485Table[index].RD.Pin)    //RE=0    //发送使能
#endif

//接收/发送使能，只能二选一
typedef enum
{
    CMD_RX_ENABLE,          //接收使能
    CMD_TX_ENABLE,          //发送使能
}_BSP_RS485_CMD;

__packed typedef struct
{
    GPIO_TypeDef* Port;     //端口号
    INT16U Pin;             //端口引脚
    INT32U PortClk;         //端口时钟
}_BSP_RS485_PIN;

__packed typedef struct
{
    USART_TypeDef* USARTx;
    _BSP_RS485_PIN RD;
    void (*RD_Init)(INT8U index);
}_BSP_RS485_CONFIG;


/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/*****************************************************************************
* Function     : RS485_RDCmd
* Description  : 485接收/发送使能
* Input        : USART_TypeDef* USARTx  --- 串口号
                 _BSP_RS485_CMD Cmd     --- CMD_RX_ENABLE：接收使能    、
                                            CMD_TX_ENABLE: 发送使能
* Output       : None
* Return       : 
* Note(s)      : 485接收使能跟发送使能只能二选一，不支持或操作
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U RS485_RDCmd(USART_TypeDef* USARTx, _BSP_RS485_CMD Cmd);

/*****************************************************************************
* Function     : RS485_Init
* Description  : 根据USARTx搜索在485表中的索引值
* Input        : USART_TypeDef* USARTx  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U RS485_Init(const USART_TypeDef* const USARTx);

/* Private functions--------------------------------------------------------------------------*/
		 
#endif //__BSP_RS485_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
