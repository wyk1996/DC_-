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
#ifndef	__HYFRAME_H_
#define	__HYFRAME_H_
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : HFQG_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HY_RecvFrameDispose(INT8U * pdata,INT16U len);

/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  : 汇誉接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HY_SendFrameDispose(void);


/*****************************************************************************
* Function     : HY_SendStartAck
* Description  : 启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   HY_SendStartAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : PreHYBill
* Description  : 保存汇誉订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   PreHYBill(_GUN_NUM gun,INT8U *pdata);

/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U HY_SendBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : HY_SendStopAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   HY_SendStopAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : HY_SendBillData
* Description  : 发送订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U HY_SendBillData(INT8U * pdata,INT16U len,INT8U ifquery);

/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetHYBatchNum(INT8U gun);

/*****************************************************************************
* Function     : HY_SendRateAck
* Description  : 费率设置应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   HY_SendRateAck(INT8U cmd);

/*****************************************************************************
* Function     : APP_GetHYNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetHYNetMoney(INT8U gun);


/*****************************************************************************
* Function     : HFQG_SendDevStateB
* Description  : 充电桩A状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   HY_SendDevStateA(void);

/*****************************************************************************
* Function     : HFQG_SendDevStateB
* Description  : 充电桩B状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   HY_SendDevStateB(void);

/*****************************************************************************
* Function     : HY_SendCardInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  HY_SendCardInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : APP_GetAPSta
* Description  : 获取安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_GetHYStartType(INT8U gun);

/*****************************************************************************
* Function     : APP_SetAPStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_SetHYStartType(INT8U gun ,_4G_START_TYPE  type);

/*****************************************************************************
* Function     : HY_SendUpdataAck
* Description  : 升级应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  HY_SendUpdataAck(INT8U num);

/***************************************************************
**Function   :HY_SendVinInfo
**Description:cmd  1 启动充电  0非启动充电
**Input      :gun: [输入/出]
**Output     :
**Return     :
**note(s)    :
**Author     :
**Create_Time:2023-2-17
***************************************************************/
INT8U  HY_SendVinInfo(INT8U gun,INT8U cmd);
#endif
/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

