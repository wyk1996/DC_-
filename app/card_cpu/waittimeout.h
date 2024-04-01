/*****************************************Copyright(H)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bmstimeout.h
* Author			  :      
* Date First Issued	: 10/6/2015
* Version			  : V0.1
* Description		: �󲿷������������������ʼ��
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
#define RX_WAIT_TIMEOUT_ENTER    0      //��ʱ�жϿ�ʼ����
#define RX_WAIT_TIMEOUT_CHECK    1      //��ʱ�жϼ��
#define INT32U_MAX_NUM          (0xFFFFFFFFu)
/* Private typedef----------------------------------------------------------------------------*/

CARD_STEP Card_FrameRx_Timeout(CARD_STEP step,INT32U timeout,INT8U flag);
//����������Ҫ�ɶ�ʹ�ã����ܵ���ʹ��



#define CARD_TIMEOUT_ENTER(step,time)   Card_FrameRx_Timeout(step,time,RX_WAIT_TIMEOUT_ENTER)
#define CARD_TIMEOUT_CHECK(step,time)   Card_FrameRx_Timeout(step,time,RX_WAIT_TIMEOUT_CHECK)
#endif //__WAIT_TIMEOUT_H_
/************************(H)COPYRIGHT 2018 �����Ƽ�*****END OF FILE****************************/
