/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.c
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
#include "GPRSMain.h"
#include "GPRSSend.h"
#include "bsp_uart.h"
#include <string.h>
#include "sysconfig.h"
/* Private define-----------------------------------------------------------------------------*/
#define   GPRSMAIN_Q_LEN  								20
//临时IP端口放在这里，后面应该重屏幕下发读取
#define GPRS_IP     "112.74.59.250"
#define GPRS_PORT   10002
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPRSMAINOSQ[GPRSMAIN_Q_LEN];					// 消息队列
static OS_EVENT *GPRSMainTaskEvent;				            // 使用的事件
_GPRS_SELECT_MODULE* CurModuleNode = NULL;					//当前模块节点
_NETWORD_INFO NetWordInfo;									//网络信息

typedef struct 
{
	INT8U ResendBillState;  //0表示不需要重发   1表示需要重发
	INT32U CurTime;			//当前发送时间发送时间
	INT32U LastTime;		//上一次发送时间
}_RESEND_BILL_CONTROL;

_RESEND_BILL_CONTROL ResendBillControl[GUN_MAX] ={ {0},{0}};

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : APP_GetGPRSMainEvent
* Description  :获取GPRSMain任务事件
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
OS_EVENT* APP_GetGPRSMainEvent(void)
{
    return GPRSMainTaskEvent;
}

/*****************************************************************************
* Function     : APP_GetCurModuleNode
* Description  : 获取当前节点
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日       
*****************************************************************************/
_GPRS_SELECT_MODULE* APP_GetCurModuleNode(void)
{
	return CurModuleNode;
}

/*****************************************************************************
* Function     : GPRS_Connect
* Description  : GPRS连接，包括模块是否存在，和连接服务器
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日       
*****************************************************************************/
static INT8U GPRS_Connect(void)
{
	OS_EVENT* pevent = Get_GPRSSendQueueEvent();
	
	//重启计数  临时这么写
	static INT8U i[3] = {0,0,0};
	if(pevent == NULL)
	{
		return FALSE;
	}
	if((CurModuleNode->FunGetExistState == NULL) || (CurModuleNode->FunSetExistState == NULL) || (CurModuleNode->FunTest == NULL) \
		|| (CurModuleNode->FunGetConnectState == NULL) )
	{
		return FALSE;
   	}
	if(CurModuleNode->FunGetExistState() == STATE_ERR)      //模块不存在
	{
		if(CurModuleNode->FunTest(pevent) == FALSE)
		{
			if(++i[0] == 5)
			{
				i[0] = 0;
				CurModuleNode->FunYouRenReset();
			}
		}
		else												//测试模块是否存在
		{
			i[0] = 0;
			CurModuleNode->FunSetExistState(STATE_OK);    	//GPRS模块存在     
		}
	}
	if((CurModuleNode->FunGetExistState()  == STATE_OK) && (CurModuleNode->FunGetCfgState() == STATE_ERR) ) 	 //配置服务器是否成功
	{
		CurModuleNode->FunConnectServer((INT8U*)GPRS_IP,GPRS_PORT,pevent);        								//重新配置服务器
	}
	
	//模块存在、配置OK、未连接成功
	if((CurModuleNode->FunGetExistState()  == STATE_OK) && (CurModuleNode->FunGetCfgState() == STATE_OK) && \
	   (CurModuleNode->FunGetConnectState() != STATE_OK) )
	{
		if(CurModuleNode->FunGetStatus() != STATE_CFG)
		{
			if(CurModuleNode->FunEnterCfg != NULL)
			{
				CurModuleNode->FunEnterCfg(pevent);  //进入配置模式
			}
		}
		else
		{
			if(CurModuleNode->FunQueryConectState(pevent) == FALSE)
			{
				if(++i[1] == 15)    //15次未查询到重连
				{
					i[1] = 0;
					CurModuleNode->FunYouRenReset();
				}
			}
			else
			{
				i[1] = 0;
			}
		}
	}
	if(CurModuleNode->FunGetConnectState() == STATE_OK)    //连接上服务器了
	{
		if(CurModuleNode->FunGetStatus() != STATE_DATA)
		{
			if(++i[2] == 3)
			{
				i[2] = 0;
				CurModuleNode->FunYouRenReset();
			}
			CurModuleNode->FunEnterData(pevent);  //进入数据模式	
		}
		else
		{
			i[2] = 0;
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : GPRSParaInit
* Description  : GPRS相关参数初始化
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日       
*****************************************************************************/
static void GPRS_ParaInit(void)
{
	//memset(&GPRSCmdState,0,sizeof(GPRSCmdState));
	;
}
	

/*****************************************************************************
* Function     : TaskGPRSMain
* Description  : gprs主任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
void TaskGPRSMain(void *pdata)
{
    INT8U err;    
    _BSP_MESSAGE *pMsg;

	GPRSMainTaskEvent = OSQCreate(GPRSMAINOSQ, GPRSMAIN_Q_LEN);//创建显示接收消息队列
	if (GPRSMainTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
		return;
	}
	GPRS_NodeInit();   //GPRS节点初始化
	CurModuleNode = GPRS_GetCurModuleNode(GPRS_USER_MODULE);
	if(CurModuleNode == NULL)			//未找到可用模块
	{
		return;
	}
	GPRS_ParaInit();
	while(1)
    {
        pMsg = OSQPend(GPRSMainTaskEvent, SYS_DELAY_500ms, &err);
        if (OS_ERR_NONE == err || pMsg != NULL)
        {
            switch (pMsg->MsgID)
            {	
				//消息处理
				case BSP_GPRS_DATA:   //来时GPRSRecv的消息（有数据接收）
					//目前是接收到什么发送什么，透传模式，主要网GPRS串口中发送数据即可
					if (BSP_UARTWrite(GPRS_UART,pMsg->pData,pMsg->DataLen) == pMsg->DataLen)
					{
						OSTimeDly(SYS_DELAY_5ms);	
					}
					break;
                default:
                    break;
            }
        }
		else
		{
			//连接GPRS
			GPRS_Connect();          //GPRS连接，包括模块是否存在，和连接服务器
		}
	}	
}

/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/

