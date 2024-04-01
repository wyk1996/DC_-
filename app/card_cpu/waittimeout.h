/*****************************************Copyright(H)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bmstimeout.h
* Author			  :      
* Date First Issued	: 10/6/2015
* Version			  : V0.1
* Description		: 大部分驱动都可以在这里初始化
*----------------------------------------历史版本信息-------------------------------------------
* History			  :
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__WAIT_TIMEOUT_H_
#define	__WAIT_TIMEOUT_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "card_cpu.h"
/* Private define-----------------------------------------------------------------------------*/       
#define RX_WAIT_TIMEOUT_ENTER    0      //超时判断开始进入
#define RX_WAIT_TIMEOUT_CHECK    1      //超时判断监测
#define INT32U_MAX_NUM          (0xFFFFFFFFu)
/* Private typedef----------------------------------------------------------------------------*/

CARD_STEP Card_FrameRx_Timeout(CARD_STEP step,INT32U timeout,INT8U flag);
//以下两函数要成对使用，不能单独使用



#define CARD_TIMEOUT_ENTER(step,time)   Card_FrameRx_Timeout(step,time,RX_WAIT_TIMEOUT_ENTER)
#define CARD_TIMEOUT_CHECK(step,time)   Card_FrameRx_Timeout(step,time,RX_WAIT_TIMEOUT_CHECK)
#endif //__WAIT_TIMEOUT_H_
/************************(H)COPYRIGHT 2018 汇誉科技*****END OF FILE****************************/
