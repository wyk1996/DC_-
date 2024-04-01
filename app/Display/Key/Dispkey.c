/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			:
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
#include <string.h>
#include "Dispkey.h"
#include "display.h"
#include "DispKeyFunction.h"
#include "DisplayMain.h"
#include "MenuDisp.h"
#include "DwinProtocol.h"
#include "Disinterface.h"
#include "FlashDataDeal.h"
#include "DispShowStatus.h"
#include "4GMain.h"
#include "controlmain.h"
#include "connectbms.h"
#include "charging_Interface.h"
#include "RTC_task.h"
#include "AdAdJust.h"
#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif //MIN


extern HandSet_Info HandSedInfo[GUN_MAX];		//手动模式信息
extern OS_EVENT  		*JustOSQ_pevent;
extern INT8U USBUPDATA;
/* Private define-----------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/********************************************汇誉屏幕*******************************************/
/***********************************************************************************************
* Function		: KeyEvent6
* Description	: 系统初始化0
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent0(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent6
* Description	: 系统初始化0
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent1(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent6
* Description	: 系统初始化0
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent2(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: HYKeyEvent3
* Description	: 充电主界面
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent3(_LCD_KEYVALUE *keyval)
{
	 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

    if ((CurMenu == NULL) || (pdisp_conrtol == NULL) || (pcur_menu == NULL) || pdisp_conrtol->ClickGunNextPage[GUN_A] == NULL \
		|| pdisp_conrtol->ClickGunNextPage[GUN_B] == NULL)
    {
        return;
    }
	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			if(GetMeterStatus(GUN_A) != _COM_NORMAL)
			{
				break;
			}
			keynum = 0;
			keynum1 = 0;
			pdisp_conrtol->CurUserGun = GUN_A;
				//刷卡结算界面
			if((pdisp_conrtol->ClickGunNextPage[GUN_A] == &HYMenu33) || (pdisp_conrtol->ClickGunNextPage[GUN_A] == &HYMenu32))
			{
				DisplayCommonMenu(pdisp_conrtol->ClickGunNextPage[GUN_A],NULL);     //跳转到刷卡结算界面
			}
			//判断是否为故障
			else if(APP_GetErrState(GUN_A) == 0x04)  //有故障，跳转到故障显示界面
			{
				//DisplayCommonMenu(&HYMenu15,NULL);
				NOP();  //周期性任务会显示故障
			}
		    //判断是否连接充电枪
			else if(GetGunState(GUN_A) == GUN_DISCONNECTED)	//枪未连接
			{
				DisplayCommonMenu(&HYMenu14,NULL);     //提示枪未连接
			}
			
			else
			{
				
				DisplayCommonMenu(pdisp_conrtol->ClickGunNextPage[GUN_A],pcur_menu);
			
			}
            break;
        case LCD_KEY5: //B枪按键
			if(GetMeterStatus(GUN_B) != _COM_NORMAL)
			{
				break;
			}
			keynum = 0;
			keynum1 = 0;
 			pdisp_conrtol->CurUserGun = GUN_B;
			//判断是否为故障
			//判断是否为故障
			//刷卡结算界面
			if((pdisp_conrtol->ClickGunNextPage[GUN_B] == &HYMenu33) || (pdisp_conrtol->ClickGunNextPage[GUN_B] == &HYMenu32))
			{
				DisplayCommonMenu(pdisp_conrtol->ClickGunNextPage[GUN_B],NULL);     //跳转到刷卡结算界面
			}
			else if(APP_GetErrState(GUN_B) == 0x04)  			//有故障，跳转到故障显示界面
			{
				//DisplayCommonMenu(&HYMenu15,NULL);
				NOP();		//周期性任务会显示故障
			}
			else if(GetGunState(GUN_B) == GUN_DISCONNECTED)	//枪未连接
			{
				DisplayCommonMenu(&HYMenu14,NULL);     //提示枪未连接
			}
			
			else
			{
				
				DisplayCommonMenu(pdisp_conrtol->ClickGunNextPage[GUN_B],pcur_menu);
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY7: //点击信息查询 刷卡密码验证成功后进入设备信息界面
			keynum = 0;
			keynum1 = 0;
			//CleanSomePlace(ADDR_MENU14_CODE,3);  //清空密码
		
		//任意构造一个非03界面的，主要区别刷卡界面时查询还是启动
		#if(CARD_USER_CODE == 1)
        	DisplayCommonMenu(&HYMenu21,&HYMenu16);
		#else
			DisplayCommonMenu(&HYMenu22,&HYMenu16);   
		#endif
            break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//			USBUPDATA = 1;
			keynum = 0;
			if(++keynum1 >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			if(++keynum >= 3)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent6
* Description	: 费率查询
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent4(_LCD_KEYVALUE *keyval)
{

	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent6
* Description	: 系统初始化0
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent5(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent6
* Description	: 系统初始化6
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent6(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent7
* Description	: 无充电记录
* Input			: _LCD_KEYVALUE *keyval 按键值指针
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日
***********************************************************************************************/
void HYKeyEvent7(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {

		case LCD_KEY1:				//主界面
			APP_ClearRecodeInfo();		//清除记录信息
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//上一页返回账户查询界面		
			APP_ClearRecodeInfo();		//清除记录信息
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
		
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent8
* Description	: 记录查询1
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent8(_LCD_KEYVALUE *keyval)
{
	
	ST_Menu * ppage;
	switch(*keyval)
    {
		case LCD_KEY1:				//主界面
				if(SYSSet.NetState == DISP_NET)
				{
					ppage =  Disp_NetPageDispos();
					DisplayCommonMenu(ppage,NULL);
				}
				else
					DisplayCommonMenu(&HYMenu3,NULL);
			
				break;
		case LCD_KEY2:				//上一页返回账户查询界面		
		//根据交易记录个数跳转到相应的界面，目前直接跳转到账户信息界面2
			//APP_SelectUpChargeRecode();	//查询上一条记录
			//DisplayCommonMenu(&gsMenu60,NULL);  
			APP_SelectUpNChargeRecode(1);
			break;
		case LCD_KEY3:				//下一页返回账户查询界面	
		//根据交易记录个数跳转到相应的界面，目前直接跳转到账户信息界面2
			//APP_SelectNextChargeRecode();	//查询下一条记录
			APP_SelectNextNChargeRecode(1);  
			break;
		case LCD_KEY4:
			APP_SelectUpNChargeRecode(10);  
			break;
		case LCD_KEY5:
			APP_SelectNextNChargeRecode(10);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent9
* Description	: 记录查询2
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent9(_LCD_KEYVALUE *keyval)
{
	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
				{
					ppage =  Disp_NetPageDispos();
					DisplayCommonMenu(ppage,NULL);
				}
				else
					DisplayCommonMenu(&HYMenu3,NULL);

			break;
		case LCD_KEY2:				//上一页返回账户查询界面		
			APP_SelectUpNChargeRecode(1);  
			break;
		case LCD_KEY4:				//上10条
			APP_SelectUpNChargeRecode(10);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent10
* Description	: 网络协议选择
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent10(_LCD_KEYVALUE *keyval)
{	
	_FLASH_OPERATION  FlashOper;
	ST_Menu * ppage;
	switch(*keyval)
    {
		case LCD_KEY1:				//点击确定返回主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:		//上一页
			DisplayCommonMenu(&HYMenu47,NULL);
			break;
		case LCD_KEY4:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 0; 
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY5:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 1;
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY6:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 2;
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY7:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 3;
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY8:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 4; 
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY9:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 5;
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY10:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 6;
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY11:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 8;
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
		
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY12:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 7; 
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
		
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY23:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 9; 
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
		
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;
		case LCD_KEY24:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 10; 
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
		
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;	
			
	  case LCD_KEY25:
			SYSSet.NetNum = 1;
			SYSSet.NetYXSelct = 11; 
			memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
			SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
			memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
			snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
		
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read net set err");
			}
			Display_ShowNet();
			break;	
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent11
* Description	: 输入充电时间界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent11(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent12
* Description	: 未能找到记录
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent12(_LCD_KEYVALUE *keyval)
{
	ST_Menu * ppage;
	switch(*keyval)
    {
		case LCD_KEY1:				//点击确定返回主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:		//上一页
			DisplayCommonMenu(&HYMenu47,NULL);
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent13
* Description	: 输入充电金额界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent13(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent14
* Description	: 提示充电枪未连接
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent14(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//上一页返回界面
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent15
* Description	: 故障信息界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent15(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//上一页返回界面
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent16
* Description	: 充电方式选择界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent16(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//上一页返回界面
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		case LCD_KEY4:				//密码输入界面
		#if(CARD_USER_CODE == 1)
			DisplayCommonMenu(&HYMenu21,&HYMenu16);	
		#else
			DisplayCommonMenu(&HYMenu22,&HYMenu16);	
		#endif
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent17
* Description	: 刷卡界面3
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent17(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent18
* Description	: 刷卡界面4
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent18(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent19
* Description	: 刷卡界面5
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent19(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent20
* Description	: 刷卡界面6
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent20(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定返回主界面
			DisplayCommonMenu(&HYMenu3,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent21
* Description	: 电卡密码输入错误界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent21(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
//		 	CleanSomePlace(ADDR_MENU14_CODE,3);  //清空密码
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		 case LCD_KEY2:							//点击上一页跳转到充电方式选择界面/或账户查询界面
		 										//进入刷卡界面必须传入上一级界面
//		 	CleanSomePlace(ADDR_MENU14_CODE,3);	//清空密码
		 	if(HYMenu21.Menu_PrePage != NULL) 	   
	 		{
	 			DisplayCommonMenu(HYMenu21.Menu_PrePage,NULL);	//切换到上一级界面	
	 		}
			else
			{			
				//DisplayCommonMenu(&HYMenu16,NULL);		//充电方式选择界面
				//直接跳转到刷卡界面，不需要充电方式选择
				#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu21,&HYMenu3);	
				#else
					DisplayCommonMenu(&HYMenu22,&HYMenu3);	
				#endif
			}
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent22
* Description	: 刷卡界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent22(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			if(SYSSet.NetState == DISP_NET)
			{
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		 case LCD_KEY2:							//点击上一页到转到进入刷卡界面的上一页
	 			//切换到上一级界面	
				if(SYSSet.NetState == DISP_NET)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);	//
				}else
				{
				#if(CARD_USER_CODE == 1)
					DisplayCommonMenu(&HYMenu3,NULL);	//提示无法识别此卡 
				#else
					DisplayCommonMenu(&HYMenu3,NULL);	//提示无法识别此卡 
				#endif
				}
			break;
		 case LCD_KEY4:		//Vin启动,临时在这里直接发
			 	if(SYSSet.NetState == DISP_NET)
				{
					if(pdisp_conrtol->NetGunState[pdisp_conrtol->CurUserGun] != GUN_IDLE)
					{
						DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
						break;
					}
					if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_JG) || (SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
					{
						Net_StartCharge(pdisp_conrtol->CurUserGun,MODE_VIN);
						Set_VIN_Success(pdisp_conrtol->CurUserGun,VIN_IDLE);
						_4G_SetStartType(pdisp_conrtol->CurUserGun,_4G_APP_VIN);			//设置为VIN启动
						DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					}
				}
				break;	
		case LCD_KEY5:		//Vin启动,临时在这里直接发
			 	if(SYSSet.NetState == DISP_NET)
				{
					if(pdisp_conrtol->NetGunState[pdisp_conrtol->CurUserGun] != GUN_IDLE)
					{
						DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
						break;
					}
					if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) ||(SYSSet.NetYXSelct == XY_HY)  || (SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4))
					{
						Net_StartCharge(pdisp_conrtol->CurUserGun,MODE_VIN);
						Set_VIN_Success(pdisp_conrtol->CurUserGun,VIN_IDLE);
						DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					}
				}
//				 #include "4GMain.h"
//				OS_EVENT* pevent = APP_Get4GMainEvent();
//				static _BSP_MESSAGE SendMsg[GUN_MAX];
//		 
//				SendMsg[pdisp_conrtol->CurUserGun].MsgID = BSP_MSGID_DISP;
//				SendMsg[pdisp_conrtol->CurUserGun].DivNum = APP_VIN_INFO;
//				SendMsg[pdisp_conrtol->CurUserGun].DataLen =pdisp_conrtol->CurUserGun;
//				OSQPost(pevent, &SendMsg[pdisp_conrtol->CurUserGun]);
//				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);	//返回上一个界面
				break;	
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent23
* Description	: 此卡已被锁提示界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent23(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			 if(SYSSet.NetState == DISP_NET)
			{
				DisplayCommonMenu(&HYMenu73,NULL);
			}else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			break;
		 case LCD_KEY2:							//点击上一页跳转到充电方式选择界面/或账户查询界面
			KeyControl(3);                    //取消按键   键控
			OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
			 if(SYSSet.NetState == DISP_NET)
			 {
				DisplayCommonMenu(HYMenu23.Menu_PrePage,NULL);
			 }else
			 {
				 DisplayCommonMenu(&HYMenu3,NULL);
			 }
		 	break;
		
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent24
* Description	: 密码错误界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent24(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		
		 case LCD_KEY2:							
	 			DisplayCommonMenu(HYMenu24.Menu_PrePage,NULL);	//返回输入密码界面
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent25
* Description	: 读卡超时提示界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent25(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		 case LCD_KEY2:							//点击上一页到转到进入刷卡界面的上一页
			DisplayCommonMenu(HYMenu25.Menu_PrePage,NULL);	//返回输入密码界面

			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 非发行方卡提示界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent26(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		 case LCD_KEY2:							//点击上一页到转到进入刷卡界面的上一页
		 
			DisplayCommonMenu(HYMenu26.Menu_PrePage,NULL);

			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent14
* Description	: 无法识别磁卡
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent27(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		 case LCD_KEY2:							//点击上一页到转到进入刷卡界面的上一页
			DisplayCommonMenu(HYMenu27.Menu_PrePage,NULL);
	 
			

			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent15
* Description	: 充电枪A启动中界面1
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent28(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		 case LCD_KEY2:							//点击上一页到转到进入刷卡界面的上一页
				#if(CARD_USER_CODE == 1)
					DisplayCommonMenu(HYMenu28.Menu_PrePage,NULL);	//提示无法识别此卡 
				#else
					DisplayCommonMenu(HYMenu28.Menu_PrePage,NULL);	//提示无法识别此卡 
				#endif
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent16
* Description	: 充电枪A启动中界面2
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent29(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		 case LCD_KEY2:							//点击上一页到转到进入刷卡界面的上一页
				#if(CARD_USER_CODE == 1)
					DisplayCommonMenu(HYMenu21.Menu_PrePage,NULL);	//提示无法识别此卡 
				#else
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);	//提示无法识别此卡 
				#endif
	
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent17
* Description	: 充电枪A启动中界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent30(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent18
* Description	: 充电枪B启动中界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent31(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent19
* Description	: 启动失败刷卡界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent32(_LCD_KEYVALUE *keyval)
{
		_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

//参数保护
	if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->ClickGunNextPage[GUN_A] == NULL) \
		|| (pdisp_conrtol->ClickGunNextPage[GUN_B] == NULL))
    {
        return;
    }
	switch(*keyval)
    {
		 case LCD_KEY1:				//点击<主界面>
			//记录下一级点击  A枪\卡号不一致\取消刷卡 下一级需要进入的界面
		 	pdisp_conrtol->ClickGunNextPage[pdisp_conrtol->CurUserGun] = &HYMenu32;
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		 case LCD_KEY2:				
			pdisp_conrtol->ClickGunNextPage[pdisp_conrtol->CurUserGun]  = &HYMenu32;
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent20
* Description	: 充电枪A启动中界面6
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent33(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

//参数保护
	if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->CurUserGun >= GUN_MAX) ||\
		(pdisp_conrtol->ClickGunNextPage[pdisp_conrtol->CurUserGun] == NULL))
    {
        return;
    }
	switch(*keyval)
    {
		case LCD_KEY2:				//确定进入上一次记录的界面
			DisplayCommonMenu(pdisp_conrtol->ClickGunNextPage[pdisp_conrtol->CurUserGun],NULL);
			break;
		case LCD_KEY1:				//进入主界面
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent21
* Description	: 卡号不一致
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent34(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
    {
		case LCD_KEY2:				//确定进入上一次记录的界面
			DisplayCommonMenu(HYMenu34.Menu_PrePage,NULL);
			break;
		default:
            break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent22
* Description	: 停止中
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent35(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent23
* Description	: A枪充电界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent36(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(SYSSet.NetState == DISP_NET)
	{
		switch(*keyval)
		{
			 case LCD_KEY1:				//点击<主界面>
				DisplayCommonMenu(HYMenu36.Menu_PrePage,NULL);
				break;
			case LCD_KEY4:				//车辆车辆信息，进A枪车辆信息界面
				//DisplayCommonMenu(&HYMenu46,NULL);
				DisplayCommonMenu(&HYMenu38,HYMenu36.Menu_PrePage);
				break;
			case LCD_KEY2:
				if((_4G_GetStartType(GUN_A) == _4G_APP_VIN) || (_4G_GetStartType(GUN_A) == _4G_APP_BCVIN)) //VIN直接结束
				{
					Net_StopCharge(GUN_A);
					NB_WriterReason(GUN_A,"E58",3);
					DisplayCommonMenu(HYMenu36.Menu_PrePage,NULL);
				}
				break;
			default:
				break;
				
		}
	}
	else
	{
	//参数保护
		if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->ClickGunNextPage[GUN_A] == NULL) \
			|| (pdisp_conrtol->ClickGunNextPage[GUN_B] == NULL) || (pdisp_conrtol->CurUserGun != GUN_A) )
		{
			return;
		}
		switch(*keyval)
		{
			 case LCD_KEY1:				//点击<主界面>
				//记录下一级点击  A枪\卡号不一致\取消刷卡 下一级需要进入的界面
				pdisp_conrtol->ClickGunNextPage[ GUN_A] = &HYMenu36;
				DisplayCommonMenu(&HYMenu3,NULL);
				break;
			 case LCD_KEY2:				//停止充电,进入充电停止中请勿把枪，发送停止命令
	//			DisplayCommonMenu(&gsMenu56,NULL);
	//			SendStopChargeMsg(GUN_A);
			 //进入刷卡结算界面
				if(SYSSet.NetState == DISP_NET)
				{
					if((_4G_GetStartType(GUN_A) == _4G_APP_VIN) || (_4G_GetStartType(GUN_A) == _4G_APP_BCVIN)) //VIN直接结束
					{
						NB_WriterReason(GUN_A,"E57",3);
						Net_StopCharge(GUN_A);
						DisplayCommonMenu(HYMenu36.Menu_PrePage,NULL);
					}
				}
				else
				{
					pdisp_conrtol->ClickGunNextPage[ GUN_A] = &HYMenu36;
					DisplayCommonMenu(&HYMenu33,NULL);
				}
				break;
			case LCD_KEY4:				//车辆车辆信息，进A枪车辆信息界面
				//DisplayCommonMenu(&HYMenu46,NULL);
				DisplayCommonMenu(&HYMenu38,NULL);
				break;
			default:
				break;
		}
	}
}

/***********************************************************************************************
* Function		: KeyEvent24
* Description	: B枪充电界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent37(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

	if(SYSSet.NetState == DISP_NET)
	{
		switch(*keyval)
		{
			 case LCD_KEY1: 			//点击<主界面>
				DisplayCommonMenu(HYMenu37.Menu_PrePage,NULL);
				break;
			case LCD_KEY4:				//车辆车辆信息，进A枪车辆信息界面
				//DisplayCommonMenu(&HYMenu46,NULL);
				DisplayCommonMenu(&HYMenu39,HYMenu37.Menu_PrePage);
				break;
			case LCD_KEY2:
				if((_4G_GetStartType(GUN_B) == _4G_APP_VIN) || (_4G_GetStartType(GUN_B) == _4G_APP_BCVIN)) //VIN直接结束
				{
					NB_WriterReason(GUN_B,"E56",3);
					Net_StopCharge(GUN_B);
					DisplayCommonMenu(HYMenu37.Menu_PrePage,NULL);
				}
				break;
			default:
				break;
				
		}
	}
	else
	{

	//参数保护
		
		if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->ClickGunNextPage[GUN_A] == NULL) \
			|| (pdisp_conrtol->ClickGunNextPage[GUN_B] == NULL) || (pdisp_conrtol->CurUserGun != GUN_B) )
		{
			return;
		}
		switch(*keyval)
		{
			 case LCD_KEY1:				//点击<主界面>
				//记录下一级点击  A枪\卡号不一致\取消刷卡 下一级需要进入的界面
				pdisp_conrtol->ClickGunNextPage[ GUN_B] = &HYMenu37;
				DisplayCommonMenu(&HYMenu3,NULL);
				break;
			 case LCD_KEY2:				//停止充电,进入充电停止中请勿把枪，发送停止命令
	//			DisplayCommonMenu(&gsMenu56,NULL);
	//			SendStopChargeMsg(GUN_A);
			 //进入刷卡结算界面
				if(SYSSet.NetState == DISP_NET)
				{
					if((_4G_GetStartType(GUN_B) == _4G_APP_VIN) || (_4G_GetStartType(GUN_B) == _4G_APP_BCVIN))  //VIN直接结束
					{
						NB_WriterReason(GUN_B,"E55",3);
						Net_StopCharge(GUN_B);
						DisplayCommonMenu(HYMenu37.Menu_PrePage,NULL);
					}
				}
				else
				{
					pdisp_conrtol->ClickGunNextPage[ GUN_B] = &HYMenu37;
					DisplayCommonMenu(&HYMenu33,NULL);
				}
				break;
			case LCD_KEY4:				//车辆车辆信息，进B枪车辆信息界面
				//DisplayCommonMenu(&HYMenu46,NULL);
				DisplayCommonMenu(&HYMenu39,NULL);
				break;
			default:
				break;
		}
	}
}

/***********************************************************************************************
* Function		: KeyEvent25
* Description	: A枪信息
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent38(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

	if(SYSSet.NetState == DISP_NET)
	{
		switch(*keyval)
		{
			 case LCD_KEY1:				//点击<主界面>
				DisplayCommonMenu(HYMenu38.Menu_PrePage,NULL);
				break;
			 case LCD_KEY2:				//点击上一页，进入A枪充电界面
				DisplayCommonMenu(&HYMenu36,NULL);
				break;
			default:
				break;
		}
	}
	else
	{
		if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->ClickGunNextPage[GUN_A] == NULL) \
			|| (pdisp_conrtol->ClickGunNextPage[GUN_B] == NULL) || (pdisp_conrtol->CurUserGun != GUN_A) )
		{
			return;
		}
		switch(*keyval)
		{
			 case LCD_KEY1:				//点击<主界面>
				//记录下一级点击  A枪\卡号不一致\取消刷卡 下一级需要进入的界面
				pdisp_conrtol->ClickGunNextPage[ GUN_A] = &HYMenu36;
				DisplayCommonMenu(&HYMenu3,NULL);
				break;
			 case LCD_KEY2:				//点击上一页，进入A枪充电界面
				DisplayCommonMenu(&HYMenu36,NULL);
				break;
			default:
				break;
		}
	}
}

/***********************************************************************************************
* Function		: HYKeyEvent39
* Description	: B枪信息
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent39(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(SYSSet.NetState == DISP_NET)
	{
			switch(*keyval)
			{
				 case LCD_KEY1: 			//点击<主界面>
					DisplayCommonMenu(HYMenu39.Menu_PrePage,NULL);
					break;
				 case LCD_KEY2: 			//点击上一页，进入A枪充电界面
					DisplayCommonMenu(&HYMenu37,NULL);
					break;
				default:
					break;
			}
		}
	else
	{

	//参数保护
		if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->ClickGunNextPage[GUN_A] == NULL) \
			|| (pdisp_conrtol->ClickGunNextPage[GUN_B] == NULL) || (pdisp_conrtol->CurUserGun != GUN_B) )
		{
			return;
		}
		switch(*keyval)
		{
			 case LCD_KEY1:				//点击<主界面>
				pdisp_conrtol->ClickGunNextPage[ GUN_B] = &HYMenu37;
				DisplayCommonMenu(&HYMenu3,NULL);
				break;
			 case LCD_KEY2:				//点击上一页，进入A枪充电界面
				DisplayCommonMenu(&HYMenu37,NULL);
				break;
			default:
				break;
		}
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A枪结算界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent40(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

//参数保护
	if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->ClickGunNextPage[GUN_A] == NULL) || (pdisp_conrtol->CurUserGun != GUN_A) )
    {
        return;
    }
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定进入主界面            只要刷卡成功后都要进入此页面，故在这里清除A枪下一级需要进入的页面
			pdisp_conrtol->ClickGunNextPage[GUN_A] = &HYMenu22;
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		
		default:
            break;
	}	
}

/***********************************************************************************************
* Function		: KeyEvent41
* Description	: B枪结算界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent41(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

//参数保护
	if (  (pdisp_conrtol == NULL)  || (pdisp_conrtol->ClickGunNextPage[GUN_B] == NULL) || (pdisp_conrtol->CurUserGun != GUN_B) )
    {
        return;
    }
	switch(*keyval)
    {
		case LCD_KEY2:				//点击确定进入主界面            只要刷卡成功后都要进入此页面，故在这里清除A枪下一级需要进入的页面
			pdisp_conrtol->ClickGunNextPage[GUN_B] = &HYMenu22;
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		
		default:
            break;
	}	
}

/***********************************************************************************************
* Function		: KeyEvent42
* Description	: 设备信息
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent42(_LCD_KEYVALUE *keyval)
{
	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//上一页返回主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
		
			break;
		case LCD_KEY3:				//下一页
			DisplayCommonMenu(&HYMenu43,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: 提示连接充电枪界面2
* Description	: 设备信息
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent43(_LCD_KEYVALUE *keyval)
{
	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//上一页返回主界面
			DisplayCommonMenu(&HYMenu42,NULL);  
			break;
		case LCD_KEY3:				//下一页
			DisplayCommonMenu(&HYMenu44,NULL);  
			break;
		default:
		    break;
	}
}


/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 设备信息
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent44(_LCD_KEYVALUE *keyval)
{
	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			break;
		case LCD_KEY2:				//上一页返回主界面
			DisplayCommonMenu(&HYMenu43,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 设备信息
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent45(_LCD_KEYVALUE *keyval)
{	
	
	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//上一页返回主界面
			DisplayCommonMenu(&HYMenu44,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 枪A车辆信息界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent46(_LCD_KEYVALUE *keyval)
{
	_BSPRTC_TIME CurRTC;
	switch(*keyval)
    {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY4:				//费率设置
			Dis_ShowStatus(DIS_ADD(48,0x10),SHOW_NULL,RED); 
			DisplayCommonMenu(&HYMenu48,&HYMenu46); 		
			Display_PricSet(0);					 //显示费率信息
			break;
		case LCD_KEY5:				//时间设置
			//获取当前时间
			GetCurTime(&CurRTC);           			//获取系统RTC
			PrintTime(INPUT_MENU54_CFG,CurRTC);     
			DisplayCommonMenu(&HYMenu54,&HYMenu46); 
			break;
		case LCD_KEY6:				//密码修改
			//DisplayCommonMenu(&HYMenu55,NULL);  
			break;
		default:
		    break;
	}
}
/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 管理员界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent47(_LCD_KEYVALUE *keyval)
{
	static INT8U code[6] = {0};
			switch(*keyval)
	   {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY4:				//手动模式
			DisplayCommonMenu(&HYMenu49,&HYMenu47);  
			break;
		case LCD_KEY5:				//系统配置
			DisplayCommonMenu(&HYMenu52,&HYMenu47);  
			break;
		case LCD_KEY6:				//卡解锁
			//DisplayCommonMenu(&HYMenu55,&HYMenu47);  
			SendCardMsg(NETCARDCMD_UNLOCK, code,sizeof(code));
			break;
		case LCD_KEY7:				//费率设置
			Dis_ShowStatus(DIS_ADD(48,0x10),SHOW_NULL,RED);   
			DisplayCommonMenu(&HYMenu48,&HYMenu47);  		
			Display_PricSet(0);					 //显示费率信息
			break;
		case LCD_KEY8:				//远程升级
			USBUPDATA = 1;
			break;
		case LCD_KEY9:				//电压校准
			DisplayCommonMenu(&HYMenu56,NULL);  
			break;
		case LCD_KEY10:				//网络设置
			Display_ShowNet();
			DisplayCommonMenu(&HYMenu10,NULL);  
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 费率设置
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent48(_LCD_KEYVALUE *keyval)
{
		switch(*keyval)
	   {
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(HYMenu48.Menu_PrePage,NULL);  
			break;
		case LCD_KEY4:				//下发时间设置
			ReadDwinVariable(DIS_ADD(48,0),16); 
			break;
		
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 手动模式
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent49(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//系统配置选择界面
			DisplayCommonMenu(&HYMenu47,NULL);
			break;
		case LCD_KEY4:				//A手动模式
			DisplayCommonMenu(&HYMenu50,NULL);  
			break;
		case LCD_KEY5:				//B枪手动模式
			DisplayCommonMenu(&HYMenu51,NULL);  
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A枪手动充电界面 
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent50(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY2:				//手动模式界面
			DisplayCommonMenu(&HYMenu49,NULL);
			//SendStopChargeMsg(GUN_A);
			break;
		case LCD_KEY4:				//A枪手动充电停止 
			//下发停止命令；
			SendStopChargeMsg(GUN_A);
			break;
		case LCD_KEY5:				//B枪手动充电启动
			ReadDwinVariable(INPUT_MENU50_HANDA,2);     //读取手动模式电压电流
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: B枪手动充电界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent51(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY2:				//手动模式界面
			DisplayCommonMenu(&HYMenu49,NULL);
			//SendStopChargeMsg(GUN_B);
			break;
		case LCD_KEY4:				//A枪手动充电停止 
			//下发停止命令；
			SendStopChargeMsg(GUN_B);
			break;
		case LCD_KEY5:				//B枪手动充电启动
			ReadDwinVariable(INPUT_MENU51_HANDB,2);     //读取手动模式电压电流
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 系统配置1
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent52(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//系统配置选择界面
			DisplayCommonMenu(&HYMenu47,NULL);
			break;
		case LCD_KEY3:				//下一页
			DisplayCommonMenu(&HYMenu53,NULL);
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 系统配置2
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent53(_LCD_KEYVALUE *keyval)
{	
	_FLASH_OPERATION  FlashOper;
	INT32U RecodeNum = 0;
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu52,NULL);
			break;
		case LCD_KEY3:				//下一页
			DisplayCommonMenu(&HYMenu57,&HYMenu53);
			Display_ShowSet3();
			break;
		case LCD_KEY4:				//记录清零
			FlashOper.DataID = PARA_1200_ID;
			FlashOper.Len = PARA_1200_FLLEN;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			FlashOper.ptr = (INT8U *)&RecodeNum;
			APP_FlashOperation(&FlashOper);
		
			FlashOper.RWChoose = FLASH_ORDER_READ;
			if(APP_FlashOperation(&FlashOper) == TRUE)
			{
				APP_SetRecodeCurNum(RecodeNum);
			}
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 系统配置3
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent54(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(HYMenu54.Menu_PrePage,NULL);
			break;
		case LCD_KEY4:				//下发时间设置
			ReadDwinVariable(INPUT_MENU54_CFG,6);
			break;
		default:
			break;
	}
}
		
		
/*********************************************************************************************      	
* Function		: KeyEvent26
* Description	: 枪解锁
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent55(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu47,NULL);
			break;

		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	:校准界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent56(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				DisplayCommonMenu(&HYMenu73,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu47,NULL);
			break;
		
		case LCD_KEY4:				//A枪校准界面
			pdisp_conrtol->CurUserGun = GUN_A;
			DisplayCommonMenu(&HYMenu58,NULL);
			break;
		case LCD_KEY5:				//B枪校准界面
			pdisp_conrtol->CurUserGun = GUN_B;
			DisplayCommonMenu(&HYMenu58,NULL);
			break;

		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent57
* Description	: 系统设置
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent57(_LCD_KEYVALUE *keyval)
{
	_FLASH_OPERATION  FlashOper;
	_BSPRTC_TIME CurRTC;
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			DisplayCommonMenu(&HYMenu73,NULL);
			else
			DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu53,NULL);
			break;
		case LCD_KEY3:				//下一页
			//获取当前时间
			GetCurTime(&CurRTC);           			//获取系统RTC
			PrintTime(INPUT_MENU54_CFG,CurRTC);     
			DisplayCommonMenu(&HYMenu54,&HYMenu57);
			break;
		case LCD_KEY4:			//单机
			SYSSet.NetState = 0;  //单机
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY5:			//网络
			SYSSet.NetState = 1;  
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY6:			//常闭
			SYSSet.LockState = 0; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY7:			//常开
			SYSSet.LockState = 1;  
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY8:			//20kw永联恒功率模块
			SYSSet.CurModule = 0; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY9:			//20kw永联非恒功率模块
			SYSSet.CurModule = 1; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY10:			//盛弘
			SYSSet.CurModule = 2; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY11:			//15kw永联恒功率模块
			SYSSet.CurModule = 3; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY12:			//15kw永联非恒功率模块
			SYSSet.CurModule = 4; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		case LCD_KEY13:			//15kw永联非恒功率模块
			SYSSet.CurModule = 5; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
			
	case LCD_KEY14:			//40kw通合
			SYSSet.CurModule = 6; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
	case LCD_KEY15:			//30kw永联
			SYSSet.CurModule = 7; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
	case LCD_KEY16:			//20kw永联 1000V
			SYSSet.CurModule = 9; 
			FlashOper.DataID = PARA_CFGALL_ID;
			FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
			FlashOper.ptr = (INT8U *)&SYSSet;
			FlashOper.RWChoose = FLASH_ORDER_WRITE;
			if(APP_FlashOperation(&FlashOper) == FALSE)
			{
				printf("Read SYS set err");
			}
			Display_ShowSet3();
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	:电压校准  后级
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent58(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				DisplayCommonMenu(&HYMenu73,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			SendStopChargeMsg(pdisp_conrtol->CurUserGun);
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu56,NULL);
			SendStopChargeMsg(pdisp_conrtol->CurUserGun);
			break;
		
		case LCD_KEY3:				//
			DisplayCommonMenu(&HYMenu59,NULL);
			SendStopChargeMsg(pdisp_conrtol->CurUserGun);
			break;
		case LCD_KEY4:				//校准点1
			HandSedInfo[pdisp_conrtol->CurUserGun].outvolt = 3000;
			HandSedInfo[pdisp_conrtol->CurUserGun].outcurr = 50;
			SendStartChargeMsg(pdisp_conrtol->CurUserGun,MODE_MAN);
			break;
		case LCD_KEY5:				//校准点2
			HandSedInfo[pdisp_conrtol->CurUserGun].outvolt = 5000;
			HandSedInfo[pdisp_conrtol->CurUserGun].outcurr = 50;
			SendStartChargeMsg(pdisp_conrtol->CurUserGun,MODE_MAN);
			break;

		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 电流校准
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent59(_LCD_KEYVALUE *keyval)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	
	static INT32U data = 0;
	static _BSP_MESSAGE send_message;

	send_message.MsgID = BSP_MSGID_DISP;
	send_message.DivNum = JUST_SW_AFTER1;
	send_message.DataLen = sizeof(data);
	send_message.pData = (INT8U *)&data;
	send_message.GunNum = 	GUN_A;
	OSQPost(JustOSQ_pevent ,&send_message);
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				DisplayCommonMenu(&HYMenu73,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			SendStopChargeMsg(pdisp_conrtol->CurUserGun);
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu58,NULL);
			SendStopChargeMsg(pdisp_conrtol->CurUserGun);
			break;
		
		case LCD_KEY3:				//
			DisplayCommonMenu(&HYMenu60,NULL);
			SendStopChargeMsg(pdisp_conrtol->CurUserGun);
			break;
		case LCD_KEY4:				//校准点1
			HandSedInfo[pdisp_conrtol->CurUserGun].outvolt = 3000;
			HandSedInfo[pdisp_conrtol->CurUserGun].outcurr = 50;
			SendStartChargeMsg(pdisp_conrtol->CurUserGun,MODE_MAN);
			break;
		case LCD_KEY5:				//校准点2
			HandSedInfo[pdisp_conrtol->CurUserGun].outvolt = 5000;
			HandSedInfo[pdisp_conrtol->CurUserGun].outcurr = 50;
			SendStartChargeMsg(pdisp_conrtol->CurUserGun,MODE_MAN);
			break;
		case LCD_KEY6:				//T1 0  A
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T1_0_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_A;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
		case LCD_KEY7:				//T1 100  A
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T1_100_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_A;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
		case LCD_KEY8:				//T2 0  A
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T2_0_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_A;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
		case LCD_KEY9:				//T2 100  A
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T2_100_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_A;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
	#if(USER_GUN != USER_SINGLE_GUN)
		case LCD_KEY10:				//T1 0  B
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T1_0_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_B;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
		case LCD_KEY11:				//T1 100  B
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T1_100_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_B;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
		case LCD_KEY12:				//T2 0  B
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T2_0_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_B;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
		case LCD_KEY13:				//T2 100  B
			send_message.MsgID = BSP_MSGID_DISP;
			send_message.DivNum = JUST_T2_100_GUNA;
			send_message.DataLen = sizeof(data);
			send_message.pData = (INT8U *)&data;
			send_message.GunNum = 	GUN_B;
			OSQPost(JustOSQ_pevent ,&send_message);
			break;
		#endif
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 枪校准
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent60(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				DisplayCommonMenu(&HYMenu73,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu59,NULL);
			break;

		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 输入查询账户密码界面
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent61(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 充电模式切换
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent62(_LCD_KEYVALUE *keyval)
{
	switch(*keyval)
	{
		case LCD_KEY1:				//主界面
			if(SYSSet.NetState == DISP_NET)
			{
				DisplayCommonMenu(&HYMenu73,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			
			break;
		case LCD_KEY2:				//上一页
			DisplayCommonMenu(&HYMenu47,NULL);
			break;
		case LCD_KEY4:				//均充
			
			break;
		case LCD_KEY5:				//轮充
		
			break;
		case LCD_KEY6:				//自动分配
		
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 账户信息界面1
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent63(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 账户信息界面2
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent64(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 提示卡号不一致
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent65(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: 提示充电停止中请勿把枪1
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent66(_LCD_KEYVALUE *keyval)
{
	;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A、B枪都在待机界面，需要显示2把枪的二维码
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent73(_LCD_KEYVALUE *keyval)
{
		 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

  	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
    {
        return;
    }
	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY) ||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)|| (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu73);  //跳转到网络版本的刷卡界面
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY) ||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)|| (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu73);  //跳转到网络版本的刷卡界面
				}
			}
			break;
		 case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
			keynum = 0;
//			USBUPDATA = 1;
			if(++keynum1 >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			//if(++keynum >= 5)
			if(++keynum >= 3)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A枪充电，B枪待机
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent74(_LCD_KEYVALUE *keyval)
{
	const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

    if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
    {
        return;
    }
	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG) ||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu74);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu74);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu74);
				}
			}
            break;
        case LCD_KEY5: //B枪按键
        	keynum1 = 0;
			keynum = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu74);  //跳转到网络版本的刷卡界面
				}
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//				USBUPDATA = 1;
			keynum = 0;
			if(++keynum1 >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道

			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A枪待机、B枪充电
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent75(_LCD_KEYVALUE *keyval)
{
	const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

   	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
    {
        return;
    }
	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu75);  //跳转到网络版本的刷卡界面
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu75);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu75);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu75);
				}
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//				USBUPDATA = 1;
			keynum = 0;
			if(++keynum1 >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A、B枪都在充电
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent76(_LCD_KEYVALUE *keyval)
{
		 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
	{
		return;
	}

	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu76);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu76);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu76);
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu76);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu76);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu76);
				}
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//				USBUPDATA = 1;
			keynum = 0;
			if(++keynum1  >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A、B枪都在充电
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent77(_LCD_KEYVALUE *keyval)
{
		 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
	{
		return;
	}

	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu77);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu77);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu77);
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu77);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu77);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu77);
				}
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//				USBUPDATA = 1;
			keynum = 0;
			if(++keynum1  >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A、B枪都在充电
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent78(_LCD_KEYVALUE *keyval)
{
		 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
	{
		return;
	}

	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu78);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu78);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu78);
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu78);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu78);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu78);
				}
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//				USBUPDATA = 1;
			keynum = 0;
			if(++keynum1  >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A、B枪都在充电
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent79(_LCD_KEYVALUE *keyval)
{
		 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
	{
		return;
	}

	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu79);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu79);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu79);
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu79);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu79);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu79);
				}
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
			keynum = 0;
			if(++keynum1  >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
//				USBUPDATA = 1;
			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A、B枪都在充电
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent80(_LCD_KEYVALUE *keyval)
{
		 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
	{
		return;
	}

	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu80);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu80);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu80);
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP)|| (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu80);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu80);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu80);
				}
			}
            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
//			DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//				USBUPDATA = 1;
			keynum = 0;
			if(++keynum1  >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: KeyEvent26
* Description	: A、B枪都在充电
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent81(_LCD_KEYVALUE *keyval)
{
		 const ST_Menu* CurMenu = GetCurMenu();
	INT32U recodenum = 0;
	static INT8U keynum = 0,keynum1 = 0;;  //隐藏按钮点击次数，必须连续三次点击才可进入  
	static INT32U lasttime = 0,curtime = 0;	
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	ST_Menu* pcur_menu = GetCurMenu();

	if ((CurMenu == NULL) || (pdisp_conrtol == NULL) )
	{
		return;
	}

	
	curtime = OSTimeGet();
	if((curtime - lasttime) > SYS_DELAY_500ms)
	{
		keynum = 0;
		keynum1 = 0;
	}
	lasttime = curtime;
    switch(*keyval)
    {
		case LCD_KEY4: //A枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
			//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE )&& (GetGunState(GUN_A) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_A;
					DisplayCommonMenu(&HYMenu22,&HYMenu81);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu81);
				}
			}
			else
			{
				if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu36,&HYMenu81);
				}
			}
            break;
        case LCD_KEY5: //B枪按键
			keynum = 0;
			keynum1 = 0;
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_TT)||   (SYSSet.NetYXSelct == XY_YL2) || (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
			{
				//枪连接而且空闲
				if((pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE )&& (GetGunState(GUN_B) == GUN_CONNECTED)) 
				{
					pdisp_conrtol->CurUserGun = GUN_B;
					DisplayCommonMenu(&HYMenu22,&HYMenu81);  //跳转到网络版本的刷卡界面
				}
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu81);
				}
			}
			else
			{		
				if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
				{
					DisplayCommonMenu(&HYMenu37,&HYMenu81);
				}
			}

            break;
        case LCD_KEY6: //点击账户查询进入账户选择页面
				keynum = 0;
				keynum1 = 0;
				APP_ClearRecodeInfo();		//清除记录信息
				recodenum = APP_GetRecodeCurNum();
				if(recodenum == 0)
				{
					DisplayCommonMenu(&HYMenu7,NULL);  //无充电记录
				}else{
					APP_SelectCurChargeRecode();		//第一次进入显示
//					DisplayCommonMenu(&HYMenu8,NULL);  //进入记录查询界面
				}
			break;
        case LCD_KEY8: // 点击费率查询
			keynum = 0;
			keynum1 = 0;
       		//DisplayCommonMenu(&gsMenu69,NULL);
	//		DisplayCommonMenu(&HYMenu4,NULL);
			Display_PricSet(0);					 //显示费率信息
            break;
		case LCD_KEY9: //设备信息
			keynum = 0;
			keynum1 = 0;
       		DisplayCommonMenu(&HYMenu42,NULL);
            break;
		case LCD_KEY10: //用户配置界面
//				USBUPDATA = 1;
			keynum = 0;
			if(++keynum1  >= 5)
			{
				keynum1 = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu4);   //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY11: //系统配置界面，隐藏，只有汇誉人员知道
			keynum1 = 0;
			if(++keynum >= 5)
			{
				keynum = 0;
				DisplayCommonMenu(&HYMenu23,&HYMenu3);				  //上一个页面主要是为了区别用户配置还是系统配置
				OSTimeDly(SYS_DELAY_250ms);                            //延时必须有，否则键控无效
				KeyControl(HYMenu23.FrameID);                    //键控
			}
            break;
		case LCD_KEY12: //A枪解锁
			if(APP_GetWorkState(GUN_A) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_A);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_A);
			}
            break;
		case LCD_KEY13: //B枪解锁
			if(APP_GetWorkState(GUN_B) != WORK_CHARGE)
			{
				BSP_OpenLOCK(GUN_B);
				OSTimeDly(SYS_DELAY_2s);
				BSP_CloseLOCK(GUN_B);
			}
            break;
        default:
            break;
    }
    return;
}

/***********************************************************************************************
* Function		: HYKeyEvent82
* Description	: 费率详情，没有下一页
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent82(_LCD_KEYVALUE *keyval)
{
	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY1:				//点击确定返回主界面	
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:		
			Display_PricSet(1);
			break;
//		case LCD_KEY3:		
//			Display_PricSet(2);
//			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: HYKeyEvent82
* Description	: 费率详情，没有下一页
* Input			: _LCD_KEYVALUE *keyval
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void HYKeyEvent83(_LCD_KEYVALUE *keyval)
{
	ST_Menu * ppage;

	switch(*keyval)
    {
		case LCD_KEY1:				//点击确定返回主界面
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
				DisplayCommonMenu(&HYMenu3,NULL);
			
			break;
		case LCD_KEY2:		
			Display_PricSet(1);
			break;
		case LCD_KEY3:		
			Display_PricSet(2);
			break;
		default:
		    break;
	}
}

/***********************************************************************************************
* Function		: DealWithKey
* Description	:按键动作
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月16日
***********************************************************************************************/
void DealWithKey(_LCD_KEYVALUE *keyval)
{
    const ST_Menu* CurMenu = GetCurMenu();
    
    if (CurMenu || CurMenu->function2)
    {
        CurMenu->function2(keyval);
    }
}
/************************(C)COPYRIGHT 2018 杭州汇誉****END OF FILE****************************/

