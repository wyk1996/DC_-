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
#include "bsp_uart.h"
#include "bsp_rs485.h"
#include "bsp_conf.h" 
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/

/* Private variables--------------------------------------------------------------------------*/   
_BSP_UART_CONTROL   UARTControl[UART_MAX_NUM];
static _BSP_UART_CONTROL*  pUart[UART_MAX_NUM];
_BSP_RECV_CONTROL RecvControl[UART_MAX_NUM] = {0};
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : UART1_PIN_CFG
* Description  : 串口1引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART1_PIN_CFG(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);       //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	    //使能APB2时钟

    //引脚复用器映射配置
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   //PA9复用为USART1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  //PA10复用为USART1
    
  	/* Configure USART1 Tx (PA9) Rx (PA10) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                //上拉使能
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*****************************************************************************
* Function     : UART2_PIN_CFG
* Description  : 串口2引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  :2018年5月25日
*****************************************************************************/
static void UART2_PIN_CFG(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);      //使能GPIOD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	   //使能APB1时钟

    //引脚复用器映射配置
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);  //PD5复用为USART2
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);  //PD6复用为USART2

  	/* Configure USART2 Tx (PA5) Rx (PA6) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               //上拉使能
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/*****************************************************************************
* Function     : UART3_PIN_CFG
* Description  : 串口3引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART3_PIN_CFG(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);       //使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);      //使能APB1时钟

    //引脚复用器映射配置
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);   //PD8复用为USART3
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);   //PD9复用为USART3

  	/* Configure USART3 Tx (PD8) Rx (PD9) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                //上拉使能
  	GPIO_Init(GPIOD, &GPIO_InitStructure);


}

/*****************************************************************************
* Function     : UART4_PIN_CFG
* Description  : 串口4引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART4_PIN_CFG(void)
{

  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);       //使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);      //使能APB1时钟

      //引脚复用器映射配置
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_UART4);   //PD8复用为USART3
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF11_UART4);   //PD8复用为USART3
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF11_UART4);   //PD9复用为USART3

   	/* Configure UART4 Tx (PD1) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                //上拉使能
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/*****************************************************************************
* Function     : UART5_PIN_CFG
* Description  : 串口5引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART5_PIN_CFG(void)
{

  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    //引脚复用器映射配置
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);  //PC12复用为UART5
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource2 , GPIO_AF_UART5);  //PD2复用为UART5

  	/* Configure UART5 Tx (PC12) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               //上拉使能
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

  	/* Configure USART5 Rx (PD2) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               //上拉使能
  	GPIO_Init(GPIOD, &GPIO_InitStructure);

}

/*****************************************************************************
* Function     : UART6_PIN_CFG
* Description  : 串口6引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART6_PIN_CFG(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

    //引脚复用器映射配置
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);   //PC6复用为USART6
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);   //PC7复用为USART6

  	/* Configure USART5 Tx (PH10) Rx (PH11) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               //上拉使能
  	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	

}

/*****************************************************************************
* Function     : UART7_PIN_CFG
* Description  : 串口7引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART7_PIN_CFG(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE);
    //引脚复用器映射配置
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_UART7);   //PE7复用为USART7
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_UART7);   //PE8复用为USART7

  	/* Configure USART5 Tx (PE7) Rx (PE8) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               //上拉使能
  	GPIO_Init(GPIOE, &GPIO_InitStructure);

}

/*****************************************************************************
* Function     : UART8_PIN_CFG
* Description  : 串口8引脚初始化
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART8_PIN_CFG(void)
{
  	GPIO_InitTypeDef GPIO_InitStructure;

    //时钟配置
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
    //引脚复用器映射配置
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_UART8);   //PE0复用为USART8
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_UART8);   //PE1复用为USART8

  	/* Configure USART5 Tx (PE7) Rx (PE8) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;               //上拉使能
  	GPIO_Init(GPIOE, &GPIO_InitStructure);

}

//串口引脚信息
static const _UART_HARDWARE_INFO Uart_HardwareTable[UART_MAX_NUM] =
{
    USART1,       UART1_PIN_CFG,
    USART2,       UART2_PIN_CFG,
    USART3,       UART3_PIN_CFG,
    UART4 ,       UART4_PIN_CFG,
    UART5 ,       UART5_PIN_CFG,
    USART6,       UART6_PIN_CFG,
	UART7 ,       UART7_PIN_CFG,
	UART8,        UART8_PIN_CFG,
};

/*****************************************************************************
* Function     : GetCommHandle
* Description  : 返回该串口的结构体
* Input        : INT8U ComPort  
* Output       : None
* Return       : S_BSP_UARTx*
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static _BSP_UART_CONTROL* GetCommHandle(INT8U ComPort)
{
    if (ComPort >= UART_MAX_NUM)
    {
        return NULL;
    }
    return &UARTControl[ComPort];
}

/*****************************************************************************
* Function     : GetComPort
* Description  : 通过USARTx获得内部索引值
* Input        : const USART_TypeDef* const USARTx  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static INT8U GetComPort(const USART_TypeDef* const USARTx)
{
    for (INT8U i = 0; i < UART_MAX_NUM; i++)
    {
        if ( (pUart[i]->pUartHardInfo) && (USARTx == pUart[i]->pUartHardInfo->UARTx) )
        {
            return i;
        }
    }
    return 0xff;
}


/*****************************************************************************
* Function     : UART_ControlClear
* Description  : 初始化读写指针
* Input        : _BSP_UART_CONTROL* pControl  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART_ControlClear(_BSP_UART_BUF* const pControl)
{
    if (pControl == NULL)
    {
        return;
    }
    pControl->pBuf = NULL;
    pControl->pBufLen = NULL;
    pControl->ReadPoint = 0;
    pControl->WritePoint = 0;
}

/*****************************************************************************
* Function     : BSPUART_GetDMAInfo
* Description  : 获取串口DMA发送信息
* Input        : const USART_TypeDef* const USARTx  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
_BSP_UART_DMA* BSPUART_GetDMAInfo(const USART_TypeDef* const USARTx)
{
    INT8U comport;
    
    //判断串口号
    if ( !IS_USART_ALL_PERIPH(USARTx) || ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) )
    {
        return NULL;
    }
    return &UARTControl[comport].Setting.DMASet;
}

/*****************************************************************************
* Function     : BSP_UARTDMAEnable
* Description  : 对串口的TX开启DMA传输
* Input        : DMA_Stream_TypeDef *DMA_Streamx  
                 INT16U len                       
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :2018年5月25日
*****************************************************************************/
INT8U BSP_UARTDMAEnable(DMA_Stream_TypeDef *DMA_Streamx, INT16U len)
{
    if (!IS_DMA_ALL_PERIPH(DMA_Streamx) || !len)
    {
        return FALSE;
    }
    DMA_Cmd(DMA_Streamx, DISABLE);                      //关闭DMA传输 
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE);	//确保DMA可以被设置  
		
	DMA_SetCurrDataCounter(DMA_Streamx,len);            //数据传输量  
	
    DMA_ITConfig(DMA_Streamx, DMA_IT_TC, ENABLE);       //开启发送完成中断

    USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);    //使能串口的DMA发送
    
	DMA_Cmd(DMA_Streamx, ENABLE);                       //开启DMA传输 

    return TRUE;
}

/*****************************************************************************
* Function     : BSP_UARTTxDMAConfigInit
* Description  : 串口发送DMA配置初始化
* Input        : const _BSP_UART_DMA* const pUartDMA  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U BSP_UARTTxDMAConfigInit(const _BSP_UART_DMA* const pUartDMA)
{
    //判断参数是否合法
    if ( (pUartDMA == NULL) || !IS_DMA_ALL_PERIPH(pUartDMA->Streamx) || !IS_DMA_CHANNEL(pUartDMA->Channel) \
        || !pUartDMA->PeripheralAddr || !pUartDMA->MemoryAddr)
    {
        return FALSE;
    }
    
    DMA_InitTypeDef  DMA_InitStructure;

    DMA_InitStructure.DMA_Channel = pUartDMA->Channel;                      //通道选择
    DMA_InitStructure.DMA_PeripheralBaseAddr = pUartDMA->PeripheralAddr;    //DMA外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr = pUartDMA->MemoryAddr;           //DMA 存储器0地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                 //存储器到外设模式
    DMA_InitStructure.DMA_BufferSize = pUartDMA->Len;                       //数据传输量 
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //外设非增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //存储器增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设数据长度:8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         //存储器数据长度:8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           //使用普通模式 
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                   //中等优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;             //存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;     //外设突发单次传输
    DMA_Init(pUartDMA->Streamx, &DMA_InitStructure);                        //初始化DMA Stream
    return TRUE;
}

/*****************************************************************************
* Function     : BSP_UARTInit
* Description  : 串口初始化，供用户调用
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void BSP_UARTInit(void)
{
    for (INT8U i = 0; i < UART_MAX_NUM; i++)
    {
        pUart[i] = &UARTControl[i];
        pUart[i]->UartTxControl.pBufLen = &pUart[i]->Setting.TxBufLen;
        pUart[i]->UartRxControl.pBufLen = &pUart[i]->Setting.RxBufLen;
        pUart[i]->pUartHardInfo = &Uart_HardwareTable[i];
        pUart[i]->SendState = UART_STATE_IDLE;   //发送空闲
    }
}

/*****************************************************************************
* Function     : BSP_UARTDeInit
* Description  : 串口重新初始化
* Input        : INT8U const ComPort  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void BSP_UARTDeInit(const USART_TypeDef* const USARTx)
{
    INT8U comport;
        
    //判断串口号
    if ( !IS_USART_ALL_PERIPH(USARTx) || ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) )
    {
        return;
    }
    
    UART_ControlClear(&pUart[comport]->UartTxControl);
    UART_ControlClear(&pUart[comport]->UartRxControl);
    memset(&pUart[comport]->Setting, 0, sizeof(_BSP_UART_SET));
    pUart[comport]->pUartHardInfo = NULL;
}

/*****************************************************************************
* Function     : BSP_UARTConfigInit
* Description  : 串口配置初始化，供用户调用
* Input        : const USART_TypeDef* const USARTx             
                 const _BSP_UART_SET* const UartSet  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U BSP_UARTConfigInit(const USART_TypeDef* const USARTx, const _BSP_UART_SET* const UartSet)
{   
    USART_InitTypeDef	USART_InitStructure;
	_BSP_UART_CONTROL *BSP_UARTx;
    INT8U comport;

    //判断串口号
    if ( !IS_USART_ALL_PERIPH(USARTx) || ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) )
    {
        return FALSE;
    }
    //检查参数是否有效
    if (UartSet == NULL)
    {
        return FALSE;
    }

    //不是调试模式下，必须要提供两个BUF
    if ( (BSPUART_GET_MODE(UartSet->Mode) != UART_DEBUG_MODE) && ( (UartSet->RxBuf == NULL) || (UartSet->TxBuf == NULL) ) )
    {
        return FALSE;
    }
    //在非调试模式下使用DMA,不支持
    if ( (BSPUART_GET_MODE(UartSet->Mode) != UART_DEBUG_MODE) && (UartSet->Mode & UART_TX_DMA) )
    {
        return FALSE;
    }
    //在DMA模式下，判断DMA数据流是否为null
    if ( (UartSet->Mode & UART_TX_DMA) && (UartSet->DMASet.Streamx == NULL) )
    {
        return FALSE;
    }

	/* 参数有效性检查 */
	if ( (comport <= 4) && ( (UartSet->DataBits < BSPUART_WORDLENGTH_7) || (UartSet->DataBits > BSPUART_WORDLENGTH_8) ) )
	{
		return FALSE;
	}
	if (UartSet->Parity > BSPUART_PARITY_EVEN)
	{
		return FALSE;
	}
	if ( (UartSet->StopBits != BSPUART_STOPBITS_1) && (UartSet->StopBits != BSPUART_STOPBITS_2) )
	{
		return FALSE;
	}
 	if ( (UartSet->BaudRate < 300) || (UartSet->BaudRate > 115200) )
 	{
		return FALSE;
 	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_7) && (UartSet->Parity == BSPUART_PARITY_NO) && (comport < 5) )
	{
		return FALSE; /* CORTEX串口不支持7位无校验方式，改为有校验的吧*/
	}

	if ( (BSP_UARTx = GetCommHandle(comport) )== NULL)                                            //获取该串口的结构体
	{
        return FALSE;
	}
	memcpy(&BSP_UARTx->Setting, (void *)UartSet, sizeof(_BSP_UART_SET));                          //拷贝用户串口配置
    pUart[comport]->UartTxControl.pBuf = pUart[comport]->Setting.TxBuf;
    pUart[comport]->UartRxControl.pBuf = pUart[comport]->Setting.RxBuf;

    if (UartSet->Mode & UART_MSG_MODE)                                                            //使用消息队列
    {
        INT8U err;
        //消息队列指针无效或者不是消息队列
        if  ( (UartSet->RxQueue == NULL) || (UartSet->RxQueue->OSEventType != OS_EVENT_TYPE_Q) )
        {
            return FALSE;
        }
        //消息地址不存在或者长度小于一个消息的长度
        if ( (UartSet->RxMsgMemBuf == NULL) || (UartSet->RxMsgMemLen < sizeof(_BSP_MESSAGE) ) )
        {
            return FALSE;
        }
        //创建一个内存管理
        pUart[comport]->UartRxInterval.pMsgMem = OSMemCreate(UartSet->RxMsgMemBuf, UartSet->RxMsgMemLen / sizeof(_BSP_MESSAGE), sizeof(_BSP_MESSAGE), &err);
        if (OS_ERR_NONE != err)
        {
            return FALSE;
        }
        pUart[comport]->UartRxInterval.RxInterval = OS_TICKS_PER_SEC * 10 / UartSet->BaudRate + 1; //采用波特率来计算帧间隔时间
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
   		OS_CPU_SR  cpu_sr = 0;
#endif	        
        OS_ENTER_CRITICAL();
        pUart[comport]->UartRxInterval.RxIntervalIndex = 0;
        OS_EXIT_CRITICAL();
    }
    if (UartSet->Mode & UART_TX_DMA)                                                               //使用发送DMA
    {
        if ( (INT32U)pUart[comport]->Setting.DMASet.Streamx > (INT32U)DMA2)
        {
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);                                    //使能DMA2时钟
        }
        else
        {
    	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);                                    //使能DMA1时钟
    	}
        DMA_DeInit(pUart[comport]->Setting.DMASet.Streamx);     
        while (DMA_GetCmdStatus(pUart[comport]->Setting.DMASet.Streamx) != DISABLE);               //等待DMA可配置 
        if (BSP_UARTTxDMAConfigInit(&(pUart[comport]->Setting.DMASet) ) == FALSE)
        {
            return FALSE;
        }
//        USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);                                               //使能串口的DMA发送
    }
    
    if (BSPUART_GET_MODE(UartSet->Mode) == UART_RS485_MODE)                                        //485模式
    {
        RS485_Init(USARTx);                                                                        //初始化RS485引脚，默认处于接收使能
    }
    pUart[comport]->pUartHardInfo->PinCfg();                                                       //初始化对应的串口引脚
	USART_InitStructure.USART_BaudRate = UartSet->BaudRate;                                        //设置波特率
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_7) && (UartSet->Parity == BSPUART_PARITY_ODD) )  //7位数据长度+奇校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_7) && (UartSet->Parity == BSPUART_PARITY_EVEN) ) //7位数据长度+偶校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}

	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_8) && (UartSet->Parity == BSPUART_PARITY_NO) )   //8位数据长度+无校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_No;
	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_8) && (UartSet->Parity == BSPUART_PARITY_ODD) )  //8位数据长度+奇校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_8) && (UartSet->Parity == BSPUART_PARITY_EVEN) ) //8位数据长度+偶校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}

	if (UartSet->StopBits == BSPUART_STOPBITS_1)                                                    //停止位
	{
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
	}
	if(UartSet->StopBits == BSPUART_STOPBITS_2)
	{
		USART_InitStructure.USART_StopBits = USART_StopBits_2;
	}
	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;                //无硬件控制流
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(BSP_UARTx->pUartHardInfo->UARTx, &USART_InitStructure);                             //初始化串口配置
    if (UART_DEBUG_MODE != BSPUART_GET_MODE(UartSet->Mode) )                                       //不是调试模式，必须开启接收中断
    {
    	USART_ITConfig(BSP_UARTx->pUartHardInfo->UARTx, /*USART_IT_TC |*/ USART_IT_RXNE, ENABLE);  //使能接收寄存器非空中断
    }
    else if ( (BSPUART_GET_MODE(UartSet->Mode) == UART_DEBUG_MODE) && (UartSet->RxBuf != NULL) ) //调试模式下提供了接收缓冲区，开启接收
    {
        USART_ITConfig(BSP_UARTx->pUartHardInfo->UARTx, /*USART_IT_TC |*/ USART_IT_RXNE, ENABLE);  //使能接收寄存器非空中断
    }
	USART_Cmd(BSP_UARTx->pUartHardInfo->UARTx,ENABLE);		                                       //使能该串口		
	USART_GetFlagStatus(BSP_UARTx->pUartHardInfo->UARTx, USART_FLAG_TC);                           //读取USART_SR寄存器，再写USART_DR，完成对TC标志位的清零，否则上电发送的第一个字节可能会丢失
	return TRUE;

}

/*****************************************************************************
* Function     : BSP_UARTConfigTimeInit
* Description  : 串口配置初始化，供用户调用,时间间隔不一样
* Input        : const USART_TypeDef* const USARTx             
                 const _BSP_UART_SET* const UartSet  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U BSP_UARTConfigTimeInit(const USART_TypeDef* const USARTx, const _BSP_UART_SET* const UartSet)
{   
    USART_InitTypeDef	USART_InitStructure;
	_BSP_UART_CONTROL *BSP_UARTx;
    INT8U comport;

    //判断串口号
    if ( !IS_USART_ALL_PERIPH(USARTx) || ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) )
    {
        return FALSE;
    }
    //检查参数是否有效
    if (UartSet == NULL)
    {
        return FALSE;
    }

    //不是调试模式下，必须要提供两个BUF
    if ( (BSPUART_GET_MODE(UartSet->Mode) != UART_DEBUG_MODE) && ( (UartSet->RxBuf == NULL) || (UartSet->TxBuf == NULL) ) )
    {
        return FALSE;
    }
    //在非调试模式下使用DMA,不支持
    if ( (BSPUART_GET_MODE(UartSet->Mode) != UART_DEBUG_MODE) && (UartSet->Mode & UART_TX_DMA) )
    {
        return FALSE;
    }
    //在DMA模式下，判断DMA数据流是否为null
    if ( (UartSet->Mode & UART_TX_DMA) && (UartSet->DMASet.Streamx == NULL) )
    {
        return FALSE;
    }

	/* 参数有效性检查 */
	if ( (comport <= 4) && ( (UartSet->DataBits < BSPUART_WORDLENGTH_7) || (UartSet->DataBits > BSPUART_WORDLENGTH_8) ) )
	{
		return FALSE;
	}
	if (UartSet->Parity > BSPUART_PARITY_EVEN)
	{
		return FALSE;
	}
	if ( (UartSet->StopBits != BSPUART_STOPBITS_1) && (UartSet->StopBits != BSPUART_STOPBITS_2) )
	{
		return FALSE;
	}
 	if ( (UartSet->BaudRate < 300) || (UartSet->BaudRate > 115200) )
 	{
		return FALSE;
 	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_7) && (UartSet->Parity == BSPUART_PARITY_NO) && (comport < 5) )
	{
		return FALSE; /* CORTEX串口不支持7位无校验方式，改为有校验的吧*/
	}

	if ( (BSP_UARTx = GetCommHandle(comport) )== NULL)                                            //获取该串口的结构体
	{
        return FALSE;
	}
	memcpy(&BSP_UARTx->Setting, (void *)UartSet, sizeof(_BSP_UART_SET));                          //拷贝用户串口配置
    pUart[comport]->UartTxControl.pBuf = pUart[comport]->Setting.TxBuf;
    pUart[comport]->UartRxControl.pBuf = pUart[comport]->Setting.RxBuf;

    if (UartSet->Mode & UART_MSG_MODE)                                                            //使用消息队列
    {
        INT8U err;
        //消息队列指针无效或者不是消息队列
        if  ( (UartSet->RxQueue == NULL) || (UartSet->RxQueue->OSEventType != OS_EVENT_TYPE_Q) )
        {
            return FALSE;
        }
        //消息地址不存在或者长度小于一个消息的长度
        if ( (UartSet->RxMsgMemBuf == NULL) || (UartSet->RxMsgMemLen < sizeof(_BSP_MESSAGE) ) )
        {
            return FALSE;
        }
        //创建一个内存管理
        pUart[comport]->UartRxInterval.pMsgMem = OSMemCreate(UartSet->RxMsgMemBuf, UartSet->RxMsgMemLen / sizeof(_BSP_MESSAGE), sizeof(_BSP_MESSAGE), &err);
        if (OS_ERR_NONE != err)
        {
            return FALSE;
        }
        pUart[comport]->UartRxInterval.RxInterval = OS_TICKS_PER_SEC * 10 / UartSet->BaudRate + 5; //采用波特率来计算帧间隔时间
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
   		OS_CPU_SR  cpu_sr = 0;
#endif	        
        OS_ENTER_CRITICAL();
        pUart[comport]->UartRxInterval.RxIntervalIndex = 0;
        OS_EXIT_CRITICAL();
    }
    if (UartSet->Mode & UART_TX_DMA)                                                               //使用发送DMA
    {
        if ( (INT32U)pUart[comport]->Setting.DMASet.Streamx > (INT32U)DMA2)
        {
            RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);                                    //使能DMA2时钟
        }
        else
        {
    	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);                                    //使能DMA1时钟
    	}
        DMA_DeInit(pUart[comport]->Setting.DMASet.Streamx);     
        while (DMA_GetCmdStatus(pUart[comport]->Setting.DMASet.Streamx) != DISABLE);               //等待DMA可配置 
        if (BSP_UARTTxDMAConfigInit(&(pUart[comport]->Setting.DMASet) ) == FALSE)
        {
            return FALSE;
        }
//        USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);                                               //使能串口的DMA发送
    }
    
    if (BSPUART_GET_MODE(UartSet->Mode) == UART_RS485_MODE)                                        //485模式
    {
        RS485_Init(USARTx);                                                                        //初始化RS485引脚，默认处于接收使能
    }
    pUart[comport]->pUartHardInfo->PinCfg();                                                       //初始化对应的串口引脚
	USART_InitStructure.USART_BaudRate = UartSet->BaudRate;                                        //设置波特率
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_7) && (UartSet->Parity == BSPUART_PARITY_ODD) )  //7位数据长度+奇校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_7) && (UartSet->Parity == BSPUART_PARITY_EVEN) ) //7位数据长度+偶校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}

	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_8) && (UartSet->Parity == BSPUART_PARITY_NO) )   //8位数据长度+无校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_No;
	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_8) && (UartSet->Parity == BSPUART_PARITY_ODD) )  //8位数据长度+奇校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}
	if ( (UartSet->DataBits == BSPUART_WORDLENGTH_8) && (UartSet->Parity == BSPUART_PARITY_EVEN) ) //8位数据长度+偶校验
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}

	if (UartSet->StopBits == BSPUART_STOPBITS_1)                                                    //停止位
	{
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
	}
	if(UartSet->StopBits == BSPUART_STOPBITS_2)
	{
		USART_InitStructure.USART_StopBits = USART_StopBits_2;
	}
	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;                //无硬件控制流
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(BSP_UARTx->pUartHardInfo->UARTx, &USART_InitStructure);                             //初始化串口配置
    if (UART_DEBUG_MODE != BSPUART_GET_MODE(UartSet->Mode) )                                       //不是调试模式，必须开启接收中断
    {
    	USART_ITConfig(BSP_UARTx->pUartHardInfo->UARTx, /*USART_IT_TC |*/ USART_IT_RXNE, ENABLE);  //使能接收寄存器非空中断
    }
    else if ( (BSPUART_GET_MODE(UartSet->Mode) == UART_DEBUG_MODE) && (UartSet->RxBuf != NULL) ) //调试模式下提供了接收缓冲区，开启接收
    {
        USART_ITConfig(BSP_UARTx->pUartHardInfo->UARTx, /*USART_IT_TC |*/ USART_IT_RXNE, ENABLE);  //使能接收寄存器非空中断
    }
	USART_Cmd(BSP_UARTx->pUartHardInfo->UARTx,ENABLE);		                                       //使能该串口		
	USART_GetFlagStatus(BSP_UARTx->pUartHardInfo->UARTx, USART_FLAG_TC);                           //读取USART_SR寄存器，再写USART_DR，完成对TC标志位的清零，否则上电发送的第一个字节可能会丢失
	return TRUE;

}

#if 1
/*****************************************************************************
* Function     : UART_RxMode
* Description  : 串口接收使能模式
* Input        : USART_TypeDef* const USARTx  
                 FunctionalState State--ENABLE：接收使能 DISABLE:接收禁止              
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void UART_RxMode(USART_TypeDef* const USARTx, FunctionalState State)
{
	volatile INT16U tmpreg = 0x00;
	
    if (!IS_USART_ALL_PERIPH(USARTx) ) 
	{
        return;
	}
    if (State == ENABLE) //接收使能
    {
        USARTx->CR1 |= USART_Mode_Rx;
    }
    else                //使能禁止
    {
        USARTx->CR1 &= ~USART_Mode_Rx;
    }
}
#endif

/*****************************************************************************
* Function     : IsFull
* Description  : 缓冲区满，此函数在写操作时使用
* Input        : INT8U ComPort  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U IsFull(const _BSP_UART_BUF* const Control)
{
    if (Control == NULL)
    {
        return FALSE;
    }

    return ( (Control->WritePoint + 1) % (*Control->pBufLen) == Control->ReadPoint);
}


/*****************************************************************************
* Function     : IsEmpty
* Description  : 缓冲区空，此函数在读操作时使用
* Input        : INT8U ComPort  
* Output       : None
* Return       : 0:非空， 1:空
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U IsEmpty(const _BSP_UART_BUF* const Control)
{
    if (Control == NULL)
    {
        return FALSE;
    }

    return (Control->ReadPoint == Control->WritePoint);
}


/*****************************************************************************
* Function     : UART_PopOneByte
* Description  : 读取一个字节
* Input        : INT8U ComPort  
* Output       : None
* Return       : -1:错误或缓冲区空 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static int UART_PopOneByte(_BSP_UART_BUF* const pControl)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    INT8U val;
    
    if (pControl == NULL)
    {
        return -1;
    }

    OS_ENTER_CRITICAL();
    if (!IsEmpty(pControl)) //判断缓冲区是否为空
    {
        //不为空,读取数据
        val = pControl->pBuf[pControl->ReadPoint];
        //调整读指针
        pControl->ReadPoint = (pControl->ReadPoint + 1) % (*pControl->pBufLen);
        OS_EXIT_CRITICAL();
        return val;
    }
    OS_EXIT_CRITICAL();
    return -1;
    
}

/*****************************************************************************
* Function     : UART_PushOneByte
* Description  : 写入一个字节
* Input        : INT8U ComPort  
                 INT8U val      
* Output       : None
* Return       : -1:错误或者缓冲区满 0/1:返回写入的字节数
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static int UART_PushOneByte(_BSP_UART_BUF* const pControl, INT8U const Val)
{   
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
    
    if (pControl == NULL)
    {
        return -1;
    }

    OS_ENTER_CRITICAL();
    if (!IsFull(pControl)) //判断缓冲区是满
    {
        //不满，写入数据
        pControl->pBuf[pControl->WritePoint] = Val;
        //调整写指针
        pControl->WritePoint = (pControl->WritePoint + 1) % (*pControl->pBufLen);
        OS_EXIT_CRITICAL();
        return 1;
    }
    OS_EXIT_CRITICAL();
    return 0;
}

/*****************************************************************************
* Function     : UART_PopBytes
* Description  : 读取多个字节数
* Input        : INT8U ComPort        ：串口号
                 INT8U* Buf           ：保存读取数据的缓冲区
                 cosnt INT16U Len     ：缓冲区大小
* Output       : None
* Return       : -1：参数错误       其他值：返回读取成功的字节数
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static int UART_PopBytes(_BSP_UART_BUF* const pControl, INT8U* const Buf, const INT16U Len)
{
    INT16U i;
    int val;
    
    if (pControl == NULL)
    {
        return -1;
    }

    if ( (Buf == NULL) || (!Len) )
    {
        return -1;
    }

    for (i = 0; i < Len; i++)
    {
        val = UART_PopOneByte(pControl);   //读一个字节数据
        if (val >= 0)                   //判断读数据是否成功
        {
            Buf[i] = val;               //成功，保存数据
        }
        else                            
        {   
            break;                      //失败，退出
        }
    }
    return i;                           //返回读取成功的字节数
}

/*****************************************************************************
* Function     : UART_PushBytes
* Description  : 写入多个字节
* Input        : INT8U ComPort     
                 const INT8U* Buf  
                 const INT16U Len  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static int UART_PushBytes(_BSP_UART_BUF* const pControl, INT8U* const Buf, const INT16U Len)
{
    INT16U i;
    
    if (pControl == NULL)
    {
        return -1;
    }

    if ( (Buf == NULL) || (!Len) )
    {
        return -1;
    }

    for (i = 0; i < Len; i++)
    {

        if (UART_PushOneByte(pControl, Buf[i]) != 1) //写入数据，返回写入成功的字节数
        {
            break;
        }
    }
    return i;                                     //返回写入成功的字节数
}


/*****************************************************************************
* Function     : BSP_UARTWrite
* Description  : 往一个串口发送数据
* Input        : INT8U ComPort    
                 INT8U *FrameBuf  
                 INT16U FrameLen  
* Output       : None
* Return       : -1：参数错误 其他值:发送成功的字节数
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
int BSP_UARTWrite(USART_TypeDef* const USARTx, INT8U* const FrameBuf, const INT16U FrameLen)
{
    INT16U sendnum;
    INT8U comport;
    
    if ( !IS_USART_ALL_PERIPH(USARTx) || (FrameBuf == NULL) || !FrameLen)
    {
        return -1;
    }
    if ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) //转化为内部索引值
    {
        return -1;
    }

    if (BSPUART_GET_MODE(pUart[comport]->Setting.Mode) == UART_RS485_MODE)  //485模式，需要先关闭接收使能，切换到发送状态
	{
		//关闭接收使能，开启发送使能
        RS485_RDCmd(USARTx, CMD_TX_ENABLE);
        OSTimeDly(SYS_DELAY_2ms); //从接收切换到发送最大100us
        //关闭串口寄存器的接收器
	    UART_RxMode(USARTx, DISABLE);
	}
    
    //把要发送的数据保存到发送缓冲区
    sendnum = UART_PushBytes(&pUart[comport]->UartTxControl, FrameBuf, FrameLen);
    //未在发送，开始发送第一个字节
    if (pUart[comport]->SendState == UART_STATE_IDLE) 
    {
        //发送第一个数据到串口
        USART_SendData(pUart[comport]->pUartHardInfo->UARTx, pUart[comport]->UartTxControl.pBuf[pUart[comport]->UartTxControl.ReadPoint]); //发送数据
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
        OS_CPU_SR  cpu_sr = 0;
#endif
        OS_ENTER_CRITICAL();
        //发送状态为正在发送
        pUart[comport]->SendState = UART_STATE_SENDING;
        pUart[comport]->UartTxControl.ReadPoint = (pUart[comport]->UartTxControl.ReadPoint + 1)  % *pUart[comport]->UartTxControl.pBufLen;
        OS_EXIT_CRITICAL();
        if (BSPUART_GET_MODE(pUart[comport]->Setting.Mode) != UART_DEBUG_MODE)
        {
            USART_ITConfig(pUart[comport]->pUartHardInfo->UARTx, USART_IT_TC , ENABLE);
        }
    }
    return sendnum;
}

/*****************************************************************************
* Function     : BSP_UARTRead
* Description  : 从一个串口读取指定数量数据
* Input        : INT8U ComPort          
                 INT8U* const FrameBuf  
                 const INT16U FrameLen  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
int BSP_UARTRead(const USART_TypeDef* const USARTx, INT8U* const FrameBuf, const INT16U FrameLen)
{
    INT8U comport;
    
    if ( !IS_USART_ALL_PERIPH(USARTx) || (FrameBuf == NULL) || !FrameLen)
    {
        return -1;
    }
    if ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) //转化为内部索引值
    {
        return -1;
    }
    
    return UART_PopBytes(&pUart[comport]->UartRxControl, FrameBuf, FrameLen);
}

/*****************************************************************************
* Function     : UART_TxISR
* Description  : 串口发送函数
* Input        : INT8U ComPort                 
                 _BSP_UART_CONTROL * pControl  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static int UART_TxISR(INT8U ComPort, _BSP_UART_BUF * pControl)
{
    int readnum;
    INT8U val;
    
    if ( (ComPort >= UART_MAX_NUM) || (pControl == NULL) || (pUart[ComPort] == NULL) )
    {
        return -1;
    }

    //从发送缓冲区读取一个字节数据
    readnum = UART_PopBytes(pControl, &val, sizeof(val) );
    if (readnum == 1) //还是数据未发送完
    {
        USART_SendData(pUart[ComPort]->pUartHardInfo->UARTx, val); //发送数据
        return readnum;
    }
    else if (!readnum) //数据已经发送完毕
    {
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
        OS_CPU_SR  cpu_sr = 0;
#endif
        OS_ENTER_CRITICAL();
        //发送完毕，发送状态为空闲
        pUart[ComPort]->SendState = UART_STATE_IDLE;
        //关闭发送中断
        USART_ITConfig(pUart[ComPort]->pUartHardInfo->UARTx, USART_IT_TC , DISABLE);
        OS_EXIT_CRITICAL();
        if (BSPUART_GET_MODE(pUart[ComPort]->Setting.Mode) == UART_RS485_MODE) //485模式
        {
            //关闭发送使能，开启接收使能
            RS485_RDCmd(pUart[ComPort]->pUartHardInfo->UARTx, CMD_RX_ENABLE);
            //开启串口的接收器使能
            UART_RxMode(pUart[ComPort]->pUartHardInfo->UARTx, ENABLE);
        }
    }
    return 0;
}

/*****************************************************************************
* Function     : UART_RxISR
* Description  : 把接收到的字节存入接收缓冲区
* Input        : _BSP_UART_CONTROL * pControl  
                 INT8U Val                     
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static int UART_RxISR(_BSP_UART_BUF * pControl, INT8U Val)
{    
    if (pControl == NULL)
    {
        return -1;
    }

    //把Val存入接收缓冲区
    if (UART_PushBytes(pControl, &Val, sizeof(Val) ) != 1)
    {
        return 1;
    }
    return 0;
}

INT8U uart_buf[5000] = {0};
INT32U uart_len = 0;
/*****************************************************************************
* Function     : UART_ISR
* Description  : 串口中断ISR
* Input        : INT8U ComPort  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void UART_ISR(const USART_TypeDef* const USARTx)
{
	static INT8U i = 0; 
	INT16U  recvofset = 0;
    INT8U rcvdata, comport;
    INT16U len;
    if (!IS_USART_ALL_PERIPH(USARTx))
    {
        return;
    }
    
    if ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) //转化为内部索引值
    {
        return;
    }

    //接收中断
    if (USART_GetITStatus(pUart[comport]->pUartHardInfo->UARTx, USART_IT_RXNE) != RESET)
    {
        /* 清除USART接收中断标志 */
		USART_ClearITPendingBit(pUart[comport]->pUartHardInfo->UARTx, USART_IT_RXNE);
        /* 从UART接收寄存器读取一个字节的数据 */
		rcvdata = USART_ReceiveData(pUart[comport]->pUartHardInfo->UARTx);
        /* 7位数据位时,Cortex最高位校验会读进来，清理掉 */
        if(pUart[comport]->Setting.DataBits == BSPUART_WORDLENGTH_7)
		{
			rcvdata	&= 0x7f;	
		}
		
		if(RecvControl[comport].RecvCmd == 1)   //数据分帧在此处处理  20231219
		{
			uart_buf[uart_len] = rcvdata;
			uart_len++;
			if(RecvControl[comport].DataFrameDispose != NULL)
			{
				if(RecvControl[comport].DataFrameDispose(uart_buf,uart_len))
				{
					uart_len = 0;
				}
			}
		}
		else
		{
			//放入接收缓冲区
			UART_RxISR(&(pUart[comport]->UartRxControl), rcvdata);
			if (pUart[comport]->Setting.Mode & UART_MSG_MODE)   //使用消息队列，清空计数
			{
	#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
				OS_CPU_SR  cpu_sr = 0;
	#endif	        
				OS_ENTER_CRITICAL();
				pUart[comport]->UartRxInterval.RxIntervalIndex = 0;
				OS_EXIT_CRITICAL();
			}
		}
    }

    //发送中断
    if (USART_GetITStatus(pUart[comport]->pUartHardInfo->UARTx, USART_IT_TC) != RESET)
    {
        /* 清除USART发送中断标志 */
		USART_ClearITPendingBit(pUart[comport]->pUartHardInfo->UARTx, USART_IT_TC);
        UART_TxISR(comport, &(pUart[comport]->UartTxControl) );
    }

}

/*****************************************************************************
* Function     : UART_ReceiveMsgHook
* Description  : 判断一帧是否完成，接收到最后一个字节后指定
                 间内没有再收到一个字节即认为一帧接收完成
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 此函数建议在系统节拍中被调用
* Contributor  : 2018年5月25日
*****************************************************************************/
void UART_ReceiveMsgHook(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
     OS_CPU_SR  cpu_sr = 0;
#endif	     
    for (INT8U comport = 0; comport < UART_MAX_NUM; comport++)
    {
		if(RecvControl[comport].RecvCmd == 1)   //数据分帧直接返回
		{
			return;
		}
		
        if (!(pUart[comport]->Setting.Mode & UART_MSG_MODE) )
        {
            continue;
        }
        if (IsEmpty(&pUart[comport]->UartRxControl) )
        {
            continue;
        }
        //间隔时间到，以消息的形式发送给应用
        if (pUart[comport]->UartRxInterval.RxIntervalIndex >=  pUart[comport]->UartRxInterval.RxInterval)
        {
            if (pUart[comport]->UartRxInterval.pMsgMem == NULL)
            {
                continue;
            }
            INT8U err;
            //获取一个消息实体
            _BSP_MESSAGE *pMsg = OSMemGet(pUart[comport]->UartRxInterval.pMsgMem, &err);
            if (OS_ERR_NONE != err)
            {
                continue;
            }
            
            pMsg->MsgID = BSP_MSGID_UART_RXOVER;
            pMsg->DivNum = comport;
            OS_ENTER_CRITICAL();  
            pMsg->pData = pUart[comport]->UartRxControl.pBuf + pUart[comport]->UartRxControl.ReadPoint;
            if (pUart[comport]->UartRxControl.WritePoint < pUart[comport]->UartRxControl.ReadPoint)
            {
                pMsg->DataLen = *pUart[comport]->UartRxControl.pBufLen - pUart[comport]->UartRxControl.ReadPoint + pUart[comport]->UartRxControl.WritePoint;
            }
            else
            {
                pMsg->DataLen = pUart[comport]->UartRxControl.WritePoint - pUart[comport]->UartRxControl.ReadPoint;
            }
            //重新开始计算间隔时间
            pUart[comport]->UartRxInterval.RxIntervalIndex = 0;
            //调整读指针
            pUart[comport]->UartRxControl.ReadPoint = pUart[comport]->UartRxControl.WritePoint;
            OS_EXIT_CRITICAL();                
            if  ( (pUart[comport]->Setting.RxQueue) && (pUart[comport]->Setting.RxQueue->OSEventType == OS_EVENT_TYPE_Q) )
            {
                if (OSQPost(pUart[comport]->Setting.RxQueue, pMsg) != OS_ERR_NONE)
                {
                    __asm("nop");
                }
            }
        }
        else
        {
            OS_ENTER_CRITICAL();
            pUart[comport]->UartRxInterval.RxIntervalIndex++;
            OS_EXIT_CRITICAL();
        }
    }
}

/*****************************************************************************
* Function     : UART_FreeOneMsg
* Description  : 释放一个UART消息,在使用串口消息方式时，使用完收到的消息必须要调用此函数
* Input        : const USART_TypeDef* const USARTx  
                 _BSP_MESSAGE *pMsg                 
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U UART_FreeOneMsg(const USART_TypeDef* const USARTx, _BSP_MESSAGE *pMsg)
{
    INT8U comport;
    
    if (!IS_USART_ALL_PERIPH(USARTx) || (pMsg == NULL) )
    {
        return FALSE;
    }
    if ( (comport = GetComPort(USARTx) ) >= UART_MAX_NUM) //转化为内部索引值
    {
        return FALSE;
    }
    if (!(pUart[comport]->Setting.Mode & UART_MSG_MODE) )
    {
        return FALSE;
    }
    if (pUart[comport]->UartRxInterval.pMsgMem == NULL)
    {
        return FALSE;
    }
    if (OSMemPut(pUart[comport]->UartRxInterval.pMsgMem, pMsg) != OS_ERR_NONE)
    {
        return FALSE;
    }
    return TRUE;
    
}

/*****************************************************************************
* Function     : UART_MsgDeal
* Description  : 对使用消息发送的串口数据进行处理，此函数在使用串口消息时必须
                 要先调用，否则数据将出错
* Input        : const _BSP_MESSAGE** pMsg  : 收到的消息本身地址
                 INT8U *pbuf                : 消息数据存放的地址
                 INT16U len                 ：pbuf的字节数
* Output       : None
* Return       : TRUE:拷贝了数据 FALSE:参数错误
* Note(s)      : 串口底层发送的消息数据可能存在缓冲区尾部跟首部两块区域，
                 消息中的数据指针是指向了起始地址，长度是尾部跟首部总和，
                 如果直接拷贝，就会出错，需要先拷贝尾部，然后再拷贝首部。
                 拷贝完成后，消息的数据指针将指向pbuf，数据长度将更改为
                 拷贝成功的字节数，应用层应判断消息的数据长度来识别是否
                 拷贝成功全部字节数，如数据长度变小，应该加大pbuf的大小
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U UART_MsgDeal(_BSP_MESSAGE** pMsg, INT8U *pbuf, INT16U len)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif	     
    INT16U bytes, offset = 0;
    INT8U comport;
    
    if ( (pMsg == NULL) || (*pMsg == NULL)|| ( (*pMsg)->MsgID != BSP_MSGID_UART_RXOVER) || ( ((*pMsg)->DivNum & 0x0f) >= UART_MAX_NUM) \
        || ((*pMsg)->pData == NULL) || (!(*pMsg)->DataLen) )
    {
        return FALSE;
    }
    if ( (pbuf == NULL) || !len)
    {
        return FALSE;
    }
    
    comport = (*pMsg)->DivNum & 0x0f;                                  //提取串口号
    
    if (!(pUart[comport]->Setting.Mode & UART_MSG_MODE) )
    {
        return FALSE;
    }
    if (pUart[comport]->UartRxInterval.pMsgMem == NULL)
    {
        return FALSE;
    }
    const INT16U inlen = (*pMsg)->DataLen;                          //备份输入的消息长度
    if ( (INT32U)(*pMsg)->pData + (INT32U)(*pMsg)->DataLen >= (INT32U)pUart[comport]->UartRxControl.pBuf + *pUart[comport]->UartRxControl.pBufLen)
    {
        //数据尾部跟首部都有，需要分两次拷贝
        //计算尾部的数据长度
        offset = (INT32U)pUart[comport]->UartRxControl.pBuf + *pUart[comport]->UartRxControl.pBufLen - (INT32U)(*pMsg)->pData;
        if (offset <= len)                                          //尾部数据可以全部拷贝
        {
            bytes = (*pMsg)->DataLen - offset;                      //计算首部的数据字节数
            bytes = (bytes <= len - offset) ? bytes : len - offset; //计算首部需要拷贝的字节数
            OS_ENTER_CRITICAL();
            //先拷贝尾部
            memcpy(pbuf, (*pMsg)->pData, offset);
            //拷贝首部
            memcpy(pbuf + offset, pUart[comport]->UartRxControl.pBuf, bytes);  //拷贝首部数据
            OS_EXIT_CRITICAL();
            (*pMsg)->pData = pbuf;                                  //计算新的数据buf
            (*pMsg)->DataLen = offset + bytes;                      //计算新的数据长度，可能比原来的小
        }
        else                                                        //只能拷贝部分尾部数据
        {
            OS_ENTER_CRITICAL();
            memcpy(pbuf, (*pMsg)->pData, len);
            OS_EXIT_CRITICAL();
            (*pMsg)->pData = pbuf;                                  //计算新的数据buf
            (*pMsg)->DataLen = len;                                 //计算新的数据长度，可能比原来的小
        }
        
    }
    else                                                            //数据是连续的，单次拷贝
    {
        bytes = ((*pMsg)->DataLen <= len) ? (*pMsg)->DataLen : len;
        OS_ENTER_CRITICAL();
        memcpy(pbuf, (*pMsg)->pData, bytes);
        OS_EXIT_CRITICAL();
        (*pMsg)->pData = pbuf;
        (*pMsg)->DataLen = bytes;
    }
    return (inlen == (*pMsg)->DataLen);                             //输入跟输出长度一致才算成功
//    return TRUE;
}

/************************(C)COPYRIGHT 2018 杭州快电****END OF FILE****************************/
