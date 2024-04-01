/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			:  
* Author			:  
* Date First Issued	:  
* Version			: V0.1
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "waittimeout.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

/***********************************************************************************************
* Function		: 
* Description	: 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20180725    TCU超时设置
***********************************************************************************************/
CARD_STEP Card_FrameRx_Timeout(CARD_STEP step,INT32U timeout,INT8U flag)
{
	static INT32U startSysTime[CARD_MAX_STEP]; //每个step独立控制，主要是因为有些步骤里需要同时判断2个超时,	
	static INT32U nowSysTime = 0;		
    
	nowSysTime = OSTimeGet();
	if (step >= CARD_MAX_STEP)
	{
			return CARD_PARA_ERR;
	}
	if(flag == RX_WAIT_TIMEOUT_ENTER)
	{
			startSysTime[(INT8U)step] = nowSysTime; //记录当前步骤开始计算超时时间
			return step;
	}
	else if(flag == RX_WAIT_TIMEOUT_CHECK)//超时判断5s
	{
			if((nowSysTime >= startSysTime[(INT8U)step]) ? ((nowSysTime - startSysTime[(INT8U)step]) >= timeout) : \
				((nowSysTime + (INT32U_MAX_NUM - startSysTime[(INT8U)step])) >= timeout))
					return  CARD_TIME_OUT;
	}
	return CARD_STEP_DEFAULT;
}
/************************(C)COPYRIGHT 2018 汇誉科技*****END OF FILE****************************/
