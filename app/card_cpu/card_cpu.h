/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: card_cpu.h
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __CARD_CUP_H_
#define __CARD_CUP_H_
/* Includes-----------------------------------------------------------------------------------*/
#include	"sysconfig.h"
#include  "bsp_rtc.h"
/* Private define-----------------------------------------------------------------------------*/

/* Private struct-----------------------------------------------------------------------------*/
typedef enum
{	
	CARD_STEP_DEFAULT = 0,
	CARD_SEARCH_STEP,			//寻卡
	CARD_KEY_STEP,					//密钥验证
	CARD_READ_STEP,		    		//读
	CARD_WRITE_STEP,				//写
	CARD_SET_STEP,					//设置
	CARD_INFODIPOSE_STEP,			//显示任务处理卡信息
	CARD_MAX_STEP,				
    CARD_TIME_OUT,					//超时
	CARD_PARA_ERR,					//参数错误
	CARD_UNLOCK						//解锁验证
}CARD_STEP;

  
typedef enum                         //当前卡状态
{	
	_Card_IDLE  = 0x00,                //卡空闲
	_Card_Work = 0x01,	               //卡工作中
}CARDWORK_STATUS;

typedef enum
{
	_Not_Obtain = 0x00,              //未获取信息 
	_Already_Obtain ,	               //已获取信息  
}GETCARD_INFO;


//寻卡结果：_SuccessRtu、_FailRtu、_DataOverTime、_NotFoundCard
//启动充电结果：SuccessRtu、_FailRtu、_DataOverTime、_NotFoundCard、_LockCard、_GreyLockFull、_FailPIN
//停止充电结果：SuccessRtu、_FailRtu、_DataOverTime、_NotFoundCard、_NOStartCard、_BalanceLack
typedef enum 
{  
	_DefaultCard = 0x00,              //默认状态
	  		
	_SuccessRtu = 0x01,               //结果成功    
	_FailRtu,                         //结果失败   
	_DataOverTime,                    //数据接收超时
	_NotFoundCard,                    //未找到卡 
	
	_LockCard,                        //密码错误次数过多，已锁卡  
  _GreyLockFull,                    //灰锁次数已满	
	_BalanceLack,                     //余额不足
	_FailPIN,                         //密码错误	
 
	_NOStartCard,                     //非启动卡
	_FailNotIssuer,					  //非发行方卡
	//_FrameLack = 0xFE,              //帧缺失	
}FRAMERTU;

typedef struct
{
    INT8U cmd;                                      //相应的指令
    void (*Frame)(void);                            //发送相应的帧
	  FRAMERTU (*Analyse)(INT8U *data, INT16U len);   //解析相应的帧
}SELECT_CARD_FUNC;

__packed typedef struct                           //命令返回结果
{
	FRAMERTU       SearchCard;                         
	FRAMERTU       PINCheck;    
	FRAMERTU       Consumption;   
}CMDRTU;

__packed typedef struct                          //电卡系统信息
{
	FRAMERTU			 Card_Result;			//刷卡返回结果
	GETCARD_INFO         Get_Info;               //获取卡信息状态（显示任务已经获取卡返回的信息，才能将卡状态设置为空闲）
	CARDWORK_STATUS      Card_Status;            //卡状态	
}SYSCARDINFO;

__packed typedef struct                          //电卡用户信息
{
	INT8U         CardID[10];                     //卡号 
	INT8U         CardCode[10];                   //密码 	ASICC
	INT8U         RetryCnt;                      //剩余密码错误次数
	INT8U         GreyLockCnt;                   //卡灰锁次数
	INT32U        balance;                       //卡内余额
	INT8U         lockstate;					//结算状态 0xff表示未结算（上锁）   0x00表示已经结算（解锁）
}USERCARDINFO;

/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : GetCardWorkStatus
* Description  : 获取卡任务当前工作状态：空闲 工作
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
CARDWORK_STATUS GetCardWorkStatus(void);



/*****************************************************************************
* Function     : GetCardInfo
* Description  : 获取相应命令对应的信息
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
USERCARDINFO* GetCardInfo(void);
#endif 
/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/

