/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
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
//#include "W5500Main.h"
#include <string.h>
#include "sysconfig.h"
#include "w5500_conf.h"
#include "w5500.h"
#include "tcp_demo.h"
#include "socket.h"
/* Private define-----------------------------------------------------------------------------*/
#define ETHMAIN_Q_LEN  15
static void *ETHMAINOSQ[ETHMAIN_Q_LEN];					// 消息队列
static OS_EVENT *ETHMainTaskEvent;				            // 使用的事件

INT8U ETHStatus = SOCK_CLOSED;
#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif //MIN
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : APP_GetETHLinKState
* Description  : 获取以太网连接状态
* Input        : void
* Output       : TRUE    连接成功
				 FALSE   连接失败 
* Return       :
* Note(s)      :
* Contributor  : 2018年6月16日        
*****************************************************************************/
INT8U APP_GetETHLinKState(void)
{
	if(ETHStatus == SOCK_ESTABLISHED)
	{
		return TRUE;
	}
	return FALSE;
}

//专门做GPRS接收处理
/*****************************************************************************
* Function     : HardReset_W5500
* Description  : 硬件复位w5500
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月16日        
*****************************************************************************/
void HardReset_W5500(void)
{
	reset_w5500();												/*硬复位W5500*/
	set_w5500_mac();											/*配置MAC地址*/
	set_w5500_ip();												/*配置IP地址*/
	socket_buf_init(txsize, rxsize);			/*初始化8个Socket的发送接收缓存大小*/
}

/*****************************************************************************
* Function     : ETH_DisposeRecvData
* Description  : 以太网接收数据处理
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月16日        
*****************************************************************************/
static void ETH_DisposeRecvData(INT8U *pdata,INT8U len)
{
	NOP();
}

/*****************************************************************************
* Function     : ETH_DisposeRecvData
* Description  : 以太网接收数据处理
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月16日        
*****************************************************************************/
static void ETH_PeriodSendData(void)
{
	NOP();
}



INT8U ETHRecvBuf[200];
/*****************************************************************************
* Function     : W5500_GetRecvDataDispose
* Description  : 获取接收数据处理
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月16日        
*****************************************************************************/
static INT8U W5500_GetRecvDataDispose(void)
{
	INT16U len;

	len=getSn_RX_RSR(SOCK_TCPC); 								  	         /*定义len为已接收数据的长度*/
	if(len>0 )
	{
		recv(SOCK_TCPC,ETHRecvBuf,MIN(len,200)); 							   		         /*接收来自Server的数据*/
	}
	return len;
}


/*****************************************************************************
* Function     : TaskW5500Connect
* Description  : 以太网连接
* Input        : void
* Output       : None
* Return       :
* Note(s)      :接收数据轮询在此任务中进行，此任务一个循环时间不能太长
* Contributor  : 2020年12月14日        
*****************************************************************************/
void TaskW5500Connect(void *pdata)
{
 
	INT8U err;   
	_BSP_MESSAGE *pMsg;
	static INT32U count;
	INT8U recvlen = 0;			//以太网数据接收长度
	
	pdata =  pdata; 
	ETHMainTaskEvent = OSQCreate(ETHMAINOSQ, ETHMAIN_Q_LEN);//
	if (ETHMainTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
		return;
	}	
	
	gpio_for_w5500_config();							/*初始化MCU相关引脚*/
	HardReset_W5500();									//硬件复位w5500
	while(1)
    {  
		pMsg = OSQPend(ETHMainTaskEvent, SYS_DELAY_1ms, &err);
        if (OS_ERR_NONE == err || pMsg != NULL)
		{
			if(ETHStatus == SOCK_ESTABLISHED)				//说明已经建立连接
			{
				;//消息处理
			}
		}
		if(PHY_check())   //网线已经连接
		{
			if(ETHStatus == SOCK_ESTABLISHED)				//说明已经建立连接
			{
				count = 0;
				//获取数据处理
				recvlen = W5500_GetRecvDataDispose();
				if(recvlen > 0)
				{
					ETH_DisposeRecvData(ETHRecvBuf,recvlen);
					send(SOCK_TCPC,ETHRecvBuf,recvlen);
				}
				//发送数据
				ETH_PeriodSendData();
			}
			
			else
			{
				//网线接入长时间为连接成功，需要硬件初始化
				if(++count  >= 10000)//粗略定时10s,后续可以根据实际情况调整
				{
					HardReset_W5500();									//硬件复位w5500
				}
			}
		}
	}	
}


/*****************************************************************************
* Function     : TaskW5500Connect
* Description  : 以太网连接
* Input        : void
* Output       : None
* Return       :
* Note(s)      :接收数据轮询在此任务中进行，此任务一个循环时间不能太长
* Contributor  : 2020年12月14日        
*****************************************************************************/
void TaskW5500Priod(void *pdata)
{
	pdata =  pdata; 
	while(1)
	{
		if(PHY_check())   //网线已经连接
		{
			ETHStatus = do_tcp_client();                    /*连接服务器*/
		}
		OSTimeDly(SYS_DELAY_500ms); 
	}
	
}

/************************(C)COPYRIGHT 2020 汇誉科技*****END OF FILE****************************/

