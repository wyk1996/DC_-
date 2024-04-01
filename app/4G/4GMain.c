/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.c
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
#include "4GMain.h"
#include "bsp_uart.h"
#include <string.h>
#include "sysconfig.h"
#include "ModuleSIM7600.h"
#include "DisplayMain.h"
#include "HYFrame.h"
#include "ZHFrame.h"
#include "HFQGFrame.h"
#include "APFrame.h"
#include "ADSamplemain.h"
#include "YKCFrame.h"
#include  "_66Frame.h"
/* Private define-----------------------------------------------------------------------------*/
#define   GPRSMAIN_Q_LEN  								20

//临时IP端口放在这里，后面应该重屏幕下发读取

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPRSMAINOSQ[GPRSMAIN_Q_LEN];					// 消息队列
static OS_EVENT *GPRSMainTaskEvent;				            // 使用的事件
OS_EVENT *SendMutex;                 //互斥锁，同一时刻只能有一个任务进行临界点访问
_HTTP_INFO HttpInfo = {0};
_FTP_INFO  FtpInfo = {0};
_RECV_OTA_FLASH_CONTROL OTAFlashControl = {0};
_NET_CONFIG_INFO  NetConfigInfo[XY_MAX] = 
{
	{XY_HY,		{116,62,125,35},8000, {0} ,		1},   
	//{XY_HY,	{116,62,125,35},8000, {"zj.fengjingit.com"} ,		1},
	
	{XY_YKC,	{121,43,69,62},8767,{0}	,		1},
	
	{XY_AP,		{114,55,186,206},5738, {0}  ,	1},  //安培
	
	{XY_XJ,	    {139,199,242,25},9703, {0} ,	1},  //小桔
	
	{XY_YL1,	{60,205,188,90},50000,{0} ,		1},  //达克云
	
	{XY_YL2,	{47,108,227,112},8000,{0} ,	  	1},  //安充
	
	{XY_YL3,	{118,190,129,186},5042,{0} ,	1},  //星星
	
	{XY_YL4,	{81,70,61,150},50000,{0} ,		1},  //塑云
	
	{XY_66,		{139,196,60,58},19040,{0} ,		1},  //66
	
	{XY_JG,		{114,55,126,89},8000,{0} ,		1},
	
	{XY_TT,		{121,36,64,61},8767,{0} ,		1},
	
	{XY_ZSH,	{36,112,48,159},8767,{0} ,		1},
};

_RESEND_BILL_CONTROL ResendBillControl[GUN_MAX] ={ {0},{0}};
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : APP_SetResendBillState
* Description  : 设置是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
void APP_SetResendBillState(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
	ResendBillControl[gun].ResendBillState = state;
	ResendBillControl[gun].SendCount = 0;
}


//桩上传结算指令
/*****************************************************************************
* Function     : ReSendBill
* Description  : 重发订单
* Input        : void *pdata  ifquery: 1 查询  0：重复发送
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U  ReSendBill(INT8U gun,INT8U* pdata,INT8U ifquery)
{
	
	if(pdata == NULL)
	{
		return FALSE;	
	}
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	ResendBillControl[gun].CurTime = OSTimeGet();		//获取当前时间
	if(ifquery)
	{
		if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
		{
			return HY_SendBillData(pdata,500,ifquery);
		}
		return TRUE;
	}
	if(ResendBillControl[gun].ResendBillState == FALSE)
	{
		return FALSE;			//不需要重发订单
	}
	if((ResendBillControl[gun].CurTime - ResendBillControl[gun].LastTime) >= SYS_DELAY_10s)
	{
		if(++ResendBillControl[gun].SendCount > 5)
		{
			ResendBillControl[gun].ResendBillState = FALSE;		//发送三次没回复就不发了
			ResendBillControl[gun].SendCount = 0;
			return FALSE;
		}
		ResendBillControl[gun].LastTime = ResendBillControl[gun].CurTime;
		if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
		{
			return HY_SendBillData(pdata,500,ifquery);
		}
		if(SYSSet.NetYXSelct == XY_66)
		{
			return _66_SendBillData(pdata,500,ifquery);
		}
//		if(SYSSet.NetYXSelct == XY_HFQG)
//		{
//			return HFQG_SendBillData(pdata,200);
//		}
		if(SYSSet.NetYXSelct == XY_AP)
		{
			return AP_SendBillData(pdata,200);
		}
		if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
		{
			return YKC_SendBillData(pdata,200);
		}
	}
	

	return TRUE;
}

/*****************************************************************************
* Function     : ReSendOffLineBill
* Description  : 
* Input        : 发送离线交易记录订单
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U  ReSendOffLineBill(void)
{
	static INT8U count = 0;			//联网状态下，连续三次发送无反应则丢失
	INT8U data[300];
	static INT8U num = 0;
	//离线交易记录超时不管A枪和B枪都一样，目前都只用A枪
	ResendBillControl[GUN_A].OFFLineCurTime = OSTimeGet();		//获取当前时间
	
	//获取是否有离线交易记录
	ResendBillControl[GUN_A].OffLineNum = APP_GetNetOFFLineRecodeNum();		//获取离线交易记录
	if(ResendBillControl[GUN_A].OffLineNum > 0)
	{
		if((ResendBillControl[GUN_A].OFFLineCurTime - ResendBillControl[GUN_A].OFFLineLastTime) >= SYS_DELAY_30s)
		{
			if(num == ResendBillControl[GUN_A].OffLineNum)
			{
				//第一次不会进来
				if(++count >= 3)
				{
					//联网状态下连续三次未返回，则不需要发送
					count = 0;
					ResendBillControl[GUN_A].OffLineNum--;
					APP_SetNetOFFLineRecodeNum(ResendBillControl[GUN_A].OffLineNum);
				}
			}
			else
			{
				count = 0;
				num = ResendBillControl[GUN_A].OffLineNum;
			}
			ResendBillControl[GUN_A].OFFLineLastTime = ResendBillControl[GUN_A].OFFLineCurTime;
			if(SYSSet.NetYXSelct == XY_AP)
			{
				APP_RWNetOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_READ,data);   //读取离线交易记录
				AP_SendOffLineBillData(data,300);
				APP_RWNetFSOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_READ,data);   //读取离线交易记录
				AP_SendOffLineBillFSData(data,300);
			}
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetGPRSMainEvent
* Description  :获取网络状态 
* Input        : 那一路
* Output       : TRUE:表示有网络	FALSE:表示无网络
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
INT8U  APP_GetNetState(INT8U num)
{
	if(STATE_OK == APP_GetAppRegisterState(num))
	{
		return TRUE;
	}
	return FALSE;
}



/*****************************************************************************
* Function     : APP_GetResendBillState
* Description  : 获取是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_GetResendBillState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	return ResendBillControl[gun].ResendBillState;
}


/*****************************************************************************
* Function     : 4G_RecvFrameDispose
* Description  :4G接收
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U _4G_RecvFrameDispose(INT8U * pdata,INT16U len)
{
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		return HY_RecvFrameDispose(pdata,len);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		return _66_RecvFrameDispose(pdata,len);
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		return HFQG_RecvFrameDispose(pdata,len);
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		return AP_RecvFrameDispose(pdata,len);
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		return YKC_RecvFrameDispose(pdata,len);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		return _66_RecvFrameDispose(pdata,len);
	}
	return TRUE;
}


/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetBatchNum(INT8U gun)
{

	
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		return APP_GetHYBatchNum(gun);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		return APP_Get66BatchNum(gun);
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		return APP_GetHFQGBatchNum(gun);
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		return APP_GetAPBatchNum(gun);
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		return APP_GetYKCBatchNum(gun);
	}
	return APP_GetHYBatchNum(gun);
}

/*****************************************************************************
* Function     : APP_GetNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetNetMoney(INT8U gun)
{
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		return APP_GetHYNetMoney(gun);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		return APP_Get66NetMoney(gun);
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		return APP_GetHFQGNetMoney(gun);
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		return APP_GetAPQGNetMoney(gun);
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		return APP_GetYKCNetMoney(gun);
	}
	return APP_GetHYNetMoney(gun);
}
/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  :4G发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U  _4G_SendFrameDispose()
{
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		HY_SendFrameDispose();
	}
	#if (ENBLE_JG == 0)   //精工的只开发精工平台
	if(SYSSet.NetYXSelct == XY_66)
	{
		_66_SendFrameDispose();
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		HFQG_SendFrameDispose();
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendFrameDispose();
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		YKC_SendFrameDispose();
	}
	#endif
	return TRUE;
}

/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   Pre4GBill(_GUN_NUM gun,INT8U *pdata)
{
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		PreHYBill(gun,pdata);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		Pre66Bill(gun,pdata);
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		PreHFQGBill(gun,pdata);
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		PreAPBill(gun,pdata);
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		PreYKCBill(gun,pdata);
	}
	return TRUE;
}



/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendDevState(_GUN_NUM gun)
{
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		if(gun == GUN_A)
		{
			HY_SendDevStateA();
		}
		if(gun == GUN_B)
		{
			HY_SendDevStateB();
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _4G_SendRateAck
* Description  : 费率应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendRateAck(INT8U cmd)
{
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		HY_SendRateAck(cmd);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		_66_SendRateAck(cmd);
	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendRateAck(cmd);
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		YKC_SendRateAck(cmd);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HY_SendQueryRateAck
* Description  : 查询费率应答 
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U    _4G_SendQueryRate(void)
{
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		YKC_SendPriReq();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _4G_SendRateMode
* Description  : 发送计费模型
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U    _4G_SendRateMode(void)
{
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC ) ||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		YKC_SendPriModel();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _4G_SendSetTimeAck
* Description  : 校时应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendSetTimeAck(void)
{

	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendSetTimeAck();
	}
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC )||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		YKC_SendSetTimeAck();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _4G_SendUpdataAck
* Description  : 远程升级应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendUpdataAck(void)
{
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC )||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		YKC_SendUpdataAck();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendBill(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		HY_SendBill(gun);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		_66_SendBill(gun);
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		HFQG_SendBill(gun);
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendBill(gun);
		AP_SendTimeSharBill(gun);		//发送分时记录
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC) ||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		if(gun == GUN_A)
		{
			YKC_SendGunABSD();
		}
		else
		{
			YKC_SendGunBBSD();
		}
		YKC_SendBill(gun);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _4G_SendCardInfo
* Description  : 发送卡鉴权
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendCardInfo(_GUN_NUM gun)
{
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendCardInfo(gun);
	}
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		HY_SendCardInfo(gun);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		_66_SendCardInfo(gun);
	}
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
			YKC_SendCardInfo(gun);
	//		YKC_SendBCCardInfo(GUN_A);
	//		OSTimeDly(SYS_DELAY_200ms); 
	//		YKC_SendBCCardInfo(GUN_B);

		
		 //YKC_SendCardInfo(gun);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _4G_GetStartType
* Description  : 获取快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
_4G_START_TYPE   _4G_GetStartType(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return _4G_APP_START;
	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		return (_4G_START_TYPE)APP_GetAPStartType(gun);
	}
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		 return (_4G_START_TYPE)APP_GetHYStartType(gun);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		 return (_4G_START_TYPE)APP_Get66StartType(gun);
	}
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		 return (_4G_START_TYPE)APP_GetYKCStartType(gun);
	}
	return _4G_APP_START;
}


/*****************************************************************************
* Function     : _4G_SetStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _4G_SetStartType(INT8U gun ,_4G_START_TYPE  type)
{
	if((type >=  _4G_APP_MAX) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	
	if(SYSSet.NetYXSelct == XY_AP)
	{
		APP_SetAPStartType(gun,type);
	}
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		APP_SetHYStartType(gun,type);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		APP_Set66StartType(gun,type);
	}
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		APP_SetYKCStartType(gun,type);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _4G_SendVinInfo
* Description  : 发送Vin鉴权
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendVinInfo(_GUN_NUM gun)
{
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendVinInfo(gun);
	}
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
			YKC_SendCardInfo(gun);
//			YKC_SendBCCardInfo(GUN_A);
//			YKC_SendBCCardInfo(GUN_B);
	}
	return TRUE;
}


/*****************************************************************************
* Function     : _4G_SendCardVinCharge
* Description  : 发送卡Vin充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendCardVinCharge(_GUN_NUM gun)
{
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendCardVinStart(gun);
	}
	return TRUE;
}
/*****************************************************************************
* Function     : _4G_SendStOPtAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _4G_SendStopAck(_GUN_NUM gun)
{	
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
	{
		HY_SendStopAck(gun);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		_66_SendStopAck(gun);
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		HFQG_SendStopAck(gun);
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendStopAck(gun);
	}
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		YKC_SendStopAck(gun);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_SendStartAck
* Description  : 合肥乾古开始充电应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   _4G_SendStartAck(_GUN_NUM gun)
{
	if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2) ||  (SYSSet.NetYXSelct == XY_JG))
	{
		HY_SendStartAck(gun);
	}
	if(SYSSet.NetYXSelct == XY_66)
	{
		_66_SendStartAck(gun);
	}
//	if(SYSSet.NetYXSelct == XY_HFQG)
//	{
//		HFQG_SendStartAck(gun);
//	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		AP_SendStartAck(gun);
	}

	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		if(_4G_GetStartType(gun) == _4G_APP_START)
		{
			YKC_SendStartAck(gun);
		}
		else
		{
			YKC_SendBCStartAck(gun);
		}
	}
	return TRUE;
}
/*****************************************************************************
* Function     : APP_GetGPRSMainEvent
* Description  :获取GPRSMain任务事件
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
OS_EVENT* APP_Get4GMainEvent(void)
{
    return GPRSMainTaskEvent;
}

/*****************************************************************************
* Function     : UART_4GWrite
* Description  :串口写入，因多个任务用到了串口写入，因此需要加互斥锁
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2020-11-26     叶喜雨
*****************************************************************************/
INT8U UART_4GWrite(INT8U* const FrameBuf, const INT16U FrameLen)
{
	INT16U len;
	INT8U err;
	
		printf("\r\n4g Send:");
	if(APP_GetModuleConnectState(0) == STATE_OK)
	{
		if(APP_GetSIM7600Mode() == MODE_DATA)
		{
			for(len = 0;len < FrameLen;len++)
			{
				printf("%02x ",FrameBuf[len]);
			}
		}
		else
		{
			for(len = 0;len < FrameLen;len++)
			{
				printf("%c",FrameBuf[len]);
			}
		}
	}
	else
	{
		for(len = 0;len < FrameLen;len++)
		{
			printf("%c",FrameBuf[len]);
		}
	}
	OSMutexPend(SendMutex, 0, &err);					//获取锁
	len = BSP_UARTWrite(GPRS_UART,FrameBuf,FrameLen);
	if(len)
	{
		OSTimeDly((FrameLen/10 + 10)*SYS_DELAY_1ms);	//等待数据发送完成  115200波特率， 1ms大概能发10个字节（大于10个字节）
		if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)||(SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4))    //连云快充双平台的时候，发送数据很快
		{
			OSTimeDly(SYS_DELAY_50ms);
		}
		
	}
	OSMutexPost(SendMutex); //释放锁

	return len;
}

/*****************************************************************************
* Function     : Connect_4G
* Description  : 4G网络连接
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日       
*****************************************************************************/
static INT8U Connect_4G(void)
{
	#define RESET_4G_COUNT	5    //目前连续1次未连接上服务器 ，则重新启动
	static INT32U i = 0;
	INT8U count;
	static INT8U num = 5;
	
	if(NetConfigInfo[SYSSet.NetYXSelct].NetNum >= 2)
	{
		if((APP_GetModuleConnectState(0) !=STATE_OK) || (APP_GetModuleConnectState(1) !=STATE_OK)) //
		{
			if(++num >= RESET_4G_COUNT)
			{
				num = 0;
				SIM7600Reset();
			}
		}
	}
	else
	{
		if(APP_GetModuleConnectState(0) !=STATE_OK) //
		{
			if(++num >= RESET_4G_COUNT)
			{
				num = 0;
				SIM7600Reset();
			}
		}
	}
	if(APP_GetSIM7600Status() != STATE_OK)  //模块存在
	{
		Module_SIM7600Test();    //测试模块是否存在 
	}
	if(APP_GetSIM7600Status() != STATE_OK)  //模块不存在
	{
		return FALSE;
	}
	//到此说明模块已经存在了模块存在了
	//连接服务器,可能又多个服务器
	for(count = 0;count < NetConfigInfo[SYSSet.NetYXSelct].NetNum;count++)
	{
		if(APP_GetModuleConnectState(count) != STATE_OK) //位连接服务器
		{
			if(count == 0)
			{
				ModuleSIM7600_ConnectServer(count,(INT8U*)NetConfigInfo[SYSSet.NetYXSelct].pIp,NetConfigInfo[SYSSet.NetYXSelct].port);	
			}
			else
			{
				ModuleSIM7600_ConnectServer(count,(INT8U*)GPRS_IP2,GPRS_PORT2);		//连接服务器
			}
		}		
		if(APP_GetModuleConnectState(count) != STATE_OK)  //模块未连接
		{
			SIM7600CloseNet(count);			//关闭网络操作
		}
	}

	return TRUE;
}


/*****************************************************************************
* Function     :Module_FatDownload
* Description  :FTP下载
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
//uint8_t FTPDownloadDispose(uint8_t * pdata,uint16_t lendata)
//{
//	 static uint8_t Lendatabyte; //长度占用几个字节
//	static char lenbuf[5] = {0};    //接收的数据长度
//	if((pdata == NULL)  || lendata < strlen("\r\n+CFTPSGET: DATA,"))
//	{
//		return FALSE;
//	}
//	
//	if(strncmp((char*)pdata,"\r\n+CFTPSGET: DATA,",strlen("\r\n+CFTPSGET: DATA,"))==0)
//    {
//        // 数据长度最长4个字节 最短1个字节，长度后，就是固定的0x0D 0x0A    等到接收的字节数量大于最多的长度时
//        if(lendata == 0)
//        {
//            for(uint8_t i=1; i<5; i++)
//            {
//                if((pdata[strlen("\r\n+CFTPSGET: DATA,")+i] == 0x0D)&&(pdata[strlen("\r\n+CFTPSGET: DATA,")+i+1] == 0x0A))
//                {
//                    memcpy(lenbuf,&pdata[strlen("\r\n+CFTPSGET: DATA,")],4);  //第一位是逗号，开始第二位就是长度，长度可能是（1-4位）,统一写4个
//                    lendata = atoi(lenbuf);    //获取的长度
//                    break;
//                }
//            }
//        }


//        if(lendata > 0)
//        {
//            //长度占用几个字节
//            if((9 < lendata) &&(lendata < 100))
//            {
//                Lendatabyte = 2;
//            }
//            else if((99<lendata) &&(lendata<1000))
//            {
//                Lendatabyte = 3;
//            }
//            else if(lendata > 999)
//            {
//                Lendatabyte = 4;
//            }
//            else
//            {
//                Lendatabyte = 1;
//            }

//            if((uint32_t)pdata >= (strlen("\r\n+CFTPSGET: DATA,\r\n") + Lendatabyte + lendata))
//            {
//                return TRUE;
//            }
//        }
//    }
//	return FALSE;
//}


#define MSG_NUM    5


uint8_t FLASHBUF[4096];
uint16_t countflash = 0;
/*****************************************************************************
* Function     : Task4GMain
* Description  : 4G主任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
void Task4GMain(void *pdata)
{
	static INT32U nowSysTime,lastSysTime;
    INT8U err;    
    _BSP_MESSAGE *pMsg;
    OS_EVENT * psendevent,*dispevent; 
	static uint8_t download_count = 0;
    static _BSP_MESSAGE send_message[GUN_MAX][MSG_NUM];
    static _BSP_MESSAGE disp_message[GUN_MAX][MSG_NUM];
	static INT8U  discount = 0,_4gcount = 0;
    _BSP_MESSAGE	* pmsg = NULL;
	SendMutex = OSMutexCreate(PRI_4GSEND_MUTEX, &err);      //创建互斥锁
	GPRSMainTaskEvent = OSQCreate(GPRSMAINOSQ, GPRSMAIN_Q_LEN);//创建显示接收消息队列
	if (GPRSMainTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
		return;
	}
	
	//打开电源
	BSP_IOClose(IO_4G_PWR);
	OSTimeDly(SYS_DELAY_100ms);
	
	BSP_IOClose(IO_4G_RES);
	BSP_IOClose(IO_4G_ONOFF);
	OSTimeDly(SYS_DELAY_5s);
	psendevent =  APP_GetSendTaskEvent();
	dispevent =  APP_GetDispEvent();
	nowSysTime = OSTimeGet();
	lastSysTime = nowSysTime;
//	
//	while(1)
//	{
//		BootLoadRead(FLASHBUF,5+countflash*4096,4096);
//		countflash++;
//	}
	while(1)
    {
		
//		ftp_test();
//		OSTimeDly(SYS_DELAY_1s);

		
     	pMsg = OSQPend(GPRSMainTaskEvent, SYS_DELAY_1s, &err);

		if(APP_GetSIM7600Mode() == MODE_DATA)
		{
			if (OS_ERR_NONE == err)
			{
				switch (pMsg->MsgID)
				{
					case BSP_MSGID_DISP:	//来自显示任务
							//消息转发
							if(pMsg->DataLen == GUN_A)
							{
								pmsg = &send_message[GUN_A][discount];
							}
							else
							{
								pmsg = &send_message[GUN_B][discount];
							}
							pmsg->MsgID = BSP_4G_MAIN;
							pmsg->DivNum = pMsg->DivNum;	
							pmsg->DataLen = pMsg->DataLen;	//枪号
							OSQPost(psendevent, pmsg);
							discount++;
							if(discount >= MSG_NUM)
							{
								discount = 0;
							}
						break;
					case BSP_4G_RECV:		//来自4G接收任务
							//消息转发
							if(pMsg->DataLen == GUN_A)
							{
								pmsg = &disp_message[GUN_A][_4gcount];
							}
							else
							{
								pmsg = &disp_message[GUN_B][_4gcount];
							}
							pmsg->MsgID = BSP_4G_MAIN;
							pmsg->DivNum = pMsg->DivNum;	
							pmsg->DataLen = pMsg->DataLen;	//枪号
							pmsg->pData =  pMsg->pData;
							OSQPost(dispevent, pmsg);
							_4gcount++;
							if(_4gcount >= MSG_NUM)
							{
								_4gcount = 0;
							}
							break;
					default:
						break;
				}
			}
			else
			{
				if(SYSSet.NetState == DISP_NET)
				{
					Connect_4G();          //4G连接，包括模块是否存在，和连接服务器
					//连续5分钟连，而且处于未插枪状态就重启
					nowSysTime = OSTimeGet();
					#if(USER_GUN != USER_SINGLE_GUN)
					if((APP_GetAppRegisterState(0) == STATE_4G_ERR) && (GetGunState(GUN_A) == GUN_DISCONNECTED) && (GetGunState(GUN_B) == GUN_DISCONNECTED))
					#else
					if((APP_GetAppRegisterState(0) == STATE_4G_ERR) && (GetGunState(GUN_A) == GUN_DISCONNECTED))
					#endif
					{
						if((nowSysTime >= lastSysTime) ? ((nowSysTime - lastSysTime) >= SYS_DELAY_10M) : \
						((nowSysTime + (0xffffffff - lastSysTime)) >= SYS_DELAY_10M))
						{
							lastSysTime = nowSysTime;
							NVIC_SystemReset();
						}
					}
					else
					{
						lastSysTime = nowSysTime;
					}
				}
			}
		}
		else if(APP_GetSIM7600Mode() == MODE_HTTP)
		{
			//远程升级
			 if(APP_GetModuleConnectState(0) == STATE_OK)
            {
                //memcpy(HttpInfo.ServerAdd,"http://hy.shuokeren.com/uploads/xiaov1.6.bin",strlen("http://hy.shuokeren.com/uploads/xiaov1.6.bin"));
                if(Module_HTTPDownload(&HttpInfo))
                {
                    //升级成功
                    download_count = 0;
                    //APP_SetUpadaState(1);   //说明升级成功
                    APP_SetSIM7600Mode(MODE_DATA);
					if(NetConfigInfo[SYSSet.NetYXSelct].NetNum == 1)
					{
						Send_AT_CIPMODE();
					}
					OSTimeDly(SYS_DELAY_1s);
					HY_SendUpdataAck(0); //表示升级成功
					OSTimeDly(SYS_DELAY_3s);
                    jump_to_app(LoadCodeAddress); // 跳转到BOOT
                }
                else
                {
                    if(++download_count > 3)
                    {
                        //连续三次升级不成功，则返回升级失败
                        download_count = 0;
						if(NetConfigInfo[SYSSet.NetYXSelct].NetNum == 1)
						{
							Send_AT_CIPMODE();
						}
						OSTimeDly(SYS_DELAY_1s);
                        HY_SendUpdataAck(1); //表示升级失败
                        NVIC_SystemReset();//升级失败，系统复位（自动上传当前的版本号）
                        APP_SetSIM7600Mode(MODE_DATA);

                    }
                }
            }
		}
		else
		{
			//FTP
			 if(APP_GetModuleConnectState(0) == STATE_OK)
            {
//                memcpy(FtpInfo.FilePath,"/cehi/UP11.bin",strlen("/cehi/UP11.bin"));
//				FtpInfo.Port = 8866;
//				memcpy(FtpInfo.usename,"admin",strlen("admin"));
//				memcpy(FtpInfo.password,"admin123",strlen("admin123"));
//				memcpy(FtpInfo.SerAdd,"121.42.236.89",strlen("121.42.236.89")); //路径和名字相同
                if(Module_FTPDownload(&FtpInfo))
                {
                    //升级成功
                    download_count = 0;
                    //APP_SetUpadaState(1);   //说明升级成功
                    APP_SetSIM7600Mode(MODE_DATA);
					if(NetConfigInfo[SYSSet.NetYXSelct].NetNum == 1)
					{
						Send_AT_CIPMODE();
					}
					OSTimeDly(SYS_DELAY_1s);
					OSTimeDly(SYS_DELAY_3s);
                    jump_to_app(LoadCodeAddress); // 跳转到BOOT
                }
                else
                {
                    if(++download_count > 3)
                    {
                       //连续三次升级不成功，则返回升级失败
                        download_count = 0;
						if(NetConfigInfo[SYSSet.NetYXSelct].NetNum == 1)
						{
							Send_AT_CIPMODE();
						}
						OSTimeDly(SYS_DELAY_1s);
                        HY_SendUpdataAck(1); //表示升级失败
                        NVIC_SystemReset();//升级失败，系统复位（自动上传当前的版本号）
                        APP_SetSIM7600Mode(MODE_DATA);
                    }
                }
            }
		}
	}	
}

/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

