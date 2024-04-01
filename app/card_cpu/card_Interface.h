/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: card_Interface.h
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
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
//	_DefaultCard = 0x00,              //默认状态
//	  		
//	_SuccessRtu = 0x01,               //结果成功    
//	_FailRtu,                         //结果失败  

//	_LockCard,                        //密码错误次数过多，已锁卡  
//  _GreyLockFull,                    //灰锁次数已满	
//	_BalanceLack,                     //余额不足
//	_FailPIN,                         //密码错误	
//	_DataOverTime,                    //数据接收超时
//	_NotFoundCard,                    //未找到卡  
//	_NOStartCard,                     //非启动卡
//	//_FrameLack = 0xFE,              //帧缺失	
//}FRAMERTU;


//__packed typedef struct
//{
//	INT32U         money;                         //消费金额
//	INT32U         SerialNum;                     //消费编号
//	_BSPRTC_TIME   TradeData;	                    //消费时间
//}CONSUMPTION;

__packed typedef struct                           //用于数据传替
{
	INT8U          password[6];                   //卡密码	 
	INT32U         money;                         //消费金额
}RECEIVEDATA;
/* ------------------------------------接口函数-----------------------------------------------*/
/*****************************************************************************
* Function     : APP_GetCardEvent
* Description  : 获取刷卡事件控制块指针
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
* Description  : 设置卡信息获取状态：获取 未获取
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :20200629
*****************************************************************************/
void SetCardInfoStatus(GETCARD_INFO state);
#endif
/************************(C)COPYRIGHT 2018 汇誉科技*****END OF FILE****************************/
