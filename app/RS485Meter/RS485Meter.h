/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: rs485meter.h
* Author			: 
* Date First Issued	: 
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:

* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__RS485METER_H_
#define	__RS485METER_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp_rtc.h"
#include "Disinterface.h"
/* Private define-----------------------------------------------------------------------------*/

//#define      READMETER_COM           USART2 //串口号
#define      READMETER_COM           UART4 //串口号
#define      DISCONNECTCNT              3

/* Private struct-----------------------------------------------------------------------------*/

typedef enum
{
	_Flag_0 = 0,
	_Flag_1,
}FLAGSTATUS;
typedef enum
{
	_Charging_Free,                       //计费空闲
	_Charging_Busy,                       //计费工作
}CHARGING_STATUS;

__packed typedef struct
{
	INT8U   FrameLackcnt;                 //帧缺失计数
	INT32U  CurRate;                      //当前费率
	CHARGING_STATUS ChargingStatus[2];    //当前计费状态   
	INT32U  CurGunRate[GUN_MAX];                      //云快充可能存在2个费率
}SYSPARAM;

__packed typedef struct
{
//	INT32U  StartPower;                //计费开始电量
//  INT32U  CurPower;                 	//当前电量值
//	INT32U  LastPower;                 //上一次电量值	
//  INT32U  CurUsedPower;              //当前阶段使用电量
	INT32U  StartPower4;                //计费开始电量
  INT32U  CurPower4;                 	//当前电量值
	INT32U  LastPower4;                 //上一次电量值	
  INT32U  CurUsedPower4;              //当前阶段使用电量
}TEMPDATA;

__packed typedef struct
{
	INT64U   CalTotalServeBill;			//计算服务总费用，保留10位小数，防止计算时候数据丢失（电量*单价      	电量4位小数 * 单价 5位小数）
	INT64U   CalTotalBill;				//计算总费用，保留10位小数，防止计算时候数据丢失（电量*单价      	电量4位小数 * 单价 5位小数）
	INT32U   TotalServeBill;			//服务总费用  4位小数
	INT32U   TotalBill;                 //总费用	  4位小数	
	//INT32U   TotalPower;                //总电量
	INT32U   TotalPower4;                //总电量 4位小数 主要显示用
	
	INT64U   CalBill[TIME_QUANTUM_MAX]; ///（电量*单价      	电量4小数 * 单价 5位小数）
	INT32U   Bill[TIME_QUANTUM_MAX];     //当前时间段费用  4位   
	INT32U   Power[TIME_QUANTUM_MAX];    //当前时间段电量  2位
	INT64U   CalServeBill[TIME_QUANTUM_MAX]; ///（电量*单价      	电量4位小数 * 单价 5位小数）
	INT32U   ServeBill[TIME_QUANTUM_MAX];//当前时间段服务费 4位   
	
	INT64U   CalJFPGBill[TIME_QUANTUM_MAX]; ///（电量*单价      	电量4位小数 * 单价 5位小数）
	INT32U   JFPGBill[TIME_QUANTUM_MAX];     //当前尖峰平谷费用   原则上是长度为4的数组  4位
	INT32U   JFPGPower[TIME_QUANTUM_MAX];    //当前尖峰平谷量     原则上是长度为4的数组  2位
	
	//可能存在扣的钱和时间显示不一样（原因，先刷卡停止，这个过程中产生了费用）
	INT32U   RealTotalBill;           	//实际扣除总费用
	INT32U   RealTotalPower;         	//时间扣除总电量
	
	INT32U   ChargeTime;                //充电时间
	_BSPRTC_TIME  StartTime;            //起始日期
	_BSPRTC_TIME  EndTime;	            //结束日期
	INT32U StartTimeCount;				//开始时间节拍
	//以下为66信息
	INT32U kwh[48];				//0.001
	INT32U pric[48];			//0.0001
	INT32U fwpric[48];			//0.0001
}USERINFO;

__packed typedef struct                  //用于记录电表参数
{
	INT8U       addr[6];                       //表地址
	INT8U       ComCnt;                        //通信次数记录
	INT8U       ComStatus;                     //通信状态
//	INT32U      Power;                         //电量,2位小数
	FLAGSTATUS  PowerChangeFlag;               //电量变动标识	
	INT32U      Power4;                         //电量,4位小数
}METERPARAM;



/* extern variables-----------------------------------------------------------------------------*/
extern OS_EVENT    *RS485Meter_Prevent;	
extern OS_EVENT    *Charging_Prevent;	
extern METERPARAM   MeterParam[2];                                         
extern SYSPARAM    SysParam;
extern USERINFO    UserBillInfo; 
extern USERINFO    GunBillInfo[2];                                              
/* Private functions--------------------------------------------------------------------------*/

#endif
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
