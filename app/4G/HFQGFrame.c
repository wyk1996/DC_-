/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------连接合肥乾古小程序---------------------------------------------
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
#include "4GRecv.h"
#include "HFQGFrame.h"
#include <string.h>
#include "sysconfig.h"
#include "DisplayMain.h"
#include "charging_Interface.h"
#include "PCUMain.h"
#include "RTC_task.h"
#include "DisplayMain.h"
/* Private define-----------------------------------------------------------------------------*/
#define HFQG_CMD_REGISTER   		0x01			//充电桩主动注册信息
#define HFQG_CMD_REREGISTER  		0x02			//pc返回确认信息

#define HFQG_CMD_HEAR				0x03			//桩主动发送心跳包
#define HFQG_CMD_REHEAR				0x04			//PC应答发送心跳包

#define HFQG_CMD_STATE				0X05			//充电桩的状态发送给PC
#define HFQG_CMD_RESTATE			0X06			//pc返回确认信息
#define HFQG_CMD_REQUERYSTATE		0X07			//pc发送查询状态，桩返回0X05指令应答

#define HFQG_CMD_STARTCHARGE		0x41			//桩确认开始充电指令
#define HFQG_CMD_RESTARTCHARGE		0x40			//PC发送开始充电指令

#define HFQG_CMD_CHARGESTATE		0x34			//当前此枪实时充电信息
#define HFQG_CMD_RECHARGESTATE		0x35			//pc返回确认实时充电信息
#define HFQG_CMD_REQUERYCHARGESTATE	0x50			//PC主动询问实时充电信息，桩返回0X34

#define HFQG_CMD_DEVSTOP			0x36			//桩主动终止充电
#define HFQG_CMD_REDEVSTOP			0x37			//后台确认桩主动终止充电

#define HFQG_CMD_REAPPSTOP			0x42			//后台主动终止充电
#define HFQG_CMD_APPSTOPACK			0x43			//桩终止充电，同时返回充电信息

#define HFQG_CMD_BILL				0x60			//充电桩上传账单结算信息
#define HFQG_CMD_REBILL				0x61			//后台确认账单结算信息


#define HFQG_RECV_FRAME_LEN   10
#define HFQG_SEND_FRAME_LEN   5



extern _PRICE_SET PriceSet;							//电价设置 

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
* Function     : get_xor_Data
* Description  : 异或校验
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U get_xor_Data(INT8U *pbuf,INT16U datalong)
{
	INT16U i;
	INT8U bbcdata;
	bbcdata = 0;
	for(i=0;i<datalong;i++)
	{
		bbcdata ^= pbuf[i];
	}
	return bbcdata;
}

/*****************************************************************************
* Function     : HFQGFreamSend
* Description  : 合肥乾古帧发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HFQGFreamSend(INT8U cmd,INT8U *pdata, INT16U len)
{
	INT8U Sendbuf[200];
	INT8U * pdevnum = APP_GetDevNum(); //设备号 (原始为ASCII)
	if((pdata == NULL) || (!len)  || (pdevnum == NULL) )
	{
		return FALSE;
	}
	//2字节帧投
	Sendbuf[0] = 0x5A;
	Sendbuf[1] = 0xA5;
	//1字节固定版本号
	Sendbuf[2] = 0x10;
	//1字节厂家
	Sendbuf[3] = 0x10;
	//2字节报文流水号
	Sendbuf[4] = 0x11;
	Sendbuf[5] = 0x00;
	//8字节终端号(BCD)
	Sendbuf[6] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	Sendbuf[7] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	Sendbuf[8] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	Sendbuf[9] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	Sendbuf[10] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	Sendbuf[11] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	Sendbuf[12] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	Sendbuf[13] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	
	//1字节命令字
	Sendbuf[14] = cmd;
	//2字节数据长度
	Sendbuf[15] = (len >> 8) & 0x00ff;
	Sendbuf[16] = len & 0x00ff;
	//N信息域
	memcpy(&Sendbuf[17],pdata,len);
	//1字节CS
	Sendbuf[17+len] = get_xor_Data(Sendbuf,17+len);
	//1字节结束
	Sendbuf[18+len] = 0x68;
	ModuleSIM7600_SendData(0, Sendbuf,(19+len)); //发送数据
	OSTimeDly(SYS_DELAY_1s); 
	return TRUE;
}

/**************************************************发送数据结构体********************************/
//注册帧
__packed typedef struct
{
	INT8U Devtype;		//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U GunNum;		//总共枪数量
	INT8U power[2];		//总功率
}_HFQG_SEND_REGISTER;

//充电桩总状态
__packed typedef struct
{
	INT8U Devtype;		//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U GunNum;		//总共枪数量
	INT8U power[2];		//总功率
	INT8U DevState;		//总体状态 0：正常 1：故障
	INT8U FailNum[4];	//系统故障代码 见 附录 1	
	INT8U DevAState;	//A枪运行状态 0：空闲 1：充电中 
	INT8U DevBState;	//B枪运行状态 0：空闲 1：充电中 
	INT8U GunAState;	//A枪位置状态 0：未插枪 1：已插枪
	INT8U GunBState;	//A枪位置状态 0：未插枪 1：已插枪 
	INT8U MeterA[4];	//A枪电表当前电度数KWH
	INT8U MeterB[4];	//B枪电表当前电度数KWH
	INT8U Latitude[10];	//充电桩经度：“113.123456”,Ascii格式，若无GPS定位，则默认填“000.000000”
	INT8U Veidoo[10];	//充电桩维度：“024.123456”,Ascii格式若无GPS定位，则默认填“000.000000”
	
}_HFQG_SEND_DEVSTATE;

//开始充电应答
__packed typedef struct
{
	INT8U DevType;		//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U CurGun;		//用户选择枪位置：0：A枪 1：B枪或者0~15
	INT8U UserType;		//用户类型：0：IC卡 1：微信号（手机号）
	INT8U UserNum[8];	//16位BCD型：ic卡号 000000-0123456789  卡号10位，多余位前面补0，微信号（手机号）：13696537467 多余位前面补0
	INT8U StartType;	//远程充电类型  0：立即充电 1：预约充电
	INT8U CmdResult;	//命令执行结果：0：执行失败（启动充电结果信息） 0xff：执行成功
	INT8U	Result;		//启动充电结果信息：0：命令执行成功   1：设备故障 2：未插枪
	
}_HFQG_SEND_STARTACK;



//实时充电信息
__packed typedef struct
{
	INT8U DevType;				//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U CurGun;				//用户选择枪位置：0：A枪 1：B枪或者0~15
	INT8U UserType;				//用户类型：0：IC卡 1：微信号（手机号）
	INT8U UserNum[8];			//16位BCD型：ic卡号 000000-0123456789  卡号10位，多余位前面补0，微信号（手机号）：13696537467 多余位前面补0
	INT8U ChargeMode;			//充电模式： 0：自动充满 1：定额充电 2：定量充电 3：定时充电 4：备用 5：手动充电
	INT8U StartPower[4];		//开始充电电度 1234.22KWH
	INT8U CurPower[4];			//当前充电电度 2000.32KWH
	INT8U AlreadyPower[4];		//已充电度 10.00KWH
	INT8U AllMoney[4];			//账户金额 1000.00元
	INT8U ChargeMoney[4];		//已充金额 100.00元
	INT8U Balance[4]; 			//账户余额  900.00元
	INT8U ChargeVol[2]; 		//充电电压：300.0v
	INT8U ChargeCur[2]; 		//充电电流：100.2A
	INT8U ChargePower[2]; 		//充电功率：30.0KW
	INT8U StartYear[2];			//开始充电时间：2016年
	INT8U StartMonth;			//开始充电时间：10月
	INT8U StartDay;				//开始充电时间：20日
	INT8U StartHour;			//开始充电时间：19时
	INT8U StartMinute;			//开始充电时间：10分
	INT8U StartSecond;			//开始充电时间：50秒
	INT8U AlreadyHour;			//已充小时：2小时
	INT8U AlreadyMinute;		//已充分钟：30分钟
	INT8U Time[2];				//剩余充电时间：200分钟
	INT8U SW1State;				//接触器1状态：0：断开 1：闭合
	INT8U SW2State;				//接触器2状态：0：断开 1：闭合
	INT8U LockState; 			//电子锁状态：0：断开 1：闭合
	INT8U StartSOC;				//开始SOC: 20%
	INT8U CurSOC;				//当前soc： 100%
	INT8U UnitBatteryminVol[2];	//单体最高电压：20.22V
	INT8U UnitBatteryMaxVol[2];	//单体最低电压：20.22V
	INT8U BatteryMaxTemp;   	//单体最高温度：0 to 250(-50 to 200)
	INT8U BatteryMinTemp;   	//单体最低温度：0 to 250(-50 to 200)
	INT8U BatteryAH[2];			//电池容量：200AH
	INT8U BatteryType;			//电池类型：
//	INT8U  Bmsneedvol[2];		//BMS请求电压：350.2V
//	INT8U  Bmsneedcur[2];		//BMS请求电压：350.2V
//	INT8U  CarVin[17];			//VIN车辆识别码，17位ASCII
}_HFQG_SEND_CHARGEDATA;

//充电桩主动发出终止充电指令。
__packed typedef struct
{
	INT8U DevType;				//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U CurGun;				//用户选择枪位置：0：A枪 1：B枪或者0~15
	INT8U UserType;				//用户类型：0：IC卡 1：微信号（手机号）
	INT8U UserNum[8];			//16位BCD型：ic卡号 000000-0123456789  卡号10位，多余位前面补0，微信号（手机号）：13696537467 多余位前面补0
	INT8U DevStop;				//终端终止充电特征字： 0x00
	INT8U StopType;				//0：人工终止 1：自动充满 2：后台终止 3：故障终止（故障终止显示原因）
	INT8U SYSFailNnum[4];		//系统故障终止原因见：附录1
	INT8U ChargeFailNnum[2];	//充电故障终止原因见：附录2
}_HFQG_SEND_DEVSTOP;



//充电桩确认终止充电指令
__packed typedef struct
{
	INT8U DevType;				//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U CurGun;				//用户选择枪位置：0：A枪 1：B枪或者0~15
	INT8U UserType;				//用户类型：0：IC卡 1：微信号（手机号）
	INT8U UserNum[8];			//16位BCD型：ic卡号 000000-0123456789  卡号10位，多余位前面补0，微信号（手机号）：13696537467 多余位前面补0
	INT8U StopState;			//终端终止充电类型：0x00：终止不成功0xff：终止充电成功
	INT8U StopType;				//终止充电 类型： 0：网页1：微信
}_HFQG_SEND_STOPACK;


//终端上传账单结算指令
__packed typedef struct
{
	INT8U ChargeMoney[4];		//充电金额：100.00元
	INT8U ChargeKwh[4];			//已充电度 10.00KWH
	INT8U ChargeServiceh[4];	//服务费：100.00元
	INT8U StopCarMoney[4];		//停车费：100.00元
}_BILL_INFO;


__packed typedef struct
{
	INT8U DevType;				//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U CurGun;				//用户选择枪位置：0：A枪 1：B枪或者0~15
	INT8U UserType;				//用户类型：0：IC卡 1：微信号（手机号）
	INT8U UserNum[8];			//16位BCD型：ic卡号 000000-0123456789  卡号10位，多余位前面补0，微信号（手机号）：13696537467 多余位前面补0
	INT8U BatchNum[15];			//交易流水号（BCD）：为账号+年月日时分秒，例如：000000123456789020161111180335
	INT8U StopType;				//终止充电原因： 0：人工终止，1自动充满，2后台终止，3故障终止
	INT8U StartYear[2];			//开始充电时间：2016年
	INT8U StartMonth;			//开始充电时间：10月
	INT8U StartDay;				//开始充电时间：20日
	INT8U StartHour;			//开始充电时间：19时
	INT8U StartMinute;			//开始充电时间：10分
	INT8U StartSecond;			//开始充电时间：50秒
	INT8U StopYear[2];			//结束充电时间：2016年
	INT8U StopMonth;			//结束充电时间：10月
	INT8U StopDay;				//结束充电时间：20日
	INT8U StopHour;				//结束充电时间：19时
	INT8U StopMinute;			//结束充电时间：10分
	INT8U StopSecond;			//结束充电时间：50秒
	INT8U AlreadyHour;			//已充小时：2小时
	INT8U AlreadyMinute;		//已充分钟：30分钟
	INT8U StartPower[4];		//开始充电电度 1234.22KWH
	INT8U StopPower[4];			//结束充电电度 2000.32KWH
	INT8U ChargePower[4];		//已充电度 10.00KWH
	INT8U StartMoney[4];		//充电前卡余额： 1000.00元
	INT8U UserMoney[4];			//本次充电金额： 100.00元
	INT8U ChargeMode;			//充电模式： 0：自动充满 1：定额充电 2：定量充电 3：定时充电 4：备用 5：手动充电
	INT8U StartSoc;				//开始SOC.12%
	INT8U StopSoc;				//结束SOC.60%
	INT8U res;					//备用
	//_BILL_INFO JiInfo1;			//尖峰1信息
	//_BILL_INFO JiInfo2;			//尖峰2信息
	//_BILL_INFO JiInfo3;			//尖峰3信息
	//_BILL_INFO GaoInfo1;		//高峰1信息
	//_BILL_INFO GaoInfo2;		//高峰2信息
	//_BILL_INFO GaoInfo3;		//高峰3信息
	//_BILL_INFO PingInfo1;		//平峰1信息
	//_BILL_INFO PingInfo2;		//平峰2信息
	//_BILL_INFO PingInfo3;		//平峰3信息
	//_BILL_INFO GuInfo1;			//古峰1信息
	//_BILL_INFO GuInfo2;			//古峰2信息
	//_BILL_INFO GuInfo3;			//古峰3信息
//	INT8U AllChargeMoney[4];	//本次充电总电费： 100.00元
//	INT8U AllChargeServiceh[4]; //本次充电总服务费： 100.00元
//	INT8U AllStopCarMoney[4];	//本次充电总服务费： 100.00元
	
}_HFQG_SEND_BILL;



_HFQG_SEND_REGISTER  	SFrame_register;		//注册帧发送
_HFQG_SEND_DEVSTATE  	SFrame_DevState;		//充电桩总状态
_HFQG_SEND_STARTACK		SFrame_StartAck;		//开始充电应答
_HFQG_SEND_CHARGEDATA	SFrame_ChargeData;		//实时充电数据
_HFQG_SEND_DEVSTOP		SFrame_DevStop;			//充电桩主动发出终止充电指令。
_HFQG_SEND_STOPACK		SFrame_StopAck;			//充电桩确认终止充电指令
_HFQG_SEND_BILL			SFrame_Bill;			//充电桩结算信息


INT8U   HFQG_SendRegister(void); //合肥乾古发送注册帧
INT8U   HFQG_SendHear(void)  ;//心跳
INT8U   HFQG_SendDevState(void); //充电桩总状态
INT8U   HFQG_SendChargeGunA(void);  //充电实时数据
INT8U   HFQG_SendChargeGunB(void);  //充电实时数据

_4G_SEND_TABLE HFQGSendTable[HFQG_SEND_FRAME_LEN] = {
	{0,    0,    SYS_DELAY_10s, 	HFQG_SendRegister		},  //发送注册帧

	{0,    0,    SYS_DELAY_5s, 		HFQG_SendHear			},	//心跳
	
	{0,    0,    SYS_DELAY_10s, 	HFQG_SendDevState		},	//充电桩总状态

	{0,    0,    SYS_DELAY_10s, 	HFQG_SendChargeGunA		},	//充电桩总状态
	
	{0,    0,    SYS_DELAY_10s, 	HFQG_SendChargeGunB		},	//充电桩总状态
	
};

/*****************************************************************************
* Function     : HFQG_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HFQG_SendFrameDispose(void)
{
	INT8U i;

	for(i = 0;i < HFQG_SEND_FRAME_LEN;i++)
	{
		if(HFQGSendTable[i].cycletime == 0)
		{
			continue;
		}
		HFQGSendTable[i].curtime = OSTimeGet();
		if((HFQGSendTable[i].curtime >= HFQGSendTable[i].lasttime) ? ((HFQGSendTable[i].curtime - HFQGSendTable[i].lasttime) >= HFQGSendTable[i].cycletime) : \
		((HFQGSendTable[i].curtime + (0xFFFFFFFFu - HFQGSendTable[i].lasttime)) >= HFQGSendTable[i].cycletime))
		{
			HFQGSendTable[i].lasttime = HFQGSendTable[i].curtime;
			if(HFQGSendTable[i].Sendfunc != NULL)
			{
				HFQGSendTable[i].Sendfunc();
			}
		}
		
	}
	return TRUE;
}


/**************************************************接收数据结构体********************************/

__packed typedef struct
{
	INT8U 	StartHour1;		//开始时间整点00
	INT8U 	StartMinute1;	//开始时间分钟00
	INT8U 	StopHour1;		//结束时间整点03
	INT8U 	StopMinute1;		//结束时间分钟00   注：00:00~03:00
	INT8U 	StartHour2;		//开始时间整点00
	INT8U 	StartMinute2;	//开始时间分钟00
	INT8U 	StopHour2;		//结束时间整点03
	INT8U 	StopMinute2;		//结束时间分钟00   注：00:00~03:00
	INT16U ChargeMoney;		//02.50元/KWH
	INT16U ChargeServiceh; 	//02.50元/KWH
	INT16U StopCarMoney;	//02.50元/小时
}_APP_BILL_INFO;

__packed typedef struct
{
	INT8U DevType;				//0：交流单充 1：交流双充 2：直流单充 3：直流双充 16：多路单充
	INT8U CurGun;				//用户选择枪位置：0：A枪 1：B枪或者0~15
	INT8U UserType;				//用户类型：0：IC卡 1：微信号（手机号）
	INT8U UserNum[8];			//16位BCD型：ic卡号 000000-0123456789  卡号10位，多余位前面补0，微信号（手机号）：13696537467 多余位前面补0
	INT8U BatchNum[15];			//交易流水号（BCD）：为账号+年月日时分秒，例如：000000123456789020161111180335
	INT32U Money;				//用户账户余额 1000.00元
	INT8U StartType;			//远程充电类型  0：立即充电 1：预约充电
	INT8U res;					//启动特征字： 0xff
	_APP_BILL_INFO JIBill;		//尖峰	
	_APP_BILL_INFO GaoBill;	//高峰	
	_APP_BILL_INFO PingBill;	//平峰
	_APP_BILL_INFO GuBill;		//谷峰	
}_HFQG_RECV_STARTCHARGE;


_HFQG_RECV_STARTCHARGE HFQGRecvStartCharge[GUN_MAX];   //A、B枪启动信息


/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetHFQGBatchNum(INT8U gun)
{
	static INT8U buf[16];		//交易流水号位16个字节，ASICC 因此取后16个数字
	INT8U i;
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	for(i = 0;i < 8;i++)
	{
		buf[2*i] = ((HFQGRecvStartCharge[gun].BatchNum[i+7] >> 4) & 0x0f) + 0x30;
		buf[2*i + 1] = (HFQGRecvStartCharge[gun].BatchNum[i+7] & 0x0f) + 0x30;
		
	}
	return buf;
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
INT32U APP_GetHFQGNetMoney(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	return HFQGRecvStartCharge[gun].Money;
}


INT8U   HFQG_RecvRegisterAck(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvHearAck(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvDevStateAck(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvDevStateQuery(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvStartCharge(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvChargeInfoAck(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvChargeInfoQuery(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvStopChargeAck(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvStopCharge(INT8U *pdata,INT16U len);
INT8U   HFQG_RecvBillAck(INT8U *pdata,INT16U len);


_4G_RECV_TABLE HFQGRecvTable[HFQG_RECV_FRAME_LEN] = {
{HFQG_CMD_REREGISTER			,	HFQG_RecvRegisterAck	}, 		//pc返回确认信息

{HFQG_CMD_REHEAR				,  	HFQG_RecvHearAck		},		//PC应答发送心跳包

{HFQG_CMD_RESTATE				,  	HFQG_RecvDevStateAck	},		//pc返回确认信息

{HFQG_CMD_REQUERYCHARGESTATE	,  	HFQG_RecvDevStateQuery	}, 		//pc发送查询状态，桩返回0X05指令应答

{HFQG_CMD_RESTARTCHARGE			,  	HFQG_RecvStartCharge	},		//PC发送开始充电指令

{HFQG_CMD_RECHARGESTATE			,  	HFQG_RecvChargeInfoAck	},		//pc返回确认实时充电信息

{HFQG_CMD_REQUERYCHARGESTATE	,  	HFQG_RecvChargeInfoQuery},		//PC主动询问实时充电信息，桩返回0X34

{HFQG_CMD_REDEVSTOP				,  	HFQG_RecvStopChargeAck	},		//后台确认桩主动终止充电

{HFQG_CMD_REAPPSTOP				,  	HFQG_RecvStopCharge		},		//后台主动终止充电

{HFQG_CMD_REBILL				,  	HFQG_RecvBillAck		},		//后台确认账单结算信息

};



/*****************************************************************************
* Function     : HFQG_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HFQG_RecvFrameDispose(INT8U * pdata,INT16U len)
{
	INT8U i = 0;
	INT8U cmd;
	INT16U datalen;
	static INT8U buf[100];

	if((pdata == NULL) || (len < 15) )
	{
		return FALSE;
	}
	//帧头帧尾判断
	if((pdata[0] != 0x5A) || (pdata[1] != 0xA5) || (pdata[len-1] !=0X68))
	{
		return FALSE;
	}
	//提取数据长度
	datalen= (pdata[15] << 8) | pdata[16];
	if(((datalen+19) != len) ||(datalen >100) )
	{
		return FALSE;
	}
	cmd = pdata[14];  //提取命令字
	//提取数据
	memcpy(buf,&pdata[17],datalen);
	
	for(i = 0;i < HFQG_RECV_FRAME_LEN;i++)
	{
		if(HFQGRecvTable[i].cmd == cmd)
		{
			if(HFQGRecvTable[i].recvfunction != NULL)
			{
				HFQGRecvTable[i].recvfunction(buf,datalen);
			}
			break;
		}
	}
	return TRUE;
}

/**************************************************发送帧****************************************/
/*****************************************************************************
* Function     : HFQG_SendRegister
* Description  : 合肥乾古发送注册帧
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日完成
*****************************************************************************/
INT8U   HFQG_SendRegister(void)
{

	static INT32U i = 0;
	if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
	{
		if(++i >= 6*60*24)		//存在已经后台已经断开，还是能收到心跳，因此注册成功后也要每隔24H发送注册帧
		{	
			i = 0;
			SFrame_register.Devtype = 0x03; //3：直流双充 
			SFrame_register.GunNum = 0x02;
			SFrame_register.power[0] = (1200 >> 8) & 0x00ff;
			SFrame_register.power[1] = 1200 & 0x00ff;

			return HFQGFreamSend(HFQG_CMD_REGISTER,(INT8U*)&SFrame_register,sizeof(_HFQG_SEND_REGISTER));
		}
	}
	else
	{
		SFrame_register.Devtype = 0x03; //3：直流双充 
		SFrame_register.GunNum = 0x02;
		SFrame_register.power[0] = (1200 >> 8) & 0x00ff;
		SFrame_register.power[1] = 1200 & 0x00ff;

		return HFQGFreamSend(HFQG_CMD_REGISTER,(INT8U*)&SFrame_register,sizeof(_HFQG_SEND_REGISTER));
	}
	return TRUE;
}


//心跳
/*****************************************************************************
* Function     : HFQG_SendHear
* Description  : 合肥乾古发送心跳帧
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   HFQG_SendHear(void)
{
	INT8U hear = 0x00;

	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}

	return HFQGFreamSend(HFQG_CMD_HEAR,&hear,sizeof(hear));
}

//充电桩总状态

/*****************************************************************************
* Function     : HFQG_SendDevState
* Description  : 合肥乾古发送充电桩总状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日完成
*****************************************************************************/
INT8U   HFQG_SendDevState(void)
{
	TEMPDATA * pmeter = NULL;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	SFrame_DevState.Devtype = 0x03; //3：直流双充 
	SFrame_DevState.GunNum = 0x02;
	SFrame_DevState.power[1] = 1200 & 0x00ff;
	SFrame_DevState.power[0] = (1200 >> 8)& 0x00ff;

	if((APP_GetErrState(GUN_A) == 0x04) && (APP_GetErrState(GUN_B) == 0x04) )
	{
		SFrame_DevState.DevState = 0x01;
	}
	else
	{
		SFrame_DevState.DevState = 0x00;
	}
	//未处理！   系统故障代码 见 附录 1	 
	memset(SFrame_DevState.FailNum,0,sizeof(SFrame_DevState.FailNum));
	//充电枪是否充电
	(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING) ? (SFrame_DevState.DevAState = 0x01) : (SFrame_DevState.DevAState = 0x00);
	(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING) ? (SFrame_DevState.DevBState = 0x01) : (SFrame_DevState.DevBState = 0x00);
	//是否插枪
	((GetGunState(GUN_A) == GUN_CONNECTED)) ? (SFrame_DevState.GunAState = 0x01) : (SFrame_DevState.GunAState = 0x00);
	((GetGunState(GUN_B) == GUN_CONNECTED)) ? (SFrame_DevState.GunBState = 0x01) : (SFrame_DevState.GunBState = 0x00);
	pmeter = APP_GetMeterPowerInfo(GUN_A);
	SFrame_DevState.MeterA[3] = pmeter->StartPower & 0x000000ff;
	SFrame_DevState.MeterA[2] = (pmeter->StartPower >> 8) & 0x000000ff;
	SFrame_DevState.MeterA[1] = (pmeter->StartPower >> 16) & 0x000000ff;
	SFrame_DevState.MeterA[0] = (pmeter->StartPower >> 24)& 0x000000ff;
	pmeter = APP_GetMeterPowerInfo(GUN_B);
	SFrame_DevState.MeterB[3] = pmeter->StartPower & 0x000000ff;
	SFrame_DevState.MeterB[2] = (pmeter->StartPower >> 8) & 0x000000ff;
	SFrame_DevState.MeterB[1] = (pmeter->StartPower >> 16) & 0x000000ff;
	SFrame_DevState.MeterB[0] = (pmeter->StartPower >> 24)& 0x000000ff;
	memcpy(SFrame_DevState.Latitude,"000.000000",sizeof(SFrame_DevState.Latitude));
	memcpy(SFrame_DevState.Veidoo,"000.000000",sizeof(SFrame_DevState.Veidoo));
	//return HFQGFreamSend(HFQG_CMD_STATE,(INT8U*)&SFrame_DevState,sizeof(_HFQG_SEND_DEVSTATE));
	return TRUE;
	
}


//充电实时数据
/*****************************************************************************
* Function     : HFQG_SendCharge
* Description  : 合肥乾古开始发送充电数据帧
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HFQG_SendChargeGunA(void)
{
	
	TEMPDATA * pmeter = NULL;
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	//启动中或者充电中发送
	if((pdisp_conrtol->NetGunState[GUN_A] != GUN_STARTING) && (pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING)) 
	{
		return FALSE;   //没有再充电，无需发送
	}
	//以下表示没有填充
	//INT8U ChargeMode;			//充电模式： 0：自动充满 1：定额充电 2：定量充电 3：定时充电 4：备用 5：手动充电
	//INT8U AllMoney[4];			//账户金额 1000.00元
	//INT8U Balance[4]; 			//账户余额  900.00元
	//INT8U StartSOC;				//开始SOC: 20%
	SFrame_ChargeData.DevType = 0x03; //3：直流双充 
	SFrame_ChargeData.CurGun = GUN_A;
	SFrame_ChargeData.UserType = 0x01; //1：微信号（手机号）
	SFrame_ChargeData.ChargeMode = 0x00;	//充电模式自动充满

	pmeter = APP_GetMeterPowerInfo(GUN_A);

	memcpy(SFrame_ChargeData.UserNum,HFQGRecvStartCharge[GUN_A].UserNum,sizeof(SFrame_ChargeData.UserNum));
	SFrame_ChargeData.StartPower[3] = pmeter->StartPower & 0x000000ff;
	SFrame_ChargeData.StartPower[2] = (pmeter->StartPower >> 8) & 0x000000ff;
	SFrame_ChargeData.StartPower[1] = (pmeter->StartPower >> 16) & 0x000000ff;
	SFrame_ChargeData.StartPower[0] = (pmeter->StartPower >> 24)& 0x000000ff;
	
	SFrame_ChargeData.CurPower[3] = pmeter->CurPower & 0x000000ff;
	SFrame_ChargeData.CurPower[2] = (pmeter->CurPower >> 8) & 0x000000ff;
	SFrame_ChargeData.CurPower[1] = (pmeter->CurPower >> 16) & 0x000000ff;
	SFrame_ChargeData.CurPower[0] = (pmeter->CurPower >> 24)& 0x000000ff;

	SFrame_ChargeData.AlreadyPower[3] = (puserinfo->TotalPower) & 0x000000ff;
	SFrame_ChargeData.AlreadyPower[2] = ((puserinfo->TotalPower) >> 8) & 0x000000ff;
	SFrame_ChargeData.AlreadyPower[1] = ((puserinfo->TotalPower) >> 16) & 0x000000ff;
	SFrame_ChargeData.AlreadyPower[0] = ((puserinfo->TotalPower) >> 24)& 0x000000ff;

	SFrame_ChargeData.ChargeMoney[3] = (puserinfo->TotalBill/100) & 0x000000ff;
	SFrame_ChargeData.ChargeMoney[2] = ((puserinfo->TotalBill/100) >> 8) & 0x000000ff;
	SFrame_ChargeData.ChargeMoney[1] = ((puserinfo->TotalBill/100) >> 16) & 0x000000ff;
	SFrame_ChargeData.ChargeMoney[0] = ((puserinfo->TotalBill/100) >> 24)& 0x000000ff;

	SFrame_ChargeData.AllMoney[3] = (HFQGRecvStartCharge[GUN_A].Money) & 0x000000ff;
	SFrame_ChargeData.AllMoney[2] = (HFQGRecvStartCharge[GUN_A].Money >> 8) & 0x000000ff;
	SFrame_ChargeData.AllMoney[1] = (HFQGRecvStartCharge[GUN_A].Money >> 16) & 0x000000ff;
	SFrame_ChargeData.AllMoney[0] = (HFQGRecvStartCharge[GUN_A].Money >> 24)& 0x000000ff;

	SFrame_ChargeData.Balance[3] = (HFQGRecvStartCharge[GUN_A].Money) & 0x000000ff;
	SFrame_ChargeData.Balance[2] = (HFQGRecvStartCharge[GUN_A].Money >> 8) & 0x000000ff;
	SFrame_ChargeData.Balance[1] = (HFQGRecvStartCharge[GUN_A].Money >> 16) & 0x000000ff;
	SFrame_ChargeData.Balance[0] = (HFQGRecvStartCharge[GUN_A].Money >> 24)& 0x000000ff;


	SFrame_ChargeData.ChargeVol[1]  = PowerModuleInfo[GUN_A].OutputInfo.Vol & 0x00ff;
	SFrame_ChargeData.ChargeVol[0]  =  (PowerModuleInfo[GUN_A].OutputInfo.Vol >> 8) & 0x00ff;

	SFrame_ChargeData.ChargeCur[1]  =  PowerModuleInfo[GUN_A].OutputInfo.Cur & 0x00ff;
	SFrame_ChargeData.ChargeCur[0]  =  (PowerModuleInfo[GUN_A].OutputInfo.Cur  >> 8) & 0x00ff;

	SFrame_ChargeData.ChargePower[1]  =  (PowerModuleInfo[GUN_A].OutputInfo.Vol * PowerModuleInfo[GUN_A].OutputInfo.Cur /10000) & 0x00ff;
	SFrame_ChargeData.ChargePower[0]  =  ((PowerModuleInfo[GUN_A].OutputInfo.Vol * PowerModuleInfo[GUN_A].OutputInfo.Cur /10000)  >> 8) & 0x00ff;

	SFrame_ChargeData.StartYear[1] =  (BCDtoHEX(puserinfo->StartTime.Year) + 0x2000) & 0x00ff;
	SFrame_ChargeData.StartYear[0] =  ((BCDtoHEX(puserinfo->StartTime.Year) +0x2000)>> 8) & 0x00ff;

	SFrame_ChargeData.StartMonth =  BCDtoHEX(puserinfo->StartTime.Month);
	SFrame_ChargeData.StartDay =  BCDtoHEX(puserinfo->StartTime.Day);
	SFrame_ChargeData.StartHour =  BCDtoHEX(puserinfo->StartTime.Hour);
	SFrame_ChargeData.StartMinute =  BCDtoHEX(puserinfo->StartTime.Minute);
	SFrame_ChargeData.StartSecond =  BCDtoHEX(puserinfo->StartTime.Second);
	SFrame_ChargeData.AlreadyHour = puserinfo->ChargeTime / 60;
	SFrame_ChargeData.AlreadyMinute = puserinfo->ChargeTime % 60;
	
	SFrame_ChargeData.Time[1] =	BMS_BCS_Context[GUN_A].RemainderTime & 0x00ff;
	SFrame_ChargeData.Time[0] = (BMS_BCS_Context[GUN_A].RemainderTime  >> 8)& 0x00ff;

	SFrame_ChargeData.SW1State = 1;
	SFrame_ChargeData.SW2State = 1;
	SFrame_ChargeData.LockState = 1;
	
	SFrame_ChargeData.CurSOC = BMS_BCS_Context[GUN_A].SOC;
	
	SFrame_ChargeData.UnitBatteryminVol[1] = BMS_BCS_Context[GUN_A].MaxUnitVolandNum & 0x00ff;
	SFrame_ChargeData.UnitBatteryminVol[0] = (BMS_BCS_Context[GUN_A].MaxUnitVolandNum  >> 8) & 0x000f;

//	SFrame_ChargeData.UnitBatteryMaxVol[1] = BMS_BCS_Context[GUN_A].MaxUnitVolandNum & 0x00ff;
//	SFrame_ChargeData.UnitBatteryMaxVol[0] = (BMS_BCS_Context[GUN_A].MaxUnitVolandNum  >> 8) & 0x000f;
	
	SFrame_ChargeData.BatteryMaxTemp = BMS_BSM_Context[GUN_A].MaxbatteryTemprature;

	SFrame_ChargeData.BatteryMinTemp = BMS_BSM_Context[GUN_A].MinbatteryTemprature;

	SFrame_ChargeData.BatteryAH[1] = (BMS_BCP_Context[GUN_A].BatteryWholeEnergy / 10) &  0x00ff;
	SFrame_ChargeData.BatteryAH[0] =( (BMS_BCP_Context[GUN_A].BatteryWholeEnergy / 10)  >> 8)  &  0x00ff;

	SFrame_ChargeData.BatteryType = BMS_BRM_Context[GUN_A].BatteryType;
	
//	SFrame_ChargeData.Bmsneedvol[1] = BMS_BCL_Context[GUN_A].DemandVol & 0x00ff;
//	SFrame_ChargeData.Bmsneedvol[0] = (BMS_BCL_Context[GUN_A].DemandVol>> 8) & 0x00ff;

//	SFrame_ChargeData.Bmsneedcur[1] =  (4000- BMS_BCL_Context[GUN_A].DemandCur) & 0x00ff;
//	SFrame_ChargeData.Bmsneedcur[0] = ( (4000- BMS_BCL_Context[GUN_A].DemandCur) >> 8) & 0x00ff;
//	
//	memcpy(SFrame_ChargeData.CarVin,BMS_BRM_Context[GUN_A].VIN.vin,sizeof(SFrame_ChargeData.CarVin));
	
	return HFQGFreamSend(HFQG_CMD_CHARGESTATE,(INT8U*)&SFrame_ChargeData,sizeof(_HFQG_SEND_CHARGEDATA));

}


//充电实时数据
/*****************************************************************************
* Function     : HFQG_SendCharge
* Description  : 合肥乾古开始发送充电数据帧
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HFQG_SendChargeGunB(void)
{
	TEMPDATA * pmeter = NULL;
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	//启动中或者充电中发送
	if((pdisp_conrtol->NetGunState[GUN_B] != GUN_STARTING) && (pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)) 
	{
		return FALSE;   //没有再充电，无需发送
	}
	//以下表示没有填充

	//INT8U ChargeMode;			//充电模式： 0：自动充满 1：定额充电 2：定量充电 3：定时充电 4：备用 5：手动充电
	//INT8U AllMoney[4];			//账户金额 1000.00元
	//INT8U Balance[4]; 			//账户余额  900.00元
	//INT8U StartSOC;				//开始SOC: 20%
	SFrame_ChargeData.DevType = 0x03; //3：直流双充 
	SFrame_ChargeData.CurGun = GUN_B;
	SFrame_ChargeData.UserType = 0x01; //1：微信号（手机号）
	SFrame_ChargeData.ChargeMode = 0x00;	//充电模式自动充满

	memcpy(SFrame_ChargeData.UserNum,HFQGRecvStartCharge[GUN_B].UserNum,sizeof(SFrame_ChargeData.UserNum));

	pmeter = APP_GetMeterPowerInfo(GUN_B);
	SFrame_ChargeData.StartPower[3] = pmeter->StartPower & 0x000000ff;
	SFrame_ChargeData.StartPower[2] = (pmeter->StartPower >> 8) & 0x000000ff;
	SFrame_ChargeData.StartPower[1] = (pmeter->StartPower >> 16) & 0x000000ff;
	SFrame_ChargeData.StartPower[0] = (pmeter->StartPower >> 24)& 0x000000ff;
	
	SFrame_ChargeData.CurPower[3] = pmeter->CurPower & 0x000000ff;
	SFrame_ChargeData.CurPower[2] = (pmeter->CurPower >> 8) & 0x000000ff;
	SFrame_ChargeData.CurPower[1] = (pmeter->CurPower >> 16) & 0x000000ff;
	SFrame_ChargeData.CurPower[0] = (pmeter->CurPower >> 24)& 0x000000ff;

	SFrame_ChargeData.AlreadyPower[3] = (puserinfo->TotalPower) & 0x000000ff;
	SFrame_ChargeData.AlreadyPower[2] = ((puserinfo->TotalPower) >> 8) & 0x000000ff;
	SFrame_ChargeData.AlreadyPower[1] = ((puserinfo->TotalPower) >> 16) & 0x000000ff;
	SFrame_ChargeData.AlreadyPower[0] = ((puserinfo->TotalPower) >> 24)& 0x000000ff;

	SFrame_ChargeData.ChargeMoney[3] = (puserinfo->TotalBill/100) & 0x000000ff;
	SFrame_ChargeData.ChargeMoney[2] = ((puserinfo->TotalBill/100) >> 8) & 0x000000ff;
	SFrame_ChargeData.ChargeMoney[1] = ((puserinfo->TotalBill/100) >> 16) & 0x000000ff;
	SFrame_ChargeData.ChargeMoney[0] = ((puserinfo->TotalBill/100) >> 24)& 0x000000ff;


	SFrame_ChargeData.AllMoney[3] = (HFQGRecvStartCharge[GUN_B].Money) & 0x000000ff;
	SFrame_ChargeData.AllMoney[2] = (HFQGRecvStartCharge[GUN_B].Money >> 8) & 0x000000ff;
	SFrame_ChargeData.AllMoney[1] = (HFQGRecvStartCharge[GUN_B].Money >> 16) & 0x000000ff;
	SFrame_ChargeData.AllMoney[0] = (HFQGRecvStartCharge[GUN_B].Money >> 24)& 0x000000ff;

	SFrame_ChargeData.Balance[3] = (HFQGRecvStartCharge[GUN_B].Money) & 0x000000ff;
	SFrame_ChargeData.Balance[2] = (HFQGRecvStartCharge[GUN_B].Money >> 8) & 0x000000ff;
	SFrame_ChargeData.Balance[1] = (HFQGRecvStartCharge[GUN_B].Money >> 16) & 0x000000ff;
	SFrame_ChargeData.Balance[0] = (HFQGRecvStartCharge[GUN_B].Money >> 24)& 0x000000ff;
	
	SFrame_ChargeData.ChargeVol[1]  =  PowerModuleInfo[GUN_B].OutputInfo.Vol & 0x00ff;
	SFrame_ChargeData.ChargeVol[0]  =  (PowerModuleInfo[GUN_B].OutputInfo.Vol >> 8) & 0x00ff;

	SFrame_ChargeData.ChargeCur[1]  = PowerModuleInfo[GUN_B].OutputInfo.Cur & 0x00ff;
	SFrame_ChargeData.ChargeCur[0]  = (PowerModuleInfo[GUN_B].OutputInfo.Cur >> 8) & 0x00ff;

	SFrame_ChargeData.ChargePower[1]  =  (PowerModuleInfo[GUN_B].OutputInfo.Vol * PowerModuleInfo[GUN_B].OutputInfo.Cur /10000) & 0x00ff;
	SFrame_ChargeData.ChargePower[0]  =  ((PowerModuleInfo[GUN_B].OutputInfo.Vol * PowerModuleInfo[GUN_B].OutputInfo.Cur /10000)  >> 8) & 0x00ff;

	SFrame_ChargeData.StartYear[1] =  (BCDtoHEX(puserinfo->StartTime.Year) + 0x2000) & 0x00ff;
	SFrame_ChargeData.StartYear[0] =  ((BCDtoHEX(puserinfo->StartTime.Year) + 0x2000 )>> 8) & 0x00ff;

	SFrame_ChargeData.StartMonth =  BCDtoHEX(puserinfo->StartTime.Month);
	SFrame_ChargeData.StartDay =  BCDtoHEX(puserinfo->StartTime.Day);
	SFrame_ChargeData.StartHour =  BCDtoHEX(puserinfo->StartTime.Hour);
	SFrame_ChargeData.StartMinute =  BCDtoHEX(puserinfo->StartTime.Minute);
	SFrame_ChargeData.StartSecond =  BCDtoHEX(puserinfo->StartTime.Second);
	SFrame_ChargeData.AlreadyHour = puserinfo->ChargeTime / 60;
	SFrame_ChargeData.AlreadyMinute = puserinfo->ChargeTime % 60;
	
	SFrame_ChargeData.Time[1] = BMS_BCS_Context[GUN_B].RemainderTime & 0x00ff;
	SFrame_ChargeData.Time[0] = (BMS_BCS_Context[GUN_B].RemainderTime  >> 8)& 0x00ff;

	SFrame_ChargeData.SW1State = 1;
	SFrame_ChargeData.SW2State = 1;
	SFrame_ChargeData.LockState = 1;

	SFrame_ChargeData.CurSOC = BMS_BCS_Context[GUN_B].SOC;
	
		SFrame_ChargeData.UnitBatteryminVol[1] = BMS_BCS_Context[GUN_B].MaxUnitVolandNum & 0x00ff;
	SFrame_ChargeData.UnitBatteryminVol[0] = (BMS_BCS_Context[GUN_B].MaxUnitVolandNum  >> 8) & 0x000f;

//	SFrame_ChargeData.UnitBatteryMaxVol[1] = BMS_BCS_Context[GUN_B].MaxUnitVolandNum & 0x00ff;
//	SFrame_ChargeData.UnitBatteryMaxVol[0] = (BMS_BCS_Context[GUN_B].MaxUnitVolandNum  >> 8) & 0x000f;
	
	SFrame_ChargeData.BatteryMaxTemp = BMS_BSM_Context[GUN_B].MaxbatteryTemprature;

	SFrame_ChargeData.BatteryMinTemp = BMS_BSM_Context[GUN_B].MinbatteryTemprature;

	SFrame_ChargeData.BatteryAH[1] = (BMS_BCP_Context[GUN_B].BatteryWholeEnergy / 10) &  0x00ff;
	SFrame_ChargeData.BatteryAH[0] =( (BMS_BCP_Context[GUN_B].BatteryWholeEnergy / 10)  >> 8)  &  0x00ff;

	SFrame_ChargeData.BatteryType = BMS_BRM_Context[GUN_B].BatteryType;
	
//	SFrame_ChargeData.Bmsneedvol[1] = BMS_BCL_Context[GUN_B].DemandVol & 0x00ff;
//	SFrame_ChargeData.Bmsneedvol[0] = (BMS_BCL_Context[GUN_B].DemandVol>> 8) & 0x00ff;

//	SFrame_ChargeData.Bmsneedcur[1] = (4000- BMS_BCL_Context[GUN_B].DemandCur) & 0x00ff;
//	SFrame_ChargeData.Bmsneedcur[0] = ( (4000- BMS_BCL_Context[GUN_B].DemandCur) >> 8) & 0x00ff;
//	
//	memcpy(SFrame_ChargeData.CarVin,BMS_BRM_Context[GUN_B].VIN,sizeof(SFrame_ChargeData.CarVin));
	
	return HFQGFreamSend(HFQG_CMD_CHARGESTATE,(INT8U*)&SFrame_ChargeData,sizeof(_HFQG_SEND_CHARGEDATA));

}

//开始充电
/*****************************************************************************
* Function     : HFQG_SendStartAck
* Description  : 合肥乾古开始充电应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HFQG_SendStartAck(_GUN_NUM gun)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();

	if((gun >= GUN_MAX) || (pdisp_conrtol == NULL))
	{
		return FALSE;
	}
	SFrame_StartAck.DevType = 0x03; //3：直流双充 
	SFrame_StartAck.CurGun = gun;
	SFrame_StartAck.UserType = 0x01; //1：微信号（手机号）
	if(SFrame_StartAck.CurGun == GUN_A)
	{
		memcpy(SFrame_StartAck.UserNum,HFQGRecvStartCharge[GUN_A].UserNum,sizeof(SFrame_StartAck.UserNum));
	}
	else
	{
		memcpy(SFrame_StartAck.UserNum,HFQGRecvStartCharge[GUN_B].UserNum,sizeof(SFrame_StartAck.UserNum));
	}
	//SFrame_StartAck.CmdResult = 0xff; // 0xff：执行成功
	//SFrame_StartAck.Result = 0x00; //0：命令执行成功
//	if(pdisp_conrtol->NetGunState[gun] == GUN_CHARGEING)
//	{
//		SFrame_StartAck.CmdResult = 0xff; // 0xff：执行成功
//	}
//	else
//	{
//		SFrame_StartAck.CmdResult = 0x00; // 0x00：执行失败
//	}
//	if(pdisp_conrtol->NetGunState[gun] == GUN_CHARGEING)
//	{
//		SFrame_StartAck.Result = 0x00; //0：命令执行成功
//	}
//	else
//	{
//		if(GetGunState(gun) == GUN_DISCONNECTED)
//		{
//			SFrame_StartAck.Result = 0x02; //2：未插枪
//		}
//		else
//		{
//			SFrame_StartAck.Result = 0x01; //1：设备故障
//		}
//	}

	SFrame_StartAck.StartType = 0;   //立即充电
	SFrame_StartAck.CmdResult = 0xff; // 0xff：执行成功
	if(GetGunState(gun) == GUN_DISCONNECTED)
	{
		SFrame_StartAck.Result = 0x02; //2：未插枪
	}
	else
	{
		SFrame_StartAck.Result = 0x00; //0：命令执行成功
	}
	return HFQGFreamSend(HFQG_CMD_STARTCHARGE,(INT8U*)&SFrame_StartAck,sizeof(_HFQG_SEND_STARTACK));
}

//充电桩主动终止充电
/*****************************************************************************
* Function     : HFQG_SendDevStop
* Description  : 充电桩主动终止充电
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_SendDevStop(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	memcpy(SFrame_DevStop.UserNum,HFQGRecvStartCharge[gun].UserNum,sizeof(SFrame_DevStop.UserNum));
	
	SFrame_DevStop.DevType = 0x03; //3：直流双充 
	SFrame_DevStop.CurGun = gun;
	SFrame_DevStop.UserType = 0x01; //1：微信号（手机号）
	SFrame_DevStop.DevStop = 0x00;
	//return HFQGFreamSend(HFQG_CMD_CHARGESTATE,(INT8U*)&SFrame_DevStop,sizeof(_HFQG_SEND_DEVSTOP));
	return TRUE;
}

//后台主动终止充电应答
/*****************************************************************************
* Function     : HFQG_SendStopAck
* Description  : 后台主动终止充电应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_SendStopAck(_GUN_NUM gun)
{
		
		//INT8U UserNum[8];			//16位BCD型：ic卡号 000000-0123456789	卡号10位，多余位前面补0，微信号（手机号）：13696537467 多余位前面补0


	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	memcpy(SFrame_StopAck.UserNum,HFQGRecvStartCharge[gun].UserNum,sizeof(SFrame_StopAck.UserNum));

	SFrame_StopAck.DevType = 0x03; //3：直流双充 
	SFrame_StopAck.CurGun = gun;
	SFrame_StopAck.UserType = 0x01; //1：微信号（手机号）
	SFrame_StopAck.StopState = 0xff;// 终止充电成功
	SFrame_StopAck.StopType = 0x01;
	return HFQGFreamSend(HFQG_CMD_APPSTOPACK,(INT8U*)&SFrame_StopAck,sizeof(_HFQG_SEND_STOPACK));
}

/*****************************************************************************
* Function     : PreHFQGBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   PreHFQGBill(_GUN_NUM gun,INT8U *pdata)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	_HFQG_SEND_BILL * pbill = (_HFQG_SEND_BILL *)pdata;
	_BSPRTC_TIME endtime;
	GetCurTime(&endtime);  

	pbill->StopType = 0;	
	pbill->ChargeMode = 0;

	pbill->DevType = 0x03; //3：直流双充 
	pbill->CurGun = gun;
	pbill->UserType = 0x01; //1：微信号（手机号）

	memcpy(pbill->UserNum,HFQGRecvStartCharge[gun].UserNum,sizeof(pbill->UserNum));
	memcpy(pbill->BatchNum,HFQGRecvStartCharge[gun].BatchNum,15);

	pbill->StartYear[1] =  (BCDtoHEX(puserinfo->StartTime.Year) + 2000) & 0x00ff;
	pbill->StartYear[0] =  (((BCDtoHEX(puserinfo->StartTime.Year) + 2000) )>> 8) & 0x00ff;
	pbill->StartMonth =  BCDtoHEX(puserinfo->StartTime.Month);
	pbill->StartDay =  BCDtoHEX(puserinfo->StartTime.Day);
	pbill->StartHour =  BCDtoHEX(puserinfo->StartTime.Hour);
	pbill->StartMinute =  BCDtoHEX(puserinfo->StartTime.Minute);
	pbill->StartSecond =  BCDtoHEX(puserinfo->StartTime.Second);

	pbill->StopYear[1] =  (BCDtoHEX(endtime.Year) + 2000) & 0x00ff;
	pbill->StopYear[0] =  (((BCDtoHEX(endtime.Year) + 2000) )>> 8) & 0x00ff;
	pbill->StopMonth =  BCDtoHEX(endtime.Month);
	pbill->StopDay =  BCDtoHEX(endtime.Day);
	pbill->StopHour =  BCDtoHEX(endtime.Hour);
	pbill->StopMinute =  BCDtoHEX(endtime.Minute);
	pbill->StopSecond =  BCDtoHEX(endtime.Second);

	pbill->AlreadyHour = puserinfo->ChargeTime / 60;
	pbill->AlreadyMinute = puserinfo->ChargeTime % 60;

	pbill->StartPower[3] = pmeter->StartPower & 0x000000ff;
	pbill->StartPower[2] = (pmeter->StartPower >> 8) & 0x000000ff;
	pbill->StartPower[1] = (pmeter->StartPower >> 16) & 0x000000ff;
	pbill->StartPower[0] = (pmeter->StartPower >> 24)& 0x000000ff;

	if(pmeter->StartPower > pmeter->CurPower)
	{
		pmeter->CurPower = pmeter->StartPower;
	}
	
	pbill->StopPower[3] = ( pmeter->CurPower)& 0x000000ff;
	pbill->StopPower[2] = (( pmeter->CurPower) >> 8) & 0x000000ff;
	pbill->StopPower[1] = (( pmeter->CurPower) >> 16) & 0x000000ff;
	pbill->StopPower[0] = ((pmeter->CurPower) >> 24)& 0x000000ff;

	pbill->ChargePower[3] = (pmeter->CurPower -pmeter->StartPower)  & 0x000000ff;
	pbill->ChargePower[2] = ((pmeter->CurPower -pmeter->StartPower) >> 8) & 0x000000ff;
	pbill->ChargePower[1] = ((pmeter->CurPower -pmeter->StartPower) >> 16) & 0x000000ff;
	pbill->ChargePower[0] = ((pmeter->CurPower -pmeter->StartPower) >> 24)& 0x000000ff;

	pbill->UserMoney[3] = (puserinfo->TotalBill/100) & 0x000000ff;
	pbill->UserMoney[2] = ((puserinfo->TotalBill/100) >> 8) & 0x000000ff;
	pbill->UserMoney[1] = ((puserinfo->TotalBill/100) >> 16) & 0x000000ff;
	pbill->UserMoney[0] = ((puserinfo->TotalBill/100) >> 24)& 0x000000ff;
	pbill->StartSoc = 0;
	pbill->StopSoc = BMS_BCS_Context[gun].SOC;

	pbill->StartMoney[3] = HFQGRecvStartCharge[gun].Money & 0x000000ff;
	pbill->StartMoney[2] = (HFQGRecvStartCharge[gun].Money >> 8) & 0x000000ff;
	pbill->StartMoney[1] = (HFQGRecvStartCharge[gun].Money >> 16) & 0x000000ff;
	pbill->StartMoney[0] = (HFQGRecvStartCharge[gun].Money >> 24)& 0x000000ff;
	return TRUE;
}


//桩上传结算指令
/*****************************************************************************
* Function     : HFQG_SendBill
* Description  : 桩上传结算指令
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_SendBill(_GUN_NUM gun)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);


	SFrame_Bill.StopType = 0;	
	SFrame_Bill.ChargeMode = 0;

	SFrame_Bill.DevType = 0x03; //3：直流双充 
	SFrame_Bill.CurGun = gun;
	SFrame_Bill.UserType = 0x01; //1：微信号（手机号）

	memcpy(SFrame_Bill.UserNum,HFQGRecvStartCharge[gun].UserNum,sizeof(SFrame_Bill.UserNum));
	memcpy(SFrame_Bill.BatchNum,HFQGRecvStartCharge[gun].BatchNum,15);

	SFrame_Bill.StartYear[1] =  (BCDtoHEX(puserinfo->StartTime.Year) + 2000) & 0x00ff;
	SFrame_Bill.StartYear[0] =  (((BCDtoHEX(puserinfo->StartTime.Year) + 2000) )>> 8) & 0x00ff;
	SFrame_Bill.StartMonth =  BCDtoHEX(puserinfo->StartTime.Month);
	SFrame_Bill.StartDay =  BCDtoHEX(puserinfo->StartTime.Day);
	SFrame_Bill.StartHour =  BCDtoHEX(puserinfo->StartTime.Hour);
	SFrame_Bill.StartMinute =  BCDtoHEX(puserinfo->StartTime.Minute);
	SFrame_Bill.StartSecond =  BCDtoHEX(puserinfo->StartTime.Second);

	SFrame_Bill.StopYear[1] =  (BCDtoHEX(puserinfo->EndTime.Year) + 2000) & 0x00ff;
	SFrame_Bill.StopYear[0] =  (((BCDtoHEX(puserinfo->EndTime.Year) + 2000) )>> 8) & 0x00ff;
	SFrame_Bill.StopMonth =  BCDtoHEX(puserinfo->EndTime.Month);
	SFrame_Bill.StopDay =  BCDtoHEX(puserinfo->EndTime.Day);
	SFrame_Bill.StopHour =  BCDtoHEX(puserinfo->EndTime.Hour);
	SFrame_Bill.StopMinute =  BCDtoHEX(puserinfo->EndTime.Minute);
	SFrame_Bill.StopSecond =  BCDtoHEX(puserinfo->EndTime.Second);

	SFrame_Bill.AlreadyHour = puserinfo->ChargeTime / 60;
	SFrame_Bill.AlreadyMinute = puserinfo->ChargeTime % 60;

	SFrame_Bill.StartPower[3] = pmeter->StartPower & 0x000000ff;
	SFrame_Bill.StartPower[2] = (pmeter->StartPower >> 8) & 0x000000ff;
	SFrame_Bill.StartPower[1] = (pmeter->StartPower >> 16) & 0x000000ff;
	SFrame_Bill.StartPower[0] = (pmeter->StartPower >> 24)& 0x000000ff;

	if(pmeter->StartPower > pmeter->CurPower)
	{
		pmeter->CurPower = pmeter->StartPower;
	}
	
	SFrame_Bill.StopPower[3] = ( pmeter->CurPower)& 0x000000ff;
	SFrame_Bill.StopPower[2] = (( pmeter->CurPower) >> 8) & 0x000000ff;
	SFrame_Bill.StopPower[1] = (( pmeter->CurPower) >> 16) & 0x000000ff;
	SFrame_Bill.StopPower[0] = ((pmeter->CurPower) >> 24)& 0x000000ff;

	SFrame_Bill.ChargePower[3] = (pmeter->CurPower -pmeter->StartPower)  & 0x000000ff;
	SFrame_Bill.ChargePower[2] = ((pmeter->CurPower -pmeter->StartPower) >> 8) & 0x000000ff;
	SFrame_Bill.ChargePower[1] = ((pmeter->CurPower -pmeter->StartPower) >> 16) & 0x000000ff;
	SFrame_Bill.ChargePower[0] = ((pmeter->CurPower -pmeter->StartPower) >> 24)& 0x000000ff;

	SFrame_Bill.UserMoney[3] = (puserinfo->TotalBill/100) & 0x000000ff;
	SFrame_Bill.UserMoney[2] = ((puserinfo->TotalBill/100) >> 8) & 0x000000ff;
	SFrame_Bill.UserMoney[1] = ((puserinfo->TotalBill/100) >> 16) & 0x000000ff;
	SFrame_Bill.UserMoney[0] = ((puserinfo->TotalBill/100) >> 24)& 0x000000ff;
	SFrame_Bill.StartSoc = 0;
	SFrame_Bill.StopSoc = BMS_BCS_Context[gun].SOC;

	SFrame_Bill.StartMoney[3] = HFQGRecvStartCharge[gun].Money & 0x000000ff;
	SFrame_Bill.StartMoney[2] = (HFQGRecvStartCharge[gun].Money >> 8) & 0x000000ff;
	SFrame_Bill.StartMoney[1] = (HFQGRecvStartCharge[gun].Money >> 16) & 0x000000ff;
	SFrame_Bill.StartMoney[0] = (HFQGRecvStartCharge[gun].Money >> 24)& 0x000000ff;

	WriterFmBill(gun,1);
	ResendBillControl[gun].CurTime = OSTimeGet();	
	ResendBillControl[gun].CurTime = ResendBillControl[gun].LastTime;
	return HFQGFreamSend(HFQG_CMD_BILL,(INT8U*)&SFrame_Bill,sizeof(_HFQG_SEND_BILL));
	
}



/*****************************************************************************
* Function     : HY_SendBillData
* Description  : 发送订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U HFQG_SendBillData(INT8U * pdata,INT8U len)
{
	if((pdata == NULL) || (len < sizeof(_HFQG_SEND_BILL)))
	{
		return FALSE;
	}
	return HFQGFreamSend(HFQG_CMD_BILL,pdata,sizeof(_HFQG_SEND_BILL));
}

/**************************************************接收帧****************************************/
/*****************************************************************************
* Function     : HFQG_RecvRegisterAck
* Description  : 注册应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvRegisterAck(INT8U *pdata,INT16U len)
{
	
	
	_BSPRTC_TIME SetTime;                       //设定时间
	INT8U times = 3;							//如果设置失败反复设置三次
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	//参照协议
	if((pdata == NULL) || (len < 12) )
	{
		return FALSE;
	}
	if(pdata[4] == 0xff) //0：注册不通过   0xff：注册通过
	{
		APP_SetAppRegisterState(LINK_NUM,STATE_OK);
		if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING)   //没在充电中读取
		{
			ReadFmBill(GUN_A);   //是否需要发送订单
		}
		if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)   //没在充电中读取
		{
			ReadFmBill(GUN_B);   //是否需要发送订单
		}
	}
	
	//校准时间
	SetTime.Year   = HEXtoBCD((INT8U)((pdata[5]<<8 | pdata[6]) - 2000));//将输入的十进制数转换成BCD
    SetTime.Month  = HEXtoBCD(pdata[7]);
    SetTime.Day    = HEXtoBCD(pdata[8]);
    SetTime.Hour   = HEXtoBCD(pdata[9]);
    SetTime.Minute = HEXtoBCD(pdata[10]);
    SetTime.Second = HEXtoBCD(pdata[11]);
	
    while(times--)
    {
        if(BSP_RTCSetTime(&SetTime) == TRUE)    //设置RTC
        {
            return TRUE;
        }                                 
    }
	
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvHear
* Description  : 应答发送心跳包
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvHearAck(INT8U *pdata,INT16U len)
{
	
	return TRUE;
}


/*****************************************************************************
* Function     : HFQG_RecvDevState
* Description  : 充电桩总状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvDevStateAck(INT8U *pdata,INT16U len)
{
	NOP();
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvDevStateQuery
* Description  :发送查询状态，桩返回0X05指令应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvDevStateQuery(INT8U *pdata,INT16U len)
{
	NOP();
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvStartChargeAck
* Description  :开始充电指令
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvStartCharge(INT8U *pdata,INT16U len)
{
	static _HFQG_RECV_STARTCHARGE startinfo;

	OS_EVENT* pevent = APP_Get4GMainEvent();
	 static _BSP_MESSAGE SendMsg[GUN_MAX];
	 INT32U data_32,data_16;
	INT8U i;
	 static _PRICE_SET price;
	 _FLASH_OPERATION  FlashOper;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();


	if(len != sizeof(_HFQG_RECV_STARTCHARGE))   //后台数据和协议对应不上
	{
		return FALSE;
	}
	memcpy(&startinfo,pdata,len);
	//大小端转换
	data_32 = 0;
	data_32 = ((startinfo.Money >> 24) & 0x000000ff) | ((startinfo.Money >> 8) & 0x0000ff00) | \
				((startinfo.Money << 8) & 0x00ff0000) |((startinfo.Money >> 24) & 0xff000000);
	startinfo.Money  = data_32;

	data_16 = 0;
	data_16 = ((startinfo.JIBill.ChargeMoney >> 8) & 0x00ff) | ((startinfo.JIBill.ChargeMoney <<  8) & 0xff00);
	startinfo.JIBill.ChargeMoney = data_16;

	data_16 = 0;
	data_16 = ((startinfo.JIBill.ChargeServiceh >> 8) & 0x00ff) | ((startinfo.JIBill.ChargeServiceh <<  8) & 0xff00);
	startinfo.JIBill.ChargeServiceh = data_16;

	data_16 = 0;
	data_16 = ((startinfo.JIBill.StopCarMoney >> 8) & 0x00ff) | ((startinfo.JIBill.StopCarMoney <<  8) & 0xff00);
	startinfo.JIBill.StopCarMoney = data_16;

	data_16 = 0;
	data_16 = ((startinfo.GaoBill.ChargeMoney >> 8) & 0x00ff) | ((startinfo.GaoBill.ChargeMoney <<  8) & 0xff00);
	startinfo.GaoBill.ChargeMoney = data_16;

	data_16 = 0;
	data_16 = ((startinfo.GaoBill.ChargeServiceh >> 8) & 0x00ff) | ((startinfo.GaoBill.ChargeServiceh <<  8) & 0xff00);
	startinfo.GaoBill.ChargeServiceh = data_16;

	data_16 = 0;
	data_16 = ((startinfo.GaoBill.StopCarMoney >> 8) & 0x00ff) | ((startinfo.GaoBill.StopCarMoney <<  8) & 0xff00);
	startinfo.GaoBill.StopCarMoney = data_16;

	data_16 = 0;
	data_16 = ((startinfo.PingBill.ChargeMoney >> 8) & 0x00ff) | ((startinfo.PingBill.ChargeMoney <<  8) & 0xff00);
	startinfo.PingBill.ChargeMoney = data_16;

	data_16 = 0;
	data_16 = ((startinfo.PingBill.ChargeServiceh >> 8) & 0x00ff) | ((startinfo.PingBill.ChargeServiceh <<  8) & 0xff00);
	startinfo.PingBill.ChargeServiceh = data_16;

	data_16 = 0;
	data_16 = ((startinfo.PingBill.StopCarMoney >> 8) & 0x00ff) | ((startinfo.PingBill.StopCarMoney <<  8) & 0xff00);
	startinfo.PingBill.StopCarMoney = data_16;

	
	data_16 = 0;
	data_16 = ((startinfo.GuBill.ChargeMoney >> 8) & 0x00ff) | ((startinfo.GuBill.ChargeMoney <<  8) & 0xff00);
	startinfo.GuBill.ChargeMoney = data_16;

	data_16 = 0;
	data_16 = ((startinfo.GuBill.ChargeServiceh >> 8) & 0x00ff) | ((startinfo.GuBill.ChargeServiceh <<  8) & 0xff00);
	startinfo.GuBill.ChargeServiceh = data_16;

	data_16 = 0;
	data_16 = ((startinfo.GuBill.StopCarMoney >> 8) & 0x00ff) | ((startinfo.GuBill.StopCarMoney <<  8) & 0xff00);
	startinfo.GuBill.StopCarMoney = data_16;


	//费率设置处理 
	//要求后台一个时间段内的第一个开始时间和第二个开始时间要一致。服务费每个时间段都一样。没有停车费！
	price.StartTime[0] =  startinfo.JIBill.StartHour1 * 60 + startinfo.JIBill.StartMinute1;   	//开始时间
	price.EndTime[0] =  startinfo.JIBill.StopHour2 * 60 + startinfo.JIBill.StopMinute2;		  	//结束时间
	price.Price[0] = startinfo.JIBill.ChargeMoney*1000;												//结束时间


	price.StartTime[1] =  startinfo.GaoBill.StartHour1 * 60 + startinfo.GaoBill.StartMinute1;   	//开始时间
	price.EndTime[1] =  startinfo.GaoBill.StopHour2 * 60 + startinfo.GaoBill.StopMinute2;		  	//结束时间
	price.Price[1] = startinfo.GaoBill.ChargeMoney*1000;		

	price.StartTime[2] =  startinfo.PingBill.StartHour1 * 60 + startinfo.PingBill.StartMinute1;   	//开始时间
	price.EndTime[2] =  startinfo.PingBill.StopHour2 * 60 + startinfo.PingBill.StopMinute2;		  	//结束时间
	price.Price[2] = startinfo.PingBill.ChargeMoney*1000;		

	price.StartTime[3] =  startinfo.GuBill.StartHour1 * 60 + startinfo.GuBill.StartMinute1;   	//开始时间
	price.EndTime[3] =  startinfo.GuBill.StopHour2 * 60 + startinfo.GuBill.StopMinute2;		  	//结束时间
	price.Price[3] = startinfo.GuBill.ChargeMoney*1000;	

	for(i = 0;i < 4;i++)
	{
		//24小时就是0
		if(price.StartTime[i] == 24*60)
		{
			price.StartTime[i] = 0;
		}
		if(price.EndTime[i] == 24*60)
		{
			price.EndTime[i] = 0;
		}
	}
	price.ServeFee[0] = startinfo.JIBill.ChargeServiceh*1000;	//充电服务费
	price.ServeFee[1] = startinfo.JIBill.ChargeServiceh*1000;	//充电服务费
	price.ServeFee[2] = startinfo.JIBill.ChargeServiceh*1000;	//充电服务费
	price.ServeFee[3] = startinfo.JIBill.ChargeServiceh*1000;	//充电服务费
	//if(strncmp((const char *)&price,(const char*)pprice,sizeof(_PRICE_SET)) != 0)
	{

			//读取所有配置信息
		FlashOper.DataID = PARA_PRICALL_ID;
		FlashOper.Len = PARA_PRICALL_FLLEN;
		FlashOper.ptr = (INT8U *)&price;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
		memcpy(&PriceSet,&price,sizeof(_PRICE_SET));
	}
	
	
	if(startinfo.CurGun == GUN_A)
	{
		memcpy(&HFQGRecvStartCharge[GUN_A],&startinfo,sizeof(_HFQG_RECV_STARTCHARGE));
		SendMsg[GUN_A].MsgID = BSP_4G_RECV;
		SendMsg[GUN_A].DataLen = GUN_A;
		SendMsg[GUN_A].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_A]);
	}
	else
	{
		memcpy(&HFQGRecvStartCharge[GUN_B],&startinfo,sizeof(_HFQG_RECV_STARTCHARGE));
		SendMsg[GUN_B].MsgID = BSP_4G_RECV;
		SendMsg[GUN_B].DataLen = GUN_B;
		SendMsg[GUN_B].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_B]);
	}
	if(startinfo.CurGun < GUN_MAX)
	{
		//发送结算
		Msg[startinfo.CurGun].MsgID = BSP_4G_MAIN;
		Msg[startinfo.CurGun].DataLen = startinfo.CurGun;
		Msg[startinfo.CurGun].DivNum = APP_START_ACK;
		OSQPost(psendevent, &Msg[startinfo.CurGun]);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvChargeInfo
* Description  :返回确认实时充电信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvChargeInfoAck(INT8U *pdata,INT16U len)
{
	NOP();
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvChargeInfoQuery
* Description  :主动询问实时充电信息，桩返回0X34
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvChargeInfoQuery(INT8U *pdata,INT16U len)
{
	NOP();
	return TRUE;
}


/*****************************************************************************
* Function     : HFQG_RecvStopChargeAck
* Description  :后台确认桩主动终止充电
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvStopChargeAck(INT8U *pdata,INT16U len)
{
	INT8U gun;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if((pdata == NULL) || (len < 2) )
	{
		return FALSE;
	}
	gun = pdata[1];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvStopCharge
* Description  :后台主动终止充电
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvStopCharge(INT8U *pdata,INT16U len)
{
	OS_EVENT* pevent = APP_Get4GMainEvent();
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	static _BSP_MESSAGE Msg[GUN_MAX];
	INT8U gun;
	if((pdata == NULL) || (len < 2) )
	{
		return FALSE;
	}
	gun = pdata[1];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	//发送停止命令
	SendMsg[gun].MsgID = BSP_4G_RECV;
	SendMsg[gun].DataLen = gun;
	SendMsg[gun].DivNum = APP_STOP_CHARGE;
	OSQPost(pevent, &SendMsg[gun]);

	//发送停止应答
	Msg[gun].MsgID = BSP_4G_MAIN;
	Msg[gun].DataLen = gun;
	Msg[gun].DivNum = APP_STOP_ACK;
	OSQPost(psendevent, &Msg[gun]);
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvBillAck
* Description  :后台确认账单结算信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   HFQG_RecvBillAck(INT8U *pdata,INT16U len)
{
	INT8U gun;

	gun = pdata[1];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	ResendBillControl[gun].ResendBillState = FALSE;			//订单确认，不需要重发订单
	ResendBillControl[gun].SendCount = 0;
	WriterFmBill((_GUN_NUM)gun,0);
	return TRUE;
}

/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

