/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bsp.c
* Author			: Ҷϲ��
* Date First Issued	: 10/12/2010
* Version			: V0.1
* Description		: �󲿷������������������ʼ��
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "bmstimeout.h"
#include "gbstandard.h"
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
* Contributor	: 20101210     Ҷϲ��
***********************************************************************************************/
_BMS_STEP BMS_FrameRx_TimeoutA(_BMS_STEP step,INT32U timeout,INT8U flag)
{
    static INT32U startSysTime[BMS_MAX_STEP]; //ÿ��step�������ƣ���Ҫ����Ϊ��Щ��������Ҫͬʱ�ж�2����ʱ,	
	static INT32U nowSysTime = 0;		
    
    nowSysTime = OSTimeGet();
    if (step >= BMS_MAX_STEP)
    {
        return BMS_PARA_ERR;
    }
    if(flag == RX_BMS_TIMEOUT_ENTER)
    {
        startSysTime[(INT8U)step] = nowSysTime; //��¼��ǰ���迪ʼ���㳬ʱʱ��
        return step;
    }
    else if(flag == RX_BMS_TIMEOUT_CHECK)//��ʱ�ж�5s
    {
        if((nowSysTime >= startSysTime[(INT8U)step]) ? ((nowSysTime - startSysTime[(INT8U)step]) >= timeout) : \
          ((nowSysTime + (INT32U_MAX_NUM - startSysTime[(INT8U)step])) >= timeout))
            return  BMS_TIME_OUT;
    }
    return BMS_STEP_DEFAULT;
}

/***********************************************************************************************
* Function		: 
* Description	: 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20101210     Ҷϲ��
***********************************************************************************************/
_BMS_STEP BMS_FrameRx_TimeoutB(_BMS_STEP step,INT32U timeout,INT8U flag)
{
    static INT32U startSysTime[BMS_MAX_STEP]; //ÿ��step�������ƣ���Ҫ����Ϊ��Щ��������Ҫͬʱ�ж�2����ʱ,	
	static INT32U nowSysTime = 0;		
    
    nowSysTime = OSTimeGet();
    if (step >= BMS_MAX_STEP)
    {
        return BMS_PARA_ERR;
    }
    if(flag == RX_BMS_TIMEOUT_ENTER)
    {
        startSysTime[(INT8U)step] = nowSysTime; //��¼��ǰ���迪ʼ���㳬ʱʱ��
        return step;
    }
    else if(flag == RX_BMS_TIMEOUT_CHECK)//��ʱ�ж�5s
    {
        if((nowSysTime >= startSysTime[(INT8U)step]) ? ((nowSysTime - startSysTime[(INT8U)step]) >= timeout) : \
          ((nowSysTime + (INT32U_MAX_NUM - startSysTime[(INT8U)step])) >= timeout))
            return  BMS_TIME_OUT;
    }
    return BMS_STEP_DEFAULT;
}

_BMS_STEP BMS_TIMEOUT_ENTER(INT8U gun,_BMS_STEP step,INT32U time)
{
	if(gun ==GUN_A)
	{
		return BMS_FrameRx_TimeoutA(step,time,RX_BMS_TIMEOUT_ENTER);
	}
	else
	{
		return BMS_FrameRx_TimeoutB(step,time,RX_BMS_TIMEOUT_ENTER);
	}
}

_BMS_STEP BMS_TIMEOUT_CHECK(INT8U gun,_BMS_STEP step,INT32U time)
{
	if(gun ==GUN_A)
	{
		return BMS_FrameRx_TimeoutA(step,time,RX_BMS_TIMEOUT_CHECK);
	}
	else
	{
		return BMS_FrameRx_TimeoutB(step,time,RX_BMS_TIMEOUT_CHECK);
	}
}
/************************(C)COPYRIGHT 2010 �����Ƽ�*****END OF FILE****************************/
