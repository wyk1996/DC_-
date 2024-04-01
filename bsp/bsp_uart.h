/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_uart.h
* Author			: 
* Date First Issued	: 2018年5月25日
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_UART_H_
#define __BSP_UART_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp_conf.h" 
#ifndef  USART_TypeDef
#include "stm32f4xx.h"
#endif


/* Private define-----------------------------------------------------------------------------*/
#define UART_MAX_NUM       (8u)     //串口总个数

/* Private typedef----------------------------------------------------------------------------*/

//工作模式
typedef enum
{
    UART_DEFAULT_MODE = 0x00,        // 普通UART口使用 
    UART_HALFDUP_MODE = 0x01,        // 半双工模式, (IRDA), 扩展串口有20MS的转换延时
    UART_RS485_MODE   = 0x02,        // 半双工模式, RS485, 扩展串口有20MS的转换延时 
    UART_DEBUG_MODE   = 0x03,        // 调试口，不使用中断
    UART_TX_DMA       = 0x40,        // 调试串口可以开始DMA模式，目前仅支持调试串口，其他串口暂不支持
    UART_MSG_MODE     = 0x80,        // 使用消息队列模式，此项可以进行或操作
}_BSPUART_MODE;

#define BSPUART_GET_MODE(mode)         (mode & (~UART_TX_DMA) & (~UART_MSG_MODE) )

//校验位
typedef enum
{
	BSPUART_PARITY_NO   = 0x00,		// 无校验(默认)
	BSPUART_PARITY_ODD  = 0x01,		// 奇校验
	BSPUART_PARITY_EVEN = 0x02,		// 偶校验
}_BSPUART_PARITY;
    
//停止位
typedef enum
{
//	BSPUART_STOPBITS_0_5= 0x05,		// 0.5个停止位
	BSPUART_STOPBITS_1  = 0x01,		// 1个停止位(默认)
//	BSPUART_STOPBITS_1_5= 0x15,		// 1.5个停止位
	BSPUART_STOPBITS_2  = 0x02,		// 2个停止位
}_BSPUART_STOPBITS;

//数据长度
typedef enum
{
	BSPUART_WORDLENGTH_5 = 0x05,	// 5位数据+校验位
	BSPUART_WORDLENGTH_6 = 0x06,	// 6位数据
	BSPUART_WORDLENGTH_7 = 0x07,	// 7位数据+唤醒位
	BSPUART_WORDLENGTH_8 = 0x08,	// 8位数据+校验位(默认)
}_BSPUART_WORDLENGTH;

typedef struct 
{
    DMA_Stream_TypeDef *Streamx;    //DMA数据流
    INT32U Channel;                 //DMA通道选择
    INT32U PeripheralAddr;          //外设地址
    INT32U MemoryAddr;              //存储器地址
    INT16U MemBufLen;               //存储器缓存大小
    INT16U Len;                     //数据传输量
}_BSP_UART_DMA;

//串口配置结构体
typedef struct
{
    INT32U BaudRate;	            // 波特率 300 ~ 115200 
    _BSPUART_WORDLENGTH  DataBits;	// 串口1..5支持7 - 8数据位，串口6,7支持5 - 8数据位
    _BSPUART_PARITY  Parity;	    // 0: No parity, 1: Odd parity, 2: Even Parity    
    _BSPUART_STOPBITS  StopBits;	// 停止位1 - 2
    INT8U  Mode;		            // 取值类型为 _BSPUART_MODE ,如UART_DEFAULT_MODE, UART_HALFDUP_MODE, UART_RS485_MODE
                                    // 串口1..5不支持7位、无校验模式
    INT8U  *RxBuf;		            // 接收缓冲区
    INT8U  *TxBuf;		            // 发送缓冲区
    INT16U RxBufLen;	            // 接收缓冲区长度
    INT16U TxBufLen; 	            // 发送缓冲区长度

    OS_EVENT *RxQueue;
    INT8U  *RxMsgMemBuf;            // 消息队列本身存放的地址
    INT16U RxMsgMemLen;             // 消息队列本身存放的地址大小
    
    _BSP_UART_DMA DMASet;
}_BSP_UART_SET;

//串口配置结构体
typedef enum
{
    MSG_NORMAL = 0x10,              //不是最后一帧，后续还有帧共同表示一个帧
    MSG_LAST   = 0x80,              //最后一帧，此帧接收完成
}_BSP_MSG_TYPE;

typedef enum
{
    UART_STATE_IDLE,                //串口未在发送，空闲
    UART_STATE_SENDING,             //串口正在发送
}_BSP_UART_STATE;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


/*******************************************************************************
*
*	   以 下 为 内 核 接 口
*
*******************************************************************************/
//串口引脚信息
typedef struct 
{
    USART_TypeDef *UARTx;   //串口号
    void (*PinCfg)(void);   //引脚端口时钟、串口本身时钟开启及引脚配置
}_UART_HARDWARE_INFO;

//串口接收/发送缓存
typedef struct
{
    INT8U  *pBuf;          //指向接收/发送缓存
    INT16U *pBufLen;       //对应缓存的大小
    INT16U WritePoint;     //写入缓存的偏移值
    INT16U ReadPoint;      //读取缓存的偏移值
}_BSP_UART_BUF;

typedef struct
{
    OS_MEM *pMsgMem;
    INT8U  RxInterval;
    INT8U  RxIntervalIndex;
}_BSP_RXINTERVAL;

typedef struct
{
    const _UART_HARDWARE_INFO* pUartHardInfo;
    _BSP_UART_BUF UartTxControl;
    _BSP_UART_BUF UartRxControl;
    _BSP_RXINTERVAL UartRxInterval;
    _BSP_UART_SET Setting;
    _BSP_UART_STATE SendState; 
}_BSP_UART_CONTROL;


typedef struct
{
	INT8U RecvCmd;						//0时间分帧，1数据分帧
	INT8U (*DataFrameDispose)(INT8U *,INT16U );   //数据分帧处理函数
}_BSP_RECV_CONTROL;
extern _BSP_RECV_CONTROL RecvControl[UART_MAX_NUM];
/*****************************************************************************
* Function     : BSPUART_GetDMAInfo
* Description  : 获取串口DMA发送信息
* Input        : const USART_TypeDef* const USARTx  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日 
*****************************************************************************/
_BSP_UART_DMA* BSPUART_GetDMAInfo(const USART_TypeDef* const USARTx);

/*****************************************************************************
* Function     : BSP_UARTDMAEnable
* Description  : 对串口的TX开启DMA传输
* Input        : DMA_Stream_TypeDef *DMA_Streamx  
                 INT16U len                       
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U BSP_UARTDMAEnable(DMA_Stream_TypeDef *DMA_Streamx, INT16U len);

/*****************************************************************************
* Function     : BSP_UARTTxDMAConfigInit
* Description  : 串口发送DMA配置初始化
* Input        : const _BSP_UART_DMA* const pUartDMA  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U BSP_UARTTxDMAConfigInit(const _BSP_UART_DMA* const pUartDMA);

/*****************************************************************************
* Function     : BSP_UARTInit
* Description  : 串口初始化，供用户调用
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void BSP_UARTInit(void);

/*****************************************************************************
* Function     : BSP_UARTDeInit
* Description  : 串口重新初始化
* Input        : INT8U const ComPort  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void BSP_UARTDeInit(const USART_TypeDef* const USARTx);

/*******************************************************************************
* Function Name  : BSP_UARTConfigInit
* Description    : 串口初始化
* Input          : ComPort:    1 ... 5
*                  Settings:   串口配置
*              	   Mail_Queue：邮箱或队列指针
* Output         : None
* Return         : 1 成功 / 0 失败 
*******************************************************************************/
INT8U BSP_UARTConfigInit(const USART_TypeDef* const USARTx, const _BSP_UART_SET* const UartSet);

/*****************************************************************************
* Function     : BSP_UARTWrite
* Description  : 往一个串口发送数据
* Input        : INT8U ComPort    
                 INT8U *FrameBuf  
                 INT16U FrameLen  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :2018年5月25日
*****************************************************************************/
int BSP_UARTWrite(USART_TypeDef* const USARTx, INT8U* const FrameBuf, const INT16U FrameLen);

/*****************************************************************************
* Function     : BSP_UARTRead
* Description  : 从一个串口读取指定数量数据
* Input        : INT8U ComPort          
                 INT8U* const FrameBuf  
                 const INT16U FrameLen  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :2018年5月25日
*****************************************************************************/
int BSP_UARTRead(const USART_TypeDef* const USARTx, INT8U* const FrameBuf, const INT16U FrameLen);

/*****************************************************************************
* Function     : UART_ISR
* Description  : 串口中断ISR
* Input        : INT8U ComPort  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void UART_ISR(const USART_TypeDef* const USARTx);

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
void UART_ReceiveMsgHook(void);

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
INT8U UART_FreeOneMsg(const USART_TypeDef* const USARTx, _BSP_MESSAGE *pMsg);

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
INT8U UART_MsgDeal(_BSP_MESSAGE** pMsg, INT8U *pbuf, INT16U len);

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
INT8U BSP_UARTConfigTimeInit(const USART_TypeDef* const USARTx, const _BSP_UART_SET* const UartSet);
#endif //__BSP_UART_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
