/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSRecv.c
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
//#include "GPSTest.h"
#include <string.h>
#include "sysconfig.h"
#include "bsp_uart.h"
/* Private define-----------------------------------------------------------------------------*/
#define GPSRECV_Q_LEN      20
#define GPS_UART 	   		UART8

#define GPS_SEND_BUF_LEN          (120u)      //显示任务发送buf大小
#define GPS_RECV_BUF_LEN          (120u)      //显示任务接收buf大小
#define GPS_Q_LEN              	(16u)       //显示任务消息队列个数
#define GPS_TEMP_BUF_LEN          (120u)      //显示任务保存一帧数据的buf大小

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPSRECVOSQ[GPSRECV_Q_LEN];					// 消息队列
static OS_EVENT *GPSRECVTaskEvent;				            // 使用的事件

static INT8U GPSSendBuf[GPS_SEND_BUF_LEN];            // 显示发送缓存
static INT8U GPSRecvBuf[GPS_RECV_BUF_LEN];            // 显示接收缓存
static INT8U GPSRecvSMem[GPS_Q_LEN * sizeof(_BSP_MESSAGE)]; //消息实体的个数，实际上就是_BSP_MESSAGE结构体的个数
static INT8U GPSTempBuf[GPS_TEMP_BUF_LEN];            // 显示保存一帧数据缓存

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//专门做GPRS接收处理
/*****************************************************************************
* Function     : GPRSRecv_HardwareInit
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
static INT8U GPSRecv_HardwareInit(void)
{
    _BSP_UART_SET UartGPSSet;                      
    
    GPSRECVTaskEvent = OSQCreate(GPSRECVOSQ, GPSRECV_Q_LEN);//创建显示接收消息队列
    if (GPSRECVTaskEvent == NULL)
    {
        OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
        return FALSE;
    }
    memset(GPSSendBuf, 0 ,sizeof(GPSSendBuf));
    memset(GPSRecvBuf, 0 ,sizeof(GPSRecvBuf));

    UartGPSSet.BaudRate = 115200;
    UartGPSSet.DataBits = BSPUART_WORDLENGTH_8;
    UartGPSSet.Parity   = BSPUART_PARITY_NO;
    UartGPSSet.StopBits = BSPUART_STOPBITS_1;
    UartGPSSet.RxBuf = GPSRecvBuf;
    UartGPSSet.RxBufLen = sizeof(GPSRecvBuf);
    UartGPSSet.TxBuf = GPSSendBuf;
    UartGPSSet.TxBufLen = sizeof(GPSSendBuf);
    UartGPSSet.Mode = UART_DEFAULT_MODE | UART_MSG_MODE;
    UartGPSSet.RxQueue = GPSRECVTaskEvent;
    UartGPSSet.RxMsgMemBuf = GPSRecvSMem;
    UartGPSSet.RxMsgMemLen = sizeof(GPSRecvSMem);
    if (BSP_UARTConfigInit(GPS_UART, &UartGPSSet) == FALSE)
    {
        return FALSE;
    }
	return  TRUE;
}

//专门做GPRS接收处理
/*****************************************************************************
* Function     : TaskGPRSRecv
* Description  : 串口测试任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月16日        
*****************************************************************************/
#if(BSP_GPSENABLE > 0)				
void TaskGPSTask(void *pdata)
{
    INT8U err;    
    _BSP_MESSAGE *pMsg;
    GPSRecv_HardwareInit();
	//这里的*后面数字是异或校验，如$PCAS01,5*19,'P' ^ 'C' ^ 'A' ^ 'S'^ '0' ^ '1' ^ ',' ^ '5' == 0x19;
  // char *str_B115200 = "$PCAS01,5*19\r\n";
    char *str_B9600  = "$PCAS01,1*1D\r\n";
    char *str_save = "$PCAS00*01\r\n";//当前配置写入flash
	BSP_UARTWrite(GPS_UART,(INT8U*)str_B9600,strlen(str_B9600));
	OSTimeDly(SYS_DELAY_500ms); 
	BSP_UARTWrite(GPS_UART,(INT8U*)str_save,strlen(str_save));
	OSTimeDly(SYS_DELAY_500ms); 
	while(1)
    {
		OSTimeDly(SYS_DELAY_50ms); 
        //从串口读取一个消息
        pMsg = OSQPend(GPSRECVTaskEvent, SYS_DELAY_500ms, &err);
        if (OS_ERR_NONE == err)
        {
            switch (pMsg->MsgID)
            {
                case BSP_MSGID_UART_RXOVER:
					if (UART_MsgDeal(&pMsg, GPSTempBuf, sizeof(GPSTempBuf)) == TRUE)
                    {

						BSP_UARTWrite(GPS_UART,pMsg->pData,pMsg->DataLen);
					}

                    UART_FreeOneMsg(GPS_UART, pMsg);
                    break;
                default:
                    break;
            }
        }
	}	
}
#endif

/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/

