/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: card_Interface.h
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __CARD_INTTERFACE_H_
#define __CARD_INTTERFACE_H_
/* Includes-----------------------------------------------------------------------------------*/
#include	"sysconfig.h"
#include  "card_cpu.h"
/* Private define-----------------------------------------------------------------------------*/

/* Private struct-----------------------------------------------------------------------------*/
//typedef enum
//{  
//	_DefaultCard = 0x00,              //Ĭ��״̬
//	  		
//	_SuccessRtu = 0x01,               //����ɹ�    
//	_FailRtu,                         //���ʧ��  

//	_LockCard,                        //�������������࣬������  
//  _GreyLockFull,                    //������������	
//	_BalanceLack,                     //����
//	_FailPIN,                         //�������	
//	_DataOverTime,                    //���ݽ��ճ�ʱ
//	_NotFoundCard,                    //δ�ҵ���  
//	_NOStartCard,                     //��������
//	//_FrameLack = 0xFE,              //֡ȱʧ	
//}FRAMERTU;


//__packed typedef struct
//{
//	INT32U         money;                         //���ѽ��
//	INT32U         SerialNum;                     //���ѱ��
//	_BSPRTC_TIME   TradeData;	                    //����ʱ��
//}CONSUMPTION;

__packed typedef struct                           //�������ݴ���
{
	INT8U          password[6];                   //������	 
	INT32U         money;                         //���ѽ��
}RECEIVEDATA;
/* ------------------------------------�ӿں���-----------------------------------------------*/
/*****************************************************************************
* Function     : APP_GetCardEvent
* Description  : ��ȡˢ���¼����ƿ�ָ��
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
*****************************************************************************/
OS_EVENT* APP_GetCardEvent(void);	 

/*****************************************************************************
* Function     : GetCardWorkStatus
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
CARDWORK_STATUS GetCardWorkStatus(void);

/*****************************************************************************
* Function     : GetCardInfo
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
USERCARDINFO* GetCardInfo(void);

/*****************************************************************************
* Function     : GetCardInfo
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
USERCARDINFO* GetGunCardInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : SetCardInfoStatus
* Description  : ���ÿ���Ϣ��ȡ״̬����ȡ δ��ȡ
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :20200629
*****************************************************************************/
void SetCardInfoStatus(GETCARD_INFO state);
#endif
/************************(C)COPYRIGHT 2018 �����Ƽ�*****END OF FILE****************************/
