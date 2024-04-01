/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSSend.c
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
#include "GPRSSend.h"
#include <string.h>
#include "ModuleManage.h"
#include "GPRSMain.h"
/* Private define-----------------------------------------------------------------------------*/
#define GPRSSend_Q_LEN      20
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPRSSendOSQ[GPRSSend_Q_LEN];					// 消息队列
static OS_EVENT *GPRSSendTaskEvent;				            // 使用的事件

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//GPRS发送表
//专门做GPRS发送处理
/*****************************************************************************
* Function     : APP_GPRSModuleSendDataDeal
* Description  : gprs模块发送过来的数据处理
* Input        : _BSP_MESSAGE *pMsg  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U APP_GPRSModuleSendDataDeal(_BSP_MESSAGE *pMsg)
{
	INT8U i = 0;
	_GPRS_SELECT_MODULE* pNode = APP_GetCurModuleNode();
	
	if((pMsg == NULL) || (pNode == NULL) )
	{
		return FALSE;
	}
	if((pMsg->DataLen == 0) || pMsg->pData == NULL)
	{
		return FALSE;
	}
	for(i = 0;i < GPRS_SEND_TABLE_LEN;i++)
	{
		if (!strncmp(pNode->SendFreamTab[i].ATCode, (char *)pMsg->pData,  pMsg->DataLen) )   //比较指令是否一致
		{
			if (pNode->SendFreamTab[i].Fun)
			{
				pNode->SendFreamTab[i].Fun(pMsg->pData, pMsg->DataLen );
				return TRUE;				
			}
			return FALSE;
		}
	}
	return FALSE;
	
}
/*****************************************************************************
* Function     : Get_GPRSSendQueueEvent
* Description  : 获取GPRS发送消息队列
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月12日
*****************************************************************************/
OS_EVENT* Get_GPRSSendQueueEvent(void)
{
    return GPRSSendTaskEvent;
}

/*****************************************************************************
* Function     : TaskGPRSSend
* Description  : gprs发送任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
void TaskGPRSSend(void *pdata)
{
    INT8U err;    
    _BSP_MESSAGE *pMsg;

	GPRSSendTaskEvent = OSQCreate(GPRSSendOSQ, GPRSSend_Q_LEN);//创建显示接收消息队列
	if (GPRSSendTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
		return;
	}
	while(1)
    {
        pMsg = OSQPend(GPRSSendTaskEvent, SYS_DELAY_500ms, &err);
        if (OS_ERR_NONE == err)
        {
            switch (pMsg->MsgID)
            {
				case BSP_GPRS_AT:
					APP_GPRSModuleSendDataDeal(pMsg);     //数据处理
                default:
                    break;
            }
        }
	}	
}


/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/

