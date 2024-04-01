/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.h
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
#ifndef	__YKCFRAME_H_
#define	__YKCFRAME_H_
#include "4GMain.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : YKC_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   YKC_RecvFrameDispose(INT8U * pdata,INT16U len);

/*****************************************************************************
* Function     : ZH_SendFrameDispose
* Description  : 汇誉接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   YKC_SendFrameDispose(void);


/*****************************************************************************
* Function     : ZH_SendStartAck
* Description  : 启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendStartAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : ZH_SendStartAck
* Description  : 启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendBCStartAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : PreZHBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   PreYKCBill(_GUN_NUM gun,INT8U *pdata);

/*****************************************************************************
* Function     : ZH_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : ZH_SendStopAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendStopAck(_GUN_NUM gun);



/*****************************************************************************
* Function     : APP_GetZHBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetYKCBatchNum(INT8U gun);

/*****************************************************************************
* Function     : ZH_SendRateAck
* Description  : 费率设置应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendRateAck(INT8U cmd);

/*****************************************************************************
* Function     : APP_GetYKCNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetYKCNetMoney(INT8U gun);

/*****************************************************************************
* Function     : YKC_SendBillData
* Description  : 云快充重发订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendSetTimeAck(void);

/*****************************************************************************
* Function     : YKC_SendBillData
* Description  : 云快充重发订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendBillData(INT8U * pdata,INT8U len);

/*****************************************************************************
* Function     : YKC_SendGunABSD
* Description  :结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunABSD(void);

/*****************************************************************************
* Function     : YKC_SendCardInfo
* Description  : 云快充发送卡鉴权
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  YKC_SendCardInfo(INT8U gun);

/*****************************************************************************
* Function     : YKC_SendBCCardInfo
* Description  : 云快充发送并充卡鉴权
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  YKC_SendBCCardInfo(INT8U gun);

/*****************************************************************************
* Function     : YKC_SendGunABSD
* Description  :结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunBBSD(void);


/*****************************************************************************
* Function     : YKC_SendPriModel
* Description  : 发送计费模型
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendPriModel(void);

/*****************************************************************************
* Function     : YKC_SendPriReq
* Description  : 发送计费请求
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendPriReq(void);

/*****************************************************************************
* Function     : APP_GetAPSta
* Description  : 获取安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_GetYKCStartType(INT8U gun);

/*****************************************************************************
* Function     : APP_SetAPStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_SetYKCStartType(INT8U gun ,_4G_START_TYPE  type);
#endif
/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

