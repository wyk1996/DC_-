/*****************************************Copyright(C)******************************************
*******************************************���ݻ���*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: DispkeyFunction.c
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 此文件用来实现按键的具体功能，如对数据的处理等操作，在Dispkey中被调用
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "DispKeyFunction.h"
#include "DwinProtocol.h"
#include "display.h"
#include "MenuDisp.h"
#include "DisplayMain.h"
#include "DispShowStatus.h"
#include "Disinterface.h"
#include "card_Interface.h"
#include "RS485Meter.h"
#include "charging_Interface.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
ST_Menu *gps_CurMenu = NULL;     //当前菜单
/* Private function prototypes----------------------------------------------------------------*/

/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : GetCurMenu
* Description  : 获取液晶当前菜单结构�?
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :  2018年7月17日
*****************************************************************************/
ST_Menu* GetCurMenu(void)  
{
    return gps_CurMenu;
}

/*****************************************************************************
* Function     : GetCurMenu
* Description  : 设置液晶当前菜单结构使
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :  2018年7月17日
*****************************************************************************/
void SetCurMenu(ST_Menu*     pmenu)
{
	if(pmenu == NULL)
	{
		return;
	}
	OSSchedLock();
	gps_CurMenu = pmenu;
	OSSchedUnlock();	
}

/**********************************汇誉屏幕*************************/
/********************************************************************
* Function Name : DisplayMenu1
* Description   : 主界面页面显示
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年6月14日  
********************************************************************/
void HYDisplayMenu0(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu8
* Description   : 付费方式选择显示
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu1(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu1
* Description   : 二维码界面显示
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu2(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电方式选择界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu3(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入充电时间界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu4(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入充电电量界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu5(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入充电电量界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu6(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu1
* Description   : 主界面页面显示
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年6月14日  
********************************************************************/
void HYDisplayMenu7(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu8
* Description   : 付费方式选择显示
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu8(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu1
* Description   : 二维码界面显示
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu9(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电方式选择界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu10(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入充电时间界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu11(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入充电电量界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu12(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入充电金额界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu13(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入卡密码界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu14(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡界面1
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu15(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡界面2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu16(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡界面3
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu17(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡界面4
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu18(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡界面5
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu19(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡界面6
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu20(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 电卡密码输入错误界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu21(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 卡内余额不足
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu22(void)
{
	static INT32U curtime = 0, lasttime = 0;
	
	curtime = OSTimeGet();
	
	
	_DISP_CONTROL* pcontrol = APP_GetDispControl();

	if(pcontrol == NULL)
	{
		return;
	}
	if(GetCardWorkStatus() != _Card_IDLE)
	{
		return;
	}
	if((curtime -  lasttime) >= SYS_DELAY_1s)
	{
		
		lasttime = curtime;
		
		if(SYSSet.NetState == DISP_NET)  //����汾ˢ��
		{
			if(pcontrol->CurUserGun == GUN_A)
			{
				if(SendCardMsg(NETCARDCMD_STARTA, pcontrol->CardCode,sizeof(pcontrol->CardCode)) == FALSE)
				{
					return;
				}
			}
			else
			{
				if(SendCardMsg(NETCARDCMD_STARTB, pcontrol->CardCode,sizeof(pcontrol->CardCode)) == FALSE)
				{
					return;
				}
			}
		}
		else
		{
			#if(CARD_USER_CODE == 1)
				if(HYMenu21.Menu_PrePage == &HYMenu3)			//上一页是充电方式选择界面
			#else
				if(HYMenu22.Menu_PrePage == &HYMenu3)			//上一页是充电方式选择界面
			#endif
				{
					
					if(pcontrol->CurUserGun == GUN_A)
					{
						if(SendCardMsg(CARDCMD_STARTA, pcontrol->CardCode,sizeof(pcontrol->CardCode)) == FALSE)
						{
							return;
						}
					}
					else
					{
						if(SendCardMsg(CARDCMD_STARTB, pcontrol->CardCode,sizeof(pcontrol->CardCode)) == FALSE)
						{
							return;
						}
					}
				}
				else
				{
					if(SendCardMsg(CARDCMD_QUERY, pcontrol->CardCode,sizeof(pcontrol->CardCode)) == FALSE)
					{
						return;
					}
				} 
			}
		}
	
    return;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 此卡已被锁提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu23(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 此卡无法识别提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu24(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 读卡超时提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu25(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu26(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 当前负荷已满，是否继续充电
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu27(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪A启动中界面1
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu28(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪A启动中界面2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu29(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪A启动中界面3
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu30(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪A启动中界面4
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu31(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪A启动中界面5
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu32(void)
{
	_DISP_CONTROL* pcontrol = APP_GetDispControl();
	_CHARGE_RECODE* pinfo = NULL;
	static INT32U TotMoney;   
	static INT32U curtime = 0, lasttime = 0;
	
	curtime = OSTimeGet();
	if(pcontrol == NULL)
	{
		return;
	}
	if(GetCardWorkStatus() != _Card_IDLE)
	{
		return;
	}
	pinfo = APP_GetChargeRecodeInfo(pcontrol->CurUserGun);

	if(pinfo == NULL)
	{
		return;
	}
	if((curtime -  lasttime) >= SYS_DELAY_1s)
	{
		lasttime = curtime;
		if(pcontrol->CurUserGun == GUN_A)
		{
			if(SendCardMsg(CARDCMD_STOPA, (void *)&TotMoney,sizeof(TotMoney) ) == FALSE)
			{
				return;
			}
		}
		else
		{
			if(SendCardMsg(CARDCMD_STOPB, (void *)&TotMoney,sizeof(TotMoney) ) == FALSE)
			{
				return;
			}
		}
	}
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡结算界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu33(void)
{
	_DISP_CONTROL* pcontrol = APP_GetDispControl();
	
	USERINFO* pric_info;
	static INT32U curtime = 0, lasttime = 0;
	
	curtime = OSTimeGet();

	pric_info = GetChargingInfo(pcontrol->CurUserGun);
	if(pcontrol == NULL)
	{
		return;
	}
	if(GetCardWorkStatus() != _Card_IDLE)
	{
		return;
	}
	
	
	if((curtime -  lasttime) >= SYS_DELAY_1s)
	{
		lasttime = curtime;
		if(pcontrol->CurUserGun == GUN_A)
		{
			if(SendCardMsg(CARDCMD_STOPA, (void *)&pric_info->TotalBill,sizeof(pric_info->TotalBill) ) == FALSE)
			{
				return;
			}
			pric_info->RealTotalBill = pric_info->TotalBill;
			pric_info->RealTotalPower = pric_info->TotalPower4;
		}
		else
		{
			if(SendCardMsg(CARDCMD_STOPB, (void *)&pric_info->TotalBill,sizeof(pric_info->TotalBill) ) == FALSE)
			{
				return;
			}
			pric_info->RealTotalBill = pric_info->TotalBill;
			pric_info->RealTotalPower = pric_info->TotalPower4;
		}
	}
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪B启动中界面1
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu34(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电停止中
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu35(void)
{

	//启动马上停止存在bug！！
//	_DISP_CONTROL* pcontrol = APP_GetDispControl();
//	if(APP_GetWorkState(pcontrol->CurUserGun) != WORK_CHARGE)   //不在工作状态，跳转到结算界面
//	{
//		if(pcontrol->CurUserGun == GUN_A)
//		{
//			DispShow_EndChargeDate(GUN_A);
//			//刷卡执行状态和停止停止状态清零
//			pcontrol->CardOver[GUN_A] = STATE_UNOVER;
//			pcontrol->StopOver[GUN_A] = STATE_UNOVER;
//			DisplayCommonMenu(&HYMenu40,&HYMenu35);      //跳转到结算界面
//		}else{
//			DispShow_EndChargeDate(GUN_B);
//			pcontrol->CardOver[GUN_B] = STATE_UNOVER;
//			pcontrol->StopOver[GUN_B] = STATE_UNOVER;
//			DisplayCommonMenu(&HYMenu41,&HYMenu35);      //跳转到结算界面
//		}
//	}
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : A枪充电界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu36(void)
{
	_CHARGEING_INFO * pinfo = APP_GetChargeingInfo(GUN_A);
	if(pinfo == NULL)
	{
		return;
	}
	//减去电池类型，电车类型单独显示
	PrintStr(DIS_ADD(HYMenu36.FrameID,0),(INT8U*)pinfo ,sizeof(_CHARGEING_INFO) - 1 );
	//显示电池类型,因电池类型从1开始，故显示基地址需要-1
	Dis_ShowStatus(0x1215,(_SHOW_NUM)(pinfo->BatteryType + (INT8U)SHOW_BATTERY_TYPE1 -1),RED);    //显示电池类型
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : B枪充电界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu37(void)
{
	_CHARGEING_INFO * pinfo = APP_GetChargeingInfo(GUN_B);
	if(pinfo == NULL)
	{
		return;
	}
	//减去电池类型，电车类型单独显示
	PrintStr(DIS_ADD(HYMenu36.FrameID,0),(INT8U*)pinfo ,sizeof(_CHARGEING_INFO) - 1 );
	//显示电池类型,因电池类型从1开始，故显示基地址需要-1
	Dis_ShowStatus(0x1295,(_SHOW_NUM)(pinfo->BatteryType + (INT8U)SHOW_BATTERY_TYPE1 -1),RED);    //显示电池类型
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪B启动中界面5
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu38(void)
{
	_CARD_INFO * pinfo = APP_GetCarInfo(GUN_A);

	PrintStr(DIS_ADD(HYMenu38.FrameID,0),(INT8U*)pinfo ,sizeof(_CARD_INFO));
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪B启动中界面6
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu39(void)
{
	_CARD_INFO * pinfo = APP_GetCarInfo(GUN_B);

	PrintStr(DIS_ADD(HYMenu39.FrameID,0),(INT8U*)pinfo ,sizeof(_CARD_INFO) );
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : A枪结算界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu40(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : B枪结算界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu41(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu42(void)
{
	DispShow_StatusDivInfo();
	//DispShow_42DivInfo();
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 提示连接充电枪界面2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu43(void)
{
	DispShow_StatusDivInfo();
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 枪A充电界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu44(void)
{
	DispShow_StatusDivInfo();
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 枪B充电界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu45(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 枪A车辆信息界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu46(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 枪B车辆信息界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu47(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡结算界面1
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu48(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡结算界面2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu49(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 手动充电电压界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu50(void)
{
	INT16U vol,cur;
	_CHARGEING_INFO * pinfo = APP_GetChargeingInfo(GUN_A);
	if(pinfo == NULL)
	{
		return;
	}
	
	vol = (pinfo->ChargeVol & 0x00ff) << 8 | (pinfo->ChargeVol & 0xff00) >> 8;
	cur = (pinfo->ChargeCur & 0x00ff) << 8 | (pinfo->ChargeCur & 0xff00) >> 8;
	PrintNum16uVariable(DIS_ADD(HYMenu50.FrameID,2),vol);
	PrintNum16uVariable(DIS_ADD(HYMenu50.FrameID,3),cur);
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 手动充电界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu51(void)
{
	INT16U vol,cur;
	_CHARGEING_INFO * pinfo = APP_GetChargeingInfo(GUN_B);
	if(pinfo == NULL)
	{
		return;
	}
	vol = (pinfo->ChargeVol & 0x00ff) << 8 | (pinfo->ChargeVol & 0xff00) >> 8;
	cur = (pinfo->ChargeCur & 0x00ff) << 8 | (pinfo->ChargeCur & 0xff00) >> 8;
	PrintNum16uVariable(DIS_ADD(HYMenu51.FrameID,2),vol);
	PrintNum16uVariable(DIS_ADD(HYMenu51.FrameID,3),cur);
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 刷卡结算界面5
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu52(void)
{
	Display_ShowSet1();
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 充电枪B启动中界面4
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu53(void)
{
	Display_ShowSet2();
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 提示直接结算可能造成的后果
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu54(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 提示卡号不一致
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu55(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 提示充电停止中请勿把枪1
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu56(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 提示充电停止中请勿把枪2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu57(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : A枪结算界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu58(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : B枪结算界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu59(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 账户查询选择界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu60(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 输入查询账户密码界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu61(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 未找到账户信息界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu62(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 账户信息界面1
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu63(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 账户信息界面2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu64(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 读卡超时提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu65(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu66(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 账户信息界面1
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu73(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 账户信息界面2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu74(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 读卡超时提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu75(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu76(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 账户信息界面2
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu77(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 读卡超时提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu78(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu79(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 读卡超时提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu80(void)
{
	;
}

/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu81(void)
{
	;
}


/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu82(void)
{
	;
}


/********************************************************************
* Function Name : DisplayMenu10
* Description   : 非发行方卡提示界面
* Input         : 
* Output        : None.
* Return        : None.
* Contributor   :  2018年7月17日
********************************************************************/
void HYDisplayMenu83(void)
{
	;
}

/********************************************************************
* Function Name : DisplayCommonMenu()
* Description   : 普通菜单显示函
* Input         : -*pMenu,当前待显菜单结构.
* 				  pPrepage,记录上一页 ,如不需要继续，则传入NULL
* Output        : None.
* Return        : None.
* Contributor   : 2018年7月17日
********************************************************************/
 void DisplayCommonMenu(struct st_menu* pMenu,struct st_menu * pPrepage)
{
    if(pMenu == NULL)
    {
        return;
    }
	if(pPrepage != NULL)
	{
		pMenu->Menu_PrePage = pPrepage;		  		//记录上一级界面
	}
    gps_CurMenu = pMenu;                      	//当前菜单指针赋值
    APP_SetCountDownTime(pMenu->CountDown);		//记录倒计时时间	
    //显示倒计时
    PrintNum16uVariable(ADDR_COUNTDOWN_TIME,pMenu->CountDown);
    PrintBackImage(gps_CurMenu->FrameID);     	//显示当前界面界面图片
      
    if(gps_CurMenu->function3 && (gps_CurMenu->function3 != (void (*))0xFFFFFFFF))
    {
        gps_CurMenu->function3();             //显示当前界面变量
    } 
}
/************************(C)COPYRIGHT 2020 杭州汇誉****END OF FILE****************************/

