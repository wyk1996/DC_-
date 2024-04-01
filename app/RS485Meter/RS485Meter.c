/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName            : rs485meter.c
* Author              : 
* Date First Issued   : 
* Version             : V0.1
* Description         : 
*----------------------------------------历史版本信息-------------------------------------------
* History             :
* Description         :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "bsp_uart.h"
#include "RS485Meter.h"
#include "charging_Interface.h"
#include "Disinterface.h"
#include "RTC_task.h"
#include "datachangeinterface.h"
#include  "card_Interface.h"
#include "RTC_task.h"
#include "DisplayMain.h"
#include "4GMain.h"
/* Private define-------------------------------------------------------*/
#define    RS485_RECV_QUEUE_LEN          (8u)    //485接收消息队列长度
#define    CHARGING_QUEUE_LEN            (16U)
#define    Meter_SEND_BUF_LEN           (100u)     
#define    Meter_RECV_BUF_LEN           (100u)    
/* Private macro--------------------------------------------------------*/
const INT8U DI_Power[4] = {0x00, 0x00, 0x00, 0x00};                        //电量数据标识
//INT8U Meter_addr1[6] = {01, 00, 00 ,00 ,00 ,00};                           //表1地址
//INT8U Meter_addr2[6] = {02, 00, 00 ,00 ,00 ,00};                           //表2地址

OS_EVENT *RS485Meter_Prevent;		                                          //定义抄表消息队列
OS_EVENT *Charging_Prevent;                                               //定义计费消息队列

static void *Charging[CHARGING_QUEUE_LEN];	                              //16个消息可以存储
static void *ReadMeter[RS485_RECV_QUEUE_LEN];	                            //8个消息可以存储

static INT8U RS485RecvSMem[RS485_RECV_QUEUE_LEN * sizeof(_BSP_MESSAGE)];  //消息实体的个数，实际上就是_BSP_MESSAGE结构体的个数
static INT8U SendMeterBuff[Meter_SEND_BUF_LEN];                           //供串口底层使用，保存发送的数据
INT8U RecvMeterBuff[Meter_RECV_BUF_LEN];                                  //供串口底层使用，保存接收到的数据

INT8U SendTempBuff[Meter_SEND_BUF_LEN];                                   //发送一帧数据缓冲区
INT8U RecvTempBuff[Meter_RECV_BUF_LEN];                                   //接收一帧数据缓冲区
/* Private variables----------------------------------------------------*/
SYSPARAM     SysParam;                                                     //系统参数
USERINFO     GunBillInfo[2];                                               //计费信息
TEMPDATA     TempPowerData[2];                                             //临时计算数据
METERPARAM   MeterParam[2];                                                //电表参数
_BSPRTC_TIME CurTimedata;                                                  //当前时间
static INT8U CurTimeFrame =  0;   //当前时间段 
static INT8U CurGunTimeFrame[GUN_MAX] = {0};   //云快充可能不同枪费率不一样


INT32U JYPower[2] = {0};   // 英利达电话数据读出来有问题，可能开始充电的时候A枪的数据直接读到B枪
/* Private function prototypes------------------------------------------*/
/* Private functions----------------------------------------------------*/

/****************************************************************************
* Function     : SetMeterStatus
* Description  : 设置电表状态
* Input        : gun 枪号
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void SetMeterStatus(_GUN_NUM gun, COMSTATUS state)
{
	 MeterParam[gun].ComStatus = state;
}

/****************************************************************************
* Function     : APP_GetMeterPowerInfo
* Description  : 回去电表读数 2位小数
* Input        : gun 枪号
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
TEMPDATA * APP_GetMeterPowerInfo(_GUN_NUM gun)
{
	 return &TempPowerData[gun];
}
/****************************************************************************
* Function     : GetPowerChangeFlag
* Description  : 获取电量变动标识
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
INT8U CompareAddr(INT8U *temp, INT8U templen, const INT8U *data, INT8U datalen)
{
	INT8U i;
	
	if(templen != datalen)
		 return FALSE;
	
  for(i=0; i<templen; i++)
  {
		if(temp[i] != data[i])
		   break;
	}
	
  if(i < templen)
    return FALSE;
  else
    return TRUE;		
}

/****************************************************************************
* Function     : GetPowerChangeFlag
* Description  : 获取电量变动标识
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
FLAGSTATUS GetPowerChangeFlag(_GUN_NUM gun)
{
	return MeterParam[gun].PowerChangeFlag;
}
/****************************************************************************
* Function     : SetPowerChangeFlag
* Description  : 设置电量变动标识
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void SetPowerChangeFlag(_GUN_NUM gun, FLAGSTATUS flag)
{
	MeterParam[gun].PowerChangeFlag = flag;
}
/****************************************************************************
* Function     : SetChargingStatus
* Description  : 设置计费状态
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void SetChargingStatus(_GUN_NUM gun, CHARGING_STATUS state)
{
	SysParam.ChargingStatus[gun] = state;
}
/****************************************************************************
* Function     : GetCurRate
* Description  : 获取当前费率
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
INT32U GetCurRate(void)
{
	return SysParam.CurRate;
}
/****************************************************************************
* Function     : CalculationRate
* Description  : 计算费率
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void CalculationRate(void)
{
	INT16U  CurTimeMinuter;          //当前时间分钟       
	_PRICE_SET *Price;
	_PRICE_SET CurPrice;             //当前费率表
	INT8U i = 0;

	if(GetCurTime(&CurTimedata) == FALSE)
		return ;   //获取时间出错，费率不更新
	
	CurTimeMinuter = ByteBcdToHex(CurTimedata.Hour) * 60 + ByteBcdToHex(CurTimedata.Minute);     //计算当前时间分钟
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		if(SYSSet.NetState == DISP_NET) //网络版本
		{
			//A费率
			//初始
			SysParam.CurGunRate[GUN_A] = CurPrice.Price[TIME_PERIOD_1] + CurPrice.ServeFee[TIME_PERIOD_1]; //全部不符合返回第一时间段的电价
			CurGunTimeFrame[GUN_A] = TIME_PERIOD_1;
			
			Price = APP_GetAPriceInfo();
			memcpy(&CurPrice, Price ,sizeof(_PRICE_SET));   //获取当前费率表格
			
			if((CurPrice.StartTime[0] == CurPrice.EndTime[0]) && (CurPrice.StartTime[0] == 0))   //一开始就形成闭环
			{
				SysParam.CurGunRate[GUN_A] = CurPrice.Price[0] + CurPrice.ServeFee[0];    //云快充为保存小数点后五位
				CurGunTimeFrame[GUN_A] = 0;
			}
			else
			{
				for(i = 0;i < TIME_PERIOD_MAX; i++)
				{
					//时间段前闭后开
					if(CurPrice.StartTime[i] >  CurPrice.EndTime[i])           //开始时间段大
					{
						if((CurTimeMinuter >= CurPrice.StartTime[i]) || (CurTimeMinuter < CurPrice.EndTime[i]))
						{
							SysParam.CurGunRate[GUN_A] = CurPrice.Price[i] + CurPrice.ServeFee[i];    //云快充为保存小数点后五位
							CurGunTimeFrame[GUN_A] = i;
							break;
						}
					}
					else                                     //结束时间段大
					{
						if((CurTimeMinuter >= CurPrice.StartTime[i]) && (CurTimeMinuter < CurPrice.EndTime[i]))
						{
							SysParam.CurGunRate[GUN_A] = CurPrice.Price[i] + CurPrice.ServeFee[i];       //云快充为保存小数点后五位
							CurGunTimeFrame[GUN_A] = i;
							break;
						}
					}
				}
			}
			
			
			
			
			//B费率
			Price = APP_GetBPriceInfo();
			memcpy(&CurPrice, Price ,sizeof(_PRICE_SET));   //获取当前费率表格
			
			if((CurPrice.StartTime[0] == CurPrice.EndTime[0]) && (CurPrice.StartTime[0] == 0))   //一开始就形成闭环
			{
				SysParam.CurGunRate[GUN_B] = CurPrice.Price[0] + CurPrice.ServeFee[0];    //云快充为保存小数点后五位
				CurGunTimeFrame[GUN_B] = 0;
				return;
			}
			for(i = 0;i < TIME_PERIOD_MAX; i++)
			{
				//时间段前闭后开
				if(CurPrice.StartTime[i] >  CurPrice.EndTime[i])           //开始时间段大
				{
					if((CurTimeMinuter >= CurPrice.StartTime[i]) || (CurTimeMinuter < CurPrice.EndTime[i]))
					{
						SysParam.CurGunRate[GUN_B] = CurPrice.Price[i] + CurPrice.ServeFee[i];    //云快充为保存小数点后五位
						CurGunTimeFrame[GUN_B] = i;
						return;
					}
				}
				else                                     //结束时间段大
				{
					if((CurTimeMinuter >= CurPrice.StartTime[i]) && (CurTimeMinuter < CurPrice.EndTime[i]))
					{
						SysParam.CurGunRate[GUN_B] = CurPrice.Price[i] + CurPrice.ServeFee[i];       //云快充为保存小数点后五位
						CurGunTimeFrame[GUN_B] = i;
						return;
					}
				}
			}
			
			SysParam.CurGunRate[GUN_B] = CurPrice.Price[TIME_PERIOD_1] + CurPrice.ServeFee[TIME_PERIOD_1]; //全部不符合返回第一时间段的电价
			CurGunTimeFrame[GUN_B] = TIME_PERIOD_1;
		}
		else
		{
			Price = APP_GetPriceInfo();
			memcpy(&CurPrice, Price ,sizeof(_PRICE_SET));   //获取当前费率表格
				
			
			if((CurPrice.StartTime[0] == CurPrice.EndTime[0]) && (CurPrice.StartTime[0] == 0))   //一开始就形成闭环
			{
				SysParam.CurRate = CurPrice.Price[0] + CurPrice.ServeFee[0]; 
				CurTimeFrame = 0;
				return;
			}
			for(i = 0;i < TIME_PERIOD_MAX; i++)
			{
				//时间段前闭后开
				if(CurPrice.StartTime[i] >  CurPrice.EndTime[i])           //开始时间段大
				{
					if((CurTimeMinuter >= CurPrice.StartTime[i]) || (CurTimeMinuter < CurPrice.EndTime[i]))
					{
						SysParam.CurRate = CurPrice.Price[i] + CurPrice.ServeFee[i];    //云快充为保存小数点后五位
						CurTimeFrame = i;
						return;
					}
				}
				else                                     //结束时间段大
				{
					if((CurTimeMinuter >= CurPrice.StartTime[i]) && (CurTimeMinuter < CurPrice.EndTime[i]))
					{
						SysParam.CurRate = CurPrice.Price[i] + CurPrice.ServeFee[i];       //云快充为保存小数点后五位
						CurTimeFrame = i;
						return;
					}
				}
			}
			
			
			SysParam.CurRate = CurPrice.Price[TIME_PERIOD_1] + CurPrice.ServeFee[TIME_PERIOD_1]; //全部不符合返回第一时间段的电价
			CurTimeFrame = TIME_PERIOD_1;
		}
	}
	else
	{
		Price = APP_GetPriceInfo();
		memcpy(&CurPrice, Price ,sizeof(_PRICE_SET));   //获取当前费率表格
			
		
		if((CurPrice.StartTime[0] == CurPrice.EndTime[0]) && (CurPrice.StartTime[0] == 0))   //一开始就形成闭环
		{
			SysParam.CurRate = CurPrice.Price[0] + CurPrice.ServeFee[0]; 
			CurTimeFrame = 0;
			return;
		}
		for(i = 0;i < TIME_PERIOD_MAX; i++)
		{
			//时间段前闭后开
			if(CurPrice.StartTime[i] >  CurPrice.EndTime[i])           //开始时间段大
			{
				if((CurTimeMinuter >= CurPrice.StartTime[i]) || (CurTimeMinuter < CurPrice.EndTime[i]))
				{
					SysParam.CurRate = CurPrice.Price[i] + CurPrice.ServeFee[i];    //云快充为保存小数点后五位
					CurTimeFrame = i;
					return;
				}
			}
			else                                     //结束时间段大
			{
				if((CurTimeMinuter >= CurPrice.StartTime[i]) && (CurTimeMinuter < CurPrice.EndTime[i]))
				{
					SysParam.CurRate = CurPrice.Price[i] + CurPrice.ServeFee[i];       //云快充为保存小数点后五位
					CurTimeFrame = i;
					return;
				}
			}
		}
		
		
		SysParam.CurRate = CurPrice.Price[TIME_PERIOD_1] + CurPrice.ServeFee[TIME_PERIOD_1]; //全部不符合返回第一时间段的电价
		CurTimeFrame = TIME_PERIOD_1;
	}
}

/*********************************************************************************************************
* Function     : MeterToDispMsg_StopCharge
* Description  : 给显示任务发送停止充电
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200702
*********************************************************************************************************/	
static void MeterToDispMsg_StopCharge(_GUN_NUM gun)
{
	static _BSP_MESSAGE message;
	static _GUN_NUM sendgun;
	OS_EVENT* pevent = APP_GetDispEvent();
						 	
	sendgun = gun;
	message.MsgID = BSP_MSGID_METER;	
	message.pData = (INT8U*)&sendgun;
	OSQPost(pevent, &message);				
}


/****************************************************************************
* Function     : CalculationBill
* Description  : 计算费用、充电时间
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void CalculationBill(_GUN_NUM gun)
{
	INT32U Curtime;	
	  INT32U  uiSec = 0;
    INT32U  uiFeeIndex = 0;
	USERCARDINFO* card_info;
	_PRICE_SET*	Price = APP_GetPriceInfo();
	card_info = GetGunCardInfo(gun);
	
	
	INT32U balance;
	
	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
	{
		if(SYSSet.NetState == DISP_NET) //网络版本
		{
			//云快充网络
			if(gun == GUN_A)
			{
				Price = APP_GetAPriceInfo();
			}
			if(gun == GUN_B)
			{
				Price = APP_GetBPriceInfo();
			}
			if(GetChargingStatus(gun) == _Charging_Busy)  //计费工作中
			{
				Curtime = GetCurUnixTimestamp();
				
				GunBillInfo[gun].ChargeTime = (Curtime - GunBillInfo[gun].StartTimeCount)  / 60; 		
				
			
				if(GetPowerChangeFlag(gun) == _Flag_1)      //电量变动
				{
					TempPowerData[gun].CurPower4 = MeterParam[gun].Power4;   //记录当前电量
					TempPowerData[gun].CurUsedPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4;  //计算当前电量变动值
					if(TempPowerData[gun].CurUsedPower4 > 50000)
					{
						TempPowerData[gun].CurPower4 = TempPowerData[gun].LastPower4;    //上一次电量赋值给当前电量
						//一次变动超过5度电，就存在问题   ，测试中存在B枪的当前电量给A枪
						NOP();
						return;
					}
					GunBillInfo[gun].TotalPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].StartPower4;   
					GunBillInfo[gun].CalTotalBill += TempPowerData[gun].CurUsedPower4 * SysParam.CurGunRate[gun];             //当前费率为5位小数  电量位4位小数
					GunBillInfo[gun].TotalBill = GunBillInfo[gun].CalTotalBill / 100000;    //4位小数
					
					GunBillInfo[gun].CalTotalServeBill += TempPowerData[gun].CurUsedPower4 * Price->ServeFee[CurGunTimeFrame[gun]]; 
					GunBillInfo[gun].TotalServeBill = GunBillInfo[gun].CalTotalServeBill / 100000;   
					//尖峰评估各自费用
					if(CurGunTimeFrame[gun] < TIME_PERIOD_MAX)
					{
						if(Price->CurTimeQuantum[Price->CurTimeQuantum[CurGunTimeFrame[gun]]] < TIME_QUANTUM_MAX)
						{
							//尖峰平谷费用电量
							GunBillInfo[gun].JFPGPower[Price->CurTimeQuantum[CurGunTimeFrame[gun]]] += (TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4);
							GunBillInfo[gun].CalJFPGBill[Price->CurTimeQuantum[CurGunTimeFrame[gun]]] += TempPowerData[gun].CurUsedPower4 * SysParam.CurGunRate[gun]; 
							GunBillInfo[gun].JFPGBill[Price->CurTimeQuantum[CurGunTimeFrame[gun]]] = GunBillInfo[gun].CalJFPGBill[Price->CurTimeQuantum[CurGunTimeFrame[gun]]] / 100000; 
							
							//分时费用电量
							GunBillInfo[gun].Power[CurGunTimeFrame[gun]] += (TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4);
							GunBillInfo[gun].CalBill[CurGunTimeFrame[gun]] += TempPowerData[gun].CurUsedPower4 * SysParam.CurGunRate[gun]; 
							GunBillInfo[gun].CalServeBill[CurGunTimeFrame[gun]] += TempPowerData[gun].CurUsedPower4 * Price->ServeFee[CurGunTimeFrame[gun]]; 
							
							GunBillInfo[gun].Bill[CurGunTimeFrame[gun]] = GunBillInfo[gun].CalBill[CurGunTimeFrame[gun]] / 100000; 
							GunBillInfo[gun].ServeBill[CurGunTimeFrame[gun]] = GunBillInfo[gun].CalServeBill[CurGunTimeFrame[gun]] / 100000; 
						}
					}
					TempPowerData[gun].LastPower4 = TempPowerData[gun].CurPower4;		                                 //更新上一次计费电量
					SetPowerChangeFlag(gun, _Flag_0);
					balance = APP_GetNetMoney(gun) * 100;
					if(((GunBillInfo[gun].TotalBill + 5000) >= (balance)) && (balance != 0))//说明卡内余额用光了
					{
						MeterToDispMsg_StopCharge(gun);					//给显示任务发送停止充电
						SetChargingStatus(gun, _Charging_Free);                          //结束计费
					}
					
				}
			}
		}
		else
		{
			if(GetChargingStatus(gun) == _Charging_Busy)  //计费工作中
			{
				Curtime = GetCurUnixTimestamp();
				
				GunBillInfo[gun].ChargeTime = (Curtime - GunBillInfo[gun].StartTimeCount)  / 60; 		
				
			
				if(GetPowerChangeFlag(gun) == _Flag_1)      //电量变动
				{
					TempPowerData[gun].CurPower4 = MeterParam[gun].Power4;   //记录当前电量
					TempPowerData[gun].CurUsedPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4;  //计算当前电量变动值
					if(TempPowerData[gun].CurUsedPower4 > 50000)
					{
						TempPowerData[gun].CurPower4 = TempPowerData[gun].LastPower4;    //上一次电量赋值给当前电量
						//一次变动超过5度电，就存在问题   ，测试中存在B枪的当前电量给A枪
						NOP();
						return;
					}
					GunBillInfo[gun].TotalPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].StartPower4;     
					GunBillInfo[gun].CalTotalBill += TempPowerData[gun].CurUsedPower4 * GetCurRate();             //当前费率为5位小数  电量位2位小数
					GunBillInfo[gun].TotalBill = GunBillInfo[gun].CalTotalBill / 100000;    //4位小数
					
					GunBillInfo[gun].CalTotalServeBill += TempPowerData[gun].CurUsedPower4 * Price->ServeFee[CurTimeFrame]; 
					GunBillInfo[gun].TotalServeBill = GunBillInfo[gun].CalTotalServeBill / 100000;   
					//尖峰评估各自费用
					if(CurTimeFrame < TIME_PERIOD_MAX)
					{
						if(Price->CurTimeQuantum[Price->CurTimeQuantum[CurTimeFrame]] < TIME_QUANTUM_MAX)
						{
							//尖峰平谷费用电量
							GunBillInfo[gun].JFPGPower[Price->CurTimeQuantum[CurTimeFrame]] += (TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4);
							GunBillInfo[gun].CalJFPGBill[Price->CurTimeQuantum[CurTimeFrame]] += TempPowerData[gun].CurUsedPower4 * GetCurRate(); 
							GunBillInfo[gun].JFPGBill[Price->CurTimeQuantum[CurTimeFrame]] = GunBillInfo[gun].CalJFPGBill[Price->CurTimeQuantum[CurTimeFrame]] / 100000; 
							
							//分时费用电量
							GunBillInfo[gun].Power[CurTimeFrame] += (TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4);
							GunBillInfo[gun].CalBill[CurTimeFrame] += TempPowerData[gun].CurUsedPower4 * GetCurRate(); 
							GunBillInfo[gun].CalServeBill[CurTimeFrame] += TempPowerData[gun].CurUsedPower4 * Price->ServeFee[CurTimeFrame]; 
							
							GunBillInfo[gun].Bill[CurTimeFrame] = GunBillInfo[gun].CalBill[CurTimeFrame] / 100000; 
							GunBillInfo[gun].ServeBill[CurTimeFrame] = GunBillInfo[gun].CalServeBill[CurTimeFrame] / 100000; 
						}
					}
					TempPowerData[gun].LastPower4 = TempPowerData[gun].CurPower4;		 
					SetPowerChangeFlag(gun, _Flag_0);
					if(SYSSet.NetState == DISP_CARD)
					{
						if(GunBillInfo[gun].TotalBill >= card_info->balance*100)//说明卡内余额用光了
						{
							MeterToDispMsg_StopCharge(gun);					//给显示任务发送停止充电
							SetChargingStatus(gun, _Charging_Free);                          //结束计费
						}
					}
					
				}
			}
		}
	}
	else
	{
		if((SYSSet.NetYXSelct == XY_66)  && (SYSSet.NetState == DISP_NET) )
		{
			if(GetChargingStatus(gun) == _Charging_Busy)  //计费工作中
			{
				Curtime = GetCurUnixTimestamp();
				
				GunBillInfo[gun].ChargeTime = (Curtime - GunBillInfo[gun].StartTimeCount)  / 60; 	
				
				Curtime += (8 * 60 * 60);		//转换为北京时间
				uiSec  = Curtime %  (24 * 60 * 60);  
				uiFeeIndex = uiSec /  (60 * 60 / 2);				
				if(uiFeeIndex >= 48)
				{
					return;
				}
				if(GetPowerChangeFlag(gun) == _Flag_1)      //电量变动
				{
					TempPowerData[gun].CurPower4 = MeterParam[gun].Power4;   //记录当前电量
					TempPowerData[gun].CurUsedPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4;  //计算当前电量变动值
					if(TempPowerData[gun].CurUsedPower4 > 50000)
					{
						TempPowerData[gun].CurPower4 = TempPowerData[gun].LastPower4;    //上一次电量赋值给当前电量
						//一次变动超过5度电，就存在问题   ，测试中存在B枪的当前电量给A枪
						NOP();
						return;
					}
					GunBillInfo[gun].TotalPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].StartPower4;     //记录总电量
					GunBillInfo[gun].CalTotalBill += TempPowerData[gun].CurUsedPower4 * \
					(GunBillInfo[gun].pric[uiFeeIndex] + GunBillInfo[gun].fwpric[uiFeeIndex]) * 10;             //当前费率为5位小数  电量位2位小数
					GunBillInfo[gun].TotalBill = GunBillInfo[gun].CalTotalBill / 100000;    //4位小数
					
					GunBillInfo[gun].CalTotalServeBill += TempPowerData[gun].CurUsedPower4 *  GunBillInfo[gun].fwpric[uiFeeIndex] * 10; 
					GunBillInfo[gun].TotalServeBill = GunBillInfo[gun].CalTotalServeBill / 100000; 

					GunBillInfo[gun].kwh[uiFeeIndex] += (TempPowerData[gun].CurUsedPower4 / 10);
					TempPowerData[gun].LastPower4 = TempPowerData[gun].CurPower4;		             
					SetPowerChangeFlag(gun, _Flag_0);
				}
			}
		}
		else
		{
			if(GetChargingStatus(gun) == _Charging_Busy)  //计费工作中
			{
				Curtime = GetCurUnixTimestamp();
				
				GunBillInfo[gun].ChargeTime = (Curtime - GunBillInfo[gun].StartTimeCount)  / 60; 		
				
				Curtime += (8 * 60 * 60);		//转换为北京时间
				uiSec  = Curtime %  (24 * 60 * 60);  
				uiFeeIndex = uiSec /  (60 * 60 / 2);				
				if(uiFeeIndex >= 48)
				{
					return;
				}
				if(GetPowerChangeFlag(gun) == _Flag_1)      //电量变动
				{
					TempPowerData[gun].CurPower4 = MeterParam[gun].Power4;
					TempPowerData[gun].CurUsedPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4;  //计算当前电量变动值
					if(TempPowerData[gun].CurUsedPower4 > 50000)
					{
						TempPowerData[gun].CurPower4 = TempPowerData[gun].LastPower4;    //上一次电量赋值给当前电量
						//一次变动超过5度电，就存在问题   ，测试中存在B枪的当前电量给A枪
						NOP();
						return;
					}
					GunBillInfo[gun].TotalPower4 = TempPowerData[gun].CurPower4 - TempPowerData[gun].StartPower4;
					GunBillInfo[gun].CalTotalBill += TempPowerData[gun].CurUsedPower4 * GetCurRate();             //当前费率为5位小数  电量位2位小数
					GunBillInfo[gun].TotalBill = GunBillInfo[gun].CalTotalBill / 100000;    //4位小数
					
					GunBillInfo[gun].kwh[uiFeeIndex] += TempPowerData[gun].CurUsedPower4;   //汇誉需要使用  20230628
					
					GunBillInfo[gun].CalTotalServeBill += TempPowerData[gun].CurUsedPower4 * Price->ServeFee[CurTimeFrame]; 
					GunBillInfo[gun].TotalServeBill = GunBillInfo[gun].CalTotalServeBill / 100000;   
					//尖峰评估各自费用
					if(CurTimeFrame < TIME_PERIOD_MAX)
					{
						if(Price->CurTimeQuantum[Price->CurTimeQuantum[CurTimeFrame]] < TIME_QUANTUM_MAX)
						{
							//尖峰平谷费用电量
							GunBillInfo[gun].JFPGPower[Price->CurTimeQuantum[CurTimeFrame]] += (TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4);
							GunBillInfo[gun].CalJFPGBill[Price->CurTimeQuantum[CurTimeFrame]] += TempPowerData[gun].CurUsedPower4 * GetCurRate(); 
							GunBillInfo[gun].JFPGBill[Price->CurTimeQuantum[CurTimeFrame]] = GunBillInfo[gun].CalJFPGBill[Price->CurTimeQuantum[CurTimeFrame]] / 100000; 
							
							//分时费用电量
							GunBillInfo[gun].Power[CurTimeFrame] += (TempPowerData[gun].CurPower4 - TempPowerData[gun].LastPower4);
							GunBillInfo[gun].CalBill[CurTimeFrame] += TempPowerData[gun].CurUsedPower4 * GetCurRate(); 
							GunBillInfo[gun].CalServeBill[CurTimeFrame] += TempPowerData[gun].CurUsedPower4 * Price->ServeFee[CurTimeFrame]; 
							
							GunBillInfo[gun].Bill[CurTimeFrame] = GunBillInfo[gun].CalBill[CurTimeFrame] / 100000; 
							GunBillInfo[gun].ServeBill[CurTimeFrame] = GunBillInfo[gun].CalServeBill[CurTimeFrame] / 100000; 
						}
					}
					TempPowerData[gun].LastPower4 = TempPowerData[gun].CurPower4;		                                 //更新上一次计费电量
					SetPowerChangeFlag(gun, _Flag_0);
					if(SYSSet.NetState == DISP_CARD)
					{
						if(GunBillInfo[gun].TotalBill >= card_info->balance*100)//说明卡内余额用光了
						{
							MeterToDispMsg_StopCharge(gun);					//给显示任务发送停止充电
							SetChargingStatus(gun, _Charging_Free);                          //结束计费
						}
					}
					
				}
			}
		}
	}
}
/****************************************************************************
* Function     : StartBillingInit
* Description  : 开始计费初始化
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
INT8U StartBillingInit(_GUN_NUM gun)
{
	 if(GetChargingStatus(gun) == _Charging_Busy)
		  return FALSE;
	 
	  memset(&GunBillInfo[gun], 0, sizeof(USERINFO));     
	  memset(&TempPowerData[gun], 0, sizeof(TEMPDATA));	
	 GunBillInfo[gun].StartTimeCount = GetCurUnixTimestamp();   //时间戳
	 
	 if(((MeterParam[gun].Power4 - JYPower[gun]) > 20000)  && (JYPower[gun] != 0))  //说明读错了
	 {
		 MeterParam[gun].Power4 = JYPower[gun];
	 }
	 
		TempPowerData[gun].StartPower4 = MeterParam[gun].Power4;           //记录开始电量
	  TempPowerData[gun].LastPower4 = TempPowerData[gun].StartPower4;    //记录上一次开始计算电量
	  GetCurTime(&GunBillInfo[gun].StartTime);                         //记录开始计费时间
	  SetChargingStatus(gun, _Charging_Busy);                          //开始计费	 	 
	 return TRUE;
}
/****************************************************************************
* Function     : ChargingInit
* Description  : 计费初始化
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void ChargingInit(void)
{
	memset(&SysParam, 0, sizeof(SYSPARAM));
	memset(&GunBillInfo[GUN_A], 0, sizeof(USERINFO));	
	memset(&GunBillInfo[GUN_B], 0, sizeof(USERINFO));
	memset(&MeterParam[GUN_A], 0, sizeof(METERPARAM));
	memset(&MeterParam[GUN_B], 0, sizeof(METERPARAM));	
	memset(&TempPowerData[GUN_A], 0, sizeof(TEMPDATA));
	memset(&TempPowerData[GUN_B], 0, sizeof(TEMPDATA));	

	Charging_Prevent = OSQCreate(Charging, CHARGING_QUEUE_LEN);    //创建计费消息队列
	if (Charging_Prevent == NULL)
	{
			OSTaskSuspend(OS_PRIO_SELF);
			return ;
	}		
	//memcpy(MeterParam[GUN_A].addr, Meter_addr1, sizeof(Meter_addr1));//表地址初始化
	//memcpy(MeterParam[GUN_B].addr, Meter_addr2, sizeof(Meter_addr2));//表地址初始化
}
/******************************************************************************
* Function Name     ：RS485MetterInit 
* Description       ：按底层初始化通讯参数
* Input             ：none                                   	    
* Output            ：none
* Note(s)           : 
* Contributor       ：2018.8.22  
******************************************************************************/
INT8U RS485MetterInit(void)
{
    _BSP_UART_SET UartRS485Set;
	
	  RS485Meter_Prevent = OSQCreate(ReadMeter, RS485_RECV_QUEUE_LEN);
    if (RS485Meter_Prevent == NULL)
    {
        printf("Create 485 queue event error, suspend RS485Meter!!!!!");
        OSTaskSuspend(OS_PRIO_SELF);
			  return FALSE;
    }	

    memset(&UartRS485Set, 0 ,sizeof(UartRS485Set) );        
    memset(RecvMeterBuff, 0 ,sizeof(RecvMeterBuff) );
    memset(SendMeterBuff, 0 ,sizeof(SendMeterBuff) );
    
    UartRS485Set.BaudRate = 9600;
    UartRS485Set.DataBits = BSPUART_WORDLENGTH_8;
    UartRS485Set.Parity   = BSPUART_PARITY_EVEN;    //偶校验
    UartRS485Set.StopBits = BSPUART_STOPBITS_1;
    UartRS485Set.RxBuf = RecvMeterBuff;
    UartRS485Set.RxBufLen = sizeof(RecvMeterBuff); 
    UartRS485Set.TxBuf = SendMeterBuff; 
    UartRS485Set.TxBufLen = sizeof(SendMeterBuff); 
    UartRS485Set.Mode = UART_RS485_MODE | UART_MSG_MODE;
    UartRS485Set.RxQueue = RS485Meter_Prevent;
    UartRS485Set.RxMsgMemBuf = RS485RecvSMem;
    UartRS485Set.RxMsgMemLen = sizeof(RS485RecvSMem);
    if (BSP_UARTConfigInit(READMETER_COM, &UartRS485Set) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

/****************************************************************************
* Function     : ChargingAnalyse
* Description  : 来自显示任务的数据解析
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void ChargingAnalyse(_PRIC_CMD div, INT8U *data)
{
	static _GUN_NUM port;
	port = (_GUN_NUM)*data;
	switch(div)
	{
		case PRIC_START:      //开始计费
			StartBillingInit(port);					 
			break;
		case PRIC_STOP:       //结束计费		
			
				//	GetCurTime(&GunBillInfo[port].EndTime);                         //记录结束计费时间
					SetChargingStatus(port, _Charging_Free);                          //结束计费	 			  
			   break;
		default:
			   break;			   
	}
}
/****************************************************************************
* Function     : SearchMeterData
* Description  : 查询电表数据
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void SearchMeterData(_GUN_NUM gun, const INT8U *DI)
{

	//68 01 00 00 00 00 00 68 11 04 33 33 34 33 B3 16
	SendTempBuff[0] = 0x68;
	SendTempBuff[1] = MeterParam[gun].addr[0];
	SendTempBuff[2] = MeterParam[gun].addr[1];
	SendTempBuff[3] = MeterParam[gun].addr[2];
	SendTempBuff[4] = MeterParam[gun].addr[3];
	SendTempBuff[5] = MeterParam[gun].addr[4];
	SendTempBuff[6] = MeterParam[gun].addr[5];


	
	
//	SendTempBuff[1] = 0x01;
//	SendTempBuff[2] = 0x00;
//	SendTempBuff[3] = 0x00;
//	SendTempBuff[4] = 0x00;
//	SendTempBuff[5] = 0x00;
//	SendTempBuff[6] = 0x00;
	
	
	SendTempBuff[7] = 0x68;
	SendTempBuff[8] = 0x11;
	SendTempBuff[9] = 0x04;
	SendTempBuff[10] = DI[3] + 0x33;
	SendTempBuff[11] = DI[2] + 0x33;	

	SendTempBuff[12] = DI[1] + 0x93;				//高精度 4位小数

	SendTempBuff[13] = DI[0] + 0x33;
	
//		SendTempBuff[10] = 0x33;
//	SendTempBuff[11] = 0x34;	
//	SendTempBuff[12] = 0x34;
//	SendTempBuff[13] = 0x35;
	SendTempBuff[14] = TeskCS(SendTempBuff, 14);
	SendTempBuff[15] = 0x16;
	
	BSP_UARTWrite(READMETER_COM, SendTempBuff, 16);	
	OSTimeDly(SYS_DELAY_50ms);
}
extern _BSP_UART_CONTROL   UARTControl[UART_MAX_NUM];
/****************************************************************************
* Function     : ChargingAnalyse
* Description  : 来自显示任务的数据解析
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void MeterDataAnalyse(INT8U *data, INT8U len)
{
	//FE FE FE FE 68 03 00 00 00 00 00 68 91 08 33 33 34 33 A4 43 34 33 87 16 ThreeByteBcdToHex（）
	INT8U i;
	INT64U LastPower, NowPower;
	INT8U tempAddr[6];
	INT8U tempData[5];
	
	if((data[0] == 0xFE)&&(data[1] == 0xFE)&&(data[2] == 0xFE)&&(data[3] == 0xFE)&&(data[4] == 0x68))
	{
		for(i=0; i<6; i++)
		{
			tempAddr[i] = data[5+i];
		}
		for(i=0; i<5; i++)
		{
			tempData[i] = data[18+i] - 0x33;
		}
	}
	else
	{
		for(i=0; i<6; i++)
		{
			tempAddr[i] = data[1+i];
		}
		//瑞银电表
		if(len == 21)
		{
			for(i=0; i<5; i++)
			{
				tempData[i] = data[14+i] - 0x33;
			}
		}
	}
	NowPower = FiveByteBcdToHex(tempData);
	//对比地址，填充电量数据
	//对比电量，置位变动标识
	
	#if(USER_GUN == USER_SINGLE_GUN)
	LastPower = MeterParam[GUN_A].Power4;
	MeterParam[GUN_A].Power4 = NowPower;
	if(LastPower != NowPower)
	{
		SetPowerChangeFlag(GUN_A, _Flag_1);
	}		
	#else
	if(CompareAddr(tempAddr, sizeof(tempAddr), MeterParam[GUN_A].addr, sizeof(MeterParam[GUN_A].addr)) == TRUE)
	{
		LastPower = MeterParam[GUN_A].Power4;
		MeterParam[GUN_A].Power4 = NowPower;
		if(LastPower != NowPower)
		{
			SetPowerChangeFlag(GUN_A, _Flag_1);
		}		
	}
	else if(CompareAddr(tempAddr, sizeof(tempAddr), MeterParam[GUN_B].addr, sizeof(MeterParam[GUN_B].addr)) == TRUE)
	{
		LastPower = MeterParam[GUN_B].Power4;
		MeterParam[GUN_B].Power4 = NowPower;
		if(LastPower != NowPower)
		{
			SetPowerChangeFlag(GUN_B, _Flag_1);
		}
	}
	else
	{
		//2021119 临时这么处理
		OSSchedLock();
		memset(UARTControl[3].UartRxControl.pBuf,0,100);
		memset(UARTControl[3].UartTxControl.pBuf,0,100);
		UARTControl[3].UartRxControl.ReadPoint = 0;
		UARTControl[3].UartRxControl.WritePoint = 0;
		UARTControl[3].UartTxControl.ReadPoint = 0;
		UARTControl[3].UartTxControl.WritePoint = 0;
		OSSchedUnlock();
		NOP(); // 现成出现英利达电表地址不对  20211119
	}
	#endif
	
}
/****************************************************************************
* Function     : ReadMeterTask
* Description  : 抄表
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月22日  
******************************************************************************/
void ReadMeterTask(void)
{
	INT8U err;
	_BSP_MESSAGE *pMsg1;   // 定义消息指针	
	static _GUN_NUM i = GUN_A;
	static INT8U count[2] = {0};
	static INT32U lastpower[2] = {0};
	
	if(SYSSet.SysSetNum.UseGun == 1)
	{
		i = GUN_A;
	}
	else
	{
		(i == GUN_A) ? (i = GUN_B):(i = GUN_A);
	}
	SearchMeterData(i, DI_Power);//抄表
	pMsg1 = OSQPend(RS485Meter_Prevent, SYS_DELAY_500ms, &err);
	if (OS_ERR_NONE == err)
	{
		switch (pMsg1->MsgID)
		{
			case BSP_MSGID_UART_RXOVER:
					OSSchedLock();
					if (UART_MsgDeal(&pMsg1, RecvTempBuff, sizeof(RecvTempBuff)) == TRUE)
					{
						MeterParam[i].ComCnt = 0;
						MeterDataAnalyse(pMsg1->pData, pMsg1->DataLen);	  //帧解析
					}
					UART_FreeOneMsg(READMETER_COM, pMsg1);
					OSSchedUnlock();
					break;
			default:
					break;
		}
		
		
		if((MeterParam[i].Power4 - lastpower[i]) < 20000)
		{
			if(++count[i] > 3)
			{
				JYPower[i] = MeterParam[i].Power4;
				count[i] = 0;
			}
			
		}
		else
		{
			count[i] = 0;
		}
		lastpower[i] = MeterParam[i].Power4;
	}	
	MeterParam[i].ComCnt++;
	if(MeterParam[i].ComCnt > DISCONNECTCNT)
	{
		SetMeterStatus(i,_COM_FAILED);
	}
	else
	{
		SetMeterStatus(i,_COM_NORMAL);
	}
}

/*****************************************************************************
* Function      : TaskRS485Meter
* Description   : 抄表\计费任务
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2018.8.22   
*****************************************************************************/
void TaskMeterStk(void *arg)
{
	RS485MetterInit();    //抄表串口初始化
	OSTimeDly(SYS_DELAY_2s);
	while(1)
	{
		OSTimeDly(SYS_DELAY_1s); 
			//获取A枪B枪表地址,当电表地址修改后及时生效
		APP_GetMeterAddr(GUN_A,MeterParam[GUN_A].addr);  
		APP_GetMeterAddr(GUN_B,MeterParam[GUN_B].addr);
		ReadMeterTask();         //抄表任务	
	}
}

/*****************************************************************************
* Function      : TaskRS485Meter
* Description   : 抄表\计费任务
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2018.8.22   
*****************************************************************************/
void TaskBillStk(void *arg)
{
	INT8U err;
	_BSP_MESSAGE *pMsg;   // 定义消息指针
	ChargingInit();
	OSTimeDly(SYS_DELAY_2s); 
//	//获取A枪B枪表地址
//	APP_GetMeterAddr(GUN_A,MeterParam[GUN_A].addr);
//	APP_GetMeterAddr(GUN_B,MeterParam[GUN_B].addr);
	while(1)
	{
		pMsg = OSQPend(Charging_Prevent, SYS_DELAY_1s, &err);
		if (OS_ERR_NONE == err)
		{
			switch (pMsg->MsgID)
			{
				case BSP_MSGID_DISP:   //来自显示任务
					  ChargingAnalyse((_PRIC_CMD)pMsg->DivNum, pMsg->pData);
					   break;
				default:
					  break;
			}
		}
		
		CalculationRate();        //计算当前费率、获取时间	
		
		CalculationBill(GUN_A);   //计算费用、充电时间
		CalculationBill(GUN_B);		
	}
}
/************************(C)COPYRIGHT 2020  杭州汇誉*****END OF FILE****************************/
