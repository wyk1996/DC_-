/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: BSP_UART.c
* Author			: 
* Date First Issued	: 2018年5月25日
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "bsp_rs485.h"
//#include "bsp_Rs485.h"

/* Private define-----------------------------------------------------------------------------*/
//串口6作为RS485使用配置
#define    RS485_1_UART            UART4    //抄表485
#define    RS485_2_UART			   /*USART7*/USART2	 //备用，与gprs公用		
/* Private typedef----------------------------------------------------------------------------*/

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : RS485_RD2Init
* Description  : UART2转RS485的RE/DE引脚配置，UART2相关配置需要调用串口驱动里的初始化函数
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void RS485_RD2Init(INT8U index);
/* Private variables--------------------------------------------------------------------------*/   
//使用485的串口表
const _BSP_RS485_CONFIG BSP_RS485Table[] =
{
    {RS485_1_UART, {RS485_1_RD_PORT2, RS485_1_RD_PIN2, RS485_RD_1_CLK2}, RS485_RD2Init},			//抄表485
	
	{RS485_2_UART, {RS485_2_RD_PORT2, RS485_2_RD_PIN2, RS485_RD_2_CLK2}, RS485_RD2Init},			//备用485与GPRS串口公用
};


/*****************************************************************************
* Function     : RS485_RD2Init
* Description  : UART2转RS485的RE/DE引脚配置，UART2相关配置需要调用串口驱动里的初始化函数
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void RS485_RD2Init(INT8U index)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

    if (index >= sizeof(BSP_RS485Table) / sizeof(BSP_RS485Table[0]) )
    {
        return;
    }

    //时钟配置
	RCC_AHB1PeriphClockCmd(BSP_RS485Table[index].RD.PortClk, ENABLE);      //使能RE/DE端口时钟
    
  	//配置MAX485的RE引脚，推挽输出，默认低电平
  	GPIO_InitStructure.GPIO_Pin = BSP_RS485Table[index].RD.Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉使能
  	GPIO_Init(BSP_RS485Table[index].RD.Port, &GPIO_InitStructure);

//    GPIO_ResetBits(RS485_1_RD_PORT2, RS485_1_RD_PIN2);      //RD = 0
  
	RS485_RECV_ENABLE(index);                           //接收使能
	// RS485_SEND_ENABLE(index);                           //接收使能
}

/*****************************************************************************
* Function     : SearchRS485Table
* Description  : 根据USARTx搜索在485表中的索引值
* Input        : const USART_TypeDef* const USARTx  
* Output       : None
* Return       : 表中的索引值，0xff:无效
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static INT8U SearchRS485Table(const USART_TypeDef* const USARTx)
{
    INT8U i;
    
    if ( !IS_USART_ALL_PERIPH(USARTx) )
    {
        return 0xff;
    }

    for (i = 0; i < sizeof(BSP_RS485Table) / sizeof(BSP_RS485Table[0]); i++)
    {
        if (USARTx == BSP_RS485Table[i].USARTx)
        {
            break;
        }
    }
    if (i == sizeof(BSP_RS485Table) / sizeof(BSP_RS485Table[0]) )
    {
        return 0xff;
    }
    return i;
}

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
INT8U RS485_RDCmd(USART_TypeDef* USARTx, _BSP_RS485_CMD Cmd)
{
    INT8U index;
    
    if ( !IS_USART_ALL_PERIPH(USARTx) )
    {
        return FALSE;
    }
    
    if ( (index = SearchRS485Table(USARTx) ) >=  sizeof(BSP_RS485Table) / sizeof(BSP_RS485Table[0]) )  //返回表的索引值
    {
        return FALSE;
    }
    
    if (Cmd == CMD_RX_ENABLE) //接收使能，RE=0 DE=1
    {
//        GPIO_SetBits(BSP_RS485Table[index].RD.Port, BSP_RS485Table[index].RD.Pin);     //RE=1
        RS485_RECV_ENABLE(index); //接收使能
    }
    else
    {
//        GPIO_ResetBits(BSP_RS485Table[index].RD.Port, BSP_RS485Table[index].RD.Pin);   //RE=0
        RS485_SEND_ENABLE(index); //发送使能
    }
    return TRUE;
}

/*****************************************************************************
* Function     : RS485_Init
* Description  : 根据USARTx搜索在485表中的索引值
* Input        : USART_TypeDef* USARTx  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U RS485_Init(const USART_TypeDef* const USARTx)
{
    INT8U index;
     
    if ( !IS_USART_ALL_PERIPH(USARTx) )
    {
        return FALSE;
    }
    
    if ( (index = SearchRS485Table(USARTx) ) >=  sizeof(BSP_RS485Table) / sizeof(BSP_RS485Table[0]) )  //返回表的索引值
    {
        return FALSE;
    }

    if (BSP_RS485Table[index].RD_Init) //执行对应的初始化函数
    {
        BSP_RS485Table[index].RD_Init(index);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/************************(C)COPYRIGHT 2018 杭州快电****END OF FILE****************************/
