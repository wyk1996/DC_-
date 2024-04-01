/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: MenuDisp.c
* Author			:
* Date First Issued	:
* Version			:
* Description		: 提供每个界面的菜单结构体，被DispKey.c调用
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013	        : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "DisplayMain.h"
#include "bsp_conf.h"
#include "DispKeyFunction.h"
#include "MenuDisp.h"
#include "bsp_uart.h"
#include "DataChangeInterface.h"
#include "DispShowStatus.h"
#include "FlashDataDeal.h"
#include "bsp_rtc.h"
#include "card_Interface.h"
#include "charging_Interface.h"
#include "RTC_task.h"
#include "4GMain.h"
#include "bsp_MPulse.h"
#include "controlmain.h"
#include "AdAdjust.h"
#include "PCUMain.h"
#include "connectbms.h"
#include "ADSamplemain.h"
#include "stm32f4xx_flash.h"
#include "version.h"
#include "app_conf.h"

/* Private define-----------------------------------------------------------------------------*/

#define DISP_SEND_BUF_LEN          (120u)
#define DISP_RECV_BUF_LEN          (120u)
#define DISPLAY_Q_LEN               80
#define DISP_TEMP_BUF_LEN          (120u)

#define MAX_PERIOD_SUM_TIME         (1440u)                   //总时长，分钟
/* Private typedef----------------------------------------------------------------------------*/
_DISP_UART_STATUS DispUartStatus = DISP_UART_IDLE;			//因显示任务和周期性检测任务需要写显示串口，故加忙状态

INT16U	ChargeRecodeCurNum;
typedef struct
{
	INT32U RecodeCurNum;			//交易记录条数
	INT32U CurReadRecodeNun;		//当前读到的交易记录的偏移条数（查询使用）
	INT32U NextReadRecodeNun;		//下一条读到的交易记录的偏移条数（查询使用）
	INT32U UpReadRecodeNun;			//上一条读到的交易记录的偏移条数（查询使用）
	INT16U CurNun;					//当前记录页码
	_CHARGE_RECODE CurRecode;	//存放当前记录
	_CHARGE_RECODE NextRecode;	//存放下一条记录
	_CHARGE_RECODE UpRecode;	//存放上一条记录
	INT32U CardNum;				//记录查询卡的卡号
} _RECODE_CONTROL;

typedef struct
{
	INT32U RecodeCurNum;			//故障/告警记录条数
	INT16U CurNun;					//当前记录页码
	_ERRWARN_RECODEINFO CurErrWarnRecod; //当前故障\告警存储
} _ERRWARN_RECODE_CONTROL;




extern OS_EVENT  		*JustOSQ_pevent; //
extern _ADC_JUST_INFO ADCJustInfo[GUN_MAX];   //AD校准信息
extern _VIN_CONTROL VinControl[GUN_MAX];

INT8U Billbuf[GUN_MAX][500] = {0};			//缓冲
INT8U FlashCardVinWLBuf[5000] = {0};		//卡Vin白名单chuli

OS_EVENT *DispMutex;                 //互斥锁，同一时刻只能有一个任务进行临界点访问
/***************************************汇誉屏幕************************************/
#define DIVINFO_SINGLE_SIZE 0x10
//与界面52排列统一
__packed typedef struct
{
	INT8U DivNum[16];		//桩编号
	INT8U ProjectNum[16];	//项目编号
	INT16U res;
	INT16U UseModule;    	//使用模块数量	BIN	1 Byte	分辨率：1个，0偏移量
	INT16U MacMaxVol;		//整机最高电压   分辨率：1V，0偏移量
	INT16U MacMinVol;		//整机最低电压	 分辨率：1V，0偏移量
	INT16U MaxOutCur;		//最大输出电流	 分辨率0.1A，0偏移量
	INT16U  UseGun;			//枪使用个数 1 Byte 	分辨率：1倍，0偏移量
	INT16U OutDcVol;		//输出过压保护阀值	BIN	2 Byte	分辨率：0.1V，0偏移量
	INT16U OutDcCur;		//输出过流保护阀值	BIN	2 Byte	分辨率：0.1A，0偏移量
	INT16U SOC;				//SOC阈值
} _SYS_MENU52_SET1;



//与界面42 43 44对应 实时设备信息显示，从避雷器故障开始
__packed typedef struct
{
	_SHOW_NUM WattMetereSignalErr;				//与电表通信状态
	_SHOW_NUM CCUSignalErr;						//与CCU信状态、
	_SHOW_NUM ChargGun_State;					//枪归位状态
	_SHOW_NUM CGunUpTemperature_Err;			//充电枪过温
	_SHOW_NUM DZLock_Err;						//电子锁
	_SHOW_NUM BatReverse_Err;					//电池反接
	_SHOW_NUM BMSCommunication_Err;				//与BMS通信状态
	INT8U cc1volt;							//CC1需要单独显示
	_SHOW_NUM JyCheck_Err;						//绝缘检测
	_SHOW_NUM DCOVP_Warn;            			//直流输出过压告警
	_SHOW_NUM DCUVP_Warn;            			//直流输出欠压告警
	_SHOW_NUM UPCUR_Warn;           			//直流输出过流告警
	_SHOW_NUM CarGuide_Err;						//充电中车辆导引
	_SHOW_NUM DCContactor_Err;					//直流母线输出接触器
} _GUN_STATUS;
__packed typedef struct
{
	_SHOW_NUM SurgeArrester_Err; 			//避雷器
	_SHOW_NUM AC_OVP;						//交流输入过压
	_SHOW_NUM AC_UVP;						//交流输入欠压
	_SHOW_NUM SysUptemp_Warn;				//充电机过温告警
	_SHOW_NUM Fan_Warn;						//风扇故障
	_SHOW_NUM ACCircuitBreaker_Err;			//交流断路器故障
	_SHOW_NUM Door_Err;						//门禁
	_SHOW_NUM Stop_Pluse;					//急停
	_SHOW_NUM ModuleACOVP_Warn;				//充电模块交流输入过压告警
	_SHOW_NUM ModuleACUVP_Warn;				//充电模块焦炉输入欠压告警
	_SHOW_NUM ModuleACPhase_Warn;			//充电模块输入缺相告警
	_SHOW_NUM ModuleDCShortCircuit_Err;		//充电模块输出短路故障
	_SHOW_NUM ModuleDCUPCUR_Warn;			//充电模块直流输出过流告警
	_SHOW_NUM ModuleDCOVP_Warn;   			//充电模块直流输出过压告警
	_SHOW_NUM ModuleDCUVP_Warn;				//充电模块直流输出欠压告警
	_GUN_STATUS GunStatus[GUN_MAX];
	INT16U GunVol[GUN_MAX];
} _DIV_STATUS;
_DIV_STATUS  DivStatue;


//与界面8/9账户信息对应
__packed typedef struct
{
	INT32U BeforeCardBalance;			//扣款后卡内余额 0.01
	INT8U BillingStatus[10];			//结算状态
	INT8U TransNum[16];      			//交易流水号
	INT32U TotPower;         			//总电量 	 0.01Kwh
	INT8U StopChargeReason[30];			//停止原因
	INT32U CardNum;        				//卡号
	INT8U ChargeType[30];        		//充电方式
	INT8U Gunnum[10];						//枪号
	INT32U TotMoney;         			//总金额   0.01元
	INT8U CarVin[18];         			//电动汽车唯一标识（车Vin码本身17位，因界面最小单位是2个字节所以写18）
	INT16U StartYear;					// 年
	INT16U StartMonth;					// 月
	INT16U StartDay;					// 日
	INT16U StartHour;					// 时
	INT16U StartMinute;					// 分
	INT16U StartSecond;					// 秒
	INT16U EndYear;						// 年
	INT16U EndMonth;					// 月
	INT16U EndDay;						// 日
	INT16U EndHour;						// 时
	INT16U EndMinute;					// 分
	INT16U EndSecond;					// 秒
	INT8U StopReason[20];				//内部人员自己使用
} _RECODE_MENU8_9_INFO;











/* Private macro------------------------------------------------------------------------------*/
static void *DisplayOSQ[DISPLAY_Q_LEN];
static OS_EVENT *DispTaskEvent;
static INT8U DispRecvSMem[DISPLAY_Q_LEN * sizeof(_BSP_MESSAGE)];
static INT8U DispSendBuf[DISP_SEND_BUF_LEN];
INT8U DispRecvBuf[DISP_RECV_BUF_LEN];
static INT8U DispTempBuf[DISP_TEMP_BUF_LEN];

_DISP_CONTROL DispControl;       				//显示任务控制结构体
_RECODE_CONTROL RecodeControl;					//交易控制
static _RECODE_MENU8_9_INFO RecodeMenu8_9Info;		//与界面8/9账户信息对应
//与界面显示保持一致
_CHARGE_INFO  ChargeInfo[GUN_MAX] = {(_CHARGE_TYPEDEF)0};		 		//充电信息（充电方式选择）
_CHARGEING_INFO ChargeingInfo[GUN_MAX] = {0};			//充电中显示的数据
_CARD_INFO CardInfo[GUN_MAX] = {0};					//充电中车辆信息
_END_CHARGE_INFO  EndChargeInfo[GUN_MAX] = {0};		//结束充电信息




_SYS_SET SYSSet;								//系统设置信息       		61个字节
_PRICE_SET PriceSet;							//电价设置
//云快充  当充电中修改费率，A枪的费率可能和B枪的费率不一样
_PRICE_SET A_PriceSet;							//电价设置
_PRICE_SET B_PriceSet;							//电价设置

HandSet_Info HandSedInfo[GUN_MAX] = {0};		//手动模式信息
_CHARGE_RECODE ChargeRecodeInfo[GUN_MAX];


_BSPRTC_TIME ReadDwRtc; //迪文屏幕读取的RTC

//_DIV_STATUS  DivStatue;									//界面65/66/67/68设备信息 从避雷器故障开始，A\B充电导引电压单独显示
/* Private variables--------------------------------------------------------------------------*/


/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


/*****************************************************************************
* Function     : NB_WriterReason
* Description  : 内部人员显示故障原因，方便调试使用
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022年8月24日
*****************************************************************************/
INT8U NB_WriterReason(INT8U gun,const char * preason,INT8U len)
{
	INT16U i;
	if((preason == NULL) || (!len) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	for(i = 0; i < sizeof(ChargeRecodeInfo[gun].StopReason); i++)
	{
		if(ChargeRecodeInfo[gun].StopReason[i] == 0)
		{
			memcpy(&ChargeRecodeInfo[gun].StopReason[i],preason,MIN((sizeof(ChargeRecodeInfo[gun].StopReason) - i),len));     //可能进入多个停止原因，都把他显示出来
			return TRUE;
		}
	}
	return TRUE;
}
/*****************************************************************************
* Function     : APP_GetCARDWL
* Description  : 获取卡白名单
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
static INT8U* APP_GetCARDWL(void)
{
	_FLASH_OPERATION  FlashOper;

	//读取数据
	FlashOper.DataID = PARA_CARDWHITEL_ID;
	FlashOper.Len = PARA_CARDWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read cardwl  err");
		return NULL;
	}
	return FlashCardVinWLBuf;
}


/*****************************************************************************
* Function     : APP_GetVinWL
* Description  : 获取VIN白名单
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U* APP_GetVinWL(void)
{
	_FLASH_OPERATION  FlashOper;

	//读取数据
	FlashOper.DataID = PARA_VINWHITEL_ID;
	FlashOper.Len = PARA_VINWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read cardwl  err");
		return NULL;
	}
	return FlashCardVinWLBuf;
}

/*****************************************************************************
* Function     : APP_GetGunNum
* Description  : 获取枪数量
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_GetGunNum(void)
{
	if(SYSSet.SysSetNum.UseGun <=  GUN_MAX)
	{
		return SYSSet.SysSetNum.UseGun;
	}
	return GUN_MAX;
}

/*****************************************************************************
* Function     : APP_GetStopChargeReason
* Description  : 最高位为1表示启动失败停止；最高位为0表示启动成功停止
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_GetStopChargeReason(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	return ChargeRecodeInfo[gun].StopChargeReason;
}

/*****************************************************************************
* Function     : APP_GetDispUartStatus
* Description  : 获取显示串口状态
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
_DISP_UART_STATUS APP_GetDispUartStatus(void)
{
	return  DispUartStatus;
}

/*****************************************************************************
* Function     : APP_SetDispUartStatus
* Description  : 设置显示串口状态
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U  APP_SetDispUartStatus(_DISP_UART_STATUS status)
{
	if(status >= DISP_UART_MAX)
	{
		return FALSE;
	}
	OSSchedLock();
	DispUartStatus = status;
	OSSchedUnlock();
	return TRUE;
}


/*****************************************************************************
* Function     : APP_GetRecodeCurNum
* Description  : 获取当前交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT32U  APP_GetRecodeCurNum(void)
{
	return RecodeControl.RecodeCurNum;
}

/*****************************************************************************
* Function     : APP_GetRecodeCurNum
* Description  : 获取当前交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
void  APP_SetRecodeCurNum(INT32U num)
{
	RecodeControl.RecodeCurNum = num;
}

/*****************************************************************************
* Function     : APP_RWChargeRecode
* Description  : 读写交易记录
* Input        :
				count  读写在第几条 1 - 1000条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_RWChargeRecode(INT16U count,_FLASH_ORDER RWChoose,_CHARGE_RECODE * precode)
{
	_FLASH_OPERATION  FlashOper;

	if((count > 1000) || (precode == NULL) )
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_RECODE_ID;
	FlashOper.ptr = (INT8U*)precode;
	FlashOper.RWChoose = RWChoose;
	APP_ChargeRecodeFlashOperation(count,&FlashOper);
	return TRUE;
}


/*****************************************************************************
* Function     : APP_RWOFFLineRe离线交易记录
* Description  : 读写网络离线交易记录
* Input        :
				count  读写在第几条 1 - 100条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_RWNetOFFLineRecode(INT16U count,_FLASH_ORDER RWChoose,INT8U  * pdata)
{
	_FLASH_OPERATION  FlashOper;

	if((count > 100) || (pdata == NULL) )
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_OFFLINEBILL_ID;
	FlashOper.ptr = (INT8U*)pdata;
	FlashOper.RWChoose = RWChoose;
	APP_ChargeOffLineFlashOperation(count,&FlashOper);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_RWNetFSOFFLineRecode
* Description  : 读写网络离线交易记录
* Input        :
				count  读写在第几条 1 - 100条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_RWNetFSOFFLineRecode(INT16U count,_FLASH_ORDER RWChoose,INT8U  * pdata)
{
	_FLASH_OPERATION  FlashOper;

	if((count > 100) || (pdata == NULL) )
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_OFFLINEBILLFS_ID;
	FlashOper.ptr = (INT8U*)pdata;
	FlashOper.RWChoose = RWChoose;
	APP_ChargeOffLineFlashOperation(count,&FlashOper);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetNetOFFLineRecodeNum
* Description  : 读写离线交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_GetNetOFFLineRecodeNum(void)
{
	INT8U num = 0;
	_FLASH_OPERATION  FlashOper;


	FlashOper.DataID = PARA_OFFLINEBILLNUM_ID;
	FlashOper.Len = PARA_OFFLINEBILLNUM_FLLEN;
	FlashOper.ptr = (INT8U *)&num;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	//最多100个
	if(num > 100)
	{
		num = 0;
		FlashOper.DataID = PARA_OFFLINEBILLNUM_ID;
		FlashOper.Len = PARA_OFFLINEBILLNUM_FLLEN;
		FlashOper.ptr = (INT8U *)&num;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
		return 0;
	}
	return num;
}


/*****************************************************************************
* Function     : APP_GetNetOFFLineRecodeNum
* Description  : 读写离线交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_SetNetOFFLineRecodeNum(INT8U num)
{
	_FLASH_OPERATION  FlashOper;

	//最多100个
	if(num > 100)
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_OFFLINEBILLNUM_ID;
	FlashOper.Len = PARA_OFFLINEBILLNUM_FLLEN;
	FlashOper.ptr = (INT8U *)&num;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);

	return TRUE;;
}



/*****************************************************************************
* Function     : SendPricMsg
* Description  : 发送计费消息
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendPricMsg(_GUN_NUM gun ,_PRIC_CMD cmd)
{
	INT8U err;
	static	_BSP_MESSAGE  msg[GUN_MAX];
	OS_EVENT* pevent = APP_GetPricEvent();
	static INT8U gunnum[GUN_MAX];
	if ((gun >= GUN_MAX) ||( cmd >=  PRIC_MAX ) || (pevent == NULL) )
	{
		return FALSE;
	}
	gunnum[gun] = gun;
	msg[gun].MsgID = BSP_MSGID_DISP;
	msg[gun].DivNum = cmd;
	msg[gun].pData = &gunnum[gun];
	msg[gun].DataLen = sizeof(INT8U);
	if((err = OSQPost(pevent, &msg[gun])) != OS_ERR_NONE)
	{
		printf("Send to price message error %d",err);
		return FALSE;
	}
	return TRUE;
}




/*****************************************************************************
* Function     : SendStartChargeMsg
* Description  : 发送给连接ccu任务发送开始充电(开始充电只发一次)
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendStartChargeMsg(_GUN_NUM gun,_CHARGE_MODE_TYPE charge_mode)
{
	static _START_CHARGE_CMD StartChargeCmd[GUN_MAX];
	static	_BSP_MESSAGE	send_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	StartChargeCmd[gun].ChargeMode = charge_mode;
	if(StartChargeCmd[gun].ChargeMode == MODE_MAN)
	{
		StartChargeCmd[gun].ChargeCur = HandSedInfo[gun].outcurr; //保留一个小数
		StartChargeCmd[gun].ChargeVol = HandSedInfo[gun].outvolt;	//保留一个小数
	}
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BCU;
	send_message[gun].DivNum = APP_CHARGE_START;
	send_message[gun].DataLen = sizeof(_START_CHARGE_CMD);
	send_message[gun].pData = (INT8U*)(&StartChargeCmd[gun]);
	send_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &send_message[gun]);	//给控制任务发送消息
	}
	else
	{
		OSQPost(Control_PeventB, &send_message[gun]);	//给控制任务发送消息
	}
	return TRUE;
}



INT8U STOPCNU = 0;
/*****************************************************************************
* Function     : SendStopChargeMsg
* Description  : 发送给连接ccu任务发送停止充电（停止充电发多次）
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendStopChargeMsg(_GUN_NUM gun)
{
	static _START_CHARGE_CMD StartChargeCmd[GUN_MAX];
	static	_BSP_MESSAGE	send_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BCU;
	send_message[gun].DivNum = APP_CHARGE_END;
	send_message[gun].DataLen = sizeof(_START_CHARGE_CMD);
	send_message[gun].pData = (INT8U*)(&StartChargeCmd[gun]);
	send_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &send_message[gun]);	//给控制任务发送消息
	}
	else
	{
		OSQPost(Control_PeventB, &send_message[gun]);	//给控制任务发送消息
	}
	return TRUE;

}

/*****************************************************************************
* Function     : SendCardMsg
* Description  : 发送给卡消息
* Input        : divnum 消息设备号        pdata数据（开始刷卡，需要传入密码） len 密码长度
				结束充电传入消费金额
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendCardMsg(_SEND_TO_CARD_CMD divnum, void * pdata,INT8U len)
{
#define CARD_MSG_NUM    10
	static	_BSP_MESSAGE  msg[CARD_MSG_NUM];
	static INT8U i = 0;
	static INT8U databuf[CARD_MSG_NUM][8];
	INT8U err;
	OS_EVENT* pevent = APP_GetCardEvent();



	if((divnum >= CARDCMD_MAX) || (pevent == NULL) )
	{
		return FALSE;
	}
	memcpy(&databuf[i][0],pdata,len);
	msg[i].MsgID = BSP_MSGID_DISP;
	msg[i].DivNum = divnum;
	msg[i].pData = &databuf[i][0];
	msg[i].DataLen = len;

	if((err = OSQPost(pevent, &msg[i])) != OS_ERR_NONE)
	{
		printf("dis Send to Card  message error %d",err);
		return FALSE;
	}
	i++;
	if(i == CARD_MSG_NUM)
	{
		i = 0;
	}
	OSTimeDly(SYS_DELAY_10ms);
	return TRUE;

}

/*****************************************************************************
* Function     : APP_GetMeterAddr
* Description  : 获取表地址
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
INT8U  APP_GetMeterAddr(_GUN_NUM gun,INT8U *pdata)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	//SYSSet.MeterAdd 为ASICC
	if(SYSSet.MeterAdd[gun][0] != 'a')
	{
		pdata[5] = ByteHexToBcd((SYSSet.MeterAdd[gun][0] - 0x30)*10 + SYSSet.MeterAdd[gun][1] - 0x30);
		pdata[4] = ByteHexToBcd((SYSSet.MeterAdd[gun][2] - 0x30)*10 + SYSSet.MeterAdd[gun][3] - 0x30);
		pdata[3] = ByteHexToBcd((SYSSet.MeterAdd[gun][4] - 0x30)*10 + SYSSet.MeterAdd[gun][5] - 0x30);
		pdata[2] = ByteHexToBcd((SYSSet.MeterAdd[gun][6] - 0x30)*10 + SYSSet.MeterAdd[gun][7] - 0x30);
		pdata[1] = ByteHexToBcd((SYSSet.MeterAdd[gun][8] - 0x30)*10 + SYSSet.MeterAdd[gun][9] - 0x30);
		pdata[0] = ByteHexToBcd((SYSSet.MeterAdd[gun][10] - 0x30)*10 + SYSSet.MeterAdd[gun][11] - 0x30);
	}
	else
	{
		//广播地址
		pdata[0] = 0xaa;
		pdata[1]= 0xaa;
		pdata[2]= 0xaa;
		pdata[3]= 0xaa;
		pdata[4]= 0xaa;
		pdata[5]= 0xaa;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : 获取电价信息
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_PRICE_SET* APP_GetPriceInfo(void)
{
	return &PriceSet;
}

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : 获取电价信息
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_PRICE_SET* APP_GetAPriceInfo(void)
{
	return &A_PriceSet;
}

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : 获取电价信息
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_PRICE_SET* APP_GetBPriceInfo(void)
{
	return &B_PriceSet;
}

/*****************************************************************************
* Function     : APP_GetWorkState
* Description  :获取故障状态
* Input        :
* Output       :
				00H：待机
				01H：工作
				02H：充电完成
				03H：充电暂停
* Return       :
* Note(s)      :
* Contributor  : 2018-8-13
*****************************************************************************/
_WORK_STAT  APP_GetWorkState(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return WORK_MAX;
	}
	return (_WORK_STAT)BCURemoteSignal[gun].state1.State.System_State;
}

/*****************************************************************************
* Function     : APP_GetErrState
* Description  :获取故障状态
* Input        :
* Output       :
				00H：正常
				01H：故障
* Return       :
* Note(s)      :
* Contributor  : 2018-8-13
*****************************************************************************/
INT8U  APP_GetErrState(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return WORK_MAX;
	}
	return (_WORK_STAT)BCURemoteSignal[gun].state1.State.System_State;
}


/*****************************************************************************
* Function     : APP_GetChargeingInfo
* Description  :提供给充电中显示界面显示
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CHARGEING_INFO * APP_GetChargeingInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &ChargeingInfo[gun];
}

/*****************************************************************************
* Function     : APP_GetCarInfo
* Description  :提供给车辆信息界面显示界面显示
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CARD_INFO * APP_GetCarInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &CardInfo[gun];
}

/*****************************************************************************
* Function     : APP_GetChargeInfo
* Description  :
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CHARGE_INFO* APP_GetChargeInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &ChargeInfo[gun];
}


/*****************************************************************************
* Function     : APP_SetCardPriceInfo
* Description  : 获取当前记录
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CHARGE_RECODE* APP_GetChargeRecodeInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &ChargeRecodeInfo[gun];
}

/*****************************************************************************
* Function     : APP_SetCountDownTime
* Description  : 设置倒计时时间   ，切换页面的时候设置倒计时时间，
				测任务中倒计时每1s -1，直到见到1为止
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
void  APP_SetCountDownTime(INT16U time)
{
	OSSchedLock();
	DispControl.CountDown = time;
	OSSchedUnlock();
}

/*****************************************************************************
* Function     : APP_GetDispControl
* Description  : 获取显示任务控制结构体
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_DISP_CONTROL* APP_GetDispControl(void)
{
	return &DispControl;
}

/*****************************************************************************
* Function     : APP_GetDispEvent
* Description  :获取显示任务事件
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
OS_EVENT* APP_GetDispEvent(void)
{
	return DispTaskEvent;
}

/*****************************************************************************
* Function     : Display_HardwareInit
* Description  : 显示任务初始化
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
INT8U Display_HardwareInit(void)
{

	_BSP_UART_SET UartGPRSSet;
	DispTaskEvent = OSQCreate(DisplayOSQ, DISPLAY_Q_LEN);
	if (DispTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
		return FALSE;
	}
	memset(DispSendBuf, 0 ,sizeof(DispSendBuf));
	memset(DispRecvBuf, 0 ,sizeof(DispRecvBuf));

	UartGPRSSet.BaudRate = 115200;
	UartGPRSSet.DataBits = BSPUART_WORDLENGTH_8;
	UartGPRSSet.Parity   = BSPUART_PARITY_NO;
	UartGPRSSet.StopBits = BSPUART_STOPBITS_1;
	UartGPRSSet.RxBuf = DispRecvBuf;
	UartGPRSSet.RxBufLen = sizeof(DispRecvBuf);
	UartGPRSSet.TxBuf = DispSendBuf;
	UartGPRSSet.TxBufLen = sizeof(DispSendBuf);
	UartGPRSSet.Mode = UART_DEFAULT_MODE | UART_MSG_MODE;
	UartGPRSSet.RxQueue = DispTaskEvent;
	UartGPRSSet.RxMsgMemBuf = DispRecvSMem;
	UartGPRSSet.RxMsgMemLen = sizeof(DispRecvSMem);
	if (BSP_UARTConfigInit(DISPLAY_UART, &UartGPRSSet) == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: CountPeriodSumTime
* Description	: 计算周期总时长
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年8月9日
***********************************************************************************************/
static INT16U CountPeriodSumTime(INT16U  *minute)
{
	INT16U time[TIME_PERIOD_MAX];
	INT16U SumTime;
	time[0] = ((minute[1]>=minute[0])?(minute[1] - minute[0]):((minute[1] + MAX_PERIOD_SUM_TIME) - minute[0]));

	time[1] = ((minute[2]>=minute[1])?(minute[2] - minute[1]):((minute[2] + MAX_PERIOD_SUM_TIME) - minute[1]));

	time[2] = ((minute[3]>=minute[2])?(minute[3] - minute[2]):((minute[3] + MAX_PERIOD_SUM_TIME) - minute[2]));

	time[3] = ((minute[0]>=minute[3])?(minute[0] - minute[3]):((minute[0] + MAX_PERIOD_SUM_TIME) - minute[3]));

	SumTime = time[0] + time[1] + time[2] + time[3];

	if(SumTime == 0)
	{
		SumTime = MAX_PERIOD_SUM_TIME;
	}

	return SumTime;
}



/***********************************************************************************************
* Function		: MENU11_TIMEDispose
* Description	: 界面3 16下发的卡密码处理
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu3_16_CodeDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT32U code;

	if((pvalue == NULL) || (len != 4) || (DispControl.CurUserGun >= GUN_MAX) )
	{
		return FALSE;
	}
	code = (pvalue[0] << 24) | (pvalue[1] << 16) | (pvalue[2] << 8) | pvalue[3];

	//将整数转换未assic
	DispControl.CardCode[0] = code % 1000000 / 100000 + '0';
	DispControl.CardCode[1] = code % 100000 / 10000 + '0';
	DispControl.CardCode[2] = code % 10000 / 1000 + '0';
	DispControl.CardCode[3] = code % 1000 / 100 + '0';
	DispControl.CardCode[4] = code % 100 / 10 + '0';
	DispControl.CardCode[5] = code % 10 / 1 + '0';

	DisplayCommonMenu(&HYMenu22,NULL);			//进入刷卡界面
	//获取卡是否为空闲
	if(GetCardWorkStatus() != _Card_IDLE)
	{
		return FALSE;
	}
	if(HYMenu21.Menu_PrePage == &HYMenu3)			//上一页是充电方式选择界面
	{
		if(DispControl.CurUserGun == GUN_A)
		{
			if(SendCardMsg(CARDCMD_STARTA, DispControl.CardCode,sizeof(DispControl.CardCode)) == FALSE)
			{
				return FALSE;
			}
		}
		else
		{
			if(SendCardMsg(CARDCMD_STARTB, DispControl.CardCode,sizeof(DispControl.CardCode)) == FALSE)
			{
				return FALSE;
			}
		}
	}
	else
	{
		if(SendCardMsg(CARDCMD_QUERY, DispControl.CardCode,sizeof(DispControl.CardCode)) == FALSE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu73_HANDADispose
* Description	: 界面73A枪手动模式处理
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu50_HANDADispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	if((pvalue == NULL) || (len != 4) )
	{
		return FALSE;
	}
	HandSedInfo[GUN_A].outvolt =  (pvalue[0] << 8 | pvalue[1])*10; //保留一个小数
	HandSedInfo[GUN_A].outcurr =  (pvalue[2] << 8 | pvalue[3])*10; //保留一个小数
	if(APP_GetWorkState(GUN_A) != WORK_CHARGE)   //不在充电中，发送启动命令
	{
		SendStartChargeMsg(GUN_A,MODE_MAN);
	}
	else									//在充电中，发送调整输出
	{
		NB_WriterReason(GUN_A,"E63",3);
		SendStopChargeMsg(GUN_A);
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu74_HANDBDispose
* Description	: 界面74B枪手动模式处理
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu51_HANDBDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	if((pvalue == NULL) || (len != 4) )
	{
		return FALSE;
	}
	HandSedInfo[GUN_B].outvolt =  (pvalue[0] << 8 | pvalue[1])*10; //保留一个小数
	HandSedInfo[GUN_B].outcurr =  (pvalue[2] << 8 | pvalue[3])*10; //保留一个小数
	if(APP_GetWorkState(GUN_B) != WORK_CHARGE)	 //不在充电中，发送启动命令
	{
		SendStartChargeMsg(GUN_B,MODE_MAN);
	}
	else									//在充电中，发送调整输出
	{
		NB_WriterReason(GUN_B,"E62",3);
		SendStopChargeMsg(GUN_B);
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_DevnumDispose
* Description	: 52系统配置桩编号处理
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_DevnumDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U dev_num[16];
	INT8U i;
	_FLASH_OPERATION  FlashOper;

	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <16; i++)
	{
		dev_num[i]  = 0x30;
	}
	if(len > 16)
	{
		len = 16;		//ASICC输入时，可能存在len大于16
	}
	for(i = 0; i < 16; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			dev_num[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.SysSetNum.DivNum,dev_num,sizeof(dev_num));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_ProNumDispose
* Description	: 52系统配置项目编号处理
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_ProNumDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U pro_num[16];
	INT8U i;
	_FLASH_OPERATION  FlashOper;


	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <16; i++)
	{
		pro_num[i]  = 0x30;
	}

	//本来是16个字节，改为4个字节
	if(len > 4)
	{
		len = 4;		//ASICC输入时，可能存在len大于4
	}
	for(i = 0; i < 4; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			pro_num[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.SysSetNum.ProjectNum,pro_num,sizeof(pro_num));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_ModuleNumDispose
* Description	: 52系统配置模块使用个数处理
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_ModuleNumDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U modulenum = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	modulenum =  pvalue[1];
	if(modulenum > 16)
	{
		return FALSE;
	}

	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);


	if(modulenum == PresetValue.PowerModelNumber)  //位改变
	{
		return TRUE;
	}
	PresetValue.PowerModelNumber = modulenum;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;

}

/***********************************************************************************************
* Function		: Munu52_MacMaxVolDispose
* Description	: 整机最高电压   分辨率：1V，0偏移量
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_MacMaxVolDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U MaxVol = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}

	MaxVol =  (pvalue[0]) << 8 | pvalue[1];
	if(MaxVol > 1000)
	{
		return FALSE;
	}
	MaxVol *= 10; //放大10倍
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);

	if(PresetValue.VolMaxOutput == MaxVol)
	{
		return TRUE;
	}
	PresetValue.VolMaxOutput = MaxVol;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_MacMinVolDispose
* Description	: 整机最低电压	 分辨率：1V，0偏移量
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_MacMinVolDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U MinVol = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	MinVol =  (pvalue[0]) << 8 | pvalue[1];
	MinVol  *= 10;	//放大10倍
	if((MinVol <100) || (MinVol > 200))
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.VolMinOutput == MinVol)
	{
		return TRUE;
	}

	PresetValue.VolMinOutput = MinVol;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_MaxOutCurDispose
* Description	: 最大输出电流	 分辨率0.1A，0偏移量
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_MaxOutCurDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U MaxOutCur = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}

	MaxOutCur =  (pvalue[0]) << 8 | pvalue[1];
	if(MaxOutCur >2500)
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.CurMaxOutput == MaxOutCur)
	{
		return TRUE;
	}
	PresetValue.CurMaxOutput = MaxOutCur;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_UseGunDispose
* Description	: 枪使用个数 1 Byte 	分辨率：1倍，0偏移量
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_UseGunDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U UseGun = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	UseGun =  pvalue[1];
	if(UseGun >4)
	{
		return FALSE;
	}
	if(SYSSet.SysSetNum.UseGun == UseGun)
	{
		return TRUE;
	}
	SYSSet.SysSetNum.UseGun = UseGun;
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_OutDcVolDispose
* Description	: 输出过压保护阀值	BIN	2 Byte	分辨率：0.1V，0偏移量
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_OutDcVolDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U OutDcVol = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}

	OutDcVol =  (pvalue[0]) << 8 | pvalue[1];
	if(OutDcVol >10500)
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.Therold_Value.OverOutVolTherold == OutDcVol)
	{
		return TRUE;
	}
	PresetValue.Therold_Value.OverOutVolTherold = OutDcVol;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_OutDcCurDispose
* Description	:输出过流保护阀值	BIN	2 Byte	分辨率：0.1A，0偏移量
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_OutDcCurDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U OutDcCur = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	OutDcCur =  (pvalue[0]) << 8 | pvalue[1];
	if(OutDcCur >2600)
	{
		return FALSE;
	}

	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.Therold_Value.OverOutCurTherold == OutDcCur)
	{
		return TRUE;
	}
	PresetValue.Therold_Value.OverOutCurTherold = OutDcCur;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_SOCDispose
* Description	:soc保护阀值	BIN	2 Byte	分辨率：1%，0偏移量
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu52_SOCDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U SOC = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	SOC =   pvalue[1];
	if(SOC >100)
	{
		return FALSE;
	}
	//SOC阀值
	SYSSet.SOCthreshold = SOC;  //
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_acinputupvalueDispose
* Description	:交流输入过压 0.1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu53_acinputupvalueDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U acinputupvalue = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	acinputupvalue =  (pvalue[0]) << 8 | pvalue[1];
	if( (acinputupvalue < 3000) ||  (acinputupvalue >5000))
	{
		return FALSE;
	}

	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.Therold_Value.OverACVolTherold == acinputupvalue)
	{
		return FALSE;
	}
	PresetValue.Therold_Value.OverACVolTherold = acinputupvalue;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_acinputdownvalueDispose
* Description	:交流输入过压 0.1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu53_acinputdownvalueDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U acinputdownvalue = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	acinputdownvalue =  (pvalue[0]) << 8 | pvalue[1];
	if( (acinputdownvalue < 3000) ||  (acinputdownvalue >5000))
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);

	if(PresetValue.Therold_Value.UnderACVolTherold == acinputdownvalue)
	{
		return TRUE;
	}
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	PresetValue.Therold_Value.UnderACVolTherold = acinputdownvalue;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_MeterAddADispose
* Description	:a枪表号处理
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu53_MeterAddADispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U MeterAddA[12];
	INT8U i;
	_FLASH_OPERATION  FlashOper;


	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <12; i++)
	{
		MeterAddA[i]  = 0x30;
	}
	if(len > 12)
	{
		len = 12;		//ASICC输入时，可能存在len大于16
	}
	for(i = 0; i < 12; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			MeterAddA[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.MeterAdd[GUN_A],MeterAddA,sizeof(MeterAddA));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_MeterAddBDispose
* Description	:B枪表号处理
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu53_MeterAddBDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U MeterAddB[12];
	INT8U i;
	_FLASH_OPERATION  FlashOper;


	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <12; i++)
	{
		MeterAddB[i]  = 0x30;
	}
	if(len > 12)
	{
		len = 12;		//ASICC输入时，可能存在len大于16
	}
	for(i = 0; i < 12; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			MeterAddB[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.MeterAdd[GUN_B],MeterAddB,sizeof(MeterAddB));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu54_SetTimeDispose
* Description	:设置时间
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu54_SetTimeDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	_BSPRTC_TIME SetTime;                       //设定时间
	INT8U times = 3;							//如果设置失败反复设置三次
	if((pvalue == NULL) || (len != 12))
	{
		return FALSE;
	}
	SetTime.Year   = HEXtoBCD((INT8U)((pvalue[0]<<8 | pvalue[1]) - 2000));//将输入的十进制数转换成BCD
	SetTime.Month  = HEXtoBCD(pvalue[3]);
	SetTime.Day    = HEXtoBCD(pvalue[5]);
	SetTime.Hour   = HEXtoBCD(pvalue[7]);
	SetTime.Minute = HEXtoBCD(pvalue[9]);
	SetTime.Second = HEXtoBCD(pvalue[11]);

	while(times--)
	{
		if(BSP_RTCSetTime(&SetTime) == TRUE)    //设置RTC
		{
			return TRUE;
		}
	}
	return FALSE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:接触器前级电压点1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu57_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_SW_FRONT1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:接触器前级电压点2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu57_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_SW_FRONT2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:接触器后级电压点1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu58_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.DivNum = JUST_SW_AFTER1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;

}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:接触器后级电压点2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu58_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_SW_AFTER2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}


/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:电流点1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu59_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_CUR1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:电流点2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu59_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_CUR2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:充电枪点1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu60_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_GUN2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.GunNum = DispControl.CurUserGun;

	send_message.MsgID = BSP_MSGID_DISP;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:充电枪点2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu60_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_GUN1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.GunNum = DispControl.CurUserGun;
	send_message.MsgID = BSP_MSGID_DISP;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu48_PrcDispose
* Description	:电价设置
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu48_PrcDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	//将下发的时间段转换为分钟，因为时间段要形成闭环，故第一个时间段的结束等于第二个时间段的开始，故只需要设置4个时间段即可
	INT16U minute[TIME_PERIOD_MAX];
	_FLASH_OPERATION  FlashOper;

	if((pvalue == NULL) || (len != 32) )
	{
		return FALSE;
	}
	minute[0] = (pvalue[0] << 8 | pvalue[1]) * 60 +  (pvalue[2] << 8 | pvalue[3]);
	minute[1] = (pvalue[4] << 8 | pvalue[5]) * 60 +	(pvalue[6] << 8 | pvalue[7]);
	minute[2] = (pvalue[8] << 8 | pvalue[9]) * 60 +	(pvalue[10] << 8 | pvalue[11]);
	minute[3] = (pvalue[12] << 8 | pvalue[13]) * 60 +	(pvalue[14] << 8 | pvalue[15]);
	//参数保护
	if((minute[0] > MAX_PERIOD_SUM_TIME) || (minute[1] > MAX_PERIOD_SUM_TIME) || (minute[2] > MAX_PERIOD_SUM_TIME)\
	        || (minute[3] > MAX_PERIOD_SUM_TIME) )
	{
		Display_PricSet(0);
		Dis_ShowStatus(ADDR_MENU81_SHOW,SHOW_CFG_FAIL,RED);    //设置失败
		//提示错误
		return FALSE;
	}
	//检查设置时间段是否合适
	if(CountPeriodSumTime(minute) != MAX_PERIOD_SUM_TIME)
	{
		Display_PricSet(0);
		Dis_ShowStatus(ADDR_MENU81_SHOW,SHOW_CFG_FAIL,RED);    //设置失败
		//提示错误
		return FALSE;
	}
	PriceSet.StartTime[0] = minute[0];
	PriceSet.EndTime[0] = minute[1];
	PriceSet.StartTime[1] = minute[1];
	PriceSet.EndTime[1] = minute[2];
	PriceSet.StartTime[2] = minute[2];
	PriceSet.EndTime[2] = minute[3];
	PriceSet.StartTime[3] = minute[3];
	PriceSet.EndTime[3] = minute[0];
	PriceSet.Price[0] = (pvalue[16] << 8 | pvalue[17])*1000;
	PriceSet.Price[1] = (pvalue[18] << 8 | pvalue[19])*1000;
	PriceSet.Price[2] = (pvalue[20] << 8 | pvalue[21])*1000;
	PriceSet.Price[3] = (pvalue[22] << 8 | pvalue[23])*1000;
	PriceSet.ServeFee[0] = (pvalue[24] << 8 | pvalue[25])*1000;
	PriceSet.ServeFee[1] = (pvalue[26] << 8 | pvalue[27])*1000;
	PriceSet.ServeFee[2] = (pvalue[28] << 8 | pvalue[29])*1000;
	PriceSet.ServeFee[3] = (pvalue[30] << 8 | pvalue[31])*1000;
//	PriceSet.ViolaFee = pvalue[26] << 8 | pvalue[27];
	PriceSet.TimeQuantumNum = 4;

	//读取所有配置信息
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = PARA_PRICALL_FLLEN;
	FlashOper.ptr = (INT8U *)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		Display_PricSet(0);
		Dis_ShowStatus(DIS_ADD(48,0x10),SHOW_CFG_FAIL,RED);    //设置失败
		printf("Writer printf set err");
	}
	Dis_ShowStatus(DIS_ADD(48,0x10),SHOW_CFG_SUCCESS,RED);    //设置失败
	return TRUE;
}


/***********************************************************************************************
* Function		: Munu10_PortSert
* Description	:端口设置
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu10_PortSert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp =  (pvalue[0] << 8) | (pvalue[1]);

	//memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
	SYSSet.Port = temp;
	NetConfigInfo[SYSSet.NetYXSelct].port = temp;
	//memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
	//snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP设置
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu10_IP1Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[0] = temp;
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
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP设置
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu10_IP2Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[1] = temp;
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
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP设置
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu10_IP3Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[2] = temp;
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
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP设置
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu10_IP4Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[3] = temp;
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
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP设置
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu10_NetNumSet(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];
	if(temp > 2)
	{
		return FALSE;
	}

	SYSSet.NetNum = temp;
	NetConfigInfo[SYSSet.NetYXSelct].NetNum = SYSSet.NetNum;
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: MENU11_TIMEDispose
* Description	: 管理员密码下发
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
static INT8U Munu3_CodeDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT32U code;
	INT32U admin_code = 888888;

	if((pvalue == NULL) || (len != 4) || (DispControl.CurUserGun >= GUN_MAX) )
	{
		return FALSE;
	}
	code = (pvalue[0] << 24) | (pvalue[1] << 16) | (pvalue[2] << 8) | pvalue[3];
	if(SYSSet.NetState == DISP_NET)
	{
		if(code == admin_code)   //密码正确，跳转到管理员界面
		{
			if(HYMenu23.Menu_PrePage == &HYMenu3)
			{
				DisplayCommonMenu(&HYMenu47,&HYMenu73);  //跳转到系统配置界面
			}
			else
			{
				DisplayCommonMenu(&HYMenu46,&HYMenu73);  //跳转到用户配置
			}
		}
		else
		{
			DisplayCommonMenu(&HYMenu24,&HYMenu73);  //密码错误
		}
	}

	else
	{
		if(code == admin_code)   //密码正确，跳转到管理员界面
		{
			if(HYMenu23.Menu_PrePage == &HYMenu3)
			{
				DisplayCommonMenu(&HYMenu47,NULL);  //跳转到系统配置界面
			}
			else
			{
				DisplayCommonMenu(&HYMenu46,NULL);  //跳转到用户配置
			}
		}
		else
		{
			DisplayCommonMenu(&HYMenu24,&HYMenu3);  //密码错误
		}

	}


	return TRUE;
}

const _DISP_ADDR_FRAME Disp_RecvFrameTable[] =
{
	/********************************汇誉屏幕*************************/
	{INPUT_MENU21_CODE          	,       Munu3_16_CodeDispose },  // 界面21输入卡密码地址
	{INPUT_MENU23_CODE          	,       Munu3_CodeDispose 	},   // 界面23输入管理员密码地址
	{INPUT_MENU50_HANDA				,		Munu50_HANDADispose  },	//A枪手动模式
	{INPUT_MENU51_HANDB				,		Munu51_HANDBDispose  },	//B枪手动模式
	{DIS_ADD(52,0)					,		Munu52_DevnumDispose }, //桩编号处理
	{DIS_ADD(52,8)					,		Munu52_ProNumDispose }, //项目编号处理
	{DIS_ADD(52,0x11)					,		Munu52_ModuleNumDispose }, 	//模块使用个数
	{DIS_ADD(52,0x12)					,		Munu52_MacMaxVolDispose },//设备最高电压
	{DIS_ADD(52,0x13)					,		Munu52_MacMinVolDispose },//设备最低电压
	{DIS_ADD(52,0x14)					,		Munu52_MaxOutCurDispose },//最大输出电流
	{DIS_ADD(52,0x15)					,		Munu52_UseGunDispose },//设置枪数
	{DIS_ADD(52,0x16)					,		Munu52_OutDcVolDispose },//过压阈值
	{DIS_ADD(52,0x17)					,		Munu52_OutDcCurDispose },//过流阀值
	{DIS_ADD(52,0x18)					,		Munu52_SOCDispose 		},//SOC阀值

	{DIS_ADD(53,0)					,		Munu53_acinputupvalueDispose },// 交流输入过压
	{DIS_ADD(53,1)					,		Munu53_acinputdownvalueDispose },//交流输入欠压
	{DIS_ADD(53,2)					,		Munu53_MeterAddADispose },//A枪表号
	{DIS_ADD(53,8)					,		Munu53_MeterAddBDispose },//B枪表号
	{INPUT_MENU54_CFG				,		Munu54_SetTimeDispose	},//时间设置

	{DIS_ADD(57,0)					,		Munu57_Just1Dispose		},//接触器前级电压校准1
	{DIS_ADD(57,1)					,		Munu57_Just2Dispose		},//接触器前级电压校准2
	{DIS_ADD(58,0)					,		Munu58_Just1Dispose		},//接触器后级电压校准1
	{DIS_ADD(58,1)					,		Munu58_Just2Dispose		},//接触器后级电压校准2
	{DIS_ADD(59,0)					,		Munu59_Just1Dispose		},//电流校准1
	{DIS_ADD(59,1)					,		Munu59_Just2Dispose		},//电流校准1
	{DIS_ADD(60,0)					,		Munu60_Just1Dispose		},//未插枪电压
	{DIS_ADD(60,1)					,		Munu60_Just2Dispose		},//插枪电压
	{DIS_ADD(48,0)					,		Munu48_PrcDispose		},//费率设置
	{DIS_ADD(10,0x11)				,		Munu10_PortSert		},			//端口设置
	{DIS_ADD(10,0x12)				,		Munu10_IP1Sert		},			//ip设置
	{DIS_ADD(10,0x13)				,		Munu10_IP2Sert		},			//ip设置
	{DIS_ADD(10,0x14)				,		Munu10_IP3Sert		},			//ip设置
	{DIS_ADD(10,0x15)				,		Munu10_IP4Sert		},			//ip设置
	{DIS_ADD(10,0x16)				,		Munu10_NetNumSet		},		//网络个数设置

};

/***********************************************************************************************
* Function		: DealWithVari
* Description	: 变量处理函数
* Input			: INT16U addr       变量地址
                  INT8U *pvalue     变量值指针
                  INT8U len         变量长度
* Output		:
* Note(s)		:
* Contributor	: 2018年7月31日
***********************************************************************************************/
static INT8U DealWithVari(INT16U addr, INT8U *pvalue,INT8U len)
{
	if (addr ==NULL ||pvalue ==NULL || !len)
	{
		return FALSE;
	}
	for (INT8U i = 0; i < sizeof(Disp_RecvFrameTable)/sizeof(_DISP_ADDR_FRAME); i++)
	{
		if (Disp_RecvFrameTable[i].variaddr == addr)                    //查找地址
		{
			if (Disp_RecvFrameTable[i].Fun)                             //找到相同变量地址
			{
				return Disp_RecvFrameTable[i].Fun(addr, pvalue, len);   //变量处理
			}
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_DisplayRecvDataAnalyze
* Description  : 数据接收解析
* Input        : INT8U *pdata
                 INT8U len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
INT8U APP_DisplayRecvDataAnalyze(INT8U *pdata, INT8U len)
{
	ST_Menu* CurMenu = GetCurMenu();
	_LCD_KEYVALUE keyval = (_LCD_KEYVALUE)0;                //取键值
	INT16U KeyID = 0;       								//按键的ID与页面的ID对应上

	if  ( (pdata == NULL) || (len < 6) || CurMenu == NULL)                       //长度数据做保护
	{
		return FALSE;
	}

	INT8U  datalen = pdata[2];                              //数据长度 = 帧长度-帧头（2byte）-自身占用空间（1byte）
	INT8U  cmd     = pdata[3];                              //命令
	INT16U lcdhead = ((pdata[0]<<8) | pdata[1]);

	if (lcdhead == DWIN_LCD_HEAD)            //判断帧头帧尾
	{
		if ( (datalen + 3 != len) )
		{
			return FALSE;
		}
		if(cmd == VARIABLE_READ)                             //读变量地址返回数据
		{
			INT16U variaddr  = ((pdata[4]<<8) | pdata[5]);  //提取变量地址
			INT8U  varilen   = pdata[6] * 2;                //提取变量数据长度(这里转换成字节)
			INT8U *varivalue = &pdata[7];                   //提取变量值开始地址

			if(variaddr == KEY_VARI_ADDR)                   //所有的按键地址都是0x0000 只是用键值去区分
			{
				KeyID = ((pdata[datalen+1]<<8) | pdata[datalen+2]);	   //取按键ID
				keyval = (_LCD_KEYVALUE)(pdata[datalen+2] & 0x7f);     //取键值,取低7位
				if(DIS_ADD(CurMenu->FrameID,keyval) == KeyID)			//只有在当前界面上的按键才有效
				{
					DealWithKey(&keyval);                       		//按键动作
				}
			}
			else                                            //变量数据返回
			{
				DealWithVari(variaddr,varivalue,varilen);   //变量数据处理
			}

		}
#if (USER_SYSTTC == 0)
		if(cmd == REGISTER_READ)      //读取迪文屏幕时间返回
		{
			if(datalen != 0x0A)
			{
				return FALSE;
			}
			ReadDwRtc.Year = pdata[6];
			ReadDwRtc.Month = pdata[7];
			ReadDwRtc.Day = pdata[8];
			ReadDwRtc.Week = pdata[9];
			ReadDwRtc.Hour = pdata[10];
			ReadDwRtc.Minute = pdata[11];
			ReadDwRtc.Second = pdata[12];
		}
#endif
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_DWRTCGetTime
* Description  : 获取DWRTC
* Input        : INT8U *pdata
                 INT8U len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
INT8U APP_DWRTCGetTime(_BSPRTC_TIME *pTime)
{
	memcpy(pTime,&ReadDwRtc,sizeof(_BSPRTC_TIME));
	return TRUE;
}

/*****************************************************************************
* Function     : APP_DisplayRecvDataAnalyze
* Description  : 数据接收解析
* Input        : INT8U *pdata
                 INT8U len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U ChargeRecode_Dispose(_GUN_NUM gun)
{
	USERINFO* pric_info = NULL;
	_FLASH_OPERATION  FlashOper;
	USERCARDINFO* card_info;
	INT8U* pdata;
	INT32U money;

	//获取费率信息
	pric_info =  GetChargingInfo(gun);
	if(pric_info == NULL)
	{
		return FALSE;
	}
	//GetCurTime(&pric_info->EndTime);                         //记录结束计费时间
	if(gun == GUN_A)
	{
		card_info = GetGunCardInfo(GUN_A);
	}
	else
	{
		card_info = GetGunCardInfo(GUN_B);
	}
	memcpy((INT8U *)&ChargeRecodeInfo[gun].StartTime,(INT8U *)&pric_info->StartTime,sizeof(pric_info->StartTime) );
	memcpy((INT8U *)&ChargeRecodeInfo[gun].EndTime,(INT8U *)&pric_info->EndTime,sizeof(pric_info->EndTime) );
	ChargeRecodeInfo[gun].TotMoney = pric_info->TotalBill/100;

	ChargeRecodeInfo[gun].TotPower = pric_info->TotalPower4/10;

	ChargeRecodeInfo[gun].ChargeTime = pric_info->ChargeTime;
	if(SYSSet.NetState == DISP_CARD)
	{
		ChargeRecodeInfo[gun].BeforeCardBalance = card_info->balance;
		if(ChargeRecodeInfo[gun].BeforeCardBalance == 0) //卡内余额等于0
		{
			//if(SHOW_STOP_ERR_NONE == ChargeRecodeInfo[gun].StopChargeReason)
			{

				ChargeRecodeInfo[gun].StopChargeReason = (INT8U)SHOW_STOP_BALANCE - (INT8U)SHOW_STOP_ERR_NONE;			//余额不足
			}
		}
	}
	else
	{
		money = APP_GetNetMoney(gun);
		if(money < ChargeRecodeInfo[gun].TotMoney)
		{
			ChargeRecodeInfo[gun].BeforeCardBalance = 0;
		}
		else
		{
			ChargeRecodeInfo[gun].BeforeCardBalance = APP_GetNetMoney(gun) - ChargeRecodeInfo[gun].TotMoney;
		}
		pdata = APP_GetBatchNum(gun);
		memcpy(ChargeRecodeInfo[gun].TransNum,pdata,sizeof(ChargeRecodeInfo[gun].TransNum));  //交易流水号
	}
	//SOC达到
	if(((ChargeingInfo[gun].ChargeSOC & 0xff00) >> 8) >= SYSSet.SOCthreshold)
	{
		//if(SHOW_STOP_ERR_NONE == ChargeRecodeInfo[DispControl.CurUserGun].StopChargeReason)
		{
			ChargeRecodeInfo[gun].StopChargeReason = (INT8U)SHOW_STOP_SOC - (INT8U)SHOW_STOP_ERR_NONE;			//设定soc达到
		}
	}




	//写交易记录
	//RecodeControl.RecodeCurNum = 250;
	RecodeControl.RecodeCurNum++;
	//写入交易记录条数
	FlashOper.DataID = PARA_1200_ID;
	FlashOper.Len = PARA_1200_FLLEN;
	FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Writer RecodeCurNum  err");
		return FALSE;
	}
	APP_RWChargeRecode(RECODE_DISPOSE1(RecodeControl.RecodeCurNum%1000),FLASH_ORDER_WRITE,&ChargeRecodeInfo[gun]);


	//保险起见，再操作一次  20220902
	//关闭指示灯
	BSP_CloseRunLed(gun);
	//关闭K1K2
	BSP_CloseK1K2(gun);
	//设置为未充电
	SetChargeRunningState(gun,FALSE);
	return TRUE;
}

INT8U BUG35CNU = 0;
/*****************************************************************************
* Function     : APP_RecvCardDispose
* Description  : 接收到卡返回命令处理
* Input        :_BSP_MESSAGE *pMsg
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U APP_RecvCardDispose(_SEND_TO_CARD_CMD cmd,FRAMERTU rtu)
{
	USERCARDINFO * puser_card_info = NULL;
	INT32U CardID;			//卡号
	ST_Menu* pcurMenu = NULL;	//当前菜单

	if(cmd >= CARDCMD_MAX)
	{
		return FALSE;
	}
	if((cmd == CARDCMD_STARTA) || (cmd == CARDCMD_STARTB) )
	{
		switch (rtu)
		{
			case _SuccessRtu:
				//跳转到启动中界面
				//电子锁上锁
				//通知计费任务开始计费
				//通知与ccu通信任务开始启动充电
				//获取卡信息
				if(cmd == CARDCMD_STARTA)
				{
					puser_card_info = GetGunCardInfo(GUN_A);
				}
				else
				{
					puser_card_info = GetGunCardInfo(GUN_B);
				}
				if(puser_card_info == NULL)
				{
					return FALSE;
				}
				//拷贝信息到交易记录中
				ChargeRecodeInfo[DispControl.CurUserGun].CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
				        (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
				//			ChargeRecodeInfo[DispControl.CurUserGun].AfterCardBalance = puser_card_info->balance;
				ChargeRecodeInfo[DispControl.CurUserGun].Gun = DispControl.CurUserGun;
				ChargeRecodeInfo[DispControl.CurUserGun].BillingStatus = RECODE_UNSETTLEMENT;	  			//初始为未结算
				if(cmd == CARDCMD_STARTA)
				{
					//DisplayCommonMenu(&gsMenu28,NULL);  //调转到A枪启动界面
					DisplayCommonMenu(&HYMenu30,NULL);  //调转到A枪启动界面
				}
				else
				{
					//DisplayCommonMenu(&gsMenu34,NULL);  //调转到b枪启动界面
					DisplayCommonMenu(&HYMenu31,NULL);  //调转到B枪启动界面
				}
				//	SendElecLockMsg(DispControl.CurUserGun,SET_ELECLOCK);	//发送上锁
				SendPricMsg(DispControl.CurUserGun,PRIC_START);			//通知开始计费
				DispControl.SendStartCharge[DispControl.CurUserGun] = TRUE;
				//SendStartChargeMsg(DispControl.CurUserGun,MODE_AUTO);	//通知启动充电
				break;
			case _DataOverTime:
			case _FailRtu:   //失败不处理
				//提示无法识别此卡
				//DisplayCommonMenu(&gsMenu24,NULL);	//提示无法识别此卡
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu28,&HYMenu21);	//提示读卡超时
#else
				DisplayCommonMenu(&HYMenu28,&HYMenu22);	//提示读卡超时
#endif
				break;
			case _BalanceLack:
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu25,&HYMenu21);	//提示无法识别此卡
#else
				DisplayCommonMenu(&HYMenu25,&HYMenu22);	//提示无法识别此卡
#endif
				break;
			case _FailPIN:   //密码错误
				//跳转到密码输入错误界面
				//获取卡信息
				//显示剩余密码输入次数
				//DisplayCommonMenu(&gsMenu21,NULL);  //跳转到面输入错误界面
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu24,&HYMenu21);	//提示无法识别此卡
#else
				DisplayCommonMenu(&HYMenu24,&HYMenu22);	//提示无法识别此卡
#endif
				break;
				//PrintNum16uVariable(ADDR_MENU21_SHOW,puser_card_info->RetryCnt); 		//显示剩余密码输入次数
			case _GreyLockFull: //此卡被锁
			case _LockCard:
				//提示卡被锁
				//DisplayCommonMenu(&gsMenu23,NULL);  //跳转提示卡被锁界面
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu26,&HYMenu21);	//提示无法识别此卡
#else
				DisplayCommonMenu(&HYMenu26,&HYMenu22);	//提示无法识别此卡
#endif
				break;
			default:
				//提示无法识别此卡
				//DisplayCommonMenu(&gsMenu24,NULL);	//提示无法识别此卡
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu27,&HYMenu21);	//提示无法识别此卡
#else
				DisplayCommonMenu(&HYMenu27,&HYMenu22);	//提示无法识别此卡
#endif
				break;
		}
	}
	if((cmd == CARDCMD_STOPA) || (cmd == CARDCMD_STOPB) )
	{
		switch (rtu)
		{
			case _SuccessRtu:
				ChargeRecodeInfo[DispControl.CurUserGun].BillingStatus = RECODE_SETTLEMENT;	  //已经结算
//				if(cmd == CARDCMD_STOPA)
//				{
//					puser_card_info = GetGunCardInfo(GUN_A);
//				}else{
//					puser_card_info = GetGunCardInfo(GUN_B);
//				}
//							//获取费率信息
//				pric_info =  GetChargingInfo(DispControl.CurUserGun);
//				if(pric_info == NULL)
//				{
//					return FALSE;
//				}
//				//GetCurTime(&pric_info->EndTime);                         //记录结束计费时间
//				if(CARDCMD_STOPA == cmd){
//					card_info = GetGunCardInfo(GUN_A);
//				}else{
//					card_info = GetGunCardInfo(GUN_B);
//				}
//				memcpy((INT8U *)&ChargeRecodeInfo[DispControl.CurUserGun].StartTime,(INT8U *)&pric_info->StartTime,sizeof(pric_info->StartTime) );
//				memcpy((INT8U *)&ChargeRecodeInfo[DispControl.CurUserGun].EndTime,(INT8U *)&pric_info->EndTime,sizeof(pric_info->EndTime) );
//				ChargeRecodeInfo[DispControl.CurUserGun].TotMoney = pric_info->TotalBill;
//				ChargeRecodeInfo[DispControl.CurUserGun].TotPower = pric_info->TotalPower;
//				ChargeRecodeInfo[DispControl.CurUserGun].ChargeTime = pric_info->ChargeTime;
//				ChargeRecodeInfo[DispControl.CurUserGun].BeforeCardBalance = card_info->balance;
//				//写交易记录
//				RecodeControl.RecodeCurNum  = 0;
//				#warning "汇誉界面  调试临时屏蔽"
//				//写入交易记录条数
//				FlashOper.DataID = PARA_1200_ID;
//				FlashOper.Len = PARA_1200_FLLEN;
//				FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
//				FlashOper.RWChoose = FLASH_ORDER_WRITE;
//				if(APP_FlashOperation(&FlashOper) == FALSE)
//				{
//					printf("Writer RecodeCurNum  err");
//				}
//				APP_RWChargeRecode(RECODE_DISPOSE1(RecodeControl.RecodeCurNum%1000),FLASH_ORDER_WRITE,&ChargeRecodeInfo[DispControl.CurUserGun]);
				if(cmd == CARDCMD_STOPA)
				{
					pcurMenu =  GetCurMenu();
					if(pcurMenu == &HYMenu32)  //当前菜单为启动失败结算界面
					{
						ChargeRecode_Dispose(GUN_A);				//交易记录处理
						DispControl.StartIntTime[GUN_A] = OSTimeGet();
						DisplayCommonMenu(&HYMenu40,&HYMenu32);	//直接倒转到A枪结算界面
						DispShow_EndChargeDate(GUN_A);			//显示结算信息
					}
					else
					{
						DispControl.CardOver[GUN_A] = STATE_OVER;  //设置刷卡执行了、
						if(DispControl.StopOver[GUN_A] ==  STATE_UNOVER)   //停止未执行
						{
							DisplayCommonMenu(&HYMenu35,NULL);	//跳转到停止中界面
							SendStopChargeMsg(GUN_A);   		//发送停止帧
							NB_WriterReason(GUN_A,"E61",3);
							BUG35CNU++;
						}
						else
						{
							//刷卡执行状态和停止停止状态清零
							DispControl.CardOver[GUN_A] = STATE_UNOVER;
							DispControl.StopOver[GUN_A] = STATE_UNOVER;
							ChargeRecode_Dispose(GUN_A);				//交易记录处理
							DispControl.StartIntTime[GUN_A] = OSTimeGet();
							DisplayCommonMenu(&HYMenu40,&HYMenu33);	//直接倒转到A枪结算界面
							DispShow_EndChargeDate(GUN_A);			//显示结算信息
						}
					}
//					SendElecLockMsg(GUN_A,SET_ELECUNLOCK);
				}
				else
				{
					pcurMenu =  GetCurMenu();
					if(pcurMenu == &HYMenu32)  //当前菜单为启动失败结算界面
					{
						ChargeRecode_Dispose(GUN_B);				//交易记录处理
						DispControl.StartIntTime[GUN_B] = OSTimeGet();
						DisplayCommonMenu(&HYMenu41,&HYMenu32);	//直接倒转到B枪结算界面
						DispShow_EndChargeDate(GUN_B);			//显示结算信息
					}
					else
					{
						DispControl.CardOver[GUN_B] = STATE_OVER;  //设置刷卡执行了、
						if(DispControl.StopOver[GUN_B] ==  STATE_UNOVER)   //停止未执行
						{
							DisplayCommonMenu(&HYMenu35,NULL);	//跳转到停止中界面
							NB_WriterReason(GUN_B,"E60",3);
							SendStopChargeMsg(GUN_B);   		//发送停止帧
							BUG35CNU++;
						}
						else
						{
							//刷卡执行状态和停止停止状态清零
							DispControl.CardOver[GUN_B] = STATE_UNOVER;
							DispControl.StopOver[GUN_B] = STATE_UNOVER;
							ChargeRecode_Dispose(GUN_B);				//交易记录处理
							DispControl.StartIntTime[GUN_B] = OSTimeGet();
							DisplayCommonMenu(&HYMenu41,&HYMenu33);	//直接倒转到A枪结算界面
							DispShow_EndChargeDate(GUN_B);			//显示结算信息
						}
					}
				}
				break;
			case _FailRtu:   		//失败不处理
			case _DataOverTime:
				//提示无法识别此卡
				pcurMenu =  GetCurMenu();
				//DisplayCommonMenu(&gsMenu24,NULL);	//提示无法识别此卡
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu28,pcurMenu);	//提示读卡超时
#else
				DisplayCommonMenu(&HYMenu28,pcurMenu);	//提示读卡超时
#endif
				break;
			case _NOStartCard:  	//卡号不一致
				//DisplayCommonMenu(&gsMenu55,NULL);      //调转到卡号不一致界面
				pcurMenu =  GetCurMenu();
				DisplayCommonMenu(&HYMenu34,pcurMenu);      //调转到卡号不一致界面
				break;
			default:
				break;
		}
	}
	if(cmd == CARDCMD_QUERY)
	{

		DisplayCommonMenu(&HYMenu6,NULL);      //电卡信息界面

		switch (rtu)
		{
			case _SuccessRtu:
				puser_card_info = GetCardInfo();
				if(puser_card_info == NULL)
				{
					return FALSE;
				}
				//显示卡信息
				CardID = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) | (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
				PrintNum32uVariable(DIS_ADD(HYMenu6.FrameID,0),CardID);
				PrintNum32uVariable(DIS_ADD(HYMenu6.FrameID,2),puser_card_info->balance);
				if(puser_card_info->lockstate == 0xff)
				{
					Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_LOCK,RED);   //未结算
				}
//				else if(puser_card_info->lockstate == 0x55){
//				Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_PERLLOCK,RED); //补充结算
//				}
				else
				{
					Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_UNLOCK,RED); //已结算
				}
				DisplayCommonMenu(&HYMenu6,NULL);      //电卡信息界面
				break;
			case _FailPIN:   //密码错误
				//跳转到密码输入错误界面
				//获取卡信息
				//显示剩余密码输入次数
				//DisplayCommonMenu(&gsMenu21,NULL);  //跳转到面输入错误界面
				DisplayCommonMenu(&HYMenu24,&HYMenu3);  //跳转到面输入错误界面
				break;
			default:
				//提示无法识别此卡
				//DisplayCommonMenu(&gsMenu24,NULL);	//提示无法识别此卡
				DisplayCommonMenu(&HYMenu27,&HYMenu3);	//提示无法识别此卡
				break;

		}
	}
	SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
	return TRUE;
}

/*****************************************************************************
* Function     : APP_RecvCardAnalyze
* Description  : 解析来自刷卡任务的消息
* Input        :_BSP_MESSAGE *pMsg
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U APP_RecvCardAnalyze(_BSP_MESSAGE *pMsg)
{
	USERCARDINFO* puser_card_info;
	INT32U CardNum;
	INT8U Card[8] = {0};
	INT8U * pdata;
	INT8U num,i;
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	if((pMsg == NULL) || (pMsg->pData == NULL) )
	{
		return FALSE;
	}
	switch(pMsg->DivNum)
	{
		case CARDCMD_QUERY:		      //查询卡内信息
			APP_RecvCardDispose((_SEND_TO_CARD_CMD)pMsg->DivNum,(FRAMERTU)(*pMsg->pData));
			break;
		case CARDCMD_STARTA:		  //A开始充电刷卡
		case CARDCMD_STOPA:
			if(DispControl.CurUserGun != GUN_A)
			{
				return FALSE;
			}
			APP_RecvCardDispose((_SEND_TO_CARD_CMD)pMsg->DivNum,(FRAMERTU)(*pMsg->pData));
			break;
		case CARDCMD_STARTB:		  //b开始充电刷卡
		case CARDCMD_STOPB:				//b停止充电刷卡
			if(DispControl.CurUserGun != GUN_B)
			{
				return FALSE;
			}
			APP_RecvCardDispose((_SEND_TO_CARD_CMD)pMsg->DivNum,(FRAMERTU)(*pMsg->pData));
			break;
		case NETCARDCMD_STARTA:		  //A枪网络开始刷卡
			//发送卡鉴权
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(DispControl.NetGunState[GUN_A] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
					return FALSE;
				}
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_A].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_A].DivNum = APP_CARD_INFO;
					SendMsg[GUN_A].DataLen =GUN_A;
					OSQPost(pevent, &SendMsg[GUN_A]);
				}
				else
				{
					//判断是否为白名单
					pdata = APP_GetCARDWL();
					num = pdata[0];	//卡白名单个数
					if(pdata == NULL)
					{
						return FALSE;
					}
					puser_card_info = GetGunCardInfo(GUN_A); //获取卡号
					CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
					          (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);

					Card[0] = HEXtoBCD(CardNum / 100000000);
					Card[1] = HEXtoBCD(CardNum % 100000000 /1000000);
					Card[2] = HEXtoBCD(CardNum % 1000000 /10000);
					Card[3] = HEXtoBCD(CardNum % 10000 /100);
					Card[4] = HEXtoBCD(CardNum % 100 /1);
					for(i = 0; i < num; i++)
					{
						if(CmpNBuf(&pdata[1+i*8],Card,8) )
						{
							_4G_SetStartType(GUN_A,_4G_APP_CARD);			//设置为卡启动
							APP_SetStartNetState(GUN_A,NET_STATE_OFFLINE);	//离线充电
							Net_StartCharge(GUN_A,MODE_AUTO);				//卡也为自动启动方式
							break;
						}
					}
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
			}
			if((SYSSet.NetYXSelct == XY_ZSH)||SYSSet.NetYXSelct == XY_HY||  (SYSSet.NetYXSelct == XY_YL2)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if(DispControl.NetGunState[GUN_A] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
					return FALSE;
				}
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_A].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_A].DivNum = APP_CARD_INFO;
					SendMsg[GUN_A].DataLen =GUN_A;
					OSQPost(pevent, &SendMsg[GUN_A]);
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
			}
			break;
		case NETCARDCMD_STARTB:		  //B枪网络开始刷卡
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(DispControl.NetGunState[GUN_B] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
					return FALSE;
				}
				//发送卡鉴权
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_B].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_B].DivNum = APP_CARD_INFO;
					SendMsg[GUN_B].DataLen =GUN_B;
					OSQPost(pevent, &SendMsg[GUN_B]);
				}
				else
				{
					//判断是否为白名单
					pdata = APP_GetCARDWL();
					num = pdata[0];	//卡白名单个数
					if(pdata == NULL)
					{
						return FALSE;
					}
					puser_card_info = GetGunCardInfo(GUN_B); //获取卡号
					CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
					          (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);

					Card[0] = HEXtoBCD(CardNum / 100000000);
					Card[1] = HEXtoBCD(CardNum % 100000000 /1000000);
					Card[2] = HEXtoBCD(CardNum % 1000000 /10000);
					Card[3] = HEXtoBCD(CardNum % 10000 /100);
					Card[4] = HEXtoBCD(CardNum % 100 /1);
					for(i = 0; i < num; i++)
					{
						if(CmpNBuf(&pdata[1+i*8],Card,8) )
						{
							_4G_SetStartType(GUN_B,_4G_APP_CARD);			//设置为卡启动
							APP_SetStartNetState(GUN_B,NET_STATE_OFFLINE);	//离线充电
							Net_StartCharge(GUN_B,MODE_AUTO);				//卡也为自动启动方式
							break;
						}
					}
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
			}

			if((SYSSet.NetYXSelct == XY_ZSH)||SYSSet.NetYXSelct == XY_HY||  (SYSSet.NetYXSelct == XY_YL2)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG) || (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if(DispControl.NetGunState[GUN_B] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
					return FALSE;
				}
				//发送卡鉴权
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_B].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_B].DivNum = APP_CARD_INFO;
					SendMsg[GUN_B].DataLen =GUN_B;
					OSQPost(pevent, &SendMsg[GUN_B]);
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
			}
			break;
		case NETCARDCMD_STOPA:		 //A枪网络停止充电
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if((FRAMERTU)(*pMsg->pData) == _SuccessRtu)
				{
					NB_WriterReason(GUN_A,"E54",3);
					Net_StopCharge(GUN_A);   //停止充电
				}
				SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
			}
			break;
		case NETCARDCMD_STOPB:		 //B枪网络停止充电
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if((FRAMERTU)(*pMsg->pData) == _SuccessRtu)
				{
					NB_WriterReason(GUN_B,"E53",3);
					Net_StopCharge(GUN_B); //停止充电
				}
				SetCardInfoStatus(_Already_Obtain);			//界面调转后使卡处于空闲状态
				break;
			}
		default:
			break;
	}
	return TRUE;
}

/*****************************************************************************
* Function	   : APP_RecvConectCCUAnalyze
* Description  : 解析来自连接ccu任务的消息
* Input 	   :_BSP_MESSAGE *pMsg
* Output	   : None
* Return	   :
* Note(s)	   :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U APP_RecvConectCCUAnalyze(_BSP_MESSAGE *pMsg)
{
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	INT8U gun;
	INT8U state;
	gun = pMsg->GunNum;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if(SYSSet.NetState == DISP_NET)
	{
		switch(pMsg->DivNum)
		{
			case APP_CHARGE_FAIL:            //启动失败
				DispControl.NetSSTState[gun] = NET_STARTFAIL;
				state = GetStartFailType(gun);
				ChargeRecodeInfo[gun].StopChargeReason = state | 0x80;		//表示启动失败
				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_PLATFORM;				//运营平台启动
				GetCurTime(&GunBillInfo[gun].EndTime);                         //记录结束计费时间
				memcpy(&ChargeRecodeInfo[gun].EndTime,&GunBillInfo[gun].EndTime,sizeof(_BSPRTC_TIME) );

				//启动失败费用肯定为0
				GunBillInfo[gun].TotalBill = 0;
				GunBillInfo[gun].TotalPower4 = 0;
				SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//通知停止计费
				ChargeRecode_Dispose((_GUN_NUM)gun);			//交易记录处理
				if(SYSSet.NetYXSelct == XY_66)
				{
					//发送开始充电应答
					SendMsg[gun].MsgID = BSP_MSGID_DISP;
					SendMsg[gun].DivNum = APP_START_ACK;
					SendMsg[gun].DataLen = gun;
					OSQPost(pevent, &SendMsg[gun]);
				}
				else
				{
					//发送开始充电应答
					SendMsg[gun].MsgID = BSP_MSGID_DISP;
					SendMsg[gun].DivNum = APP_STOP_BILL;
					SendMsg[gun].DataLen = gun;
					OSQPost(pevent, &SendMsg[gun]);
				}
				if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
				{
					_4G_SetStartType(gun,_4G_APP_START);			//设置为app启动
				}
				break;
			case APP_CHARGE_SUCCESS:         //启动成功
				DispControl.NetSSTState[gun] = NET_IDLE;
				DispControl.NetGunState[gun] = GUN_CHARGEING;
				memset(&ChargeingInfo[gun],0,sizeof(_CHARGEING_INFO));
				//ChargeRecodeInfo[gun].StopChargeReason =state | 0x80;
				ChargeRecodeInfo[gun].StopChargeReason  = 0;
				APP_Set_ERR_Branch(gun,STOP_ERR_NONE);
				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_PLATFORM;				//运营平台启动
				if(SYSSet.NetState == DISP_NET)
					WriterFmBill((_GUN_NUM)gun,1);
				//发送开始充电应答
				if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
				{
					if((SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2) || (SYSSet.NetYXSelct == XY_66))
					{

						SendMsg[gun].MsgID = BSP_MSGID_DISP;
						SendMsg[gun].DivNum = APP_START_ACK;
						SendMsg[gun].DataLen = gun;
						OSQPost(pevent, &SendMsg[gun]);
					}
//					#if(APP_USER_TYPE ==  APP_USER_YKC)
//					SendMsg[gun].MsgID = BSP_MSGID_DISP;
//					SendMsg[gun].DivNum = APP_SJDATA_QUERY;   //发送实时数据，状态发生了改变
//					SendMsg[gun].DataLen = gun;
//					OSQPost(pevent, &SendMsg[gun]);
//					#endif
					if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
					{
						SendMsg[gun].MsgID = BSP_MSGID_DISP;
						SendMsg[gun].DivNum = APP_VIN_INFO_START;
						SendMsg[gun].DataLen = gun;
						OSQPost(pevent, &SendMsg[gun]);
					}
				}
				break;
			case APP_CHARGE_END:
				if(DispControl.StopOverFrame[gun] == STATE_UNOVER) //不加保护可能存在执行多个停止完成，导致界面跳转错误
				{
					SendMsg[gun].DivNum = APP_STOP_BILL;
					DispControl.NetSSTState[gun] = NET_IDLE;
					DispControl.NetGunState[gun] = GUN_IDLE;
					DispControl.StartIntTime[gun] = OSTimeGet();
					DispControl.StopOverFrame[gun] = STATE_OVER;
					ChargeRecodeInfo[gun].BillingStatus = RECODE_SETTLEMENT;		//已经结算

					ChargeRecodeInfo[gun].StopChargeReason = APP_Get_ERR_Branch(gun);


					if(ChargeRecodeInfo[gun].StopChargeReason == STOP_HANDERR)
					{
						//有些车子充电充到百分之90多  就自动断开枪   20210720
						if(BMS_BCS_Context[gun].SOC > 90)
						{
							ChargeRecodeInfo[gun].StopChargeReason = STOP_ERR_NONE;
						}
					}
					SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//通知停止计费
					GetCurTime(&GunBillInfo[gun].EndTime);                         //记录结束计费时间

					memset(&ChargeingInfo[gun],0,sizeof(_CHARGEING_INFO));	//防止再次进入充电界面显示很大的数据
					ChargeRecode_Dispose((_GUN_NUM)gun);								//交易记录处理


					SendMsg[gun].MsgID = BSP_MSGID_DISP;
					SendMsg[gun].DataLen = gun;
					if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
					{
						if((VinControl[gun].VinState ==  VIN_FAIL) && (_4G_GetStartType(gun) == _4G_APP_VIN) )  //汇誉鉴权失败不发送订单
						{
							_4G_SetStartType(gun,_4G_APP_START);			//设置为app启动
							break;
						}
						_4G_SetStartType(gun,_4G_APP_START);			//设置为app启动
					}
					OSQPost(pevent, &SendMsg[gun]);
				}
				break;

			default:
				break;
		}

	}
	else
	{
		ST_Menu* pcur = GetCurMenu();

		if(pMsg == NULL || DispControl.CurUserGun >= GUN_MAX)
		{
			return FALSE;
		}

		if((pcur == &HYMenu50) || (pcur == &HYMenu51) || (pcur == &HYMenu57)  || (pcur == &HYMenu58) || (pcur == &HYMenu59) )  //手动模式下或校准模式下当前页面无需跳转
		{
			return FALSE;
		}

		switch(pMsg->DivNum)
		{
			case APP_CHARGE_FAIL:            //启动失败
				DispControl.StopOverFrame[gun] = STATE_UNOVER;  //停止完成帧未处理
				state = GetStartFailType(gun);
				ChargeRecodeInfo[gun].StopChargeReason = state | 0x80;		//表示启动失败
				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_DEVICE;				//目前启动方式都是充电机启动
				GetCurTime(&GunBillInfo[gun].EndTime);                         //记录结束计费时间
				memcpy(&ChargeRecodeInfo[gun].EndTime,&GunBillInfo[gun].EndTime,sizeof(_BSPRTC_TIME) );
				//Dis_ShowStartErr(ADDR_MENU70_SHOW,(INT8U)StartOver_Info[DispControl.CurUserGun].startfailreason);//显示启动失败原因
				DisplayCommonMenu(&HYMenu32,NULL);		//启动失败界面
				if(pMsg->DivNum == APP_CHARGE_FAIL)
				{
					Dis_ShowStatus(DIS_ADD(HYMenu32.FrameID,0),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
				}
				else
				{

					Dis_ShowStatus(DIS_ADD(HYMenu32.FrameID,0),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
				}
				SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//通知停止计费
				break;
			case APP_CHARGE_SUCCESS:         //启动成功
				APP_Set_ERR_Branch(gun,STOP_ERR_NONE);
				memset(&ChargeingInfo[GUN_A],0,sizeof(_CHARGEING_INFO));
				memset(&ChargeingInfo[GUN_B],0,sizeof(_CHARGEING_INFO));

				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_DEVICE;				//目前启动方式都是充电机启动
				//ChargeRecodeInfo[DispControl.CurUserGun].ChargeType = Start_Info[DispControl.CurUserGun].modetype;  //充电方式

				if(gun == GUN_A)
				{
					//DisplayCommonMenu(&gsMenu44,NULL);		//调转到A枪充电界面
					HYDisplayMenu36();						//防止数据显示的是之前的数据
					DisplayCommonMenu(&HYMenu36,NULL);		//调转到A枪充电界面
				}
				else
				{
					//DisplayCommonMenu(&gsMenu45,NULL);		//调转到b枪充电界面
					HYDisplayMenu37();						//防止数据显示的是之前的数据
					DisplayCommonMenu(&HYMenu37,NULL);		//调转到A枪充电界面
				}

				break;
				//停止成功，可能CCU先发送过来 不能使用DispControl.CurUserGun
			case APP_CHARGE_END:
				if(DispControl.StopOverFrame[gun] == STATE_UNOVER) //不加保护可能存在执行多个停止完成，导致界面跳转错误
				{
					DispControl.StopOverFrame[gun] = STATE_OVER;
					//ChargeRecodeInfo[DispControl.CurUserGun].SOC = StopOver_Info[DispControl.CurUserGun].EndChargeSOC;
					ChargeRecodeInfo[gun].StopChargeReason = APP_Get_ERR_Branch(gun);
					SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//通知停止计费
					GetCurTime(&GunBillInfo[gun].EndTime);                         //记录结束计费时间
					//需要判断是主动停止还是被动停止
					if(gun == GUN_A)
					{
						memset(&ChargeingInfo[GUN_A],0,sizeof(_CHARGEING_INFO));	//防止再次进入充电界面显示很大的数据
						//计费单元主动停止

						DispControl.StopOver[GUN_A] = STATE_OVER;  //设置停止执行了、
						if(DispControl.CardOver[GUN_A] ==  STATE_UNOVER)   //刷卡未执行
						{
							DispControl.ClickGunNextPage[GUN_A] = &HYMenu33;
							if((pcur == &HYMenu36) || (pcur == &HYMenu30))	//在充电中界面或者再启动中界面
							{
								DisplayCommonMenu(&HYMenu33,NULL);      //跳转到刷卡结算界面
							}
						}
						else
						{
							//CCU停止
							//刷卡执行状态和停止停止状态清零
							DispControl.CardOver[GUN_A] = STATE_UNOVER;
							DispControl.StopOver[GUN_A] = STATE_UNOVER;

							//可能存在扣的钱和时间显示不一样（原因，先刷卡停止，这个过程中产生了费用）
							if((GunBillInfo[GUN_A].TotalBill >= GunBillInfo[GUN_A].RealTotalBill) \
							        && (GunBillInfo[GUN_A].TotalBill <= (GunBillInfo[GUN_A].RealTotalBill + 10000)))
							{
								//在一元之内
								GunBillInfo[GUN_A].TotalBill = GunBillInfo[GUN_A].RealTotalBill;
								GunBillInfo[GUN_A].TotalPower4 = GunBillInfo[GUN_A].RealTotalPower;
							}
							ChargeRecode_Dispose(GUN_A);				//交易记录处理
							DisplayCommonMenu(&HYMenu40,&HYMenu35);      //跳转到结算界面
							DispShow_EndChargeDate(GUN_A);
							DispControl.StartIntTime[GUN_A] = OSTimeGet();

						}
					}
					else
					{
						memset(&ChargeingInfo[GUN_B],0,sizeof(_CHARGEING_INFO));	//防止再次进入充电界面显示很大的数据

						DispControl.StopOver[GUN_B] = STATE_OVER;  //设置停止执行了、
						if(DispControl.CardOver[GUN_B] ==  STATE_UNOVER)   //刷卡未执行
						{
							DispControl.ClickGunNextPage[GUN_B] = &HYMenu33;
							if((pcur == &HYMenu37) || (pcur == &HYMenu31))	//在充电中界面或者再启动中界面
							{
								DisplayCommonMenu(&HYMenu33,NULL);      //跳转到刷卡结算界面
							}
						}
						else
						{
							//CCU停止
							//刷卡执行状态和停止停止状态清零
							DispControl.CardOver[GUN_B] = STATE_UNOVER;
							DispControl.StopOver[GUN_B] = STATE_UNOVER;


							//可能存在扣的钱和时间显示不一样（原因，先刷卡停止，这个过程中产生了费用）
							if((GunBillInfo[GUN_B].TotalBill >= GunBillInfo[GUN_B].RealTotalBill) \
							        && (GunBillInfo[GUN_B].TotalBill <= (GunBillInfo[GUN_B].RealTotalBill + 10000)))
							{
								//在一元之内
								GunBillInfo[GUN_B].TotalBill = GunBillInfo[GUN_B].RealTotalBill;
								GunBillInfo[GUN_B].TotalPower4 = GunBillInfo[GUN_B].RealTotalPower;
							}

							ChargeRecode_Dispose(GUN_B);				//交易记录处理
							DisplayCommonMenu(&HYMenu41,&HYMenu35);      //跳转到结算界面
							DispShow_EndChargeDate(GUN_B);
							DispControl.StartIntTime[GUN_B] = OSTimeGet();
						}
					}
				}

				//Dis_ShowStartErr(ADDR_MENU48_SHOW,0);	//不显示显示启动失败原因
				break;
			default:
				break;
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Display_ShowSet1
* Description  : 显示费率设置 		一共12个 entrn 0表示第一次进入   1表示上一页   2表示下一页
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_PricSet(INT8U entrn)
{

	ST_Menu * ppage;
	_FLASH_OPERATION  FlashOper;
	static INT8U i;
	INT8U num;
	INT8U starthour[TIME_PERIOD_MAX],startminute[TIME_PERIOD_MAX];
	INT8U stophour[TIME_PERIOD_MAX],stopminute[TIME_PERIOD_MAX];
	//读取费率
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = MIN(PARA_PRICALL_FLLEN,sizeof(PriceSet) );
	FlashOper.ptr = (INT8U *)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
		return;
	}

	if(GetCurMenu() !=  &HYMenu48)   //不在配置界面处理
	{
		if(entrn == 0)
		{
			i = 0;
		}
		else if(entrn == 1)   //上一页
		{
			if(i == 0)
			{
				if(SYSSet.NetState == DISP_NET)
				{
					ppage =  Disp_NetPageDispos();
					DisplayCommonMenu(ppage,NULL);
				}
				else
				{
					DisplayCommonMenu(&HYMenu3,NULL);
				}
				return;
			}
			else
			{
				i--;
			}
		}
		else
		{
			i++;  //下一页
		}
		if((PriceSet.TimeQuantumNum == 0) || (PriceSet.TimeQuantumNum > 48))
		{

			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			return;
		}
		num = (PriceSet.TimeQuantumNum - 1) / 4;
		if(i > num)
		{

			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			return;
		}
		else if(i == num)
		{
			DisplayCommonMenu(&HYMenu82,NULL);		//没有下一页
		}
		else
		{
			DisplayCommonMenu(&HYMenu83,NULL);		//有下一页
		}
	}

	starthour[0+i*4] = PriceSet.StartTime[0+i*4] / 60;
	startminute[0+i*4] = PriceSet.StartTime[0+i*4] % 60;
	starthour[1+i*4] = PriceSet.StartTime[1+i*4] / 60;
	startminute[1+i*4] = PriceSet.StartTime[1+i*4] % 60;
	starthour[2+i*4] = PriceSet.StartTime[2+i*4] / 60;
	startminute[2+i*4] = PriceSet.StartTime[2+i*4] % 60;
	starthour[3+i*4] = PriceSet.StartTime[3+i*4] / 60;
	startminute[3+i*4] = PriceSet.StartTime[3+i*4] % 60;

	stophour[3+i*4] = PriceSet.EndTime[3+i*4] / 60;
	stopminute[3+i*4] = PriceSet.EndTime[3+i*4] % 60;

	if(GetCurMenu() !=  &HYMenu48)   //不在配置界面处理
	{
		PrintNum16uVariable(DIS_ADD(82,0),starthour[0+i*4]);
		PrintNum16uVariable(DIS_ADD(82,1),startminute[0+i*4]);
		PrintNum16uVariable(DIS_ADD(82,2),starthour[1+i*4]);
		PrintNum16uVariable(DIS_ADD(82,3),startminute[1+i*4]);
		PrintNum16uVariable(DIS_ADD(82,4) ,starthour[2+i*4]);
		PrintNum16uVariable(DIS_ADD(82,5),startminute[2+i*4]);
		PrintNum16uVariable(DIS_ADD(82,6),starthour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(82,7),startminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(82,8),stophour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(82,9),stopminute[3+i*4]);


		PrintNum16uVariable(DIS_ADD(82,10),PriceSet.Price[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,11),PriceSet.Price[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,12),PriceSet.Price[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,13),PriceSet.Price[3+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,14),PriceSet.ServeFee[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,15),PriceSet.ServeFee[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,16),PriceSet.ServeFee[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,17),PriceSet.ServeFee[3+i*4]/1000);

		PrintNum16uVariable(DIS_ADD(83,0),starthour[0+i*4]);
		PrintNum16uVariable(DIS_ADD(83,1),startminute[0+i*4]);
		PrintNum16uVariable(DIS_ADD(83,2),starthour[1+i*4]);
		PrintNum16uVariable(DIS_ADD(83,3),startminute[1+i*4]);
		PrintNum16uVariable(DIS_ADD(83,4) ,starthour[2+i*4]);
		PrintNum16uVariable(DIS_ADD(83,5),startminute[2+i*4]);
		PrintNum16uVariable(DIS_ADD(83,6),starthour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(83,7),startminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(83,8),stophour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(83,9),stopminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(83,10),PriceSet.Price[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,11),PriceSet.Price[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,12),PriceSet.Price[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,13),PriceSet.Price[3+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,14),PriceSet.ServeFee[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,15),PriceSet.ServeFee[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,16),PriceSet.ServeFee[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,17),PriceSet.ServeFee[3+i*4]/1000);
	}
	else
	{
		//配置界面
		PrintNum16uVariable(DIS_ADD(48,0),starthour[0+i*4]);
		PrintNum16uVariable(DIS_ADD(48,1),startminute[0+i*4]);
		PrintNum16uVariable(DIS_ADD(48,2),starthour[1+i*4]);
		PrintNum16uVariable(DIS_ADD(48,3),startminute[1+i*4]);
		PrintNum16uVariable(DIS_ADD(48,4) ,starthour[2+i*4]);
		PrintNum16uVariable(DIS_ADD(48,5),startminute[2+i*4]);
		PrintNum16uVariable(DIS_ADD(48,6),starthour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(48,7),startminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(48,8),PriceSet.Price[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,9),PriceSet.Price[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,10),PriceSet.Price[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,11),PriceSet.Price[3+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,12),PriceSet.ServeFee[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,13),PriceSet.ServeFee[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,14),PriceSet.ServeFee[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,15),PriceSet.ServeFee[3+i*4]/1000);
	}
	//PrintNum16uVariable(ADDR_MENU81_PRC + 13,PriceSet.ViolaFee);
}

/*****************************************************************************
* Function     : Display_ShowSet1
* Description  : 初始化显示设置1 		(flash没写入暂时未作处理)
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowSet1(void)
{
	_SYS_MENU52_SET1 sys_set1;


	sys_set1.MacMaxVol = (((PresetValue.VolMaxOutput/10) & 0x00ff) << 8) | (((PresetValue.VolMaxOutput/10) & 0xff00) >> 8);
	sys_set1.MacMinVol = (((PresetValue.VolMinOutput/10) & 0x00ff) << 8) | (((PresetValue.VolMinOutput/10) & 0xff00) >> 8);
	sys_set1.MaxOutCur = ((PresetValue.CurMaxOutput & 0x00ff) << 8) | ((PresetValue.CurMaxOutput & 0xff00) >> 8);
	sys_set1.OutDcCur = ((PresetValue.Therold_Value.OverOutCurTherold & 0x00ff) << 8) | ( (PresetValue.Therold_Value.OverOutCurTherold & 0xff00) >> 8);
	sys_set1.OutDcVol = ((PresetValue.Therold_Value.OverOutVolTherold & 0x00ff) << 8) | ((PresetValue.Therold_Value.OverOutVolTherold & 0xff00) >> 8);
	sys_set1.UseModule = (PresetValue.PowerModelNumber << 8);
	sys_set1.UseGun = (SYSSet.SysSetNum.UseGun << 8);
	sys_set1.SOC = (SYSSet.SOCthreshold << 8);
	memcpy(sys_set1.DivNum,SYSSet.SysSetNum.DivNum,MIN(sizeof(sys_set1.DivNum),sizeof(SYSSet.SysSetNum.DivNum)) );
	memcpy(sys_set1.ProjectNum ,SYSSet.SysSetNum.ProjectNum,MIN(sizeof(sys_set1.ProjectNum),sizeof(SYSSet.SysSetNum.ProjectNum)) );
	PrintStr(ADDR_MENU52_CFG,(INT8U *)&sys_set1,sizeof(_SYS_MENU52_SET1));

}

/*****************************************************************************
* Function     : Display_ShowSet2
* Description  : 初始化显示设置2   		(flash没写入暂时未作处理)
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowSet2(void)
{
	PrintNum16uVariable(ADDR_MENU53_CFG,PresetValue.Therold_Value.OverACVolTherold);
	PrintNum16uVariable(ADDR_MENU53_CFG + 1,PresetValue.Therold_Value.UnderACVolTherold);
	PrintStr(ADDR_MENU53_CFG + 2,SYSSet.MeterAdd[GUN_A],12);
	PrintStr(ADDR_MENU53_CFG + 8, SYSSet.MeterAdd[GUN_B],12);
}



/*****************************************************************************
* Function     : Display_ShowSet2
* Description  : 初始化显示设置3
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowSet3(void)
{
//	SHOW_NET_DJ,
//	SHOW_NET_WL,
//	SHOW_LOCK_CLOSE,
//	SHOW_LOCK_OPEN,
//	SHOW_MODULE_YL_20KWH,
//	SHOW_MODULE_YL_20KW,
//	SHOW_MODULE_SH,
//	SHOW_MODULE_YL_15KWH,
//	SHOW_MODULE_YL_15KW,
	if(SYSSet.NetState == 0)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,0),SHOW_NET_DJ);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,0),SHOW_NET_WL);
	}
	if(SYSSet.LockState == 0)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,10),SHOW_LOCK_CLOSE);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,10),SHOW_LOCK_OPEN);
	}

	if(SYSSet.CurModule == 0)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_20KWH);
	}
	else if(SYSSet.CurModule == 1)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_20KW);
	}
	else if(SYSSet.CurModule == 2)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_SH);
	}
	else if(SYSSet.CurModule == 3)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_15KWH);
	}
	else if (SYSSet.CurModule == 4)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_15KW);
	}
	else if (SYSSet.CurModule == 5)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_TH_30KW);
	}
	else if (SYSSet.CurModule == 6)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_TH_40KW);
	}
	else if (SYSSet.CurModule == 9)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_20KW_1000V);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_30KW);
	}
}


/*****************************************************************************
* Function     : Display_ShowNet
* Description  : 显示网络协议选择
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowNet(void)
{
	INT16U port;
	INT16U NetNum;
	INT16U IP[4];
	INT8U i;
	INT8U buf[12];
	if(SYSSet.NetYXSelct == 0)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_HY);
	}
	else if(SYSSet.NetYXSelct == 1)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YKC);
	}
	else if(SYSSet.NetYXSelct == 2)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_AP);
	}
	else if(SYSSet.NetYXSelct == 3)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_XJ);
	}
	else if(SYSSet.NetYXSelct == 4)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL1);
	}
	else if(SYSSet.NetYXSelct == 5)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL2);
	}
	else if(SYSSet.NetYXSelct == 6)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL3);
	}
	else if(SYSSet.NetYXSelct == 7)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL4);
	}
	else if(SYSSet.NetYXSelct == 8)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_66);
	}
	else if(SYSSet.NetYXSelct == 9)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_JG);
	}
	else if(SYSSet.NetYXSelct == 10)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_TT);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_ZSH);
	}
	port = (SYSSet.Port & 0x00ff) << 8 | (SYSSet.Port & 0xff00) >> 8;
	NetNum = (SYSSet.NetNum & 0x00ff) << 8 | (SYSSet.NetNum & 0xff00) >> 8;
	for(i = 0; i < 4; i++)
	{
		IP[i] = (SYSSet.IP[i] & 0x00ff) << 8 | 0;
	}
	memcpy(buf,&port,sizeof(port));
	memcpy(&buf[2],IP,8);
	memcpy(&buf[10],&NetNum,sizeof(NetNum));
	PrintStr(DIS_ADD(HYMenu10.FrameID,0x11),buf ,sizeof(buf));
}

/*****************************************************************************
* Function     : Display_ParaInit
* Description  : 参数初始化
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
static void Display_ParaInit(void)
{
	INT8U MeterAdd[12] = {'a','a','a','a','a','a','a','a','a','a','a','a'};
	//INT8U MeterAdd[12] = {'0','0','0','0','0','0','0','0','0','0','0','1'};
	INT8U MeterAddA[12] = {'0','0','0','0','0','0','0','0','0','0','0','1'};
	INT8U MeterAddB[12] = {'0','0','0','0','0','0','0','0','0','0','0','2'};
	INT8U num = 0xFF;
	_FLASH_OPERATION  FlashOper;
	//初始状态下点击A枪或者B枪应该在充电方式选择界面
	DispControl.ClickGunNextPage[GUN_A]  = &HYMenu22;
	DispControl.ClickGunNextPage[GUN_B]  = &HYMenu22;
	DispControl.CurUserGun = GUN_A;		//默认使用A枪
	DispControl.GurLockGun = GUN_UNDEFIN;	//当前解锁抢号为未定义
	DispControl.CardOver[GUN_A] = STATE_UNOVER;
	DispControl.StopOver[GUN_A] = STATE_UNOVER;
	DispControl.CardOver[GUN_B] = STATE_UNOVER;
	DispControl.StopOver[GUN_B] = STATE_UNOVER;
	DispControl.StopOverFrame[GUN_A] = STATE_UNOVER;
	DispControl.StopOverFrame[GUN_B] = STATE_UNOVER;
	DispControl.SendStartCharge[GUN_A] = FALSE;
	DispControl.SendStartCharge[GUN_B] = FALSE;
	DispControl.CountDown = 0;			//初始化倒计时为0，不需要切换
	SetCurMenu(&HYMenu3);					//设置当前界面为主界面（主界面开始有按钮）
	memset(ChargeInfo,0,(sizeof(_CHARGE_INFO) * GUN_MAX) );
	//读取所有配置信息
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}

#if(USER_GUN == USER_SINGLE_GUN)
	SYSSet.SysSetNum.UseGun = 1;  //
#else
	SYSSet.SysSetNum.UseGun = 2;  //
#endif

	if((SYSSet.NetYXSelct ==0xFF)||(SYSSet.NetYXSelct >= XY_MAX))
	{
		SYSSet.NetYXSelct = XY_HY;  //
		memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
		SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
		SYSSet.NetNum = 1;
	}

	//网络个数，最多2个
	if((SYSSet.NetNum ==0xFF)||(SYSSet.NetNum > 2))
	{
		SYSSet.NetNum = 1;
	}

	if((SYSSet.SOCthreshold ==0xFF)||(SYSSet.SOCthreshold  == 0))
	{
		//SOC阀值
		SYSSet.SOCthreshold = 98;  //
		FlashOper.DataID = PARA_CFGALL_ID;
		FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
		FlashOper.ptr = (INT8U *)&SYSSet;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("Read SYS set err");
		}
	}

	if(SYSSet.MeterAdd[GUN_A][0] == 0xff)
	{
#if(USER_GUN == USER_SINGLE_GUN)
		memcpy(SYSSet.MeterAdd[GUN_A],MeterAdd,12);
#else
		memcpy(SYSSet.MeterAdd[GUN_A],MeterAddA,12);
		memcpy(SYSSet.MeterAdd[GUN_B],MeterAddB,12);
#endif
	}
	SYSSet.GunTemp = 0;
	if((SYSSet.NetState != DISP_NET) && (SYSSet.NetState != DISP_CARD))
	{
		SYSSet.NetState = DISP_CARD;  //单机
	}
	if(SYSSet.LockState == 0xff)
	{
		SYSSet.LockState = 1;  //常闭
	}
	if(SYSSet.CurModule == 0xff)
	{
		SYSSet.CurModule = 0;  //永联恒功率
	}
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}

	//读取费率
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = MIN(PARA_PRICALL_FLLEN,sizeof(PriceSet) );
	FlashOper.ptr = (INT8U *)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}





	memset((INT8U*)&RecodeControl,0,sizeof(_RECODE_CONTROL));
	//读取交易记录条数
	FlashOper.DataID = PARA_1200_ID;
	FlashOper.Len = PARA_1200_FLLEN;
	FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}



	if(RecodeControl.RecodeCurNum == 0xffffffff)      //20220721  主要是防止存储数据上电丢失
	{
		OSTimeDly(SYS_DELAY_100ms);
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("Read SYS set err");
		}
	}


	if(RecodeControl.RecodeCurNum == 0xffffffff)
	{
		RecodeControl.RecodeCurNum = 0;
		FlashOper.DataID = PARA_1200_ID;
		FlashOper.Len = PARA_1200_FLLEN;
		FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("Read SYS set err");
		}
	}

	//读取网络状态下离线交易记录个数
	num = APP_GetNetOFFLineRecodeNum();
	if(num > 100)
	{
		APP_SetNetOFFLineRecodeNum(0);
	}


	FlashOper.DataID = PARA_CARDWHITEL_ID;
	FlashOper.Len = PARA_CARDWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if((FlashCardVinWLBuf[0] == 0xff) && (FlashCardVinWLBuf[1] == 0xff) && (FlashCardVinWLBuf[2] == 0xff) && (FlashCardVinWLBuf[3] == 0xff) \
	        &&(FlashCardVinWLBuf[4] == 0xff) && (FlashCardVinWLBuf[5] == 0xff) && (FlashCardVinWLBuf[6] == 0xff) && (FlashCardVinWLBuf[7] == 0xff))
	{
		FlashCardVinWLBuf[0] = 0;  //卡个数
		FlashOper.DataID = PARA_CARDWHITEL_ID;
		FlashOper.Len = PARA_CARDWHITEL_FLLEN;
		FlashOper.ptr = FlashCardVinWLBuf;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
	}


	FlashOper.DataID = PARA_VINWHITEL_ID;
	FlashOper.Len = PARA_VINWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if((FlashCardVinWLBuf[0] == 0xff) && (FlashCardVinWLBuf[1] == 0xff) && (FlashCardVinWLBuf[2] == 0xff) && (FlashCardVinWLBuf[3] == 0xff) \
	        &&(FlashCardVinWLBuf[4] == 0xff) && (FlashCardVinWLBuf[5] == 0xff) && (FlashCardVinWLBuf[6] == 0xff) && (FlashCardVinWLBuf[7] == 0xff))
	{
		FlashCardVinWLBuf[0] = 0;   //VIN个数
		FlashOper.DataID = PARA_VINWHITEL_ID;
		FlashOper.Len = PARA_VINWHITEL_FLLEN;
		FlashOper.ptr = FlashCardVinWLBuf;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
	}

	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = PARA_PRICALL_FLLEN;
	FlashOper.ptr = (INT8U*)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if((PriceSet.StartTime[0] == 0xff) || (PriceSet.TimeQuantumNum == 0xff))
	{
		memset(&PriceSet,0,sizeof(PriceSet));
		PriceSet.StartTime[0] = 0;
		PriceSet.EndTime[0] = 0;
		PriceSet.Price[0] = 100000;
		PriceSet.CurTimeQuantum[0] = TIME_QUANTUM_J;
		PriceSet.ServeFee[0] = 0;
		PriceSet.TimeQuantumNum = 1;

		FlashOper.DataID = PARA_PRICALL_ID;
		FlashOper.Len = PARA_PRICALL_FLLEN;
		FlashOper.ptr = (INT8U*)&PriceSet;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
	}

	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
	{
		//云快充  当充电中修改费率，A枪的费率可能和B枪的费率不一样
		memcpy(&A_PriceSet,&PriceSet,sizeof(PriceSet));
		memcpy(&B_PriceSet,&PriceSet,sizeof(PriceSet));
	}



	//读取"CCU"储存信息
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);

	if((PresetValue.Therold_Value.OverACVolTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.OverACVolTherold==0))
	{
		//交流输入过压阀值
		PresetValue.Therold_Value.OverACVolTherold=AC_POWER_OVER_VAL;
	}
	if((PresetValue.Therold_Value.UnderACVolTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.UnderACVolTherold==0))
	{
		//交流输入欠压阀值
		PresetValue.Therold_Value.UnderACVolTherold=AC_POWER_UNDER_VAL;
	}
	if((PresetValue.Therold_Value.OverOutVolTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.OverOutVolTherold==0))
	{
		//输出电压阀值
		PresetValue.Therold_Value.OverOutVolTherold=THEROLD_VOL_OUTPUT;
	}
	if((PresetValue.Therold_Value.OverOutCurTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.OverOutCurTherold==0))
	{
		//输出电流阀值
		PresetValue.Therold_Value.OverOutCurTherold=THEROLD_CUR_OUTPUT;
	}
	//电流阈值国网测试的时候才有用，平时使用就设置为最大值   20220118 叶
	PresetValue.Therold_Value.OverOutCurTherold=THEROLD_CUR_OUTPUT;

	if((PresetValue.PowerModelNumber==0xFF)||(PresetValue.PowerModelNumber==0))
	{
		//模块数量
		PresetValue.PowerModelNumber=MODULE_MAX_NUM;
	}
	if((PresetValue.VolMaxOutput==0xFFFF)||(PresetValue.VolMaxOutput==0))
	{
		//最大输出电压
		PresetValue.VolMaxOutput=MAX_VOL_OUTPUT;
	}
	if((PresetValue.CurMaxOutput==0xFFFF)||(PresetValue.CurMaxOutput==0))
	{
		//最大输出电流
		PresetValue.CurMaxOutput=MAX_CUR_OUTPUT;
	}
	if((PresetValue.VolMinOutput==0xFFFF)||(PresetValue.VolMinOutput > 2000))
	{
		//最小输出电压为200V,保护一下
		PresetValue.VolMinOutput=MIN_VOL_OUTPUT;
	}

	if((PresetValue.ChargeType==0xFF)||(PresetValue.ChargeType==0))
	{
		//充电方式
		PresetValue.ChargeType=0x01;
	}
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);


	//绝缘检测校准值  其它的是非进口芯片
	FlashOper.DataID = PARA_JUST_ID;
	FlashOper.Len = PARA_JUST_FLLEN;
	FlashOper.ptr = (INT8U*)ADCJustInfo;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);


#if(USER_importand == 1)
	//非进口芯片
	ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1387;
	ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1834;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
	ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
	ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;
	ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1249;
	ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1788;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
	ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
	ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;
	ADCJustInfo[GUN_A].JYVolKB.line_X1 = 670;
	ADCJustInfo[GUN_A].JYVolKB.line_X2 = 1023;
	ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
	ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;

	ADCJustInfo[GUN_B].BatVoltKB.line_X1 = 1387;
	ADCJustInfo[GUN_B].BatVoltKB.line_X2 =1834;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y2 =50000;
	ADCJustInfo[GUN_B].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_Y2-ADCJustInfo[GUN_B].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_X2-ADCJustInfo[GUN_B].BatVoltKB.line_X1);
	ADCJustInfo[GUN_B].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_X2;
	ADCJustInfo[GUN_B].HeadOutKB.line_X1 = 1249;
	ADCJustInfo[GUN_B].HeadOutKB.line_X2 =1788;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y2 =600;
	ADCJustInfo[GUN_B].HeadOutKB.line_K =  (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_Y2-ADCJustInfo[GUN_B].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_X2-ADCJustInfo[GUN_B].HeadOutKB.line_X1);
	ADCJustInfo[GUN_B].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_X2;
	ADCJustInfo[GUN_B].JYVolKB.line_X1 = 670;
	ADCJustInfo[GUN_B].JYVolKB.line_X2 =1023;
	ADCJustInfo[GUN_B].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_B].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_B].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_Y2-ADCJustInfo[GUN_B].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_X2-ADCJustInfo[GUN_B].JYVolKB.line_X1);
	ADCJustInfo[GUN_B].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_B].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_B].JYVolKB.line_X2;
	ADCJustInfo[GUN_A].T1_0 = 1700; //0
	ADCJustInfo[GUN_A].T1_120 = 2215; //120
	ADCJustInfo[GUN_A].T2_0 = 1700;
	ADCJustInfo[GUN_A].T2_120 = 2215;
	ADCJustInfo[GUN_B].T1_0 = 1700;
	ADCJustInfo[GUN_B].T1_120 = 2215;
	ADCJustInfo[GUN_B].T2_0 = 1700;
	ADCJustInfo[GUN_B].T2_120 = 2215;
#else
	//=====进口芯片型号1200B
	ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1225;  //校准A枪工作电压
	ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1675;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
	ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
	ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;

	ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1065;  //校准A枪线电压
	ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1605;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
	ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
	ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;

	ADCJustInfo[GUN_A].JYVolKB.line_X1 = 517;  //校准绝缘检测电压
	ADCJustInfo[GUN_A].JYVolKB.line_X2 = 873;
	ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
	ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;

	ADCJustInfo[GUN_B].BatVoltKB.line_X1 = 1225;
	ADCJustInfo[GUN_B].BatVoltKB.line_X2 =1675;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y2 =50000;
	ADCJustInfo[GUN_B].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_Y2-ADCJustInfo[GUN_B].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_X2-ADCJustInfo[GUN_B].BatVoltKB.line_X1);
	ADCJustInfo[GUN_B].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_X2;
	ADCJustInfo[GUN_B].HeadOutKB.line_X1 = 1065;
	ADCJustInfo[GUN_B].HeadOutKB.line_X2 =1605;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y2 =600;
	ADCJustInfo[GUN_B].HeadOutKB.line_K =  (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_Y2-ADCJustInfo[GUN_B].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_X2-ADCJustInfo[GUN_B].HeadOutKB.line_X1);
	ADCJustInfo[GUN_B].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_X2;
	ADCJustInfo[GUN_B].JYVolKB.line_X1 = 517;
	ADCJustInfo[GUN_B].JYVolKB.line_X2 =873;
	ADCJustInfo[GUN_B].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_B].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_B].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_Y2-ADCJustInfo[GUN_B].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_X2-ADCJustInfo[GUN_B].JYVolKB.line_X1);
	ADCJustInfo[GUN_B].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_B].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_B].JYVolKB.line_X2;

	ADCJustInfo[GUN_A].T1_0 = 1700; //0
	ADCJustInfo[GUN_A].T1_120 = 2215; //120
	ADCJustInfo[GUN_A].T2_0 = 1700;
	ADCJustInfo[GUN_A].T2_120 = 2215;
	ADCJustInfo[GUN_B].T1_0 = 1700;
	ADCJustInfo[GUN_B].T1_120 = 2215;
	ADCJustInfo[GUN_B].T2_0 = 1700;
	ADCJustInfo[GUN_B].T2_120 = 2215;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);
#endif



//		#if(USER_GUN == USER_SINGLE_GUN)
//		if(ADCJustInfo[GUN_A].BatVoltKB.line_X1 == 0xFFFFFFFF)      //20220721  主要是防止存储数据上电丢失
//		{
////			//集成单枪进口2
//		ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1447;
//		ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1974;
//		ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
//		ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
//		ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
//		ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;
//		ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1289;
//		ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1939;
//		ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
//		ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
//		ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
//		ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;
//		ADCJustInfo[GUN_A].JYVolKB.line_X1 = 597;
//		ADCJustInfo[GUN_A].JYVolKB.line_X2 = 1019;
//		ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
//		ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
//		ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
//		ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;
//		ADCJustInfo[GUN_A].T1_0 = 2015; //0
//		ADCJustInfo[GUN_A].T1_120 = 2630; //120
//		ADCJustInfo[GUN_A].T2_0 = 2015;
//		ADCJustInfo[GUN_A].T2_120 = 2630;
//		ADCJustInfo[GUN_B].T1_0 = 2015;
//		ADCJustInfo[GUN_B].T1_120 = 2630;
//		ADCJustInfo[GUN_B].T2_0 = 2015;
//		ADCJustInfo[GUN_B].T2_120 = 2630;
//		FlashOper.RWChoose = FLASH_ORDER_WRITE;
//		APP_FlashOperation(&FlashOper);
//	}
//////
//#else

//	if(ADCJustInfo[GUN_A].BatVoltKB.line_X1 == 0xFFFFFFFF)      //20220721  主要是防止存储数据上电丢失
//	{


	//进口隔离芯片----双枪----原来
//	ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1225;  //校准A枪工作电压
//	ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1675;
//	ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
//	ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
//	ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
//	ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;

//	ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1065;  //校准A枪线电压
//	ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1605;
//	ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
//	ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
//	ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
//	ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;

//	ADCJustInfo[GUN_A].JYVolKB.line_X1 = 517;  //校准绝缘检测电压
//	ADCJustInfo[GUN_A].JYVolKB.line_X2 = 873;
//	ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
//	ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
//	ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
//	ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;

//	ADCJustInfo[GUN_B].BatVoltKB.line_X1 = 1225;
//	ADCJustInfo[GUN_B].BatVoltKB.line_X2 =1675;
//	ADCJustInfo[GUN_B].BatVoltKB.line_Y1 = 30000;
//	ADCJustInfo[GUN_B].BatVoltKB.line_Y2 =50000;
//	ADCJustInfo[GUN_B].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_Y2-ADCJustInfo[GUN_B].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_X2-ADCJustInfo[GUN_B].BatVoltKB.line_X1);
//	ADCJustInfo[GUN_B].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_X2;
//	ADCJustInfo[GUN_B].HeadOutKB.line_X1 = 1065;
//	ADCJustInfo[GUN_B].HeadOutKB.line_X2 =1605;
//	ADCJustInfo[GUN_B].HeadOutKB.line_Y1 = 400;
//	ADCJustInfo[GUN_B].HeadOutKB.line_Y2 =600;
//	ADCJustInfo[GUN_B].HeadOutKB.line_K =  (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_Y2-ADCJustInfo[GUN_B].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_X2-ADCJustInfo[GUN_B].HeadOutKB.line_X1);
//	ADCJustInfo[GUN_B].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_X2;
//	ADCJustInfo[GUN_B].JYVolKB.line_X1 = 517;
//	ADCJustInfo[GUN_B].JYVolKB.line_X2 =873;
//	ADCJustInfo[GUN_B].JYVolKB.line_Y1 = 15000;
//	ADCJustInfo[GUN_B].JYVolKB.line_Y2 = 25000;
//	ADCJustInfo[GUN_B].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_Y2-ADCJustInfo[GUN_B].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_X2-ADCJustInfo[GUN_B].JYVolKB.line_X1);
//	ADCJustInfo[GUN_B].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_B].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_B].JYVolKB.line_X2;

//	ADCJustInfo[GUN_A].T1_0 = 1700; //0
//	ADCJustInfo[GUN_A].T1_120 = 2215; //120
//	ADCJustInfo[GUN_A].T2_0 = 1700;
//	ADCJustInfo[GUN_A].T2_120 = 2215;
//	ADCJustInfo[GUN_B].T1_0 = 1700;
//	ADCJustInfo[GUN_B].T1_120 = 2215;
//	ADCJustInfo[GUN_B].T2_0 = 1700;
//	ADCJustInfo[GUN_B].T2_120 = 2215;
//	FlashOper.RWChoose = FLASH_ORDER_WRITE;
//	APP_FlashOperation(&FlashOper);

//	}



	if(SYSSet.NetYXSelct == XY_66)
	{
		memset(&SYSSet.SysSetNum.DivNum[10],0,6);
		memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
		memcpy(NetConfigInfo[SYSSet.NetYXSelct].pIp,"pile.coulomb-charging.com",strlen("pile.coulomb-charging.com"));
	}




	NetConfigInfo[SYSSet.NetYXSelct].NetNum =  SYSSet.NetNum;
	NetConfigInfo[SYSSet.NetYXSelct].port = SYSSet.Port;
	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);

//

//	SYSSet.NetYXSelct = XY_YKC;
//	SYSSet.NetState = DISP_NET;  //单机
//	SYSSet.NetYXSelct = XY_HY;
//	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",116,62,125,35);
//	char * dev_num = "2000000000000000";
//	memcpy(SYSSet.SysSetNum.DivNum,dev_num,sizeof(SYSSet.SysSetNum.DivNum));
}
/*****************************************************************************
* Function     : CountDownDispose
* Description  : 倒计时处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static void CountDownDispose(INT32U time)
{
	static INT16U i = 0;

	if(++i >= (SYS_DELAY_1s/time) )    //1s 钟倒计时-1
	{
		if(	DispControl.CountDown > 1) //直到倒计时见到1为止
		{
			DispControl.CountDown--;
			//显示倒计时
			PrintNum16uVariable(ADDR_COUNTDOWN_TIME,DispControl.CountDown);
		}
		i = 0;
	}
}

/*****************************************************************************
* Function     : Disp_ShowRTC
* Description  : 显示RTC
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U Disp_ShowRTC(void)
{
	_BSPRTC_TIME CurRTC;

	if(GetCurTime(&CurRTC))           			//获取系统RTC
	{
		Dis_ShowTime(ADDR_TIME,CurRTC);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Disp_ShowRTC
* Description  : 显示RTC
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U Disp_ShowGunT(void)
{
	INT16U T1A,T2A,T1B,T2B;

//	#define ADDR_GUNA_T1			(0x1D8A)					//所有界面的倒计时都用一个地址
//#define ADDR_GUNA_T2			(0x1D8B)					//所有界面的倒计时都用一个地址
//#define ADDR_GUNB_T1			(0x1D8C)					//所有界面的倒计时都用一个地址
//#define ADDR_GUNB_T2			(0x1D8D)					//所有界面的倒计时都用一个地址
	if(SYSSet.GunTemp == 0)
	{
		T1A = GetRM_GunT1Temp(GUN_A);
		T2A = GetRM_GunT2Temp(GUN_A);
		PrintNum16uVariable(ADDR_GUNA_T1,T1A/100);
		PrintNum16uVariable(ADDR_GUNA_T2,T2A/100);
#if(USER_GUN != USER_SINGLE_GUN)
		T1B = GetRM_GunT1Temp(GUN_B);
		T2B = GetRM_GunT2Temp(GUN_B);
		PrintNum16uVariable(ADDR_GUNB_T1,T1B/100);
		PrintNum16uVariable(ADDR_GUNB_T2,T2B/100);
#endif
	}


	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_ChargeingDate
* Description  : 将遥测数据拷贝和充电信息到充电界面显示数据的结构体中，
				 若在充电中，则每个5s发送给界面显示
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U DispShow_ChargeingDate(INT32U time)
{
	INT8U i = 0;
	static INT16U count[GUN_MAX];
	INT32U balance;
	USERINFO* pric_info;
	USERCARDINFO* card_info;

	for(i = 0; i < GUN_MAX; i++)
	{

		card_info = GetGunCardInfo((_GUN_NUM)i);
		pric_info = GetChargingInfo((_GUN_NUM) i);
		if(pric_info == NULL)
		{
			return FALSE;
		}
		ChargeingInfo[i].ChargeTime = (pric_info->ChargeTime & 0x00ff) << 8 | (pric_info->ChargeTime & 0xff00) >> 8;

		if(SYSSet.NetYXSelct == XY_66)
		{
			ChargeingInfo[i].ChargeMoney  = 0;		//66是后台计算费率界面都显示0
		}
		else
		{
			ChargeingInfo[i].ChargeMoney =( ( ((pric_info->TotalBill/100) & 0x000000ff) << 24) | ( ((pric_info->TotalBill/100) & 0x0000ff00) << 8) |\
			                                (((pric_info->TotalBill/100) & 0x00ff0000) >> 8) | ( ((pric_info->TotalBill/100) & 0xff000000) >> 24));
		}

		ChargeingInfo[i].ChargeEle = ( ((pric_info->TotalPower4/10) & 0x000000ff) << 24) | ( ((pric_info->TotalPower4/10) & 0x0000ff00) << 8) |\
		                             (((pric_info->TotalPower4/10) & 0x00ff0000) >> 8) | ( ((pric_info->TotalPower4/10) & 0xff000000) >> 24) ;
		//卡内余额缩小10倍（显示界面长度不够）
		if(SYSSet.NetState == DISP_CARD)
		{
			ChargeingInfo[i].CardMoney = ( ((card_info->balance/10) & 0x000000ff) << 24) | (((card_info->balance/10) & 0x0000ff00) << 8) |\
			                             (((card_info->balance/10) & 0x00ff0000) >> 8) | ( ((card_info->balance/10) & 0xff000000) >> 24) ;
		}
		else
		{
			balance = APP_GetNetMoney(i);
			ChargeingInfo[i].CardMoney = ( ((balance/10) & 0x000000ff) << 24) | (((balance/10) & 0x0000ff00) << 8) |\
			                             (((balance/10) & 0x00ff0000) >> 8) | ( ((balance/10) & 0xff000000) >> 24) ;
		}
		ChargeingInfo[i].ChargeSOC = (BMS_BCS_Context[i].SOC & 0x00ff) << 8 | 0x00;
		ChargeingInfo[i].ChargeVol = (PowerModuleInfo[i].OutputInfo.Vol & 0x00ff) << 8 | (PowerModuleInfo[i].OutputInfo.Vol & 0xff00) >> 8;
		ChargeingInfo[i].ChargeCur = ( PowerModuleInfo[i].OutputInfo.Cur & 0x00ff) << 8 | ( PowerModuleInfo[i].OutputInfo.Cur  & 0xff00) >> 8;
		if(ChargeingInfo[i].ChargeCur == 0XA00F)  //4000
		{
			ChargeingInfo[i].ChargeCur  = 0; //说明一开始数据没有传过来
		}
		//电池分类一共8中
		if(BMS_BRM_Context[i].BatteryType < 9)
		{
			ChargeingInfo[i].BatteryType = BMS_BRM_Context[i].BatteryType;		//电池类型
		}
		else
		{
			ChargeingInfo[i].BatteryType = 9;			//其它电池
		}

		if(APP_GetWorkState((_GUN_NUM)i) == WORK_CHARGE)							//在充电中每隔5s发送
		{
			if(++count[i] >= (SYS_DELAY_5s/time) )
			{
				//发送数据
				if(i == GUN_A)
				{
					//减去电池类型，电车类型单独显示
					PrintStr(DIS_ADD(HYMenu36.FrameID,0),(INT8U*)&ChargeingInfo[GUN_A],sizeof(_CHARGEING_INFO) - 1 );
					//显示电池类型,因电池类型从1开始，故显示基地址需要-1
					// Dis_ShowStatus(DIS_ADD(HYMenu36.FrameID,15),(_SHOW_NUM)(ChargeingInfo[GUN_A].BatteryType + (INT8U)SHOW_BATTERY_TYPE1 -1),WHITE);    //显示电池类型
				}
				else
				{
					PrintStr(DIS_ADD(HYMenu37.FrameID,0),(INT8U*)&ChargeingInfo[GUN_B] ,sizeof(_CHARGEING_INFO) - 1 );
					//显示电池类型,因电池类型从1开始，故显示基地址需要-1
					// Dis_ShowStatus(DIS_ADD(HYMenu37.FrameID,15),(_SHOW_NUM)(ChargeingInfo[GUN_B].BatteryType + (INT8U)SHOW_BATTERY_TYPE1 -1),WHITE);    //显示电池类型
				}
				count[i] = 0;
			}
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_CardDate
* Description  : 将遥测数据拷贝和充电信息到充电界面显示数据的结构体中，
				 若在充电中，则每个5s发送给界面显示
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U DispShow_CardDate(INT32U time)
{
	INT8U i = 0;
	static INT16U count[GUN_MAX];

	for(i = 0; i < GUN_MAX; i++)
	{
		CardInfo[i].bmsneedvolt =  (BMS_BCL_Context[i].DemandVol & 0x00ff) << 8 |  (BMS_BCL_Context[i].DemandVol & 0xff00) >> 8;
		CardInfo[i].bmsneedcurr =  ( (4000-BMS_BCL_Context[i].DemandCur) & 0x00ff) << 8 | ( (4000-BMS_BCL_Context[i].DemandCur) & 0xff00) >> 8;
		CardInfo[i].remaindertime = (BMS_BCS_Context[i].RemainderTime & 0x00ff) << 8 |   (BMS_BCS_Context[i].RemainderTime & 0xff00) >> 8;
		CardInfo[i].bmsMaxVoltage = ((BMS_BCP_Context[i].MaxVoltage) & 0x00ff) << 8 |  ((BMS_BCP_Context[i].MaxVoltage )& 0xff00) >> 8;
		CardInfo[i].bmsMaxcurrent =  ( (4000-BMS_BCP_Context[i].MaxCurrent) & 0x00ff) << 8 | ( (4000-BMS_BCP_Context[i].MaxCurrent) & 0xff00) >> 8;
		CardInfo[i].MaxTemprature =  ( ( BMS_BCP_Context[i].MaxTemprature - 50) & 0x00ff) << 8  | 0x00;
		CardInfo[i].unitbatterymaxvol = (BMS_BCP_Context[i].UnitBatteryMaxVol & 0x00ff) << 8 |  (BMS_BCP_Context[i].UnitBatteryMaxVol & 0xff00) >> 8;
		CardInfo[i].chargevolmeasureval =  ((BMS_BCS_Context[i].ChargeVolMeasureVal/10) & 0x00ff) << 8 |  (BMS_BCS_Context[i].ChargeVolMeasureVal & 0xff00) >> 8;
		if(APP_GetWorkState((_GUN_NUM)i) == WORK_CHARGE)							//在充电中每隔5s发送
		{
			if(++count[i] >= (SYS_DELAY_5s/time) )
			{
				//发送数据
				if(i == GUN_A)
				{
					PrintStr(DIS_ADD(HYMenu38.FrameID,0),(INT8U*)&CardInfo[GUN_A],sizeof(_CARD_INFO) );

				}
				else
				{
					PrintStr(DIS_ADD(HYMenu39.FrameID,0),(INT8U*)&CardInfo[GUN_B] ,sizeof(_CARD_INFO) );
				}
				count[i] = 0;
			}
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_EndChargeDate
* Description  : 结束充电显示数据
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U DispShow_EndChargeDate(_GUN_NUM gun)
{
	INT8U state;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	EndChargeInfo[gun].StartYear = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Year) << 8) | 0;
	EndChargeInfo[gun].StartMouth = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Month) << 8) | 0;
	EndChargeInfo[gun].StartDay = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Day) << 8) | 0;
	EndChargeInfo[gun].StartHour = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Hour) << 8) | 0;
	EndChargeInfo[gun].StartMinute = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Minute) << 8) | 0;
	EndChargeInfo[gun].StartSecond = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Second) << 8) | 0;
	EndChargeInfo[gun].StopYear = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Year) << 8) | 0;
	EndChargeInfo[gun].StopMouth = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Month) << 8) | 0;
	EndChargeInfo[gun].StopDay = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Day) << 8) | 0;
	EndChargeInfo[gun].StopHour = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Hour) << 8) | 0;
	EndChargeInfo[gun].StopMinute = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Minute) << 8) | 0;
	EndChargeInfo[gun].StopSecond = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Second) << 8) | 0;
	EndChargeInfo[gun].TotPower = ( (ChargeRecodeInfo[gun].TotPower & 0x000000ff) << 24) | ( (ChargeRecodeInfo[gun].TotPower & 0x0000ff00) << 8) |\
	                              ((ChargeRecodeInfo[gun].TotPower & 0x00ff0000) >> 8) | ( (ChargeRecodeInfo[gun].TotPower & 0xff000000) >> 24) ;
	EndChargeInfo[gun].TotMoney = ( (ChargeRecodeInfo[gun].TotMoney & 0x000000ff) << 24) | ( (ChargeRecodeInfo[gun].TotMoney & 0x0000ff00) << 8) |\
	                              ( (ChargeRecodeInfo[gun].TotMoney & 0x00ff0000) >> 8) | ( (ChargeRecodeInfo[gun].TotMoney & 0xff000000) >> 24);
	EndChargeInfo[gun].BeforeCardBalance = ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0x000000ff) << 24) | ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0x0000ff00) << 8) |\
	                                       ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0x00ff0000) >> 8) | ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0xff000000) >> 24);
	EndChargeInfo[gun].EndChargeReason = (INT8U)ChargeRecodeInfo[gun].StopChargeReason;

	state = GetStartFailType(gun);
	if(gun == GUN_A)
	{
		//需要减去停止原因，停止原因单独显示
		PrintStr(DIS_ADD(HYMenu40.FrameID,0),(INT8U*)&EndChargeInfo[GUN_A],sizeof(_END_CHARGE_INFO) - 1 );
		if(HYMenu40.Menu_PrePage == &HYMenu32)  //启动失败直接跳转过来
		{
			//显示启动失败原因
			Dis_ShowStatus(DIS_ADD(HYMenu40.FrameID,0x15),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
		}
		else
		{
			Dis_ShowStatus(DIS_ADD(HYMenu40.FrameID,0x15),(_SHOW_NUM)(EndChargeInfo[GUN_A].EndChargeReason+SHOW_STOP_ERR_NONE),RED);
		}

	}
	if(gun == GUN_B)
	{
		//需要减去停止原因，停止原因单独显示
		PrintStr(DIS_ADD(HYMenu41.FrameID,0),(INT8U*)&EndChargeInfo[GUN_B],sizeof(_END_CHARGE_INFO) - 1 );
		if(HYMenu41.Menu_PrePage == &HYMenu32)  //启动失败直接跳转过来
		{
			//显示启动失败原因
			Dis_ShowStatus(DIS_ADD(HYMenu41.FrameID,0x15),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
		}
		else
		{
			Dis_ShowStatus(DIS_ADD(HYMenu41.FrameID,0x15),(_SHOW_NUM)(EndChargeInfo[GUN_B].EndChargeReason+SHOW_STOP_ERR_NONE),RED);
		}
	}
	return FALSE;
}

/*****************************************************************************
* Function     : DispShow_NetState
* Description  : 显示网络状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U DispShow_NetState(void)
{

	static INT8U state = 0,laststate = 0xff;   //变化了才执行
	if(APP_GetNetState(0) == TRUE)
	{
		state = 0;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01E0,1);
		}
	}
	else
	{
		state = 1;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01E0,0);
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_CSQState
* Description  : 显示信号强度
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U DispShow_CSQState(void)
{

	static INT8U state = 0,laststate = 0xff;   //变化了才执行

	//CSQ 0~31 99
	//分5个等级
	if((APP_GetCSQNum() == 0) || (APP_GetCSQNum() == 99) )
	{

		state = 0;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,0);
		}
	}
	else if(APP_GetCSQNum() < 5)
	{
		state = 1;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,1);
		}
	}
	else if(APP_GetCSQNum() < 20)
	{
		state = 2;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,2);
		}
	}
	else if(APP_GetCSQNum() < 25)
	{
		state = 3;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,3);
		}
	}
	else
	{
		state = 4;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,4);
		}
	}
	return TRUE;
}


extern _MSP_DI MSPIo;
/*****************************************************************************
* Function     : DispShow_State
* Description  : 主界面显示A/B 枪状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U DispShow_State(INT32U time)
{
	INT8U i = 0,num;
	static INT16U count[GUN_MAX];
	static INT8U cnt[GUN_MAX] = {0,0};
	_SHOW_NUM show_err[GUN_MAX];
	static INT8U chargeicon[GUN_MAX] = {0},lastchargeicon[GUN_MAX] = {1}; //主界面充电图标是否显示 0:不显示  1:显示
	static INT32U failcount[GUN_MAX] = {0};
	static INT8U laststate[GUN_MAX] = {GUN_IDLE};
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	num = GUN_MAX;
	if((SYSSet.SysSetNum.UseGun <= GUN_MAX) && (SYSSet.SysSetNum.UseGun != 0) )
	{
		num = SYSSet.SysSetNum.UseGun;
	}
	for(i = 0; i < num; i++)
	{
		show_err[(_GUN_NUM)i] = SHOW_SYS_NULL;	//初始化不显示
		if((APP_GetErrState((_GUN_NUM)i) == 0x04) || (GetMeterStatus((_GUN_NUM)i) == _COM_FAILED)) 				//有故障
		{
			if(SYSSet.NetState == DISP_NET)
			{
				DispControl.NetGunState[(_GUN_NUM)i] = GUN_FAIL;
			}
			chargeicon[i] = 0;//不显示充电图标
			if (GetEmergencyState(i) == EMERGENCY_PRESSED)
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_EMERGENCY;  //急停
			}
#if ENBLE_JG
			else if (BSP_MPLSState(BSP_DI_DOOR)) //门禁
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_MJ;  //
			}
			else if(BSP_MPLSState(BSP_DI_W) == 0)  //水浸
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_SJ;
			}
			else if(BSP_MPLSState(BSP_DI_QX) == 1)  //倾斜
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_QX;
			}
			else if(MSPIo.SWDC_A)  //A枪直接接触器
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_ADC;
			}
			else if(MSPIo.SWDC_B)  //B枪直接接触器
			{

				show_err[(_GUN_NUM)i] = SHOW_SYS_BDC;
			}
			else if(MSPIo.QDC)  //桥接
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_QJ;
			}
#endif
			else
			{
				//CCU写的代码，这些故障拔枪后故障会消息，需要调试插拔枪
				//			SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 0);
				//			SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 5, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_3, 2, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 2, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 5, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 6, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 7, 0);
				//			SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_3, 7, 0);
				//	   		SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_3, 5, 0);
				if(((BCURemoteSignal[(_GUN_NUM)i].state2.byte & 0x30) != 0) || ((BCURemoteSignal[(_GUN_NUM)i].state3.byte & 0xA4) != 0) \
				        || ((BCURemoteSignal[(_GUN_NUM)i].state4.byte& 0xE4) != 0) )
				{
					show_err[(_GUN_NUM)i] = SHOW_SYS_AGAINGUN;	//再次插枪
				}
				else
				{
					show_err[(_GUN_NUM)i] = SHOW_SYS_OTHERFAIL;	//系统故障
				}
			}

		}
		else
		{
			if(SYSSet.NetState == DISP_NET)
			{
				if(DispControl.NetGunState[(_GUN_NUM)i] == GUN_FAIL)
				{
					DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
				}
			}
			if(APP_GetErrState((_GUN_NUM)i) == 0x01)  //正在充电
			{
				chargeicon[i] = 1;//显示充电图标
				show_err[(_GUN_NUM)i] = SHOW_SYS_CHARGE;
				if(SYSSet.NetState == DISP_NET)
				{
					DispControl.NetGunState[(_GUN_NUM)i] = GUN_CHARGEING;
				}

			}
			else
			{
				chargeicon[i] = 0;//不显示充电图标
				if(SYSSet.NetState == DISP_NET)
				{
					//启动失败
					//请插电枪
					if(GetGunState(i) == GUN_DISCONNECTED)
					{
						DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
						show_err[(_GUN_NUM)i] = SHOW_SYS_GUN;   //枪连接充电枪
						DispControl.NetSSTState[(_GUN_NUM)i]  = NET_IDLE;
					}
					else
					{
						//正在启动
						if(DispControl.NetSSTState[(_GUN_NUM)i] == NET_STARTING)
						{
							failcount[i] = 0;
							DispControl.NetGunState[(_GUN_NUM)i] = GUN_STARTING;
							show_err[(_GUN_NUM)i] = SHOW_SYS_START;   //正在启动
						}
						else if(DispControl.NetSSTState[(_GUN_NUM)i] == NET_STOPING)
						{
							if(failcount[i]++ >= (SYS_DELAY_10s/time) )   //连续10s显示停止中则为空闲
							{
								if(APP_GetErrState((_GUN_NUM)i) != 0x01)  //不在充电中
								{
									failcount[i] = 0;
									DispControl.NetSSTState[(_GUN_NUM)i] = NET_IDLE;
									DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
									show_err[(_GUN_NUM)i] = SHOW_SYS_NET;  //显示扫描充电
								}
							}
							else
							{
								DispControl.NetGunState[(_GUN_NUM)i] = GUN_STOPING;
								show_err[(_GUN_NUM)i] = SHOW_SYS_STOP;   //正在停止
							}
						}
						else if(DispControl.NetSSTState[(_GUN_NUM)i] == NET_STARTFAIL)
						{
							if(failcount[i]++ >= (SYS_DELAY_5s/time) )   //连续5s显示启动失败后则为空闲
							{
								failcount[i] = 0;
								DispControl.NetSSTState[(_GUN_NUM)i] = NET_IDLE;
								DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
								show_err[(_GUN_NUM)i] = SHOW_SYS_NET;  //显示扫描充电
							}
							else
							{
								DispControl.NetGunState[(_GUN_NUM)i] = GUN_STARTFAIL;
								show_err[(_GUN_NUM)i] = SHOW_SYS_FAIL;   //启动失败
							}
						}
						else
						{
							failcount[i] = 0;
							DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
							show_err[(_GUN_NUM)i] = SHOW_SYS_NET;  //显示扫描充电
						}
					}
				}
				else
				{
					if(i == GUN_A)
					{
						if((DispControl.ClickGunNextPage[GUN_A] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_A] == &HYMenu33))
						{
							show_err[(_GUN_NUM)i] = SHOW_SYS_CARD; //请刷卡结算
						}
					}

					if(i == GUN_B)
					{
						if((DispControl.ClickGunNextPage[GUN_B] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_B] == &HYMenu33))
						{
							show_err[(_GUN_NUM)i] = SHOW_SYS_CARD;	//请刷卡结算
						}
					}
				}
			}
		}

		if(++count[i] >= (SYS_DELAY_1s/time) )
		{
			count[i] = 0;
			(cnt[i] == 0)?(cnt[i] = 1):(cnt[i] = 0);
#if ENBLE_JG

			if((show_err[(_GUN_NUM)i] == SHOW_SYS_EMERGENCY) || (show_err[(_GUN_NUM)i] == SHOW_SYS_MJ)|| \
			        (show_err[(_GUN_NUM)i] == SHOW_SYS_SJ) || (show_err[(_GUN_NUM)i] == SHOW_SYS_ADC)||\
			        (show_err[(_GUN_NUM)i] == SHOW_SYS_BDC) || (show_err[(_GUN_NUM)i] == SHOW_SYS_QJ) || (show_err[(_GUN_NUM)i] == SHOW_SYS_PCUTIMEOUT)   )
#else
			if((show_err[(_GUN_NUM)i] == SHOW_SYS_EMERGENCY) || (show_err[(_GUN_NUM)i] == SHOW_SYS_PCUTIMEOUT)   )
#endif
			{
				//系统故障显示
				if(cnt[GUN_A] == 0)
				{
					Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x20),(_SHOW_NUM)show_err[(_GUN_NUM)i],RED);
				}
				else
				{
					Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x20),(_SHOW_NUM)show_err[(_GUN_NUM)i],BLACK);
				}
				Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x0),SHOW_SYS_NULL,RED);  //不显示
				Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x10),SHOW_SYS_NULL,RED);  //不显示
			}
			else
			{
				Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x20),SHOW_SYS_NULL,RED);  //不显示
				if(i == GUN_A)
				{
					if(cnt[GUN_A] == 0)
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0),(_SHOW_NUM)show_err[(_GUN_NUM)i],RED);
					}
					else
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0),(_SHOW_NUM)show_err[(_GUN_NUM)i],BLACK);
					}
				}
				if(i == GUN_B)
				{
					if(cnt[GUN_B] == 0)
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x10),(_SHOW_NUM)show_err[(_GUN_NUM)i],RED);
					}
					else
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x10),(_SHOW_NUM)show_err[(_GUN_NUM)i],BLACK);
					}
				}
			}
		}

		if(chargeicon[i] != lastchargeicon[i])
		{
			lastchargeicon[i] = chargeicon[i];
			if(GUN_A == i)
			{
				PrintIcon(0x01C0,chargeicon[i]);
			}
			else
			{
				PrintIcon(0x01D0,chargeicon[i]);
			}
		}
		DispShow_CSQState();
		DispShow_NetState();
	}

	if(((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC) ||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))  && (SYSSet.NetState == DISP_NET))
	{
		for(i = 0; i < GUN_MAX; i++)
		{
			if((laststate[i] != DispControl.NetGunState[(_GUN_NUM)i]) && \
			        ((DispControl.NetGunState[(_GUN_NUM)i] == GUN_IDLE) || (DispControl.NetGunState[(_GUN_NUM)i] == GUN_CHARGEING) \
			         ||(DispControl.NetGunState[(_GUN_NUM)i] == GUN_FAIL)))
			{
				SendMsg[i].MsgID = BSP_MSGID_DISP;
				SendMsg[i].DivNum = APP_SJDATA_QUERY;   //发送实时数据，状态发生了改变
				SendMsg[i].DataLen = i;
				OSQPost(pevent, &SendMsg[i]);
				laststate[i] = DispControl.NetGunState[(_GUN_NUM)i];
			}
		}
	}
	return TRUE;
}

/****************************************************************************
* Function     : 界面63，界面64显示记录
* Description  :
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U DispShow_Recode(_CHARGE_RECODE * precode,ST_Menu *pMenu)
{

	if((precode == NULL) || (pMenu == NULL) )
	{
		return FALSE;
	}
	memset(&RecodeMenu8_9Info,0,sizeof(RecodeMenu8_9Info));
	RecodeMenu8_9Info.BeforeCardBalance = ( (precode->BeforeCardBalance & 0x000000ff) << 24) | ( (precode->BeforeCardBalance & 0x0000ff00) << 8) |\
	                                      ((precode->BeforeCardBalance & 0x00ff0000) >> 8) | ( (precode->BeforeCardBalance & 0xff000000) >> 24) ;

	if(precode->BillingStatus == RECODE_SETTLEMENT) //显示已经结算
	{
		Dis_ShowCopy(RecodeMenu8_9Info.BillingStatus,SHOW_CARD_UNLOCK);
	}
	else if(precode->BillingStatus == RECODE_REPLSETTLEMENT)
	{
		Dis_ShowCopy(RecodeMenu8_9Info.BillingStatus,SHOW_CARD_PERLLOCK);//补充结算
		//Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_PERLLOCK,RED); //补充结算
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.BillingStatus,SHOW_CARD_LOCK);
	}
	memcpy(RecodeMenu8_9Info.TransNum,precode->TransNum,sizeof(precode->TransNum) );
	RecodeMenu8_9Info.TotPower =  ( (precode->TotPower & 0x000000ff) << 24) | ( (precode->TotPower & 0x0000ff00) << 8) |\
	                              ((precode->TotPower & 0x00ff0000) >> 8) | ( (precode->TotPower & 0xff000000) >> 24) ;
	if(precode->StopChargeReason >= 0x80)    //最高位为1表示启动失败停止；最高位为0表示启动成功停止
	{
		Dis_ShowCopy(RecodeMenu8_9Info.StopChargeReason,(_SHOW_NUM)(precode->StopChargeReason - 0x80 + SHOW_STARTEND_SUCCESS));
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.StopChargeReason,(_SHOW_NUM)(precode->StopChargeReason + SHOW_STOP_ERR_NONE));
	}
	RecodeMenu8_9Info.CardNum =  ( (precode->CardNum & 0x000000ff) << 24) | ( (precode->CardNum & 0x0000ff00) << 8) |\
	                             ((precode->CardNum & 0x00ff0000) >> 8) | ( (precode->CardNum & 0xff000000) >> 24) ;

	if(precode->StartType == TYPE_START_DEVICE)
	{
		Dis_ShowCopy(RecodeMenu8_9Info.ChargeType,SHOW_START_CARD); //显示都为刷卡启动
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.ChargeType,SHOW_START_APP); //显示为APP启动
	}
	//端口号
	if(precode->Gun == GUN_A)
	{
		Dis_ShowCopy(RecodeMenu8_9Info.Gunnum,SHOW_GUN_A);
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.Gunnum,SHOW_GUN_B);
	}
	RecodeMenu8_9Info.TotMoney =  ( (precode->TotMoney & 0x000000ff) << 24) | ( (precode->TotMoney & 0x0000ff00) << 8) |\
	                              ((precode->TotMoney & 0x00ff0000) >> 8) | ( (precode->TotMoney & 0xff000000) >> 24) ;
	memcpy(RecodeMenu8_9Info.CarVin,precode->CarVin,sizeof(precode->CarVin) );
	memcpy(RecodeMenu8_9Info.TransNum,precode->TransNum,sizeof(RecodeMenu8_9Info.TransNum) );

	RecodeMenu8_9Info.StartYear = (BCDtoHEX(precode->StartTime.Year) << 8) | 0;
	RecodeMenu8_9Info.StartMonth = (BCDtoHEX(precode->StartTime.Month) << 8) | 0;
	RecodeMenu8_9Info.StartDay = (BCDtoHEX(precode->StartTime.Day) << 8) | 0;
	RecodeMenu8_9Info.StartHour = (BCDtoHEX(precode->StartTime.Hour) << 8) | 0;
	RecodeMenu8_9Info.StartMinute = (BCDtoHEX(precode->StartTime.Minute) << 8) | 0;
	RecodeMenu8_9Info.StartSecond = (BCDtoHEX(precode->StartTime.Second) << 8) | 0;
	RecodeMenu8_9Info.EndYear = (BCDtoHEX(precode->EndTime.Year) << 8) | 0;
	RecodeMenu8_9Info.EndMonth = (BCDtoHEX(precode->EndTime.Month) << 8) | 0;
	RecodeMenu8_9Info.EndDay = (BCDtoHEX(precode->EndTime.Day) << 8) | 0;
	RecodeMenu8_9Info.EndHour = (BCDtoHEX(precode->EndTime.Hour) << 8) | 0;
	RecodeMenu8_9Info.EndMinute = (BCDtoHEX(precode->EndTime.Minute) << 8) | 0;
	RecodeMenu8_9Info.EndSecond = (BCDtoHEX(precode->EndTime.Second) << 8) | 0;

	memcpy(RecodeMenu8_9Info.StopReason,precode->StopReason,sizeof(RecodeMenu8_9Info.StopReason));


	//数据太大一次性发会有问题
	PrintStr(DIS_ADD(pMenu->FrameID,0),(INT8U*)&RecodeMenu8_9Info,34);
	PrintStr(DIS_ADD(pMenu->FrameID,17),(INT8U*)RecodeMenu8_9Info.StopChargeReason,34 );
	PrintStr(DIS_ADD(pMenu->FrameID,(34)),(INT8U*)RecodeMenu8_9Info.ChargeType,44);
	PrintStr(DIS_ADD(pMenu->FrameID,56),(INT8U*)RecodeMenu8_9Info.CarVin,42);
	PrintStr(DIS_ADD(pMenu->FrameID,77),(INT8U*)RecodeMenu8_9Info.StopReason,20);
	return TRUE;

}

/*****************************************************************************
* Function     : APP_ClearRecodeInfo
* Description  :清除记录信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_ClearRecodeInfo(void)
{
	RecodeControl.UpReadRecodeNun =0;
	RecodeControl.CurReadRecodeNun = 0;
	RecodeControl.NextReadRecodeNun = 0;
	RecodeControl.CurNun = 0;
	return TRUE;
}

/*****************************************************************************
* Function     : APP_SelectCurChargeRecode
* Description  :查询当前交易记录，并显示，第一次进入
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectCurChargeRecode(void)
{
	INT32U i =0;
	//交易记录最多1000条

	if(RecodeControl.RecodeCurNum == 0 )
	{
		DisplayCommonMenu(&HYMenu7,NULL);  //未找到充电记录
		return FALSE;
	}
	for(i = RecodeControl.RecodeCurNum; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum) ; i--)
	{
		//充最新的一条开始查找
		APP_RWChargeRecode( RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.CurRecode);
		//查询卡号是否一致
		//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.CurRecode.CardNum,6) == TRUE)
		{
			RecodeControl.CurReadRecodeNun = i;
			break;
		}
	}
	if(i ==  RECODE_DISPOSE2(RecodeControl.RecodeCurNum))
	{
		RecodeControl.CurReadRecodeNun = 0;
	}
	else
	{
		RecodeControl.CurNun++;  //页面+1
		//查询是否有下一条记录，决定页面跳转到8界面 还是9界面
		for(i = RecodeControl.CurReadRecodeNun - 1; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum); i--)
		{
			APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.NextRecode);
			//查询卡号是否一致
			//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.NextRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
			{
				RecodeControl.NextReadRecodeNun = i;
				break;
			}
		}
	}
	if(i ==  RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //下一页无
	{
		RecodeControl.NextReadRecodeNun = 0;
	}


	if(RecodeControl.CurReadRecodeNun  == 0)  //本卡无记录
	{
		DisplayCommonMenu(&HYMenu7,NULL);  //未能找到账户信息
	}
	else if(RecodeControl.NextReadRecodeNun == 0) //只有一条记录
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
		DisplayCommonMenu(&HYMenu9,NULL);
	}
	else	//有多条
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
		DisplayCommonMenu(&HYMenu8,NULL);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_SelectNextChargeRecode
* Description  :查询下一条交易记录，并显示
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectNextChargeRecode(void)
{
	INT32U i;
	if(RecodeControl.NextReadRecodeNun == 0)  //无下一条
	{
		return FALSE;
	}
	//当前赋值为上一条
	RecodeControl.UpReadRecodeNun = RecodeControl.CurReadRecodeNun;
	memcpy(&RecodeControl.UpRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
	//下一条复制给当前
	RecodeControl.CurReadRecodeNun = RecodeControl.NextReadRecodeNun;
	memcpy(&RecodeControl.CurRecode,&RecodeControl.NextRecode,sizeof(_CHARGE_RECODE));//将之前的下一条拷贝到当前条
	RecodeControl.NextReadRecodeNun = 0;
	//查询是否有下一条记录，决定页面跳转到63界面 还是64界面
	for(i = RecodeControl.CurReadRecodeNun - 1; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum); i--)
	{
		APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.NextRecode);
		//查询卡号是否一致
		//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.NextRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
		{
			RecodeControl.NextReadRecodeNun = i;
			break;
		}
	}
	RecodeControl.CurNun++;  //页面+1
	if(i == RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //下一页无
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
		DisplayCommonMenu(&HYMenu9,NULL);
		RecodeControl.NextReadRecodeNun = 0;
	}
	else	//有多条
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
		DisplayCommonMenu(&HYMenu8,NULL);
	}
	return TRUE;
}


/*****************************************************************************
* Function     : APP_SelectNextNChargeRecode
* Description  :查询下N条交易记录，并显示
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectNextNChargeRecode(INT16U num)
{
	INT32U i;

	while(num--)
	{
		if(RecodeControl.NextReadRecodeNun == 0)  //无下一条
		{
			DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
			PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
			DisplayCommonMenu(&HYMenu9,NULL);
			RecodeControl.NextReadRecodeNun = 0;
			return TRUE;
		}
		//当前赋值为上一条
		RecodeControl.UpReadRecodeNun = RecodeControl.CurReadRecodeNun;
		memcpy(&RecodeControl.UpRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
		//下一条复制给当前
		RecodeControl.CurReadRecodeNun = RecodeControl.NextReadRecodeNun;
		memcpy(&RecodeControl.CurRecode,&RecodeControl.NextRecode,sizeof(_CHARGE_RECODE));//将之前的下一条拷贝到当前条
		RecodeControl.NextReadRecodeNun = 0;
		//查询是否有下一条记录，决定页面跳转到63界面 还是64界面
		for(i = RecodeControl.CurReadRecodeNun - 1; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum); i--)
		{
			APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.NextRecode);
			//查询卡号是否一致
			//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.NextRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
			{
				RecodeControl.NextReadRecodeNun = i;
				break;
			}
		}
		RecodeControl.CurNun++;  //页面+1
		if(i == RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //下一页无
		{
			RecodeControl.NextReadRecodeNun = 0;
		}
	}
	if(i == RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //下一页无
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
		DisplayCommonMenu(&HYMenu9,NULL);
		RecodeControl.NextReadRecodeNun = 0;
	}
	else	//有多条
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
		DisplayCommonMenu(&HYMenu8,NULL);
	}
	return TRUE;
}
/*****************************************************************************
* Function     : APP_SelectUpChargeRecode
* Description  :查询上N条交易记录，并显示
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectUpNChargeRecode(INT16U num)
{
	INT32U i;
	ST_Menu * ppage;
	while(num--)
	{
		if(RecodeControl.UpReadRecodeNun == 0)  //无上一条
		{
			APP_ClearRecodeInfo();		//清除记录信息
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}

			return TRUE;
		}
		//当前条赋值给下一条
		RecodeControl.NextReadRecodeNun = RecodeControl.CurReadRecodeNun;
		memcpy(&RecodeControl.NextRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
		//上一条赋值给当前条
		RecodeControl.CurReadRecodeNun = RecodeControl.UpReadRecodeNun;
		memcpy(&RecodeControl.CurRecode,&RecodeControl.UpRecode,sizeof(_CHARGE_RECODE) );
		//查询上一条记录
		for(i = RecodeControl.CurReadRecodeNun + 1; i <= RecodeControl.RecodeCurNum ; i++)
		{
			APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.UpRecode);
			//查询卡号是否一致
			//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.UpRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
			{
				RecodeControl.UpReadRecodeNun = i;
				break;
			}
		}
		if(i > RecodeControl.RecodeCurNum) 		//上一页无记录
		{
			RecodeControl.UpReadRecodeNun = 0;
		}
		RecodeControl.CurNun--;  //页面-1
		if(RecodeControl.CurNun == 1)
		{
			break;
		}
	}
	//点击上一页，肯定有下一页，所以调转到63界面
	DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
	PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
	DisplayCommonMenu(&HYMenu8,NULL);
	return TRUE;
}


/*****************************************************************************
* Function     : APP_SelectUpChargeRecode
* Description  :查询下一条交易记录，并显示
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectUpChargeRecode(void)
{
	INT32U i;
	if(RecodeControl.UpReadRecodeNun == 0)  //无上一条
	{
		APP_ClearRecodeInfo();		//清除记录信息
		DisplayCommonMenu(&HYMenu3,NULL);   //返回主界面
		return TRUE;
	}
	//当前条赋值给下一条
	RecodeControl.NextReadRecodeNun = RecodeControl.CurReadRecodeNun;
	memcpy(&RecodeControl.NextRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
	//上一条赋值给当前条
	RecodeControl.CurReadRecodeNun = RecodeControl.UpReadRecodeNun;
	memcpy(&RecodeControl.CurRecode,&RecodeControl.UpRecode,sizeof(_CHARGE_RECODE) );
	//查询上一条记录
	for(i = RecodeControl.CurReadRecodeNun + 1; i <= RecodeControl.RecodeCurNum ; i++)
	{
		APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.UpRecode);
		//查询卡号是否一致
		//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.UpRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
		{
			RecodeControl.UpReadRecodeNun = i;
			break;
		}
	}
	if(i > RecodeControl.RecodeCurNum) 		//上一页无记录
	{
		RecodeControl.UpReadRecodeNun = 0;
	}
	RecodeControl.CurNun--;  //页面-1
	//点击上一页，肯定有下一页，所以调转到63界面
	DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
	PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //显示页码
	DisplayCommonMenu(&HYMenu8,NULL);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetBillInfo
* Description  : 获取枪订单信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U *APP_GetBillInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &Billbuf[gun][1];
}


/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  : 在充电中周期性储存记录交易记录   				  billflag:0表示已经结算			billflag：1表示未结算
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U WriterFmBill(_GUN_NUM gun,INT8U billflag)
{
//第一个字节表示时候已经结算     				  billflag:0表示已经结算			billflag：表示未结算
	_FLASH_OPERATION  FlashOper;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	Billbuf[gun][0] = billflag;
	if(billflag != 0)
	{
		Pre4GBill(gun,&Billbuf[gun][1]);
	}

	//读取所有配置信息
	if(gun == GUN_A )
	{
		FlashOper.DataID = PARA_BILLGUNA_ID;
	}
	else
	{
		FlashOper.DataID = PARA_BILLGUNB_ID;
	}
	if(billflag == 0)
	{
		FlashOper.Len =	1;	//0只是置下是否发送标志位
	}
	else
	{
		FlashOper.Len =	500;
	}
	FlashOper.ptr = &Billbuf[gun][0];
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Writ Bill Error");
	}
	return TRUE;
}



/*****************************************************************************
* Function     : WriterFmOFFlineBill
* Description  : 保存安培快充离线交易记录
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U WriterFmOFFlineBill(_GUN_NUM gun)
{
//第一个字节表示时候已经结算     				  billflag:0表示已经结算			billflag：表示未结算
	_FLASH_OPERATION  FlashOper;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	//读取所有配置信息
	if(gun == GUN_A )
	{
		FlashOper.DataID = PARA_BILLGUNA_ID;
	}
	else
	{
		FlashOper.DataID = PARA_BILLGUNB_ID;
	}
//	if(billflag == 0)
//	{
//		FlashOper.Len =	1;	//0只是置下是否发送标志位
//	}
//	else
//	{
//		FlashOper.Len =	200;
//	}
	FlashOper.ptr = &Billbuf[gun][0];
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Writ Bill Error");
	}
	return TRUE;
}

/*****************************************************************************
* Function     : ReadFmBill
* Description  : 读取订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U ReadFmBill(_GUN_NUM gun)
{
	_FLASH_OPERATION  FlashOper;

	FlashOper.RWChoose = FLASH_ORDER_READ;
	FlashOper.Len = 500;


	FlashOper.DataID = PARA_BILLGUNA_ID;
	FlashOper.ptr = &Billbuf[GUN_A][0];
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read Bill Error");
	}

	FlashOper.DataID = PARA_BILLGUNB_ID;
	FlashOper.ptr = &Billbuf[GUN_B][0];
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read Bill Error");
	}

	if(gun == GUN_A)
	{
		if(Billbuf[GUN_A][0] == 1)
		{
			APP_SetResendBillState(GUN_A,1);
		}
		else
		{

			APP_SetResendBillState(GUN_A,0);
		}
	}
	if(gun == GUN_B)
	{
		if(Billbuf[GUN_B][0] == 1)
		{
			APP_SetResendBillState(GUN_B,1);
		}
		else
		{

			APP_SetResendBillState(GUN_B,0);
		}
	}
	return TRUE;
}


/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  : 在充电中周期性储存记录交易记录
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
static INT8U Period_WriterFmBill(INT32U time)
{
	static INT16U count[GUN_MAX] = {0,0};
	_BSPRTC_TIME CurRTC;

	BSP_RTCGetTime(&CurRTC);

	if(APP_GetWorkState(GUN_A) == WORK_CHARGE)
	{
		if(++count[GUN_A] >= ((SYS_DELAY_5M*4)/time) )   //之前为30s存一下，flash用不了多久，目前临时改成5分钟一次 20210623
			//if(++count[GUN_A] >= ((SYS_DELAY_5s)/time) )
		{

			count[GUN_A] = 0;
			//写入A枪记录
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(APP_GetStartNetState(GUN_A) == NET_STATE_ONLINE)
				{
					WriterFmBill(GUN_A,1);			//在线保存
				}
			}
			else
			{
				WriterFmBill(GUN_A,1);
			}

		}
	}
	else
	{
		count[GUN_A] = 0;
	}
	if(APP_GetWorkState(GUN_B) == WORK_CHARGE)
	{
		if(++count[GUN_B] >= ((SYS_DELAY_5M*4)/time) )   //之前为30s存一下，flash用不了多久，目前临时改成5分钟一次 20210623
			//	if(++count[GUN_B] >= ((SYS_DELAY_5s)/time) )
		{

			count[GUN_B] = 0;
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(APP_GetStartNetState(GUN_B) == NET_STATE_ONLINE)
				{
					WriterFmBill(GUN_B,1);			//在线保存
				}

			}
			else
			{
				WriterFmBill(GUN_B,1);
			}
			//写入A枪记录
		}
	}
	else
	{
		count[GUN_B] = 0;
	}
	return TRUE;
}


/*****************************************************************************
* Function     : DispShow_42DivInfo
* Description  : 状态显示
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U DispShow_StatusDivInfo(void)
{
	INT16U i = 0;
	_SHOW_NUM * pshow_num;
	pshow_num = (_SHOW_NUM *)&DivStatue;
	INT16U gunvol[GUN_MAX];
	ST_Menu* pMenu = GetCurMenu();

	gunvol[GUN_A] = DivStatue.GunVol[GUN_A];
	gunvol[GUN_B] = DivStatue.GunVol[GUN_B];
	for(i = 0; i < (sizeof(DivStatue)/(sizeof(_SHOW_NUM))); i++) //一个页面一共有18个状态
	{
		//全部显示存在界面卡顿
		if((pMenu == &HYMenu42) && (i < 18))
		{
			if(*(pshow_num + i) == SHOW_DIV_ERR)
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),RED);  //目前全部显示红色
			}
			else
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),BLACK);  //目前全部显示红色
			}
		}
		if((i >= 18) && (i < 36) && (pMenu == &HYMenu43))
		{
			if(*(pshow_num + i) == SHOW_DIV_ERR)
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),RED);  //目前全部显示红色
			}
			else
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),BLACK);  //目前全部显示红色
			}
			PrintNum16uVariable(DIS_ADD(43,0),gunvol[GUN_A]);
		}
		if((i >= 36) && (pMenu == &HYMenu44))
		{
			if(*(pshow_num + i) == SHOW_DIV_ERR)
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),RED);  //目前全部显示红色
			}
			else
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),BLACK);  //目前全部显示红色
			}
			PrintNum16uVariable(DIS_ADD(44,0),gunvol[GUN_B]);
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  :周期性拷贝设备信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
static INT8U Period_GerDivStatus()
{
	INT8U i = 0;
	//部分AB枪，故获取A枪的即可
	//避雷器
	BCURemoteSignal[GUN_A].state2.State.SurgeArrester_Err == 0 ? (DivStatue.SurgeArrester_Err = SHOW_DIV_NORMAL) :(DivStatue.SurgeArrester_Err = SHOW_DIV_ERR);
	//交流输入过压
	BCURemoteSignal[GUN_A].state3.State.AC_OVP == 0 ? (DivStatue.AC_OVP = SHOW_DIV_NORMAL) :(DivStatue.AC_OVP = SHOW_DIV_ERR);
	//交流输入欠压
	BCURemoteSignal[GUN_A].state3.State.AC_UVP == 0 ? (DivStatue.AC_UVP = SHOW_DIV_NORMAL) :(DivStatue.AC_UVP = SHOW_DIV_ERR);

	//充电机过温告警
	DivStatue.SysUptemp_Warn = SHOW_DIV_NORMAL;
	//BCURemoteSignal[GUN_A].state1.State.SysUptemp_Warn == 0 ? (DivStatue.SysUptemp_Warn = SHOW_DIV_NORMAL) :(DivStatue.SysUptemp_Warn = SHOW_DIV_ERR);
	//风扇故障
	BCURemoteSignal[GUN_A].state2.State.Fan_Warn == 0 ? (DivStatue.Fan_Warn = SHOW_DIV_NORMAL) :(DivStatue.Fan_Warn = SHOW_DIV_ERR);
	//交流断路器故障
	BCURemoteSignal[GUN_A].state1.State.ACCircuitBreaker_Err == 0 ?  (DivStatue.ACCircuitBreaker_Err = SHOW_DIV_NORMAL) :(DivStatue.ACCircuitBreaker_Err = SHOW_DIV_ERR);
	//门禁
	if(BSP_MPLSState(BSP_DI_DOOR))
	{
		DivStatue.Door_Err = SHOW_DIV_ERR;
	}
	else
	{
		DivStatue.Door_Err = SHOW_DIV_NORMAL;
	}
	//BCURemoteSignal[GUN_A].state1.State.Door_Err == 0 ? (DivStatue.Door_Err = SHOW_DIV_NORMAL) :(DivStatue.Door_Err = SHOW_DIV_ERR);
	//急停
	BCURemoteSignal[GUN_A].state1.State.Stop_Pluse == 0 ? (DivStatue.Stop_Pluse = SHOW_DIV_NORMAL) :(DivStatue.Stop_Pluse = SHOW_DIV_ERR);
	//充电模块交流输入过压告警
	BCURemoteSignal[GUN_A].state4.State.ModuleACOVP_Warn == 0 ? (DivStatue.ModuleACOVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleACOVP_Warn = SHOW_DIV_ERR);
	//充电模块焦炉输入欠压告警
	BCURemoteSignal[GUN_A].state4.State.ModuleACUVP_Warn == 0 ? (DivStatue.ModuleACUVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleACUVP_Warn = SHOW_DIV_ERR);
	//充电模块输入缺相告警
	BCURemoteSignal[GUN_A].state4.State.ModuleACPhase_Warn == 0? (DivStatue.ModuleACPhase_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleACPhase_Warn = SHOW_DIV_WARNING);
	//充电模块输出短路故障
	BCURemoteSignal[GUN_A].state4.State.ModuleDCShortCircuit_Err == 0? (DivStatue.ModuleDCShortCircuit_Err = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCShortCircuit_Err = SHOW_DIV_ERR);
	//充电模块直流输出过流告警
	BCURemoteSignal[GUN_A].state4.State.ModuleDCUPCUR_Warn == 0 ?  (DivStatue.ModuleDCUPCUR_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCUPCUR_Warn = SHOW_DIV_ERR);
	//充电模块直流输出过压告警
	BCURemoteSignal[GUN_A].state4.State.ModuleDCOVP_Warn == 0 ?  (DivStatue.ModuleDCOVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCOVP_Warn = SHOW_DIV_WARNING);
	//充电模块直流输出欠压告警
	BCURemoteSignal[GUN_A].state5.State.ModuleDCUVP_Warn == 0 ?  (DivStatue.ModuleDCUVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCUVP_Warn = SHOW_DIV_WARNING);
	//需要分A/B枪

	for(i = 0; i < (INT8U)GUN_MAX; i++)
	{
		if(GetMeterStatus((_GUN_NUM)i) == _COM_NORMAL)
		{
			DivStatue.GunStatus[i].WattMetereSignalErr = SHOW_DIV_NORMAL;
		}
		else
		{
			DivStatue.GunStatus[i].WattMetereSignalErr = SHOW_DIV_ERR;
		}
		DivStatue.GunStatus[i].CCUSignalErr = SHOW_DIV_NORMAL;
		//DivStatue.GunVol[i] = ((YC_Info[i].cc1volt/10) & 0x00ff) << 8 | ((YC_Info[i].cc1volt/10) & 0xff00) >> 8;
		DivStatue.GunVol[i] =  BSP_GetHandVolt((_GUN_NUM)i) / 10; //保留一个小数
		//充电枪归位状态
#if ENBLE_JG
		BSP_MPLSState(BSP_DI_QGW) == 0 ? (DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_HOMING) :(DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_UNHOMING);
#else
		BCURemoteSignal[i].state3.State.ChargGun_State == 0 ? (DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_HOMING) :(DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_UNHOMING);
#endif
		//充电枪过温状态
		BCURemoteSignal[i].state3.State.CGunUpTemperature_Err == 0 ? (DivStatue.GunStatus[i].CGunUpTemperature_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].CGunUpTemperature_Err = SHOW_DIV_WARNING);
		//电子锁
		BCURemoteSignal[i].state2.State.DZLock_Err == 0 ? (DivStatue.GunStatus[i].DZLock_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DZLock_Err = SHOW_DIV_ERR);
		//电池反接
		BCURemoteSignal[i].state2.State.BatReverse_Err == 0 ? (DivStatue.GunStatus[i].BatReverse_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].BatReverse_Err = SHOW_DIV_ERR);
		//与BMS通信状态
		BCURemoteSignal[i].state3.State.BMSCommunication_Err == 0 ? (DivStatue.GunStatus[i].BMSCommunication_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].BMSCommunication_Err = SHOW_DIV_ERR);
		//绝缘检测
		BCURemoteSignal[i].state2.State.JyCheck_Err == 0 ? (DivStatue.GunStatus[i].JyCheck_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].JyCheck_Err = SHOW_DIV_ERR);
		//充电模块直流输出过流告警
		BCURemoteSignal[i].state3.State.DCOVP_Warn == 0 ? (DivStatue.GunStatus[i].DCOVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DCOVP_Warn = SHOW_DIV_ERR);
		//充电模块直流输出过压告警
		BCURemoteSignal[i].state3.State.DCUVP_Warn == 0 ? (DivStatue.GunStatus[i].DCUVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DCUVP_Warn = SHOW_DIV_ERR);
		//充电模块直流输出欠压告警
		BCURemoteSignal[i].state3.State.UPCUR_Warn == 0 ? (DivStatue.GunStatus[i].UPCUR_Warn = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].UPCUR_Warn = SHOW_DIV_ERR);
		//充电中车辆导引
		BCURemoteSignal[i].state2.State.CarGuide_Err == 0 ? (DivStatue.GunStatus[i].CarGuide_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].CarGuide_Err = SHOW_DIV_ERR);
		//直流输出接触器

		BCURemoteSignal[i].state1.State.DCContactor_Err == 0 ? (DivStatue.GunStatus[i].DCContactor_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DCContactor_Err = SHOW_DIV_ERR);
		//A/B枪cc1需要单独显示，赋值为 SHOW_MAX，表示状态不显示
		DivStatue.GunStatus[i].cc1volt = SHOW_MAX;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Dispose_SOCThreshold
* Description  :soc阈值处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
static INT8U Dispose_SOCThreshold(void)
{
	INT8U i;
	static INT16U lastsoc[GUN_MAX] = {0};

	for(i = 0; i < GUN_MAX; i++)
	{
		if(APP_GetWorkState((_GUN_NUM)i) == WORK_CHARGE)
		{
			if(ChargeingInfo[i].ChargeSOC != lastsoc[i])
			{
				lastsoc[i] = ChargeingInfo[i].ChargeSOC;
				if(SYSSet.SOCthreshold != 100)
				{
					if(((ChargeingInfo[i].ChargeSOC & 0xff00) >> 8) >= SYSSet.SOCthreshold)  //SOC达到阈值发送停止
					{
						NB_WriterReason(i,"E59",3);
						SendStopChargeMsg((_GUN_NUM)i);
					}
				}
			}
		}
	}
	return FALSE;
}

/***********************************************************************************************
* Function		: APP_GetDevNum
* Description	: 获取设备号
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 20210111
***********************************************************************************************/
INT8U * APP_GetDevNum( void)
{
	return SYSSet.SysSetNum.DivNum;
}


/***********************************************************************************************
* Function		: DisplayGunQRCode
* Description	: 显示二维码
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
INT8U DisplayGunQRCode(_GUN_NUM gun)
{
	INT8U buf[100]  = {0};
	//char * net = "http://smallprogram.evchong.com/EVCHONG_SP/startCharge/";  //森通
	char * net = "https://api.huichongchongdian.com/cpile/coffee/";
	//char * net = "https://zj.fengjingit.com/cpile/";				//贵州
	char * jgnet = "https://www.jgpowerunit.com/cpile/";  //精工
	char * acnet = "https://www.chuangyuechongdian.com/cpile/";
	char * xxcnet = "https://qrcode.starcharge.com/#/";
	char * sycnet = "https://er.quicklycharge.com/scancode/connectorid/";

	char * ykcnet = "http://www.ykccn.com/MPAGE/index.html?pNum=";
	char * ykzsh = "https://ne.gdsz.sinopec.com/h5/MPAGE/index.html?pNum=";
	//char * ykcnet = "http://wx.evking.cn/app/download?orgNo=MA59MJR32&code=";  //蔚景云
	//char * ykcnet = "http://www.coulomb-charging.com/scans/result.html?data=";	 //库伦
	char * ttcnet = "https://nev.chinatowercom.cn?pNum=";

	char * dkycnet = "https://qrcode.dazzlesky.com/scancode/connectorid/";
	char * xjnet = "https://epower.xiaojukeji.com/epower/static/resources/xcxconf/XIAOJU.101437000.";
	char * annet = "https://zjec.evshine.cn/scan/scan/scanTransfer?gunNo=";
	INT8U code[17];
	INT8U apcode[18];
	INT8U len;


	if(gun > GUN_MAX)
	{
		return FALSE;
	}
	if(SYSSet.NetYXSelct == XY_HY)
	{
		len = strlen(net);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,net,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
		if(gun == GUN_A)
		{
			code[16] = '0';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0800,buf,17+len);
		}
		else
		{
			code[16] = '1';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0880,buf,17+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_JG)
	{
		len = strlen(jgnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,jgnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
		if(gun == GUN_A)
		{
			code[16] = '0';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0800,buf,17+len);
		}
		else
		{
			code[16] = '1';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0880,buf,17+len);
		}
	}

	if (SYSSet.NetYXSelct == XY_YL2)
	{
		len = strlen(acnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,acnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
		if(gun == GUN_A)
		{
			code[16] = '0';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0800,buf,17+len);
		}
		else
		{
			code[16] = '1';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0880,buf,17+len);
		}
	}
	if (SYSSet.NetYXSelct == XY_YL1)
	{

		len = strlen(dkycnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,dkycnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		len = strlen(annet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,annet,len);
		memcpy(apcode,SYSSet.SysSetNum.DivNum,16);
		if(gun == GUN_A)
		{
			apcode[16] = '0';
			apcode[17] = '1';
			memcpy(&buf[len],apcode,18);
#if(USER_GUN == USER_SINGLE_GUN)
			DisplayQRCode(0x0800,buf,len + 16);
#else
			DisplayQRCode(0x0800,buf,len + 18);
#endif
		}
		else
		{
			apcode[16] = '0';
			apcode[17] = '2';
			memcpy(&buf[len],apcode,18);
			DisplayQRCode(0x0880,buf,len + 18);
		}
	}


	if(SYSSet.NetYXSelct == XY_YKC)
	{
		len = strlen(ykcnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,ykcnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_ZSH)
	{
		len = strlen(ykzsh);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,ykzsh,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}

	if(SYSSet.NetYXSelct == XY_TT)
	{
		len = strlen(ttcnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,ttcnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_YL3)
	{
		len = strlen(xxcnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,xxcnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[8] = '0';
			code[9] = '1';
			memcpy(&buf[len],code,10);
			DisplayQRCode(0x0800,buf,10+len);
		}
		else
		{
			code[8] = '0';
			code[9] = '2';
			memcpy(&buf[len],code,10);
			DisplayQRCode(0x0880,buf,10+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_YL4)
	{
		len = strlen(sycnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,sycnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}

	if(SYSSet.NetYXSelct == XY_XJ)
	{
		len = strlen(xjnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,xjnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
//		if(gun == GUN_A)
//		{
//			memcpy(&buf,SYSSet.SysSetNum.DivNum,14);
//			buf[14] = '0';
//			buf[15] = '1';
//			DisplayQRCode(0x0800,buf,16);
//		}else{
//			memcpy(&buf,SYSSet.SysSetNum.DivNum,14);
//			buf[14] = '0';
//			buf[15] = '2';
//			DisplayQRCode(0x0880,buf,16);
//		}

	return TRUE;
}

/*****************************************************************************
* Function     : Dispose_ACSW
* Description  :交流接触器控制
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
static INT8U Dispose_ACSW(void)
{
	static INT32U lasttime;
	INT32U nowSysTime = OSTimeGet();
#if(USER_GUN != USER_SINGLE_GUN)
	if((GetGunState(GUN_A) == GUN_DISCONNECTED)  && (GetGunState(GUN_B) == GUN_DISCONNECTED))
	{
		if((nowSysTime >= lasttime) ? ((nowSysTime - lasttime) >= SYS_DELAY_1M) : \
		        ((nowSysTime + (0xFFFFFFFF - lasttime)) >= SYS_DELAY_1M))
		{
			lasttime = nowSysTime;
			BSP_IOClose(IO_RLY_K3);
		}
	}
	else
	{
		DispControl.StartIntTime[GUN_A] = OSTimeGet() + SYS_DELAY_5s;
		DispControl.StartIntTime[GUN_B] = OSTimeGet() + SYS_DELAY_5s;
		lasttime = nowSysTime;
		BSP_IOOpen(IO_RLY_K3);
	}
#else
	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
	{
		if((nowSysTime >= lasttime) ? ((nowSysTime - lasttime) >= SYS_DELAY_1M) : \
		        ((nowSysTime + (0xFFFFFFFF - lasttime)) >= SYS_DELAY_1M))
		{
			lasttime = nowSysTime;
			BSP_IOClose(IO_RLY_K3);
		}
	}
	else
	{
		DispControl.StartIntTime[GUN_A] = OSTimeGet() + SYS_DELAY_5s;
		lasttime = nowSysTime;
		BSP_IOOpen(IO_RLY_K3);
	}
#endif
}

/*****************************************************************************
* Function     : Dispose_SendStartCharge
* Description  :发送启动充电，主要为了处理上个停止到下一个开始必须间隔一定的时间,CCU需要停止状态处理。目前定义未8s
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
static INT8U Dispose_SendStartCharge(void)
{
	INT8U gun;
	INT32U nowSysTime = OSTimeGet();
	for(gun = 0; gun < GUN_MAX; gun++)
	{
		if(DispControl.SendStartCharge[gun] == TRUE)
		{
			if((nowSysTime >= DispControl.StartIntTime[gun]) ? ((nowSysTime - DispControl.StartIntTime[gun]) >= SYS_DELAY_10s) : \
			        ((nowSysTime + (0xFFFFFFFF - DispControl.StartIntTime[gun])) >= SYS_DELAY_10s))
			{
				DispControl.SendStartCharge[gun] = FALSE;
				DispControl.StartIntTime[gun] = nowSysTime;
				if(DispControl.StartType == MODE_VIN)
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_VIN);
				}
				else
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_AUTO);	//通知启动充电
				}
			}
		}
	}
	return TRUE;
}




/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : 周期性检测任务
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
ST_Menu * Disp_NetPageDispos(void)
{
	INT8U   i;
	INT8U gun_state[GUN_MAX];   //0:有故障、待机 			1充电

	for(i = 0; i < GUN_MAX; i++)
	{
		if(DispControl.NetGunState[(_GUN_NUM)i] == GUN_IDLE) 				//空闲
		{
			gun_state[i] = 0;   //二维码界面
		}
		else
		{
			gun_state[i] = 1;	//无二维码界面
		}
	}

	if((gun_state[GUN_A] == 0) && (gun_state[GUN_B] == 0))
	{
		return &HYMenu73;
	}
	else if((gun_state[GUN_A] == 1) && (gun_state[GUN_B] == 0))
	{
		if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
		{
			return &HYMenu77;
		}
		else
		{
			return &HYMenu74;
		}
	}
	else if((gun_state[GUN_A] == 0) && (gun_state[GUN_B] == 1))
	{
		if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
		{
			return &HYMenu78;
		}
		else
		{
			return &HYMenu75;
		}
	}
	else
	{
		//4种可能
		if((DispControl.NetGunState[GUN_A] == GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] == GUN_CHARGEING) )
		{
			return &HYMenu79;
		}
		else if((DispControl.NetGunState[GUN_A] == GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] != GUN_CHARGEING) )
		{
			if(DispControl.NetGunState[GUN_B] == GUN_IDLE)
			{
				return &HYMenu77;
			}
			else
			{
				return &HYMenu81;
			}
		}
		else if((DispControl.NetGunState[GUN_A] != GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] == GUN_CHARGEING) )
		{
			if(DispControl.NetGunState[GUN_A] == GUN_IDLE)
			{
				return &HYMenu78;
			}
			else
			{
				return &HYMenu80;
			}
		}
		else
		{
			return &HYMenu76;
		}
	}
}


/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : 周期性检测任务
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U  Net_StartCharge(_GUN_NUM gunnum,INT8U type)
{
	if(gunnum >= GUN_MAX)
	{
		return FALSE;
	}
	OSSchedLock();

	ChargeRecodeInfo[gunnum].Gun = gunnum;
	ChargeRecodeInfo[gunnum].BillingStatus = RECODE_UNSETTLEMENT;	  			//初始为未结算
	SendPricMsg(gunnum,PRIC_START);			//通知开始计费
	DispControl.SendStartCharge[gunnum] = TRUE;
	DispControl.NetSSTState[gunnum] = NET_STARTING;
	DispControl.StartType = type;		//启动方式
	OSSchedUnlock();
	return TRUE;
}

/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : 周期性检测任务
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U  Net_StopCharge(_GUN_NUM gunnum)
{
	if(gunnum >= GUN_MAX)
	{
		return FALSE;
	}
	OSSchedLock();
	SendStopChargeMsg(gunnum);			//通知A枪关机
	DispControl.NetSSTState[gunnum] = NET_STOPING;
	OSSchedUnlock();
	return TRUE;
}

/*****************************************************************************
* Function     : NetCardDispose
* Description  : 网络版刷卡处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月16日
*****************************************************************************/
static void NetCardDispose(INT32U time)
{
	ST_Menu* CurMenu = GetCurMenu();
	static INT8U gun = 0;
	INT8U buf[2] = {0x55,0x55};
	if(GetCardWorkStatus() != _Card_IDLE)     //卡空闲才发送
	{
		return;
	}
	if((SYSSet.NetYXSelct != XY_AP) && (SYSSet.NetYXSelct != XY_HY)&& (SYSSet.NetYXSelct != XY_JG)&&(SYSSet.NetYXSelct != XY_YL2) \
	        && (SYSSet.NetYXSelct != XY_ZSH)	&& (SYSSet.NetYXSelct != XY_YKC)&& (SYSSet.NetYXSelct != XY_TT)&& (SYSSet.NetYXSelct != XY_YL3)&& (SYSSet.NetYXSelct != XY_YL4) && ((SYSSet.NetYXSelct != XY_XJ)) \
	        && ((SYSSet.NetYXSelct != XY_YL1)))
	{
		return;
	}

	if(DispControl.NetGunState[gun] == GUN_CHARGEING)  //充电状态发送关机指令
	{
		if((_4G_GetStartType(gun) == _4G_APP_CARD) || (_4G_GetStartType(gun) == _4G_APP_BCCARD)) //刷卡启动才发送
		{
			if(gun == GUN_A)
			{
				SendCardMsg(NETCARDCMD_STOPA, (void *)&buf,sizeof(buf));
			}
			else
			{
				SendCardMsg(NETCARDCMD_STOPB, (void *)&buf,sizeof(buf));
			}
		}
	}
	gun ++;
	if(gun >= GUN_MAX)
	{
		gun = GUN_A;
	}

}

/*****************************************************************************
* Function     : bug_dispose
* Description  :拔枪，界面却在充电中，需要复位   20230308
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
static INT8U bug_dispose(void)
{
	INT8U gun;
	INT32U nowSysTime = OSTimeGet();
	for(gun = 0; gun < GUN_MAX; gun++)
	{
		if((APP_GetErrState((_GUN_NUM)gun) == 0x01)  &&  ( (GetRM_HandVolt(gun) == INTERFACE_UNFULCONNECTED) ||
		        (GetGunState(gun) == GUN_DISCONNECTED)) ) //正在充电)
		{
			if((nowSysTime >= DispControl.StartIntTime[gun]) ? ((nowSysTime - DispControl.StartIntTime[gun]) >= SYS_DELAY_10s) : \
			        ((nowSysTime + (0xFFFFFFFF - DispControl.StartIntTime[gun])) >= SYS_DELAY_10s))
			{
				DispControl.SendStartCharge[gun] = FALSE;
				DispControl.StartIntTime[gun] = nowSysTime;
				if(DispControl.StartType == MODE_VIN)
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_VIN);
				}
				else
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_AUTO);	//通知启动充电
				}
			}
		}
	}
	return TRUE;
}

#define DEFPOWER    500		//离线情况下默认50kw输出
INT16U curpowera = 0;
INT16U curpowerb = 0;
/*****************************************************************************
* Function     : Period_PowerRegulate
* Description  :精工功率调节
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2023年12月8日
*****************************************************************************/
static INT8U Period_PowerRegulate(void)
{
	INT16U kwa = 0,kwb = 0;
	INT16U power = 0;
	INT8U * pdevnum = APP_GetDevNum();


#if (ENBLE_JG == 0)   //非精工 无需功率调节
	curpowera = 2000;
	curpowerb = 2000;
#else
	OSSchedLock();

	if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
	{
		kwa = PowerModuleInfo[GUN_A].OutputInfo.Vol * PowerModuleInfo[GUN_A].OutputInfo.Cur / 10000;
	}
	if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
	{
		kwb = PowerModuleInfo[GUN_B].OutputInfo.Vol * PowerModuleInfo[GUN_B].OutputInfo.Cur / 10000;
	}

	if(APP_GetModuleConnectState(0) == STATE_OK)
	{
		power = APP_GetHYCurPower();  //在线实时调节
	}
	else
	{
		power = DEFPOWER;				//未在线默认输入
	}
	if(power < 100)  					//总功率不能少于100kw 少于100kw 则关机
	{
		if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
		{
			Net_StopCharge(GUN_A);
		}
		if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
		{
			Net_StopCharge(GUN_B);
		}
	}


	//2把枪都在充电多出的部分平均分
	if((DispControl.NetGunState[GUN_A] == GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] == GUN_CHARGEING))
	{
		if((kwb+kwa) >= power)   //总功率大
		{
			//按比例减少
			curpowerb = (kwb * (power * 100 / (kwb+kwa))) / 100;
			curpowera = (kwa * (power * 100 / (kwb+kwa))) / 100;
		}
		else
		{
			curpowera = (power - kwa - kwb)/2+kwa;
			curpowerb = (power - kwa - kwb)/2+kwb;
		}
	}
	else if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
	{
		//A枪再充电
		if (GetGunState(GUN_B) == GUN_CONNECTED)
		{
			curpowera = power - 50;			//预留5kw给B
			curpowerb = 50;
		}
		else
		{
			curpowera = power;
			curpowerb = 0;
		}
	}
	else if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
	{
		//B枪再充电
		if (GetGunState(GUN_A) == GUN_CONNECTED)
		{
			curpowerb = power - 50;			//预留5kw给a
			curpowera = 50;
		}
		else
		{
			curpowerb = power;
			curpowera = 0;
		}
	}
	else
	{
		curpowerb = power/2;
		curpowera = power/2;
	}
	OSSchedUnlock();
#endif
	return TRUE;
}

/*****************************************************************************
* Function     : DispPeriod
* Description  : 显示任务500ms执行一次   必须发在TaskDisplay 显示任务中，发送数据保持在同一个出口
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static void DispPeriod(void)
{
	char buf[30] = {0};
	static INT32U delay = SYS_DELAY_500ms;   //周期性检测周期，不能超过1s
	//枪未连接，点击主界面的枪号，进入充电选择界面


	ST_Menu* CurMenu;
	static ST_Menu * ppage = NULL,*plastpage = NULL;
	CurMenu = GetCurMenu();
	static ST_Menu* lastMenu = NULL;
	static INT8U LastGunState[GUN_MAX] = {GUN_DISCONNECTED};      //枪状态改变需要发送实时数据
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	//网络版本页面切换


	//显示软件版本号+桩编号
	if(lastMenu != CurMenu)
	{
		lastMenu =CurMenu;
		snprintf((char *)buf, sizeof(buf), "DC%02d%02d%02d",SYS_APP_VER[0],SYS_APP_VER[1],SYS_APP_VER[2]);

		PrintStr(0x0100,(INT8U *)buf,strlen(buf));
		PrintStr(0x0200,(INT8U *)SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum)); //程序版本
	}


	if(SYSSet.NetState == DISP_NET)
	{
		if((CurMenu == &HYMenu73) || (CurMenu == &HYMenu74) || (CurMenu == &HYMenu75) || (CurMenu == &HYMenu76) \
		        || (CurMenu == &HYMenu77) || (CurMenu == &HYMenu78) || (CurMenu == &HYMenu79) ||  (CurMenu == &HYMenu80) || (CurMenu == &HYMenu81))
		{
			ppage = Disp_NetPageDispos();
			//连续2次一样，则切换，防止闪切,约为1s切一下
			if(plastpage != ppage)
			{
				plastpage = ppage;
			}
			else
			{
				if(ppage != CurMenu)
				{
					DisplayCommonMenu(ppage,NULL);
				}
			}
		}

		//枪状态改变需要发送实时数据
		if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
		{
			if(GetGunState(GUN_A) != LastGunState[GUN_A])
			{
				SendMsg[GUN_A].MsgID = BSP_MSGID_DISP;
				SendMsg[GUN_A].DivNum = APP_SJDATA_QUERY;   //发送实时数据，状态发生了改变
				SendMsg[GUN_A].DataLen = GUN_A;
				OSQPost(pevent, &SendMsg[GUN_A]);

				LastGunState[GUN_A] =GetGunState(GUN_A);
			}
			if(GetGunState(GUN_B) != LastGunState[GUN_B])
			{
				SendMsg[GUN_B].MsgID = BSP_MSGID_DISP;
				SendMsg[GUN_B].DivNum = APP_SJDATA_QUERY;   //发送实时数据，状态发生了改变
				SendMsg[GUN_B].DataLen = GUN_B;
				OSQPost(pevent, &SendMsg[GUN_B]);

				LastGunState[GUN_B] =GetGunState(GUN_B);
			}
		}
	}

	if(SYSSet.NetState == DISP_NET)
	{
		NOP();
	}
	else
	{
		if(GetGunState(GUN_A) == GUN_DISCONNECTED)
		{
			if((DispControl.ClickGunNextPage[GUN_A] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_A] == &HYMenu33) || \
			        (DispControl.ClickGunNextPage[GUN_A] == &HYMenu36) )
			{
				DispControl.CardOver[GUN_A] = STATE_UNOVER;
				DispControl.StopOver[GUN_A] = STATE_UNOVER;
				ChargeRecode_Dispose(GUN_A);				//交易记录处理
				DispControl.ClickGunNextPage[GUN_A] = &HYMenu22;
				DisplayCommonMenu(&HYMenu3,NULL);
			}
		}
		if(GetGunState(GUN_B) == GUN_DISCONNECTED)
		{
			if((DispControl.ClickGunNextPage[GUN_B] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_B] == &HYMenu33) || \
			        (DispControl.ClickGunNextPage[GUN_B] == &HYMenu37) )
			{
				DispControl.CardOver[GUN_B] = STATE_UNOVER;
				DispControl.StopOver[GUN_B] = STATE_UNOVER;
				ChargeRecode_Dispose(GUN_B);				//交易记录处理
				DispControl.ClickGunNextPage[GUN_B] = &HYMenu22;
				DisplayCommonMenu(&HYMenu3,NULL);
			}
		}
	}
	CountDownDispose(delay);				//倒计时处理
	Period_GerDivStatus();
	Disp_ShowRTC();							//界面显示RTC
	Disp_ShowGunT();						//显示枪温
	DispShow_ChargeingDate(delay);			//充电界面数据拷贝，以及若在充电中，每隔1秒给显示任务发送数据
	DispShow_CardDate(delay);				//车辆信息界面界面数据拷贝，以及若在充电中，每隔1秒给显示任务发送数据
	//拔枪，界面却在充电中，需要复位   20230308
	Dispose_ACSW();			//交流接触器控制
	DispShow_State(delay);					//主界面显示A B枪状态
	Dispose_SOCThreshold();//SOC达到阈值检测
	Dispose_SendStartCharge(); //发送启动充电，主要为了处理上个停止到下一个开始必须间隔一定的时间,CCU需要停止状态处理。目前定义未8s
	if(SYSSet.NetState == DISP_NET)
	{
		//网络模式下功率调节
		if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
		{
			Period_PowerRegulate();					//网络模式下功率调节
		}
		Period_WriterFmBill(delay);	         	//在充电中周期性储存记录交易记录
		NetCardDispose(delay);					//网络状态下安培快充周期性发送刷卡指令
	}
}





/****************************************擦除flash  必须关闭喂狗*********************************/
//#include "stm32f4xx_flash.h"
//#define FLASH_BASE_ADD  0x080F0000


//#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
//#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

//uint32_t GetSector(uint32_t Address)
//{
//  uint32_t sector = 0;
//
//  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
//  {
//    sector = FLASH_Sector_0;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
//  {
//    sector = FLASH_Sector_1;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
//  {
//    sector = FLASH_Sector_2;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
//  {
//    sector = FLASH_Sector_3;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
//  {
//    sector = FLASH_Sector_4;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
//  {
//    sector = FLASH_Sector_5;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
//  {
//    sector = FLASH_Sector_6;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
//  {
//    sector = FLASH_Sector_7;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
//  {
//    sector = FLASH_Sector_8;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
//  {
//    sector = FLASH_Sector_9;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
//  {
//    sector = FLASH_Sector_10;
//  }
//  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
//  {
//    sector = FLASH_Sector_11;
//  }

//  return sector;
//}



//INT8U write_flash(uint32_t add,uint16_t *FlashWriteBuf,uint16_t len)
//{
//	if((add < ADDR_FLASH_SECTOR_0) || (add > 0x080FFFFF) )
//	{
//		return FALSE;
//	}

//	FLASH_Unlock();	//解锁
//    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
//                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

//	if (FLASH_COMPLETE != FLASH_EraseSector(GetSector(add),VoltageRange_2)) //擦除扇区内容
//    {
//		return FALSE;
//	}
//
//	for (int i = 0; i < len/2; i++)
//	{
//		if (FLASH_COMPLETE != FLASH_ProgramHalfWord(add, FlashWriteBuf[i]))	//写入16位数据
//		{
//			return FALSE;
//		}
//		add += 2;	//16位数据偏移两个位置
//	}

//	FLASH_Lock();	//上锁
//
//	return TRUE;
//}

//INT16U read_flash(INT32U add)
//{
//	return *(INT16U*)add;
//}

//INT8U read_flashbuf(INT32U add,INT16U * buf,INT16U len)
//{
//	INT8U num;
//
//	for(num = 0;num < len/2;num++)
//	{
//		buf[num] = read_flash(add);
//		add +=2;
//	}
//}

//INT16U WFlashbuf[5] = {0x1111,0x2222,0x3333,0x4444,0x5555};
//INT16U RFlashbuf[5] = {0};

//INT8U Nflash_test(void)
//{
//	write_flash(ADDR_FLASH_SECTOR_11,WFlashbuf,sizeof(WFlashbuf));
//	read_flashbuf(ADDR_FLASH_SECTOR_11,RFlashbuf,sizeof(RFlashbuf));
//	__NOP();
//}
/*************************************flash  end*************************************************/
/*****************************************************************************
* Function     : DispPeriod
* Description  : 显示任务500ms执行一次   必须发在TaskDisplay 显示任务中，发送数据保持在同一个出口
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static void Display66GunQRCode(void)
{
	static INT8U gunshow[GUN_MAX] = {0};
	INT8U len;
	INT8U Devlen;
	INT8U buf[17] = {0};

	memcpy(buf,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
	if(SYSSet.NetYXSelct == XY_66)
	{
		//SYSSet.SysSetNum.DivNum[15] = 0;
		if(gunshow[GUN_A] == 0)
		{
			if(Qrcode[GUN_A][0] != 0X00)
			{
				gunshow[GUN_A] = 1;
				len = strlen((char*)&Qrcode[GUN_A][0]);
				Devlen = strlen((char*)buf);
				memcpy(&Qrcode[GUN_A][len],SYSSet.SysSetNum.DivNum,Devlen);
				Qrcode[GUN_A][len + Devlen] = '0';
				Qrcode[GUN_A][len + Devlen + 1] = '1';
				//DisplayQRCode(0x0800,&Qrcode[GUN_A][0],len + Devlen + 2);
				DisplayQRCode(0x0800,&Qrcode[GUN_A][0],len);

			}
		}
		if(gunshow[GUN_B] == 0)
		{
			if(Qrcode[GUN_B][0] != 0X00)
			{
				gunshow[GUN_B] = 1;
				len = strlen((char*)&Qrcode[GUN_B][0]);
				Devlen = strlen((char*)buf);
				memcpy(&Qrcode[GUN_B][len],SYSSet.SysSetNum.DivNum,Devlen);
				Qrcode[GUN_B][len + Devlen] = '0';
				Qrcode[GUN_B][len + Devlen + 1] = '2';
				//DisplayQRCode(0x0880,&Qrcode[GUN_B][0],len + Devlen + 2);
				DisplayQRCode(0x0880,&Qrcode[GUN_B][0],len);
			}
		}

	}
}


/*****************************************************************************
* Function     : TaskDisplay
* Description  : 显示任务
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月16日
*****************************************************************************/
void TaskDisplay(void *pdata)
{

	INT8U err;
	_BSP_MESSAGE *pMsg;
	const ST_Menu* CurMenu;
	_LCD_KEYVALUE key;
	static INT32U curtime,lasttime;
	//ST_Menu * ppage = NULL;

	//DispMutex = OSMutexCreate(PRI_DIS_MUTEX, &err);      //创建互斥锁

	//内部flash  配置
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_PrefetchBufferCmd(ENABLE);

	OSTimeDly(SYS_DELAY_500ms);
	Display_HardwareInit();
	OSTimeDly(SYS_DELAY_500ms);
	//系统初始化界面
//	DisplayCommonMenu(&gsMenu0,NULL);
	//初始化完成跳转到主界面，目前只是延时5s


	Display_ParaInit();				//参数初始化

	if(SYSSet.NetState == DISP_NET)
	{
		ReadFmBill(GUN_A);					//读取订单
		ReadFmBill(GUN_B);					//读取订单
		DisplayCommonMenu(&HYMenu73,NULL);
		//显示A、B枪二维码
		DisplayGunQRCode(GUN_A);
		DisplayGunQRCode(GUN_B);
	}
	else
	{
		DisplayCommonMenu(&HYMenu3,NULL);
	}
	OSTimeDly(SYS_DELAY_2s);
//	INT8U paddr[16] = {0x33,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x31,0x30,0x30,0x30,0x32};
//	Munu52_DevnumDispose(0,paddr,16);
	//PrintIcon(0x12b0,0);

	//Nflash_test();

	while(1)
	{
		if((APP_GetSIM7600Mode() == MODE_HTTP)  || (APP_GetSIM7600Mode() == MODE_FTP))   //远程升级其他无关数据帧都不不要发送和处理
		{
			OSTimeDly(SYS_DELAY_1000ms);
			continue; //未连接上服务器，AT指令处理
		}
		pMsg = OSQPend(DispTaskEvent, SYS_DELAY_500ms, &err);
		curtime = OSTimeGet();
		if (OS_ERR_NONE == err)
		{
			switch (pMsg->MsgID)
			{
				case BSP_MSGID_UART_RXOVER:
					if (UART_MsgDeal(&pMsg, DispTempBuf, sizeof(DispTempBuf)) == TRUE)
					{
						APP_DisplayRecvDataAnalyze(pMsg->pData, pMsg->DataLen); //数据解析
					}
					UART_FreeOneMsg(DISPLAY_UART, pMsg);
					break;
				case BSP_MSGID_CARD:
					APP_RecvCardAnalyze(pMsg);										//接收卡任务消息解析
					break;
				case BSP_MSGID_CONTROL:
					APP_RecvConectCCUAnalyze(pMsg);									//来自控制任务的消息
					break;
				case BSP_MSGID_METER:
					if(SYSSet.NetState == DISP_NET) //网络版本
					{
						//余额用完了
						if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
						{
							APP_Set_ERR_Branch(pMsg->pData[0],STOP_BALANCE);
							NB_WriterReason((_GUN_NUM)pMsg->DataLen,"E52",3);
							Net_StopCharge((_GUN_NUM)pMsg->pData[0]);  //云快充金额用完要主动停止
						}
					}
					else
					{
						NB_WriterReason((_GUN_NUM)pMsg->pData[0],"E50",3);
						SendStopChargeMsg((_GUN_NUM)pMsg->pData[0]);   							//发送停止帧
					}
					break;
				case BSP_4G_MAIN:		//来自APP发送过来的消息
					if(pMsg->DataLen >= GUN_MAX)  //枪号
					{
						break;
					}
					if(pMsg->DivNum ==  APP_START_CHARGE) //开始充电
					{
						if(DispControl.NetGunState[pMsg->DataLen] == GUN_IDLE)
						{
							if(DispControl.NetSSTState[pMsg->DataLen] != NET_STARTING)
							{
								HYDisplayMenu37();						//防止数据显示的是之前的数据
								Net_StartCharge((_GUN_NUM)pMsg->DataLen,MODE_AUTO);
							}
						}
					}
					if(pMsg->DivNum ==  APP_STOP_CHARGE) //结束充电
					{
						if(DispControl.NetGunState[pMsg->DataLen] == GUN_CHARGEING)
						{
							if(DispControl.NetSSTState[pMsg->DataLen] != NET_STOPING)
							{
								NB_WriterReason((_GUN_NUM)pMsg->DataLen,"E51",3);
								Net_StopCharge((_GUN_NUM)pMsg->DataLen);
							}
						}
					}
					if(pMsg->DivNum ==  APP_START_VIN) //VIN鉴权
					{
						if((_4G_GetStartType((_GUN_NUM)pMsg->DataLen) == _4G_APP_VIN) || (_4G_GetStartType((_GUN_NUM)pMsg->DataLen) == _4G_APP_BCVIN))
						{
							if(pMsg->pData[0])
							{
								//鉴权成功
								Set_VIN_Success((_GUN_NUM)pMsg->DataLen,VIN_SECCSEE);
							}
							else
							{
								Set_VIN_Success((_GUN_NUM)pMsg->DataLen,VIN_FAIL);
								//鉴权失败
							}
						}
					}
					if(pMsg->DivNum ==  APP_START_QRCODE) //二维码
					{
						if(pMsg->DataLen == GUN_A)
						{
							pMsg->pData[140] = 0;
							DisplayQRCode(0x0800,pMsg->pData,strlen((char *)pMsg->pData));
						}
						if(pMsg->DataLen == GUN_B)
						{
							pMsg->pData[140] = 0;
							DisplayQRCode(0x0880,pMsg->pData,strlen((char *)pMsg->pData));
						}
					}

					break;
				default:
					break;
			}
		}

		//获取当前界面
		CurMenu = GetCurMenu();                                         //保护
		if (CurMenu && CurMenu->function3)
		{
			CurMenu->function3();                                       //数据显示
		}
		if(	DispControl.CountDown == 1)									//倒计时减到1
		{
			//页面切换，构造当前页面的《上一页》或《主界面》按钮效果
			if((CurMenu == &HYMenu8) || (CurMenu == &HYMenu9) || (CurMenu == &HYMenu36) ||(CurMenu == &HYMenu37) \
			        || (CurMenu == &HYMenu42) || (CurMenu == &HYMenu43) || (CurMenu == &HYMenu44) || (CurMenu == &HYMenu32) || (CurMenu == &HYMenu33) )
			{
				//记录查询或者充电界面或者设备信息界面或者刷卡界面超时直接返回主界面
				key = LCD_KEY1;
			}
			else
			{
				key = LCD_KEY2;
			}
			if((CurMenu == &HYMenu36) && (SYSSet.SysSetNum.UseGun == 1) && (SYSSet.NetState == DISP_CARD))
			{
				NOP(); //单枪单机在充电界面不跳转
			}
			else
			{
				DealWithKey(&key);
			}
		}

//		#if(SYSSet.NetState == DISP_NET)
//		CurMenu = GetCurMenu();
//		//网络版本页面切换
//		if((CurMenu == &HYMenu73) || (CurMenu == &HYMenu74) || (CurMenu == &HYMenu75) || (CurMenu == &HYMenu76) \
//			|| (CurMenu == &HYMenu77) || (CurMenu == &HYMenu78) || (CurMenu == &HYMenu79))
//		{
//			ppage = Disp_NetPageDispos();
//			if(ppage != CurMenu)
//			{
//				DisplayCommonMenu(ppage,NULL);
//			}
//		}
//		#endif
		//500ms 进入周期性任务
		if(SYSSet.NetYXSelct == XY_66)
		{
			Display66GunQRCode();
		}
		if((curtime - lasttime) > SYS_DELAY_500ms)
		{
			lasttime = curtime;
			DispPeriod();
		}
	}
}
/************************(C)COPYRIGHT 2020 汇誉科技*****END OF FILE****************************/
