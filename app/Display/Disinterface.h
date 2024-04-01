/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: displaymain.h
* Author			:
* Date First Issued	: 
* Version			: 
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013	        : 
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __DISINTERFACE_H_
#define __DISINTERFACE_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp_rtc.h"
#include "FlashDataDeal.h"
//#include 	"connectccu_interface.h"
/* Private define-----------------------------------------------------------------------------*/
typedef enum
{
	TIME_PERIOD_1 = 0,
	TIME_PERIOD_2,
	TIME_PERIOD_3,
	TIME_PERIOD_4,
	TIME_PERIOD_5,
	TIME_PERIOD_6,
	TIME_PERIOD_7,
	TIME_PERIOD_8,
	TIME_PERIOD_9,
	TIME_PERIOD_10,
	
	TIME_PERIOD_11,
	TIME_PERIOD_12,
	TIME_PERIOD_13,
	TIME_PERIOD_14,
	TIME_PERIOD_15,
	TIME_PERIOD_16,
	TIME_PERIOD_17,
	TIME_PERIOD_18,
	TIME_PERIOD_19,
	TIME_PERIOD_20,
	
	TIME_PERIOD_21,
	TIME_PERIOD_22,
	TIME_PERIOD_23,
	TIME_PERIOD_24,
	TIME_PERIOD_25,
	TIME_PERIOD_26,
	TIME_PERIOD_27,
	TIME_PERIOD_28,
	TIME_PERIOD_29,
	TIME_PERIOD_30,
	
	TIME_PERIOD_31,
	TIME_PERIOD_32,
	TIME_PERIOD_33,
	TIME_PERIOD_34,
	TIME_PERIOD_35,
	TIME_PERIOD_36,
	TIME_PERIOD_37,
	TIME_PERIOD_38,
	TIME_PERIOD_39,
	TIME_PERIOD_40,
	
	TIME_PERIOD_41,
	TIME_PERIOD_42,
	TIME_PERIOD_43,
	TIME_PERIOD_44,
	TIME_PERIOD_45,
	TIME_PERIOD_46,
	TIME_PERIOD_47,
	TIME_PERIOD_48,
	TIME_PERIOD_MAX,
}_TIME_PERIOD;

typedef enum
{
	TIME_QUANTUM_J = 0,	//尖
	TIME_QUANTUM_F,		//峰
	TIME_QUANTUM_P,		//平
	TIME_QUANTUM_G,		//古
	TIME_QPERIOD_5,
	TIME_QPERIOD_6,
	TIME_QPERIOD_7,
	TIME_QPERIOD_8,
	TIME_QPERIOD_9,
	TIME_QPERIOD_10,
	TIME_QPERIOD_11,
	TIME_QPERIOD_12,
	TIME_QUANTUM_MAX,
}_TIME_QUANTUM;
//发送给计费任务cmd，消息的DivNum中体现
typedef enum
{
	PRIC_START = 0,
	PRIC_STOP,
	PRIC_MAX,
}_PRIC_CMD;
//发送个卡任务cmd ，消息的 DivNum中体现
typedef enum
{
	CARDCMD_DEFAULT = 0,    //默认值 
	CARDCMD_QUERY = 1,		  //查询卡内信息
	CARDCMD_STARTA ,		  //A开始充电刷卡
	CARDCMD_STOPA,			  //A停止充电刷卡
	CARDCMD_STARTB,				//b开始充电刷卡
	CARDCMD_STOPB,				//b停止充电刷卡

	NETCARDCMD_STARTA ,		  		//网络A开始充电刷卡
	NETCARDCMD_STOPA,			  	//网络A停止充电刷卡
	NETCARDCMD_STARTB,				//网络b开始充电刷卡
	NETCARDCMD_STOPB,				//网络b停止充电刷卡
	NETCARDCMD_QUERY,		  		//查询卡内信息,只是查询卡号
	NETCARDCMD_UNLOCK,
	CARDCMD_MAX,
}_SEND_TO_CARD_CMD;

//结算状态
typedef enum
{
    RECODE_UNDEFINE = 0,                        //未定义
    RECODE_SETTLEMENT,                          //已结算
    RECODE_UNSETTLEMENT,                        //未结算
	RECODE_REPLSETTLEMENT,						//补充结算
}_BILLING_STATUS;


typedef enum
{
	CHARGE_AUTO = 0,     //自动充满
	CHARGE_TIME,		 //按时间充
	CHARGE_MONEY,		//按金额充
	CHARGE_ELE,			//按电量充
}_CHARGE_TYPEDEF;
//充电方式
typedef struct
{
	
	_CHARGE_TYPEDEF ChargeType;
	INT16U ChargeTime;       // 充电时间
	INT16U ChargeMoney;		 //充电金额
	INT16U ChargeEle;		 //按电量充	
}_CHARGE_INFO;

//启动方式枚举
typedef enum 
{
    TYPE_START_DEVICE = 1,          //1:充电系统启动
    TYPE_START_PLATFORM,            //2:运营平台发送启动
    TYPE_START_OTHER,               //3:其他方式启动
}_PROT_START_TYPE;

/* Private variables--------------------------------------------------------------------------*/
__packed typedef struct
{
	INT8U DivNum[16];		//桩编号
	INT8U ProjectNum[16];	//项目编号
	INT8U  UseGun;			//枪使用个数 1 Byte 	分辨率：1倍，0偏移量
}_SYS_SETNUM;				//编号设置



__packed typedef struct
{
	INT8U chargetype;			 //充电方式0：自动分配1：均充2：轮充
}_SYS_SET4;

__packed typedef struct
{
	//电价设置  目前最多48个时间段，没有24点，24点就是0点
	INT16U StartTime[TIME_PERIOD_MAX];			//开始时间     转换为分钟  分辨率为 1分钟
	INT16U EndTime[TIME_PERIOD_MAX];			//结算时间 转换为分钟  分辨率为 1分钟
	INT32U Price[TIME_PERIOD_MAX];				//电价 	0.00001元 	   ，5位小数
	_TIME_QUANTUM CurTimeQuantum[TIME_PERIOD_MAX];		//当前处于哪个时间段  尖 峰 平 古
	INT32U ServeFee[TIME_PERIOD_MAX];							//服务费			0.00001元 
	INT8U TimeQuantumNum;						//时间段个数								
}_PRICE_SET;//电价设置


//交易记录
__packed typedef struct
{
	INT32U BeforeCardBalance;			//扣款后卡内余额 0.01（ok）
	_BILLING_STATUS BillingStatus;    	//结算标志，1 已结算，2 未结算 3补充结算（ok）
	INT8U TransNum[16];      			//交易流水号
	INT32U TotPower;         			//总电量 	 0.01Kwh
	INT8U StopChargeReason;  			//停止充电原因（ok）  最高位为1表示启动失败停止；最高位为0表示启动成功停止
	INT32U  CardNum;					//卡号(ok)
	_PROT_START_TYPE StartType;        	//启动方式(ok)
	_GUN_NUM Gun;						//枪号(ok)
	INT32U TotMoney;         			//总金额   0.01元
	INT8U CarVin[17];         			//电动汽车唯一标识（ok）
	INT16U ChargeTime;					//充电时间
	_BSPRTC_TIME StartTime;	 			//交易开始时间
	_BSPRTC_TIME EndTime; 	 			//交易结束时间 	
	INT8U StopReason[20]; 				//停止原因
}_CHARGE_RECODE;


//故障\告警对应数据存储在flash中 共25个字节
__packed typedef struct
{
	INT8U GunNum;			//充电接口
	INT8U ErrReason;
	INT8U ErrNum[9];		//遥信1遥信2所有数据
	_BSPRTC_TIME StartTime;	//故障开始时间
	_BSPRTC_TIME EndTime;	//故障结束时间
}_ERRWARN_RECODEINFO;

__packed typedef struct 
{
	_SYS_SETNUM SysSetNum;		 //桩编号项目编号设置
	INT8U MeterAdd[GUN_MAX][12];		//表地址
	INT8U SOCthreshold;		//SOC阈值
	INT8U NetState;			//网络状态  	 0  单机 	1网络
	INT8U LockState;		//电子锁状态	 0  常闭 	1常开
	INT8U CurModule;		//当前模块		0:20kw永联恒功率  1:20kw永联非恒功率   2:盛弘 3:15kw永联恒功率  4:15kw永联非恒功率	5:盛洪20kw模块
	INT8U NetYXSelct;		//网络协议选择
	INT8U IP[4];			//IP
	INT16U Port;				//端口
	INT8U NetNum;			//网络个数
	INT8U GunTemp;			//枪温是否屏蔽   0表示不屏蔽  1表示屏蔽
}_SYS_SET;
//保存数据到flash（交易记录，故障记录各1000条，配置信息）
__packed typedef struct
{
	_SYS_SET	SysSet;			 		//系统配置信息
	_PRICE_SET PriceSet;		 		//电价配置
	INT32U	ChargeRecodeCurNum;	 		//当前交易记录个数，若超过1000，则直接覆盖前面的
	INT32U 	FaultRecodeCurNum;	 		//当前故障记录个数，若超过1000，则直接覆盖前面的
	INT32U  SDBuf1;				 		//留给操作sd卡使用的buf1
	INT32U  SDBuf2;				 		//留给操作sd卡使用的buf2
	_CHARGE_RECODE ChargeRecode; 		//交易记录
	_ERRWARN_RECODEINFO  FaultRecode;	 //故障/告警记录 
}_SAVE_FLASH_DATA;


/* Private typedef----------------------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

/*****************************************************************************
* Function     : APP_GetDispEvent
* Description  : 获取显示事件控制块指针
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14 
*****************************************************************************/
OS_EVENT* APP_GetDispEvent(void);

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : 获取电价信息
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14 
*****************************************************************************/
_PRICE_SET* APP_GetPriceInfo(void);

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : 获取电价信息
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14 
*****************************************************************************/
_PRICE_SET* APP_GetAPriceInfo(void);

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : 获取电价信息
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14 
*****************************************************************************/
_PRICE_SET* APP_GetBPriceInfo(void);

/*****************************************************************************
* Function     : APP_GetMeterAddr
* Description  : 获取表地址
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14 
*****************************************************************************/
INT8U  APP_GetMeterAddr(_GUN_NUM gun,INT8U *pdata);

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
INT8U APP_RWChargeRecode(INT16U count,_FLASH_ORDER RWChoose,_CHARGE_RECODE * precode);
#endif
/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/
