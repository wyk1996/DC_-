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
#ifndef	__APFRAME_H_
#define	__APFRAME_H_
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : AP_RecvFrameDispose
* Description  :安培快充接收处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   AP_RecvFrameDispose(INT8U * pdata,INT16U len);

/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  : 汇誉接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   AP_SendFrameDispose(void);


/*****************************************************************************
* Function     : HY_SendStartAck
* Description  : 启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   AP_SendStartAck(_GUN_NUM gun);


/*****************************************************************************
* Function     : APP_SetAPStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_SetAPStartType(INT8U gun ,_4G_START_TYPE  type);

/*****************************************************************************
* Function     : APP_GetAPSta
* Description  : 获取安培快充启动方式  0表示App启动 1表示刷卡启动
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_GetAPStartType(INT8U gun);

/*****************************************************************************
* Function     : PreHYBill
* Description  : 保存汇誉订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   PreAPBill(_GUN_NUM gun,INT8U *pdata);

/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U AP_SendBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : AP_SendTimeSharBill
* Description  : 上传分时交易记录   充电完成后，在B12发送完成后上报。
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendTimeSharBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : HY_SendStopAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   AP_SendStopAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : HY_SendBillData
* Description  : 发送订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U AP_SendBillData(INT8U * pdata,INT8U len);

/*****************************************************************************
* Function     : AP_SendOffLineBillData
* Description  : 发送离线交易记录订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U AP_SendOffLineBillData(INT8U * pdata,INT16U len);

/*****************************************************************************
* Function     : AP_SendOffLineBillData
* Description  : 发送离线交易记录订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U AP_SendOffLineBillFSData(INT8U * pdata,INT16U len);

/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetAPBatchNum(INT8U gun);

/*****************************************************************************
* Function     : APP_GetHYNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetAPNetMoney(INT8U gun);


/*****************************************************************************
* Function     : HFQG_SendDevStateB
* Description  : 充电桩A状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   AP_SendDevStateA(void);

/*****************************************************************************
* Function     : HFQG_SendDevStateB
* Description  : 充电桩B状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   AP_SendDevStateB(void);

/*****************************************************************************
* Function     : APP_GetAPBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetAPBatchNum(INT8U gun);

/*****************************************************************************
* Function     : APP_GetAPQGNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetAPQGNetMoney(INT8U gun);

/*****************************************************************************
* Function     : HY_SendRateAck
* Description  : 费率设置应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   AP_SendRateAck(INT8U cmd);

/*****************************************************************************
* Function     : AP_SendSetTimeAck
* Description  : 设置时间应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendSetTimeAck(void);

/*****************************************************************************
* Function     : AP_SendCardInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendCardInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : AP_SendVinInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendVinInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : AP_SendCardVinStart
* Description  :刷卡VIN启动
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendCardVinStart(_GUN_NUM gun);
#endif
/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

