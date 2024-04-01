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
#include "GPRSRecv.h"
#include <string.h>
#include "bsp_uart.h"
#include "ModuleManage.h"
#include "GPRSMain.h"
/* Private define-----------------------------------------------------------------------------*/
#define GPRSRECV_Q_LEN      20

#define GPRS_SEND_BUF_LEN          (120u)      //显示任务发送buf大小
#define GPRS_RECV_BUF_LEN          (120u)      //显示任务接收buf大小
#define GPRS_Q_LEN              	(16u)       //显示任务消息队列个数
#define GPRS_TEMP_BUF_LEN          (120u)      //显示任务保存一帧数据的buf大小

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPRSRECVOSQ[GPRSRECV_Q_LEN];					// 消息队列
static OS_EVENT *GPRSRECVTaskEvent;				            // 使用的事件

static INT8U GPRSSendBuf[GPRS_SEND_BUF_LEN];            // 显示发送缓存
static INT8U GPRSRecvBuf[GPRS_RECV_BUF_LEN];            // 显示接收缓存
static INT8U GPRSRecvSMem[GPRS_Q_LEN * sizeof(_BSP_MESSAGE)]; //消息实体的个数，实际上就是_BSP_MESSAGE结构体的个数
static INT8U GPRSTempBuf[GPRS_TEMP_BUF_LEN];            // 显示保存一帧数据缓存

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
static INT8U GPRSRecv_HardwareInit(void)
{
    _BSP_UART_SET UartGPRSSet;                      
    
    GPRSRECVTaskEvent = OSQCreate(GPRSRECVOSQ, GPRSRECV_Q_LEN);//创建显示接收消息队列
    if (GPRSRECVTaskEvent == NULL)
    {
        OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
        return FALSE;
    }
    memset(GPRSSendBuf, 0 ,sizeof(GPRSSendBuf));
    memset(GPRSRecvBuf, 0 ,sizeof(GPRSRecvBuf));

    UartGPRSSet.BaudRate = 115200;
    UartGPRSSet.DataBits = BSPUART_WORDLENGTH_8;
    UartGPRSSet.Parity   = BSPUART_PARITY_NO;
    UartGPRSSet.StopBits = BSPUART_STOPBITS_1;
    UartGPRSSet.RxBuf = GPRSRecvBuf;
    UartGPRSSet.RxBufLen = sizeof(GPRSRecvBuf);
    UartGPRSSet.TxBuf = GPRSSendBuf;
    UartGPRSSet.TxBufLen = sizeof(GPRSSendBuf);
    UartGPRSSet.Mode = UART_DEFAULT_MODE | UART_MSG_MODE;
    UartGPRSSet.RxQueue = GPRSRECVTaskEvent;
    UartGPRSSet.RxMsgMemBuf = GPRSRecvSMem;
    UartGPRSSet.RxMsgMemLen = sizeof(GPRSRecvSMem);
    if (BSP_UARTConfigTimeInit(GPRS_UART, &UartGPRSSet) == FALSE)  //和一般串口时间间隔不一样
    {
        return FALSE;
    }
	return  TRUE;
}

/*****************************************************************************
* Function     : APP_GPRSRecvDataDeal
* Description  : GPRS接收数据处理
* Input        : _BSP_MESSAGE *pMsg  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月12日        
*****************************************************************************/
INT8U APP_GPRSRecvDataDeal(_BSP_MESSAGE *pMsg)
{
	_GPRS_SELECT_MODULE* pNode = APP_GetCurModuleNode();
	_BSP_MESSAGE * pMessage = NULL;
	static _BSP_MESSAGE send_message;
	INT8U i = 0,err= 0;
	OS_EVENT* pEvent = APP_GetGPRSMainEvent();
	
	if((pMsg == NULL) || (pNode == NULL) || (pEvent == NULL))
	{
		return FALSE;
	}
	if((pMsg->pData == NULL) ||(pMsg->DataLen == 0) || (pNode->FunRecvDispose == NULL) || (pNode->FunGetStatus == NULL) )
	{
		return FALSE;
	}
	if((pNode->FunGetStatus() == STATE_DATA) && (pNode->FunGetConnectState() == STATE_OK) )   //进入通信状态,直接通知GPRSMain有数据接收
	{
		send_message.MsgID = BSP_GPRS_DATA;
		send_message.pData = pMsg->pData;				
		send_message.DataLen = pMsg->DataLen;
		if((err = OSQPost(pEvent, &send_message)) != OS_ERR_NONE)
		{
			printf("Send message error %d",err);
			return FALSE;
		}
		return TRUE;
	}
	//数据清零
	pMessage->pData = NULL;
	pMessage->DataLen = 0;
	pMessage = pNode->FunRecvDispose(pMsg);					//不同模块有不同特性，需要对模块接收数据按照模块的特性进行处理
	if((pMessage == NULL) || (pMessage->DataLen == 0) ||(pMessage->pData == NULL) )
	{
		return FALSE;
	}
	for (i = 0; i < GPRS_RECV_TABLE_LEN; i++)
    {
        if (!strncmp(pNode->RecvFreamTab[i].ATCode, (char*)pMessage->pData,pMessage->DataLen) )
        {
            if (pNode->RecvFreamTab[i].Fun)
            {
			    pNode->RecvFreamTab[i].Fun(pMsg->pData, pMsg->DataLen);    //传入还是传数据接收的原始数据
            }
			return TRUE;
        }
    }
	return TRUE;
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
void TaskGPRSRecv(void *pdata)
{
    INT8U err;    
    _BSP_MESSAGE *pMsg;
	//临时使用UART1 (显示任务串口调试)
    GPRSRecv_HardwareInit();
	while(1)
    {
        //从串口读取一个消息
        pMsg = OSQPend(GPRSRECVTaskEvent, SYS_DELAY_500ms, &err);
        if (OS_ERR_NONE == err)
        {
            switch (pMsg->MsgID)
            {
                case BSP_MSGID_UART_RXOVER:
					if (UART_MsgDeal(&pMsg, GPRSTempBuf, sizeof(GPRSTempBuf)) == TRUE)
                    {					
						pMsg->pData[pMsg->DataLen] = '\0';		  //保险，可能存在有些数据接收后没有/0
//                        printf("GPRS recv [%s]", pMsg->pData);    //打印接收到的数据
                        APP_GPRSRecvDataDeal(pMsg);						
					}

                    UART_FreeOneMsg(GPRS_UART, pMsg);
                    break;
                default:
                    break;
            }
        }
	
	}	
}


/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/

