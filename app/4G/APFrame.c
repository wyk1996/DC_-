/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*---------------------------------------------------------------------------------------
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
/**********************************问题点********************************/
/*
1、计费需要重新处理下
2、B2长度不可变的码
*/
#include "4GMain.h"
#include "4GRecv.h"
#include "APFrame.h"
#include <string.h>
#include "sysconfig.h"
#include "DisplayMain.h"
#include "charging_Interface.h"
#include "RTC_task.h"
#include "DisplayMain.h"
#include "card_Interface.h"
#include "DataChangeInterface.h"
#include "PCUMain.h"
/* Private define-----------------------------------------------------------------------------*/
#define AP_RECV_TABLE_NUM   16
#define AP_SEND_FRAME_LEN	7
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )


__packed typedef struct{
	//CP56格式
	INT16U Second;  //0.001秒
	INT8U	Minute;
	INT8U	Hour;
	INT8U	Day;
	INT8U	Month;
	INT8U	Year;
}AP_TIME;

//充电实时数据
__packed typedef struct{
	uint8_t      	devnum[8];		//充电桩编号
	uint8_t     	gun_index;		//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	uint8_t			gunstate;		//连接确认开关状态     布尔型, 变化上传;0关，1开是否插枪，0表示未插枪，1表示已插枪
	uint16_t 		work_stat;		//工作状态  0离线1故障2待机（空闲）8充电启动中3工作（充电）7 预约9 充电完成10 定时充电
	uint8_t			DCUVol;			//直流母线输出过压告警
	uint8_t			DCQVol;			//直流母线输出欠压告警
	uint8_t			batteryUCur;	//蓄电池充电过流告警
	//精确到小数点后一位
	uint16_t 		ac_a_vol;		//交流A相充电电压	
	uint16_t 		ac_b_vol;		//流B相充电电压
	uint16_t 		ac_c_vol;		//交流C相充电电压
	uint16_t 		ac_a_cur;		//交流A相充电电流
	uint16_t 		ac_b_cur;		//交流B相充电电流
	uint16_t 		ac_c_cur;		//交流C相充电电流
	uint16_t		InYPower;		//输入侧总有功功率
	uint16_t		InWPower;		//输入侧无功功率
	uint16_t		InPF;			//输入侧功率因数
	uint16_t		dc_charge_voltage;		//直流充电电压	
	uint16_t 		dc_charge_current;		//直流充电电流    精确到小数点后二位
	uint8_t			OutDCSWState;			//输出继电器状态   布尔型, 变化上传;0关，1开
	uint8_t			BMSState;				//BMS通信异常    布尔型, 变化上传；0正常，1异常
	uint8_t			batteryLinkState;		//电池连接状态
	uint16_t		batteryMaxVol;			//单体电池最高电压  精确到小数点后三位
	uint16_t		batteryMinVol;			//单体电池最低电压  精确到小数点后三位
	uint32_t 		ChargeKwh; 				//有功总电量  精确到小数点后三位
	uint32_t 		ChargeJKwh; 			//尖电量  精确到小数点后三位
	uint32_t 		ChargeFKwh; 			//峰电量  精确到小数点后三位
	uint32_t 		ChargePKwh; 			//平电量  精确到小数点后三位
	uint32_t 		ChargeGKwh; 			//谷电量  精确到小数点后三位
	uint16_t		soc;					//整数
	uint16_t		ChargeTime;				//累计充电时间	单位：min
	uint8_t			CarVin[32];				//车辆vin码
	uint8_t			Allfail[21];			//没有具体写了。。
	uint32_t		ChargePowerMoney;		//充电费  精确到小数点后两位	
	uint32_t		ChargeFWMoney;		//服务费  精确到小数点后两位
	uint32_t charge_full_time_left;		//距离满电剩余充电时间(min)	
	uint8_t OrderNum[16];					//本次充电的订单号，非充电中时清空
	uint8_t	FailNum[8];					//其他故障代码 0正常，非0 故障，具体查看协议。
}AP_send_B1;


//刷卡鉴权
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		CardNum[8];			//物理卡号  压缩BCD码 8Byte
	INT8U		Code[16];			//密码	都填写0
	INT32U		Money;				//卡余额	圈存卡内余额  都填写0
	INT8U CarVin[32];				//车Vin
}AP_send_B6;

//VIN鉴权
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U		Code[16];			//密码	都填写0
	INT8U CarVin[32];				//车Vin
}AP_send_B8;

//刷卡Vin启动
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		CardNum[8];			//物理卡号  压缩BCD码 8Byte
	INT8U		Code[16];			//密码	都填写0
	INT32U		Money;				//卡余额	圈存卡内余额  都填写0
	INT8U CarVin[32];				//车Vin
	INT8U	 ChargeType;				//启动充电控制方式	1：电量控制充电，2：时间控制充电，3：金额控制充电，4：充满为止  写04
	INT8U	StartCmdData[4];			//启动充电控制数据  启动控制数据，全部为0
}AP_send_B10;

//充电实时数据
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U			bmsversion[3];		//BMS通信协议版本号
	INT8U battype;			//01H:铅酸;02镍氢;03磷酸铁锂;04锰酸锂;05钴酸锂;06三元材料;07聚合物锂离子;08钛酸锂;FFH其他电池
	INT16U batedcapacity;						//整车动力蓄电池系统额定容量0.1
	INT16U ratedvol;								//整车动力蓄电池系统额定总电压0.1
	INT8U battername[4];				//电池生产厂商名称
	INT8U batterinfo[19];			//电池信息，具体为细分详见协议  都填写 0xff
	INT16U unitbatterymaxvol;						//0.01V/位 数据范围0-24V
	INT16U bmsMaxcurrent;							//0.1A/位 -400A偏移量
	INT16U batterywholeenergy;						//动力蓄电池标称总能量0.1
	INT16U chgmaxoutvol;							//最高允许充电总电压 0.1
	INT8U MaxTemprature;							//1摄氏度/位  -50度偏移量  数据范围-50+200
	INT16U batterysoc;								//0.1%/位 数据范围0-100% 充电之前的SOC
	INT16U chargevolmeasureval;				//整车动力蓄电池当前电池电压  0.1V/位
	INT16U  bmsneedvolt;			//电压需求  0.1V/位  充电过程中动态变化
	INT16U  bmsneedcurr;			//电流需求   0.1A/位,-400A偏移量 充电过程中 动态变化
	INT8U  chargemode;				//充电模式 0x01恒压充电, 0x02恒流充电
	INT16U outvolt;					//充电电压测量值    0.1V/位
	INT16U outcurr;					//充电电流测量值   0.1A/位，-400A偏移量
	INT16U MaxVolGroup;			//最高单体动力蓄电池电压及其组号 1-12位电池电压 0.01V/位，数据范围0-24V, 13-16位 组号,1/位数据范围 0-15
	INT16U  remaindertime;			//估算剩余充电时间 0-600分钟，超过600按600发送
	INT16U MaxVolNum;			//最高单体动力蓄电池电压所在编号   数据范围1-256
	INT8U  batpackmaxtemp;			//最高单体动力蓄电池电压所在编号 50度偏移 数据范围 -50-+200
	INT8U  batpackmaxnum;			//最高温度监测点编号
	INT8U  batpackmintemp;			//最低动力储电池温度
	INT8U  batpackminnum;			//最低动力蓄电池温度检测点编号
	INT8U batstate[3];			//未细分
	INT8U ifcharge;				//充电允许    00禁止,01允许
	INT8U moduleinfo[10];		//模块信息目前都为0
	INT16U  DCouttemp1;				//充电桩温度精确到小数点后二位
	INT16U  DCouttemp2;				// 充电枪温度 精确到小数点后二位
	INT16U  Moduletemp3;				//充电模块温度 精确到小数点后二位
	INT16U  batmaxvolt;				//单体电池最高电压  0.001  精确到小数点后三位
	INT16U  batminvolt;				//单体电池最低电压 	0.001  精确到小数点后三位

}AP_SEND_B37;

//启停应答发送
//充电实时数据
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		ifsuccess;			//成功标志 0：成功，1：失败
	INT8U 		failcause[2];		//启动充电失败原因   0000：成功，0001：正在充电中，0002：系统故障， 0003：其他原因
	INT8U Cmd;						//控制命令    0：停止充电，1：启动充电，2：定时充电启动，3：预约充电启动
	AP_TIME CmdTime;				//控制时间  CP56Time2a格式
	INT8U DrderNum[16];				//订单号     平台订单号
}AP_SEND_B5;	

//发送交易记录上传
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		DrderNum[16];		//交易流水号  压缩BCD码
	INT8U		CardNum[8];		//物理卡号  16位编码
	AP_TIME	StartTime;				//开始时间  CP56Time2a格式
	AP_TIME StopTime;				//结束时间  CP56Time2a格式
	INT16U	ChargeTime;				//累计充电时间   单位：min
	INT8U 	MeterInfo[32];			//尖峰平古起始停止电表读数，目前都为0
	INT32U 		ChargeJKwh; 			//尖电量  精确到小数点后2位
	INT32U 		ChargeFKwh; 			//峰电量  精确到小数点后2位
	INT32U 		ChargePKwh; 			//平电量  精确到小数点后2位
	INT32U 		ChargeGKwh; 			//谷电量  精确到小数点后2位
	INT32U 		ChargeAllKwh; 			//总电量  精确到小数点后2位
	INT32U 		ChargeStartKwh; 		//总起示值  精确到小数点后2位
	INT32U 		ChargeStopKwh; 			//总止示值  精确到小数点后2位
	INT16U		StartSoc;				//充电前SOC
	INT16U		StopSoc;				//结束后SOC
	INT8U 	CardVin[32];				//电动汽车唯一标识
	INT16U	StopReason;					//停止原因，详见协议
	INT32U ChargeFee;					//充电费    精确到小数点后两位
	INT32U ServeFee;					//服务费    精确到小数点后两位
	INT8U CarNmb[8];					//车牌号	
	INT8U ifCarNmb;						//确认车牌号功能是否开启   桩的参数设置，确认车牌号开关是否开启（0，未开启；1,开启）Boss增加参数控制，下发给桩，控制参数的开关
}AP_SEND_B12;	

//离线交易记录
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		DrderNum[16];		//交易流水号  压缩BCD码
	INT8U		CardNum[8];		//物理卡号  16位编码
	AP_TIME	StartTime;				//开始时间  CP56Time2a格式
	AP_TIME StopTime;				//结束时间  CP56Time2a格式
	INT16U	ChargeTime;				//累计充电时间   单位：min
	INT8U 	MeterInfo[32];			//尖峰平古起始停止电表读数，目前都为0
	INT16U 	type;					//计量类型  0001-里程 0002-充电量 0003-放电量
	INT32U 		ChargeStartKwh; 		//总起示值  精确到小数点后2位
	INT32U 		ChargeStopKwh; 			//总止示值  精确到小数点后2位
//	INT8U PricInfo[48];					//不具体细分   目前全部为0
	
	uint32_t		JBill;				//尖电价  精确到小数点后五位
	uint32_t 		ChargeJKwh; 			//尖电量  精确到小数点后2位
	uint32_t		JMoney;				//尖金额  精确到小数点后2位
	
	uint32_t		FBill;				//峰电价  精确到小数点后五位
	uint32_t 		ChargeFKwh; 			//峰电量  精确到小数点后2位
	uint32_t		FMoney;				//尖金额  精确到小数点后2位
	
	
	uint32_t		PBill;				//平电价  精确到小数点后五位
	uint32_t 		ChargePKwh; 			//平电量  精确到小数点后2位
	uint32_t		PMoney;				//尖金额  精确到小数点后2位
	
	uint32_t		GBill;				//谷电价  精确到小数点后五位
	uint32_t 		ChargeGKwh; 			//谷电量  精确到小数点后2位
	uint32_t		GMoney;				//尖金额  精确到小数点后2位
	
	INT32U 		ChargeAllKwh; 			//总电量  精确到小数点后2位
	INT16U		StartSoc;				//充电前SOC
	INT16U		StopSoc;				//结束后SOC
	INT8U 	CardVin[32];				//电动汽车唯一标识
	INT16U	StopReason;					//停止原因，0：未知，1：充满，2：主动停止（按键），3：主动停止（远程），4：紧急停机，5：故障停止，6：其他（BMS协议的原因）
	INT32U ChargeFee;					//充电费    精确到小数点后两位
	INT32U ServeFee;					//服务费    精确到小数点后两位
	INT8U CarNmb[8];					//车牌号	
	INT8U ifCarNmb;						//确认车牌号功能是否开启   桩的参数设置，确认车牌号开关是否开启（0，未开启；1,开启）Boss增加参数控制，下发给桩，控制参数的开关
}AP_SEND_B15;	


__packed typedef struct{
	_TIME_QUANTUM CurTimeQuantum;		//当前处于哪个时间段  尖 峰 平 古
	INT32U Power;				//电量  精度：0.01度(千瓦时)
	INT32U Price;				//电费  精度：0.01度(千瓦时)
	INT32U	ServeFee;			//服务费
}_PRIC_INFO;
//发送交易记录上传     充电完成后，在B12发送完成后上报。
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		DrderNum[16];		//交易流水号  压缩BCD码
	INT8U		PricTypeID[8];		//计费模型ID     运营管理系统产生
	INT8U		StartTime[7];		//切换时间  CP56Time2a格式
	INT8U		Timenum;			//时段个数
	_PRIC_INFO	PricInfo[TIME_QUANTUM_MAX];	//费率信息，最多十二个时间段
}AP_SEND_B53;
	

//刷卡白名单下发
__packed typedef struct
{
	INT8U	CardNum[8];			//卡号
	INT8U ifSuccess;			//1-成功 2-失败
}_AP_CARD_STATE;

__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	AP_TIME     time;				//时间戳序号
	INT8U 		CardNub[8];			//白名单编号
	INT8U 		Nnm;				//白名单数量  不多于255 ，一次最多12张
	_AP_CARD_STATE	CardState[12];	//最多十二个
}AP_SEND_B18;

//接收VIN
__packed typedef struct
{
	INT8U	VinNum[17];			//卡号
	INT8U ifSuccess;			//1-成功 2-失败
}_AP_VIN_STATE;

__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	AP_TIME     time;				//时间戳序号
	INT8U 		VinNub[8];			//白名单编号
	INT8U 		Nnm;				//白名单数量  不多于255 ，一次最多12张
	_AP_VIN_STATE	VinState[12];	//最多十二个
}AP_SEND_B20;

//清空白名单应答
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	AP_TIME     time;				//时间戳序号
	INT8U 		VinNub[8];			//白名单编号
	INT8U		ifseccess;			//0成功 1失败
}AP_SEND_B22;

AP_send_B1 APSendB1[GUN_MAX]; 	//发送充电桩实时数据
AP_send_B6 APSendB6[GUN_MAX];	//在线刷卡鉴权
AP_send_B8 APSendB8[GUN_MAX];	//在线Vin鉴权
AP_send_B10 APSendB10[GUN_MAX];	//在线刷卡启动充电
AP_SEND_B37 APSendB37[GUN_MAX]; //车辆信息
AP_SEND_B5	APSendStartAckB5[GUN_MAX];			//发送启动应答信息
AP_SEND_B5	APSendStopAckB5[GUN_MAX];			//发送停止应答信息
AP_SEND_B12	APSendBilB12[GUN_MAX];			//上传在线交易记录
AP_SEND_B15 APSendBilB15[GUN_MAX];			//上传离线交易记录     结构体与实际的报文长度对应不上
AP_SEND_B53 APSendBilB53[GUN_MAX];			//上传分时交易记录   充电完成后，在B12发送完成后上报。
AP_SEND_B18	APSendCardB18;					//卡白名单下发
AP_SEND_B20	APSendVinB20;					//VIN白名单下发
AP_SEND_B22 APSendVinCardResB20;			//清空白名单
_4G_START_TYPE APStartType[GUN_MAX] ={ _4G_APP_START,_4G_APP_START};					//0表示App启动 1表示刷卡启动
_START_NET_STATE StartNetState[GUN_MAX] = {NET_STATE_ONLINE,NET_STATE_ONLINE};		//启动的时候状态


INT8U  SendB53State = 0;			//是否发送分时计费  0表示不发送分时计费   1表示发送分时计费
extern _PRICE_SET PriceSet;							//电价设置   
/*******************************************接收数据*************************************/
//接收数据
typedef struct
{
	INT8U * pdata;
	INT8U len;
	 INT8U  (*recvfunction)(INT8U *,INT16U);				//传入数据长度
}_AP_RECV_DISPOSE;

//下发费率1
__packed typedef struct{
	uint8_t      	devnum[8];		//充电桩编号
	uint8_t			PricTypeID[8];	//计费模型ID     运营管理系统产生
	uint8_t			StartTime[7];	//生效时间  CP56Time2a格式
	uint8_t			StopTime[7];	//失效时间
	uint16_t		UserState;		//执行状态  01-有效 02-无效
	uint16_t		type;			//计量类型 01-里程 02-充电量 03-放电量
	uint8_t			FricNum[48];	//费率号
	uint8_t			FricNumber;		//费率数量  必须为4
	uint32_t		Fric[4];			//4个费率  精确到小数点后五位
	uint32_t		ServiceFric;
}AP_RECV_B2;

//下发费率2
__packed typedef struct{
	INT8U 	CurTimeQuantum;			//充电桩编号  1：尖2：峰3：平4：谷
	INT8U 	StartTime[2];			//24小时制，格式：时分（HHmm）  BCD码
	INT8U 	StopTime[2];			//24小时制，格式：时分（HHmm）	BCD码
	INT32U 	Fric;					//精确到小数点后五位
	INT32U	ServiceFric;			//精确到小数点后五位
}SETPRIC_INFO;
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U		PricTypeID[8];		//计费模型ID     运营管理系统产生
	INT8U		StartTime[7];		//生效时间  CP56Time2a格式
	INT8U		StopTime[7];		//失效时间
	INT16U		UserState;			//执行状态  01-有效 02-无效
	INT8U		FricNumber;			//最多不超过12个
	SETPRIC_INFO SetPricInfo[12];	//费率信息
}AP_RECV_B47;


//下发启停充电
__packed typedef struct{
	uint8_t 	devnum[8];		//充电桩编号
	INT8U     	gun_index;		//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U     	CmdST;			//0：停止充电，1：启动充电
	INT8U StartTJ;				//启动充电条件 0：即时充电，1：定时充电
	INT8U StartType;			//1：电量控制充电，2：时间控制充电，3：金额控制充电，4：充满为止
	INT32U StartCmdData;		//启动充电控制数据
	INT8U UserNum[8];			//用户编号    压缩BCD码  8Byte  平台交易流水后16位编码，用做在线交易记录中的物理卡号
	INT8U DrderNum[16];			//订单号  压缩BCD码
}AP_RECV_B4;

//刷卡鉴权回复（在线刷卡充电）
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		CardNum[8];			//物理卡号  压缩BCD码 8Byte
	INT8U CarVin[32];				//车Vin
	INT8U BillBum[8];				//计费模型编码 BCD 8Byte
	INT32U Money;					//账户余额  精确到小数点后两位
	INT8U	IfSuccess;				//鉴权成功标志
	INT8U FailCause[2];				//鉴权失败原因  BCD  5001:无效卡号5003:余额不足5004:卡锁住5007:无效卡5011:套餐余额不足5015:无效车状态5016:无效账户状态5017:密码错误一次5018:密码错误两次5019:密码错误三次5099:系统错误
	INT32U	SurplusKM;				//剩余里程 精确到小数点后两位
	INT32U	ChargeKwH;				//可充电量 精确到小数点后两位
	INT32U SurplusNum;				//剩余次数  精确到小数点后两位
	INT8U ChargeType;				//充电方式 0 : 有卡充电	1 : 无卡充电
	//INT8U CarNum[8];				//车牌号
}AP_RECV_B7;

//VIN鉴权回复（在线VIN充电）
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U CarVin[32];				//车Vin
	INT8U BillBum[8];				//计费模型编码 BCD 8Byte
	INT32U Money;					//账户余额  精确到小数点后两位
	INT8U	IfSuccess;				//鉴权成功标志
	//5017:客户信息不存在5018:车辆信息不存在5019:客户禁用充电5099:系统错误
	INT8U FailCause[2];				//鉴权失败原因  6001:无效VIN码6002:VIN码未入白名单6003:枪口不可用5003:余额不足5011:套餐余额不足5012:对应的产品信息不存在5015:无效车状态5016:无效账户状态
	INT32U	SurplusKM;				//剩余里程 精确到小数点后两位
	INT32U	ChargeKwH;				//可充电量 精确到小数点后两位
	INT32U SurplusNum;				//剩余次数  精确到小数点后两位
	INT8U ChargeType;				//充电方式 0 : 有卡充电	1 : 无卡充电
	//INT8U CarNum[8];				//车牌号
}AP_RECV_B9;
//刷卡启动
__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U ifSuccess;				//通知成功标志 布尔型（1，通知成功；0，通知失败）
	INT8U FailInfo[2];				//通知失败原因 BCD 5001:无效卡号5003:余额不足5004:卡锁住5007:无效卡5011:套餐余额不足5015:无效车状态 5016:无效账户状态5017:密码错误一次5018:密码错误两次5019:密码错误三次5099:系统错误
	INT8U DrderNum[16];				//订单号  压缩BCD码
//	INT8U CarInfo[17];				//车架号
//	INT8U CarNum[8];				//车牌号
}AP_RECV_B11;


//刷卡白名单下发
__packed typedef struct
{
	INT8U	CardNum[8];			//卡号
	INT8U State;				//1-新增 2-删除，第N个白名单状态	
}_AP_CARD_INFO;

__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	AP_TIME     time;				//时间戳序号
	INT8U 		CardNub[8];			//白名单编号
	INT8U 		Nnm;				//白名单数量  不多于255 ，一次最多12张
	_AP_CARD_INFO	CardInfo[12];	//最多十二个
}_AP_RECV_B17;

//接收VIN
__packed typedef struct
{
	INT8U	VIN[17];			//卡号
	INT8U State;				//1-新增 2-删除，第N个白名单状态	
}_AP_VIN_INFO;

__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	AP_TIME     time;				//时间戳序号
	INT8U 		VinNub[8];			//白名单编号
	INT8U 		Nnm;				//白名单数量  不多于255 ，一次最多12张
	_AP_VIN_INFO	VinInfo[12];	//最多十二个
}_AP_RECV_B19;

__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	AP_TIME     time;				//时间戳序号
	INT8U 		VinNub[8];			//白名单编号
	INT8U 		type;				//1:充点卡，2:VIN码
}_AP_RECV_B21;


__packed typedef struct{
	INT8U      	devnum[8];			//充电桩编号
	INT8U     	gun_index;			//充电接口标识  充电桩为一桩多充时用来标记接口号，一桩一充时此项为0。
	INT8U 		ifSuccess;			//0：成功，1：失败
	INT8U DrderNum[16];				//32位交易代码，桩自己生成 
}_AP_RECV_B16;

AP_RECV_B2 	APRecvB2;					//费率设置B2	
AP_RECV_B47 APRecvB47;					//费率设置B47
AP_TIME APRecvSetTime;			//设置时间
AP_RECV_B4		APRecvStartCmd[GUN_MAX];	//接收到启动命令
AP_RECV_B4		APRecvStopCmd[GUN_MAX];		//接收到启动命令
AP_RECV_B7		APRecvCardCmd[GUN_MAX];		//接收到卡鉴权命令
AP_RECV_B9		APRecvVinCmd[GUN_MAX];		//接收到VIN码鉴权命令
AP_RECV_B11    	APRecvCardVinStart[GUN_MAX];	//接收到刷卡启动命令
_AP_RECV_B17	APCardWL;					//卡白名单接收
_AP_RECV_B19	APVinWL;					//VIN白名单
_AP_RECV_B21	APVinCardRes;				//白名单清空
_AP_RECV_B16	APOFFLineRec;				//离线交易记录
//（除了B47,B52,B53,B55以外）：充电桩编号前内容不用变，按照示例来，充电桩编号后内容根据实际值来。B47,B52,B53,B55 的长度是可扩展的。
INT8U RecvRegister[2] = {0x68,0x01};			//注册帧返回（协议返回）
//费率下发返回 B2
//当数据长度为0xff,则数据长度为可变，则不比较数据长度
INT8U RecvCardB7[15] = {0x68,0x5A,0x00,0x00,0x00,0x00,0x85,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x02};
INT8U RecvCardB9[15] = {0x68,0x52,0x00,0x00,0x00,0x00,0x85,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x34};
INT8U RecvCardStartB11[15] = {0x68,0x29,0x00,0x00,0x00,0x00,0x85,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x0C};
INT8U RecvPricB2[15] = {0x68,0x74,0x00,0x00,0x00,0x00,0x85,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x05};
INT8U RecvPricB47[15] = {0x68,0xff,0x00,0x00,0x00,0x00,0x85,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x40};
INT8U RecvStartApp[6] = {0x68,0x04,0x0B,0x00,0x00,0x00};
INT8U RecvHear[6] = {0x68,0x04,0x83,0x00,0x00,0x00};
INT8U RecvTime[14] = {0x68,0x13,0x00,0x00,0x00,0x00,0x67,0x00,0x06,0x00,0x00,0x00,0x00,0x00};		//对时间
INT8U RecvSTCmd[15]  = {0x68,0x35,0x00,0x00,0x00,0x00,0x85,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x15}; 	//启停命令
INT8U RecvOnlineBillCmd[15]  = {0x68,0x27,0x00,0x00,0x00,0x00,0x82,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x02}; 	//在线交易记录
INT8U RecvB54BillCmd[15]  = {0x68,0x27,0x00,0x00,0x00,0x00,0x85,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x43}; 		//在线分时明细交易记录
INT8U RecvB17CardWL[15]  = {0x68,0xff,0x00,0x00,0x00,0x00,0x85,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x32}; 		//卡白名单接收
INT8U RecvB19VINWL[15]  = {0x68,0xff,0x00,0x00,0x00,0x00,0x85,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x33}; 		//vin白名单接收
INT8U RecvB21VINCardRes[15]  = {0x68,0x25,0x00,0x00,0x00,0x00,0x85,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x35}; 		//vin白名单接收
INT8U RecvB16APOFFLineRec[15]  = {0x68,0x27,0x00,0x00,0x00,0x00,0x82,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x03}; 		//vin白名单接收
static INT8U   AP_RegisteeAck(INT8U *pdata,INT16U len);   		//注册帧返回（协议返回）
static INT8U   AP_RecvPricB2(INT8U *pdata,INT16U len);   			//费率设置
static INT8U   AP_RecvPricB47(INT8U *pdata,INT16U len);   			//费率设置
static INT8U   AP_RecvStartAppAck(INT8U *pdata,INT16U len);   	//启动确认帧
static INT8U   AP_RecvHearAck(INT8U *pdata,INT16U len);   		//心跳确认帧
static INT8U   AP_RecvTime(INT8U *pdata,INT16U len);   			//校准时间
static INT8U   AP_RecvOnlineBillAck(INT8U *pdata,INT16U len);	//返回在线交易记录
static INT8U   AP_RecvB54BillAck(INT8U *pdata,INT16U len);		//在线分时明细交易记录
static INT8U   AP_RecvSTCmd(INT8U *pdata,INT16U len);			//接收倒启停命令
static INT8U   AP_RecvCardB7(INT8U *pdata,INT16U len);			//接收到卡鉴权
static INT8U   AP_RecvCardB9(INT8U *pdata,INT16U len);
static INT8U   AP_RecvCardVinStartB11(INT8U *pdata,INT16U len);	//接收到卡VIN启动命令
static INT8U   AP_RecvB17CardWL(INT8U *pdata,INT16U len);		//开白名单接收
static INT8U AP_RecvB19VINWL(INT8U *pdata,INT16U len);			//Vin白名单接收
static INT8U AP_RecvB21VINCardRes(INT8U *pdata,INT16U len);			//Vin白名单接收
static INT8U AP_RecvB16OFFLineRco(INT8U *pdata,INT16U len);			//离线交易记录应答
_AP_RECV_DISPOSE  APRecvDisposeTable[AP_RECV_TABLE_NUM] = {
	{RecvRegister,			2,		AP_RegisteeAck					},  //注册帧返回（协议返回）
	
	{RecvPricB2,			15,		AP_RecvPricB2						},  //费率设置
	
	{RecvPricB47,			15,		AP_RecvPricB47						},  //费率设置
	
	{RecvStartApp,			6,		AP_RecvStartAppAck				},  //登录
	
	{RecvHear,				6,		AP_RecvHearAck					},  //心跳确认帧
	
	{RecvTime,				14,		AP_RecvTime						},  //校时
	
	{RecvOnlineBillCmd,		15,		AP_RecvOnlineBillAck			},  //返回在线交易记录
	
	{RecvB54BillCmd,		15,		AP_RecvB54BillAck				},  //在线分时明细交易记录
	
	{RecvSTCmd,				15,		AP_RecvSTCmd					},  //接收到启停命令
	
	{RecvCardB7,			15,		AP_RecvCardB7					},  //卡鉴权

	{RecvCardB9,			15,		AP_RecvCardB9					},  //Vin鉴权	
	
	{RecvCardStartB11,		15,		AP_RecvCardVinStartB11			},  //接收到卡Vin启动命令
	
	{RecvB17CardWL,			15,		AP_RecvB17CardWL				},	//接收到卡白名单
	
	{RecvB19VINWL,			15,		AP_RecvB19VINWL					},	//接收到VIN白名单
	
	{RecvB21VINCardRes,		15,		AP_RecvB21VINCardRes			},	//接收到白名单清空
	
	{RecvB16APOFFLineRec,	15,		AP_RecvB16OFFLineRco			},	//接收到白名单清空	
	
};

/***********************发送数据*********************************************/
static INT8U  AP_SendStartApp(void);	//登录下发U帧
static INT8U  AP_SendHear(void);		//下发心跳帧
static INT8U   AP_SendRegister(void);	//发送注册帧
static INT8U  AP_SendChargeInfoA(void);	//A	枪状态帧
static INT8U  AP_SendChargeInfoB(void); //B 枪状态帧
static INT8U AP_SendCarInfoA(void); 	//A枪车辆信息，充电中发送
static INT8U AP_SendCarInfoB(void); 	//B枪车辆信息，充电中发送

static INT8U  AP_SendSetPricB2Ack(void);
static INT8U  AP_SendSetPricB47Ack(void);
_4G_SEND_TABLE APSendTable[AP_SEND_FRAME_LEN] = {
	{0,    0,    SYS_DELAY_10s, 	AP_SendStartApp			},  //U帧

	{0,    0,    SYS_DELAY_30s, 	AP_SendHear				},	//心跳
	
	{0,    0,    SYS_DELAY_10s, 	AP_SendRegister			},	//注册帧

	{0,    0,    SYS_DELAY_30s, 	AP_SendChargeInfoA		},	//充电桩A状态
	
	{0,    0,    SYS_DELAY_30s, 	AP_SendChargeInfoB		},	//充电桩B状态
	
	{0,    0,    SYS_DELAY_30s, 	AP_SendCarInfoA			},	//A枪车辆信息，充电中发送
	
	{0,    0,    SYS_DELAY_30s, 	AP_SendCarInfoB			},	//B枪车辆信息，充电中发送

};

INT8U FlashDisposeBuf[5000];				//flash处理缓存


/*****************************************************************************
* Function     : AP_CardWhiteListDispose
* Description  : 卡白名单处理
* Input        :
				pcardnud 卡号
				num		 个数
				state    1 表示新增  0表示删减
				
				flash说明
				出厂时第一个字节为0，后面所有都为0XFF
				1第一个字节表示白名单数量,后面都为开号，卡号为8个字节	
		
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_CardWhiteListDispose(INT8U*  pcardnum,INT16U num,INT8U state)
{
	_FLASH_OPERATION  FlashOper;
	INT8U curnum,i,j,ifwriter;
	if(pcardnum == NULL || num > 12)		//一次增减少最多十二个
	{
		return FALSE;
	}
	 //读取数据
	FlashOper.DataID = PARA_CARDWHITEL_ID;
	FlashOper.Len = PARA_CARDWHITEL_FLLEN;
	FlashOper.ptr = FlashDisposeBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read cardwl  err");
	}
	ifwriter  = 0;
	curnum = FlashDisposeBuf[0];
	if(state)  //增加卡号
	{
		for(j = 0;j < num;j++)
		{
			if(curnum == 255)
			{
				break;
			}
			for(i = 0;i < curnum;i++)
			{
			
				if(CmpNBuf(&FlashDisposeBuf[1+i*8],&pcardnum[8*j],8))
				{
					break;  //说明之前就有
				}
			}
			if(i == curnum)
			{
				//说明之前没有这个卡号，添加进来
				memcpy(&FlashDisposeBuf[1+curnum*8],&pcardnum[8*j],8);
				ifwriter = 1;
				curnum++;
				FlashDisposeBuf[0] = curnum;
			}
		}
	}
	else
	{
		//减少卡号
		for(j = 0;j < num;j++)
		{
			if(curnum == 0)
			{
				break;
			}
			for(i = 0;i < curnum;i++)
			{
				if(CmpNBuf(&FlashDisposeBuf[1+i*8],&pcardnum[8*j],8))
				{
					  //说明之前就有
					memset(&FlashDisposeBuf[1+i*8],0xff,8);
					curnum --;
					ifwriter = 1;
					if(curnum > i)
					{
						//后面数据往前面移动
						memcpy(&FlashDisposeBuf[1+i*8],&FlashDisposeBuf[1+curnum*8],8);
						memset(&FlashDisposeBuf[1+curnum*8],0xff,8);
					}
				}
			}
		}
		FlashDisposeBuf[0] = curnum;
	}
	if(ifwriter)
	{
		FlashOper.RWChoose = FLASH_ORDER_WRITE;				//写数据			
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("WRITE cardwl  err");
		}
	}
	return TRUE;
}


/*****************************************************************************
* Function     : AP_VinWhiteListDispose
* Description  : VIN白名单处理
* Input        :
				pcardnud 卡号
				num		 个数
				state    1 表示新增  0表示删减
				
				flash说明
				出厂时第一个字节为0，后面所有都为0XFF
				1第一个字节表示白名单数量,后面都为开号，卡号为8个字节	
		
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_VinWhiteListDispose(INT8U*  pcardnum,INT16U num,INT8U state)
{
		_FLASH_OPERATION  FlashOper;
	INT8U curnum,i,j,ifwriter;
	if(pcardnum == NULL || num > 12)		//一次增减少最多十二个
	{
		return FALSE;
	}
	 //读取数据
	FlashOper.DataID = PARA_VINWHITEL_ID;
	FlashOper.Len = PARA_VINWHITEL_FLLEN;
	FlashOper.ptr = FlashDisposeBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read cardwl  err");
	}
	ifwriter  = 0;
	curnum = FlashDisposeBuf[0];
	if(state)  //增加VIN
	{
		for(j = 0;j < num;j++)
		{
			if(curnum == 255)
			{
				break;
			}
			for(i = 0;i < curnum;i++)
			{
			
				if(CmpNBuf(&FlashDisposeBuf[1+i*17],&pcardnum[17*j],17))
				{
					break;  //说明之前就有
				}
			}
			if(i == curnum)
			{
				//说明之前没有这个卡号，添加进来
				memcpy(&FlashDisposeBuf[1+curnum*17],&pcardnum[17*j],17);
				ifwriter = 1;
				curnum++;
				FlashDisposeBuf[0] = curnum;
			}
		}
	}
	else
	{
		//减少vin
		for(j = 0;j < num;j++)
		{
			if(curnum == 0)
			{
				break;
			}
			for(i = 0;i < curnum;i++)
			{
				if(CmpNBuf(&FlashDisposeBuf[1+i*17],&pcardnum[17*j],17))
				{
					  //说明之前就有
					memset(&FlashDisposeBuf[1+i*17],0xff,17);
					curnum --;
					ifwriter = 1;
					if(curnum > i)
					{
						//后面数据往前面移动
						memcpy(&FlashDisposeBuf[1+i*17],&FlashDisposeBuf[1+curnum*17],17);
						memset(&FlashDisposeBuf[1+curnum*17],0xff,17);
					}
				}
			}
		}
		FlashDisposeBuf[0] = curnum;
	}
	if(ifwriter)
	{
		FlashOper.RWChoose = FLASH_ORDER_WRITE;				//写数据			
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("WRITE cardwl  err");
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetAPSta
* Description  : 获取安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_GetAPStartType(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return _4G_APP_START;
	}
	return (INT8U)APStartType[gun];
}


/*****************************************************************************
* Function     : APP_SetAPStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_SetAPStartType(INT8U gun ,_4G_START_TYPE  type)
{
	if((type >=  _4G_APP_MAX) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	
	APStartType[gun] = type;
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetStartNetState
* Description  : 获取启动方式网络状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_GetStartNetState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return _4G_APP_START;
	}
	return (INT8U)StartNetState[gun];
}

/*****************************************************************************
* Function     : APP_SetStartNetState
* Description  : 设置启动方式网络状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_SetStartNetState(INT8U gun ,_START_NET_STATE  type)
{
	if((type >=  NET_STATE_MAX) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	
	StartNetState[gun] = type;
	return TRUE;
}



/*****************************************************************************
* Function     : HY_SendRateAck
* Description  : 费率设置应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   AP_SendRateAck(INT8U cmd)
{
	if(cmd == 1)
	{
		AP_SendSetPricB2Ack();
	}
	else
	{
		AP_SendSetPricB47Ack();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetAPBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetAPBatchNum(INT8U gun)
{
	static INT8U buf[16];		//交易流水号位16个字节，ASICC 因此取后16个数字

	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	memcpy(buf,APRecvStartCmd[gun].DrderNum,16);		//订单号的后16位唯一
	return buf;
}

/*****************************************************************************
* Function     : APP_GetAPQGNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetAPQGNetMoney(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	return 0; //目前全部返回0
}

/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  : 汇誉接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   AP_SendFrameDispose(void)
{
	INT8U i;

	for(i = 0;i < AP_SEND_FRAME_LEN;i++)
	{
		if(APSendTable[i].cycletime == 0)
		{
			continue;
		}
		APSendTable[i].curtime = OSTimeGet();
		if((APSendTable[i].curtime >= APSendTable[i].lasttime) ? ((APSendTable[i].curtime - APSendTable[i].lasttime) >= APSendTable[i].cycletime) : \
		((APSendTable[i].curtime + (0xFFFFFFFFu - APSendTable[i].lasttime)) >= APSendTable[i].cycletime))
		{
			APSendTable[i].lasttime = APSendTable[i].curtime;
			if(APSendTable[i].Sendfunc != NULL)
			{
				APSendTable[i].Sendfunc();
			}
		}
		
	}
	return TRUE;
}


/*****************************************************************************
* Function     : AP_SendRegister
* Description  : 注册 (协议选择)
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_SendRegister(void)
{
	INT8U Sendbuf[20];
	INT8U * pdevnum = APP_GetDevNum(); 
	if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
	{
		return  FALSE;
	}
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x01;	//协议
	
	//设备编号
	//8字节终端号(BCD)
	Sendbuf[9] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	Sendbuf[8] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	Sendbuf[7] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	Sendbuf[6] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	Sendbuf[5] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	Sendbuf[4] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	Sendbuf[3] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	Sendbuf[2] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	return ModuleSIM7600_SendData(0, Sendbuf,12); //发送数据
}

/*****************************************************************************
* Function     : AP_SendStartApp
* Description  : 用于启动应用传输层
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendStartApp(void)
{
	if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
	{
		return  FALSE;
	}
	INT8U Sendbuf[20];
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x04;
	Sendbuf[2] = 0x07;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	return ModuleSIM7600_SendData(0, Sendbuf,6); //发送数据
}


/*****************************************************************************
* Function     : AP_SendHear
* Description  : 发送心跳 30s发送一次
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendHear(void)
{
	INT8U Sendbuf[20];
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x04;
	Sendbuf[2] = 0x43;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	return ModuleSIM7600_SendData(0, Sendbuf,6); //发送数据
}

/*****************************************************************************
* Function     : AP_SendChargeInfo
* Description  : 上报频率，充电中是30秒，待机2分钟；
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendChargeInfoA(void)
{
//	static INT8U laststate = 2;
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	//memset(APSendB1[GUN_A].devnum,0,sizeof(AP_send_B1));
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)	//没有注册成功
	{
		return  FALSE;
	}
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0xA6;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x86;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	//设备编号
	APSendB1[GUN_A].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendB1[GUN_A].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendB1[GUN_A].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendB1[GUN_A].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendB1[GUN_A].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendB1[GUN_A].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendB1[GUN_A].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendB1[GUN_A].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	

	APSendB1[GUN_A].gun_index = 0;
	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
	{
		APSendB1[GUN_A].gunstate = 0;
	}
	else
	{
		APSendB1[GUN_A].gunstate = 1;
	}
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		APSendB1[GUN_A].work_stat = 3;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_STARTING)
	{
		APSendB1[GUN_A].work_stat = 8;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)
	{
		APSendB1[GUN_A].work_stat = 1; //故障
	}else
	{
		APSendB1[GUN_A].work_stat = 2; //待机
	}
	//具体故障原因
	memset(APSendB1[GUN_A].Allfail,0,sizeof(APSendB1[GUN_A].Allfail));
	
	if (GetEmergencyState(GUN_A) == EMERGENCY_PRESSED)
	{
		APSendB1[GUN_A].Allfail[3] = 1;	//急停
	}
//	else if((YX1_Info[GUN_A].state3.State.AC_OVP) || (YX1_Info[GUN_A].state3.State.AC_UVP))
//	{
//		APSendB1[GUN_A].Allfail[6] = 1;	//交流输入异常
//	}
//	else if(YX1_Info[GUN_A].state3.State.DCOVP_Warn)
//	{
//		APSendB1[GUN_A].Allfail[10] = 1;	//输出过呀告警
//	}
//	else if(YX1_Info[GUN_A].state3.State.UPCUR_Warn)
//	{
//		APSendB1[GUN_A].Allfail[11] = 1;	//输出过流告警
//	}
//	else
//	{
//		NOP();
//	}
	
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		APSendB1[GUN_A].dc_charge_voltage = PowerModuleInfo[GUN_A].OutputInfo.Vol;			//电压
		APSendB1[GUN_A].dc_charge_current  =  PowerModuleInfo[GUN_A].OutputInfo.Cur*10;	//电流  精确到小数点后二位
		APSendB1[GUN_A].OutDCSWState = 0x01; //输出继电器状态 
		APSendB1[GUN_A].batteryLinkState = 0x01; //电池连接状态
		if(BMS_BSM_Context[GUN_A].MaxbatteryTemprature > 50)
		{
			APSendB1[GUN_A].batteryMaxVol =(BMS_BSM_Context[GUN_A].MaxbatteryTemprature - 50 )  * 1000;	//单体电池最高电压  0.001  精确到小数点后三位
			
		}
		if(BMS_BSM_Context[GUN_A].MinbatteryTemprature > 50)
		{
			APSendB1[GUN_A].batteryMinVol = (BMS_BSM_Context[GUN_A].MinbatteryTemprature - 50) * 1000;	//单体电池最低电压 	0.001  精确到小数点后三位
		}
		APSendB1[GUN_A].ChargeKwh = puserinfo->TotalPower4/100;   //小数点三位
		APSendB1[GUN_A].ChargeJKwh = puserinfo->JFPGPower[0]/100;
		APSendB1[GUN_A].ChargeFKwh = puserinfo->JFPGPower[1]/100;
		APSendB1[GUN_A].ChargePKwh = puserinfo->JFPGPower[2]/100;
		APSendB1[GUN_A].ChargeGKwh = puserinfo->JFPGPower[3]/100;
		APSendB1[GUN_A].soc = (INT16U)BMS_BCS_Context[GUN_A].SOC;
		APSendB1[GUN_A].ChargeTime = puserinfo->ChargeTime;		//累计充电时间	单位：min
		memcpy(APSendB1[GUN_A].CarVin,BMS_BRM_Context[GUN_A].VIN,17);
		APSendB1[GUN_A].ChargePowerMoney = (puserinfo->TotalBill - puserinfo->TotalServeBill)/100;
		APSendB1[GUN_A].ChargeFWMoney = puserinfo->TotalServeBill/100;
		APSendB1[GUN_A].charge_full_time_left =  BMS_BCS_Context[GUN_A].RemainderTime;
		//	uint8_t OrderNum;					//本次充电的订单号，非充电中时清空	
		if(APStartType[GUN_A] == _4G_APP_START)
		{

			memcpy(APSendB1[GUN_A].OrderNum,APRecvStartCmd[GUN_A].DrderNum,sizeof(APSendB1[GUN_A].OrderNum));
		}
		else
		{
			memcpy(APSendB1[GUN_A].OrderNum,APRecvCardVinStart[GUN_A].DrderNum,sizeof(APRecvCardVinStart[GUN_A].DrderNum));
		}

	}
	memcpy(&Sendbuf[11],(INT8U*)&APSendB1[GUN_A],sizeof(AP_send_B1));

	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_send_B1) +11); //发送数据
}

/*****************************************************************************
* Function     : AP_SendChargeInfo
* Description  : 上报频率，充电中是30秒，待机2分钟；
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendChargeInfoB(void)
{
	//static INT8U laststate = 2;
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	//memset(APSendB1[GUN_B].devnum,0,sizeof(AP_send_B1));
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)	//没有注册成功
	{
		return  FALSE;
	}
	if(APP_GetGunNum() == 1)
	{
		return FALSE;
	}
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0xA6;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x86;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	//设备编号
	APSendB1[GUN_B].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendB1[GUN_B].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendB1[GUN_B].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendB1[GUN_B].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendB1[GUN_B].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendB1[GUN_B].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendB1[GUN_B].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendB1[GUN_B].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	

	APSendB1[GUN_B].gun_index = 1;
	if(GetGunState(GUN_B) == GUN_DISCONNECTED)
	{
		APSendB1[GUN_B].gunstate = 0;
	}
	else
	{
		APSendB1[GUN_B].gunstate = 1;
	}
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		APSendB1[GUN_B].work_stat = 3;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_STARTING)
	{
		APSendB1[GUN_B].work_stat = 8;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)
	{
		APSendB1[GUN_B].work_stat = 1; //故障
	}else
	{
		APSendB1[GUN_B].work_stat = 2; //待机
	}
		//具体故障原因
	memset(APSendB1[GUN_B].Allfail,0,sizeof(APSendB1[GUN_B].Allfail));
	if (GetEmergencyState(GUN_B) == EMERGENCY_PRESSED)
	{
		APSendB1[GUN_B].Allfail[3] = 1;	//急停
	}
//	else if((YX1_Info[GUN_B].state3.State.AC_OVP) || (YX1_Info[GUN_B].state3.State.AC_UVP))
//	{
//		APSendB1[GUN_B].Allfail[6] = 1;	//交流输入异常
//	}
//	else if(YX1_Info[GUN_B].state3.State.DCOVP_Warn)
//	{
//		APSendB1[GUN_B].Allfail[10] = 1;	//输出过呀告警
//	}
//	else if(YX1_Info[GUN_B].state3.State.UPCUR_Warn)
//	{
//		APSendB1[GUN_B].Allfail[11] = 1;	//输出过流告警
//	}
//	else
//	{
//		NOP();
//	}
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		APSendB1[GUN_B].dc_charge_voltage = PowerModuleInfo[GUN_B].OutputInfo.Vol;			//电压
		APSendB1[GUN_B].dc_charge_current  =  PowerModuleInfo[GUN_B].OutputInfo.Cur*10;	//电流  精确到小数点后二位
		APSendB1[GUN_B].OutDCSWState = 0x01; //输出继电器状态 
		APSendB1[GUN_B].batteryLinkState = 0x01; //电池连接状态
		if(BMS_BSM_Context[GUN_B].MaxbatteryTemprature > 50)
		{
			APSendB1[GUN_B].batteryMaxVol =(BMS_BSM_Context[GUN_B].MaxbatteryTemprature - 50 )  * 1000;	//单体电池最高电压  0.001  精确到小数点后三位
			
		}
		if(BMS_BSM_Context[GUN_B].MinbatteryTemprature > 50)
		{
			APSendB1[GUN_B].batteryMinVol = (BMS_BSM_Context[GUN_B].MinbatteryTemprature - 50) * 1000;	//单体电池最低电压 	0.001  精确到小数点后三位
		}
		APSendB1[GUN_B].ChargeKwh = puserinfo->TotalPower4/100;   //小数点三位
		APSendB1[GUN_B].ChargeJKwh = puserinfo->JFPGPower[0]/100;
		APSendB1[GUN_B].ChargeFKwh = puserinfo->JFPGPower[1]/100;
		APSendB1[GUN_B].ChargePKwh = puserinfo->JFPGPower[2]/100;
		APSendB1[GUN_B].ChargeGKwh = puserinfo->JFPGPower[3]/100;
		APSendB1[GUN_B].soc = (INT16U)BMS_BCS_Context[GUN_B].SOC;
		APSendB1[GUN_B].ChargeTime = puserinfo->ChargeTime;		//累计充电时间	单位：min
		memcpy(APSendB1[GUN_B].CarVin,BMS_BRM_Context[GUN_B].VIN,17);
		APSendB1[GUN_B].ChargePowerMoney = (puserinfo->TotalBill - puserinfo->TotalServeBill)/100;
		APSendB1[GUN_B].ChargeFWMoney = puserinfo->TotalServeBill/100;
		APSendB1[GUN_B].charge_full_time_left =  BMS_BCS_Context[GUN_B].RemainderTime;
		//	uint8_t OrderNum;					//本次充电的订单号，非充电中时清空		
		if(APStartType[GUN_B] == _4G_APP_START)
		{

			memcpy(APSendB1[GUN_B].OrderNum,APRecvStartCmd[GUN_B].DrderNum,sizeof(APSendB1[GUN_B].OrderNum));
		}
		else
		{
			memcpy(APSendB1[GUN_B].OrderNum,APRecvCardVinStart[GUN_B].DrderNum,sizeof(APRecvCardVinStart[GUN_B].DrderNum));
		}
	}
	memcpy(&Sendbuf[11],(INT8U*)&APSendB1[GUN_B],sizeof(AP_send_B1));

	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_send_B1) +11); //发送数据
}


/*****************************************************************************
* Function     : AP_SendCarInfo
* Description  : 充电过程中上报频率30秒
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendCarInfoA(void)
{
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	memset(APSendB37[GUN_A].devnum,0,sizeof(AP_SEND_B37));
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)	//没有注册成功
	{
		return  FALSE;
	}
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING)
	{
		return TRUE;		//不在充电中不发送
	}
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x6C;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x1E;
	//设备编号
	APSendB37[GUN_A].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendB37[GUN_A].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendB37[GUN_A].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendB37[GUN_A].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendB37[GUN_A].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendB37[GUN_A].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendB37[GUN_A].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendB37[GUN_A].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendB37[GUN_A].gun_index = 0x00;
	memcpy(APSendB37[GUN_A].bmsversion,BMS_BRM_Context[GUN_A].BMSVersion,3);
	APSendB37[GUN_A].battype = BMS_BRM_Context[GUN_A].BatteryType;
	APSendB37[GUN_A].batedcapacity = BMS_BRM_Context[GUN_A].RatedVol;
	APSendB37[GUN_A].ratedvol = BMS_BRM_Context[GUN_A].RatedVol;
	memset(APSendB37[GUN_A].batterinfo,0xff,19);//电池信息，具体为细分详见协议  都填写 0xff
	APSendB37[GUN_A].unitbatterymaxvol = BMS_BCP_Context[GUN_A].UnitBatteryMaxVol;
	APSendB37[GUN_A].bmsMaxcurrent = BMS_BCP_Context[GUN_A].MaxCurrent;
	APSendB37[GUN_A].batterywholeenergy = BMS_BCP_Context[GUN_A].BatteryWholeEnergy;
	APSendB37[GUN_A].chgmaxoutvol = BMS_BCP_Context[GUN_A].MaxVoltage;
	APSendB37[GUN_A].MaxTemprature = BMS_BCP_Context[GUN_A].MaxTemprature;
	APSendB37[GUN_A].batterysoc =BMS_BCP_Context[GUN_A].BatterySOC;
	APSendB37[GUN_A].chargevolmeasureval = BMS_BCS_Context[GUN_A].ChargeVolMeasureVal;
	APSendB37[GUN_A].bmsneedvolt = BMS_BCL_Context[GUN_A].DemandVol;
	APSendB37[GUN_A].bmsneedcurr = BMS_BCL_Context[GUN_A].DemandCur;
	APSendB37[GUN_A].chargemode = 0x02;    //充电模式 0x01恒压充电, 0x02恒流充电
	APSendB37[GUN_A].outvolt = PowerModuleInfo[GUN_A].OutputInfo.Vol;		//充电电压测量值    0.1V/位
	APSendB37[GUN_A].outcurr = 4000 -PowerModuleInfo[GUN_A].OutputInfo.Cur;		//充电电流测量值   0.1A/位，-400A偏移量
	
	APSendB37[GUN_A].MaxVolGroup = BMS_BCS_Context[GUN_A].MaxUnitVolandNum;//最高单体动力蓄电池电压及其组号 1-12位电池电压 0.01V/位，数据范围0-24V, 13-16位 组号,1/位数据范围 0-15
	APSendB37[GUN_A].remaindertime = BMS_BCS_Context[GUN_A].RemainderTime;		//估算剩余充电时间 0-600分钟，超过600按600发送
	APSendB37[GUN_A].MaxVolNum = BMS_BSM_Context[GUN_A].MaxUnitVolandNum;		//最高单体动力蓄电池电压所在编号   数据范围1-256
	APSendB37[GUN_A].batpackmaxtemp = BMS_BSM_Context[GUN_A].MaxbatteryTemprature;	//最高单体动力蓄电池电压所在编号 50度偏移 数据范围 -50-+200
	APSendB37[GUN_A].batpackmaxnum  = BMS_BSM_Context[GUN_A].MaxTempMeasurepnum;			//最高温度监测点编号
	APSendB37[GUN_A].batpackmintemp = BMS_BSM_Context[GUN_A].MinbatteryTemprature;//最低动力储电池温度
	APSendB37[GUN_A].batpackminnum = BMS_BSM_Context[GUN_A].MinTempMeasurepnum;	//最低动力蓄电池温度检测点编号
	APSendB37[GUN_A].ifcharge = 0x01;//充电允许    00禁止,01允许
	APSendB37[GUN_A].DCouttemp1 = 6000;
	APSendB37[GUN_A].DCouttemp2 = 6000;
	APSendB37[GUN_A].Moduletemp3 = 5000;	//充电模块温度 精确到小数点后二位
	if(BMS_BSM_Context[GUN_A].MaxbatteryTemprature > 50)
	{
		APSendB37[GUN_A].batmaxvolt =(BMS_BSM_Context[GUN_A].MaxbatteryTemprature - 50 )  * 1000;	//单体电池最高电压  0.001  精确到小数点后三位
		
	}
	if(BMS_BSM_Context[GUN_A].MinbatteryTemprature > 50)
	{
		APSendB37[GUN_A].batminvolt = (BMS_BSM_Context[GUN_A].MinbatteryTemprature - 50) * 1000;	//单体电池最低电压 	0.001  精确到小数点后三位
	}
	memcpy(&Sendbuf[15],(INT8U*)&APSendB37[GUN_A],sizeof(AP_SEND_B37));

	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_SEND_B37) +15); //发送数据
}

/*****************************************************************************
* Function     : AP_SendCarInfo
* Description  : 充电过程中上报频率30秒
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendCarInfoB(void)
{
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	memset(APSendB37[GUN_B].devnum,0,sizeof(AP_SEND_B37));
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)	//没有注册成功
	{
		return  FALSE;
	}
	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)
	{
		return TRUE;		//不在充电中不发送
	}
	if(APP_GetGunNum() == 1)
	{
		return FALSE;
	}
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x6C;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x1E;
	//设备编号
	APSendB37[GUN_B].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendB37[GUN_B].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendB37[GUN_B].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendB37[GUN_B].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendB37[GUN_B].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendB37[GUN_B].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendB37[GUN_B].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendB37[GUN_B].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendB37[GUN_B].gun_index = GUN_B;
	memcpy(APSendB37[GUN_B].bmsversion,BMS_BRM_Context[GUN_B].BMSVersion,3);
	APSendB37[GUN_B].battype = BMS_BRM_Context[GUN_B].BatteryType;
	APSendB37[GUN_B].batedcapacity = BMS_BRM_Context[GUN_B].RatedVol;
	APSendB37[GUN_B].ratedvol = BMS_BRM_Context[GUN_B].RatedVol;
	memset(APSendB37[GUN_B].batterinfo,0xff,19);//电池信息，具体为细分详见协议  都填写 0xff
	APSendB37[GUN_B].unitbatterymaxvol = BMS_BCP_Context[GUN_B].UnitBatteryMaxVol;
	APSendB37[GUN_B].bmsMaxcurrent = BMS_BCP_Context[GUN_B].MaxCurrent;
	APSendB37[GUN_B].batterywholeenergy = BMS_BCP_Context[GUN_B].BatteryWholeEnergy;
	APSendB37[GUN_B].chgmaxoutvol = BMS_BCP_Context[GUN_B].MaxVoltage;
	APSendB37[GUN_B].MaxTemprature = BMS_BCP_Context[GUN_B].MaxTemprature;
	APSendB37[GUN_B].batterysoc =BMS_BCP_Context[GUN_B].BatterySOC;
	APSendB37[GUN_B].chargevolmeasureval = BMS_BCS_Context[GUN_B].ChargeVolMeasureVal;
	APSendB37[GUN_B].bmsneedvolt = BMS_BCL_Context[GUN_B].DemandVol;
	APSendB37[GUN_B].bmsneedcurr = BMS_BCL_Context[GUN_B].DemandCur;
	APSendB37[GUN_B].chargemode = 0x02;    //充电模式 0x01恒压充电, 0x02恒流充电
	APSendB37[GUN_B].outvolt = PowerModuleInfo[GUN_B].OutputInfo.Vol;		//充电电压测量值    0.1V/位
	APSendB37[GUN_B].outcurr = 4000 -PowerModuleInfo[GUN_B].OutputInfo.Cur;		//充电电流测量值   0.1A/位，-400A偏移量
	
	APSendB37[GUN_B].MaxVolGroup = BMS_BCS_Context[GUN_B].MaxUnitVolandNum;//最高单体动力蓄电池电压及其组号 1-12位电池电压 0.01V/位，数据范围0-24V, 13-16位 组号,1/位数据范围 0-15
	APSendB37[GUN_B].remaindertime = BMS_BCS_Context[GUN_B].RemainderTime;		//估算剩余充电时间 0-600分钟，超过600按600发送
	APSendB37[GUN_B].MaxVolNum = BMS_BSM_Context[GUN_B].MaxUnitVolandNum;		//最高单体动力蓄电池电压所在编号   数据范围1-256
	APSendB37[GUN_B].batpackmaxtemp = BMS_BSM_Context[GUN_B].MaxbatteryTemprature;	//最高单体动力蓄电池电压所在编号 50度偏移 数据范围 -50-+200
	APSendB37[GUN_B].batpackmaxnum  = BMS_BSM_Context[GUN_B].MaxTempMeasurepnum;			//最高温度监测点编号
	APSendB37[GUN_B].batpackmintemp = BMS_BSM_Context[GUN_B].MinbatteryTemprature;//最低动力储电池温度
	APSendB37[GUN_B].batpackminnum = BMS_BSM_Context[GUN_B].MinTempMeasurepnum;	//最低动力蓄电池温度检测点编号
	APSendB37[GUN_B].ifcharge = 0x01;//充电允许    00禁止,01允许
	APSendB37[GUN_B].DCouttemp1 = 6000;
	APSendB37[GUN_B].DCouttemp2 = 6000;
	APSendB37[GUN_B].Moduletemp3 = 5000;	//充电模块温度 精确到小数点后二位
	if(BMS_BSM_Context[GUN_B].MaxbatteryTemprature > 50)
	{
		APSendB37[GUN_B].batmaxvolt =(BMS_BSM_Context[GUN_B].MaxbatteryTemprature - 50 )  * 1000;	//单体电池最高电压  0.001  精确到小数点后三位
		
	}
	if(BMS_BSM_Context[GUN_B].MinbatteryTemprature > 50)
	{
		APSendB37[GUN_B].batminvolt = (BMS_BSM_Context[GUN_B].MinbatteryTemprature - 50) * 1000;	//单体电池最低电压 	0.001  精确到小数点后三位
	}
	memcpy(&Sendbuf[15],(INT8U*)&APSendB37[GUN_B],sizeof(AP_SEND_B37));

	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_SEND_B37) +15); //发送数据
}

/*****************************************************************************
* Function     : AP_SendCardInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendCardInfo(_GUN_NUM gun)
{
	INT8U Sendbuf[200] = {0};
	INT32U CardNum;
	INT8U * pdevnum = APP_GetDevNum(); 
	USERCARDINFO * puser_card_info = NULL;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	puser_card_info = GetGunCardInfo(gun); //获取卡号
	CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
				(puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
	memset(APSendB6[gun].devnum,0,sizeof(AP_send_B6));
	
	//685200000000820006000000000001
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x52;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x01;
	APSendB6[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendB6[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendB6[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendB6[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendB6[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendB6[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendB6[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendB6[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendB6[gun].gun_index = gun;
	//卡号需要反过来？ 20210623
	APSendB6[gun].CardNum[7] = HEXtoBCD(CardNum / 100000000);
	APSendB6[gun].CardNum[6] = HEXtoBCD(CardNum % 100000000 /1000000);
	APSendB6[gun].CardNum[5] = HEXtoBCD(CardNum % 1000000 /10000);
	APSendB6[gun].CardNum[4] = HEXtoBCD(CardNum % 10000 /100);
	APSendB6[gun].CardNum[3] = HEXtoBCD(CardNum % 100 /1);
	memcpy(&Sendbuf[15],(INT8U*)&APSendB6[gun],sizeof(AP_send_B6));
	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_send_B6) +15); //发送数据
}

/*****************************************************************************
* Function     : AP_SendVinInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendVinInfo(_GUN_NUM gun)
{
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	memset(APSendB6[gun].devnum,0,sizeof(AP_send_B6));
	
	//684600000000820006000000000016
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x46;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x16;
	APSendB8[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendB8[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendB8[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendB8[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendB8[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendB8[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendB8[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendB8[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendB8[gun].gun_index = gun;
	
//	#warning "临时这么写 20210722"
//	memset(APSendB8[gun].CarVin,0x30,17);
//	APSendB8[gun].CarVin[0] = 0x30;
//	APSendB8[gun].CarVin[1] = 0x31;
//	APSendB8[gun].CarVin[2] = 0x32;
//	APSendB8[gun].CarVin[3] = 0x33;

	memcpy(APSendB8[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
	memcpy(&Sendbuf[15],(INT8U*)&APSendB8[gun],sizeof(APSendB8));
	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(APSendB8) +15); //发送数据
}


/*****************************************************************************
* Function     : AP_SendCardVinStart
* Description  :刷卡Vin启动
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendCardVinStart(_GUN_NUM gun)
{
	INT8U Sendbuf[200] = {0};
	INT32U CardNum;
	INT8U * pdevnum = APP_GetDevNum(); 
	USERCARDINFO * puser_card_info = NULL;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(APSendB10[gun].devnum,0,sizeof(AP_send_B10));
	if(APStartType[gun] == _4G_APP_CARD)
	{
		puser_card_info = GetGunCardInfo(gun); //获取卡号
		CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
					(puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
	}
	if(APStartType[gun] == _4G_APP_VIN)
	{
//		#warning "临时这么写 20210722"
//		memset(APSendB10[gun].CarVin,0x30,17);
//		APSendB10[gun].CarVin[0] = 0x30;
//		APSendB10[gun].CarVin[1] = 0x31;
//		APSendB10[gun].CarVin[2] = 0x32;
//		APSendB10[gun].CarVin[3] = 0x33;
		memcpy(APSendB10[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
	}
	//68570000000082000600000000000E
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x57;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x0E;
	APSendB10[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendB10[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendB10[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendB10[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendB10[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendB10[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendB10[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendB10[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendB10[gun].gun_index = gun;
	//卡号需要反过来？ 20210623
	if(APStartType[gun] == _4G_APP_CARD)
	{
		APSendB10[gun].CardNum[7] = HEXtoBCD(CardNum / 100000000);
		APSendB10[gun].CardNum[6] = HEXtoBCD(CardNum % 100000000 /1000000);
		APSendB10[gun].CardNum[5] = HEXtoBCD(CardNum % 1000000 /10000);
		APSendB10[gun].CardNum[4] = HEXtoBCD(CardNum % 10000 /100);
		APSendB10[gun].CardNum[3] = HEXtoBCD(CardNum % 100 /1);
	}
	APSendB10[gun].ChargeType = 0x04;			//自动充满
	memcpy(&Sendbuf[15],(INT8U*)&APSendB10[gun],sizeof(AP_send_B10));
	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_send_B10) +15); //发送数据
}


/*****************************************************************************
* Function     : AP_SendStartAck
* Description  : 发送启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendStartAck(_GUN_NUM gun)
{
	
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	_BSPRTC_TIME CurRTC;
	if(gun >= GUN_MAX)
	{
		return  FALSE;
	}
	
	if(APStartType[gun] != _4G_APP_START)
	{
		return TRUE;			//卡启动不用发送启动应答
	}
	USERINFO * puserinfo  = GetChargingInfo(gun);
	
	BSP_RTCGetTime(&CurRTC);   
	memset(APSendStartAckB5[gun].devnum,0,sizeof(AP_SEND_B5));
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x31;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x85;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x15;
	APSendStartAckB5[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendStartAckB5[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendStartAckB5[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendStartAckB5[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendStartAckB5[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendStartAckB5[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendStartAckB5[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendStartAckB5[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendStartAckB5[gun].gun_index = gun;
	
	if(pdisp_conrtol->NetGunState[gun] == GUN_CHARGEING)
	{
		APSendStartAckB5[gun].ifsuccess = 0;  //成功标志 0：成功，1：失败
		APSendStartAckB5[gun].failcause[1] = 0x00;  //启动充电失败原因   0000：成功，0001：正在充电中，0002：系统故障， 0003：其他原因
	}
	else
	{
		APSendStartAckB5[gun].ifsuccess = 1;  //成功标志 0：成功，1：失败
		APSendStartAckB5[gun].failcause[1] = 0x03;  //启动充电失败原因   0000：成功，0001：正在充电中，0002：系统故障， 0003：其他原因
	}
	APSendStartAckB5[gun].Cmd = 0x01;   //控制命令    0：停止充电，1：启动充电，2：定时充电启动，3：预约充电启动
	
	APSendStartAckB5[gun].CmdTime.Second = BCDtoHEX(CurRTC.Second) * 1000;
	APSendStartAckB5[gun].CmdTime.Day = BCDtoHEX(CurRTC.Day);
	APSendStartAckB5[gun].CmdTime.Hour = BCDtoHEX(CurRTC.Hour);
	APSendStartAckB5[gun].CmdTime.Minute = BCDtoHEX(CurRTC.Minute);
	APSendStartAckB5[gun].CmdTime.Month = BCDtoHEX(CurRTC.Month);
	APSendStartAckB5[gun].CmdTime.Year = BCDtoHEX(CurRTC.Year);
	memcpy(APSendStartAckB5[gun].DrderNum,APRecvStartCmd[gun].DrderNum,16);
	
	memcpy(&Sendbuf[15],(INT8U*)&APSendStartAckB5[gun],sizeof(AP_SEND_B5));
	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_SEND_B5) +15); //发送数据
}


/*****************************************************************************
* Function     : AP_SendStopAck
* Description  : 发送停止应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendStopAck(_GUN_NUM gun)
{
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	_BSPRTC_TIME CurRTC;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	USERINFO * puserinfo  = GetChargingInfo(gun);
	
	BSP_RTCGetTime(&CurRTC);   
	memset(APSendStopAckB5[gun].devnum,0,sizeof(AP_SEND_B5));
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0x31;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x85;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x15;
	APSendStopAckB5[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendStopAckB5[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendStopAckB5[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendStopAckB5[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendStopAckB5[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendStopAckB5[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendStopAckB5[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendStopAckB5[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendStopAckB5[gun].gun_index = gun;

	APSendStopAckB5[gun].ifsuccess = 0;  //成功标志 0：成功，1：失败
	APSendStopAckB5[gun].failcause[1] = 0x00;  //启动充电失败原因   0000：成功，0001：正在充电中，0002：系统故障， 0003：其他原因
	APSendStopAckB5[gun].Cmd = 0x00;   //控制命令    0：停止充电，1：启动充电，2：定时充电启动，3：预约充电启动
	APSendStopAckB5[gun].CmdTime.Second = BCDtoHEX(CurRTC.Second) * 1000;
	APSendStopAckB5[gun].CmdTime.Day = BCDtoHEX(CurRTC.Day);
	APSendStopAckB5[gun].CmdTime.Hour = BCDtoHEX(CurRTC.Hour);
	APSendStopAckB5[gun].CmdTime.Minute = BCDtoHEX(CurRTC.Minute);
	APSendStopAckB5[gun].CmdTime.Month = BCDtoHEX(CurRTC.Month);
	APSendStopAckB5[gun].CmdTime.Year = BCDtoHEX(CurRTC.Year);
	memcpy(APSendStopAckB5[gun].DrderNum,APRecvStartCmd[gun].DrderNum,16);
	
	memcpy(&Sendbuf[15],(INT8U*)&APSendStopAckB5[gun],sizeof(AP_SEND_B5));
	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_SEND_B5) +15); //发送数据
}

/*****************************************************************************
* Function     : AP_SendOnlineBill
* Description  : 发送在线交易记录
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendBill(_GUN_NUM gun)
{
	INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(gun);
	INT8U errcode;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	memset(APSendBilB12[gun].devnum,0,sizeof(AP_SEND_B12));
	
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0xB1;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x02;
	APSendBilB12[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendBilB12[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendBilB12[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendBilB12[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendBilB12[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendBilB12[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendBilB12[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendBilB12[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendBilB12[gun].gun_index = gun;
	if(APStartType[gun] == _4G_APP_START)
	{
		//memcpy(APSendBilB12[gun].CardNum,APRecvStartCmd[gun].UserNum,sizeof(APRecvStartCmd[gun].UserNum));
		memcpy(APSendBilB12[gun].DrderNum,APRecvStartCmd[gun].DrderNum,sizeof(APRecvStartCmd[gun].UserNum));
		//memcpy(APSendBilB12[gun].CardVin,CcuStartOverContext[GUN_A].vin,17);
	}
	else if(APStartType[gun] == _4G_APP_CARD)
	{
		//卡启动
		memcpy(APSendBilB12[gun].CardNum,APRecvCardCmd[gun].CardNum,sizeof(APRecvCardCmd[gun].CardNum));
		memcpy(APSendBilB12[gun].DrderNum,APRecvCardVinStart[gun].DrderNum,sizeof(APRecvCardVinStart[gun].DrderNum));
		//memcpy(APSendBilB12[gun].CardVin,CcuStartOverContext[GUN_A].vin,17);
	}
	else
	{
		//memcpy(APSendBilB12[gun].CardNum,APRecvCardCmd[gun].CardNum,sizeof(APRecvCardCmd[gun].CardNum));
		memcpy(APSendBilB12[gun].DrderNum,APRecvCardVinStart[gun].DrderNum,sizeof(APRecvCardVinStart[gun].DrderNum));
		memcpy(APSendBilB12[gun].CardVin,BMS_BRM_Context[gun].VIN,17);
//		memset(APSendBilB12[gun].CardVin,0x30,17);
//		APSendBilB12[gun].CardVin[0] = 0x30;
//		APSendBilB12[gun].CardVin[1] = 0x31;
//		APSendBilB12[gun].CardVin[2] = 0x32;
//		APSendBilB12[gun].CardVin[3] = 0x33;
	}
	errcode = APP_GetStopChargeReason(gun);   //获取故障码
	if(errcode & 0x80) //启动失败
	{
		APSendBilB12[gun].StopReason = 6;		//启动失败都为其他原因
		APSendBilB12[gun].StartTime.Second = BCDtoHEX(puserinfo->StartTime.Second) * 1000;
		APSendBilB12[gun].StartTime.Day = BCDtoHEX(puserinfo->StartTime.Day);
		APSendBilB12[gun].StartTime.Hour = BCDtoHEX(puserinfo->StartTime.Hour);
		APSendBilB12[gun].StartTime.Minute = BCDtoHEX(puserinfo->StartTime.Minute);
		APSendBilB12[gun].StartTime.Month = BCDtoHEX(puserinfo->StartTime.Month);
		APSendBilB12[gun].StartTime.Year = BCDtoHEX(puserinfo->StartTime.Year);
		
		APSendBilB12[gun].StopTime.Second = 0;
		APSendBilB12[gun].StopTime.Day = BCDtoHEX(puserinfo->EndTime.Day);
		APSendBilB12[gun].StopTime.Hour = BCDtoHEX(puserinfo->EndTime.Hour);
		APSendBilB12[gun].StopTime.Minute = BCDtoHEX(puserinfo->EndTime.Minute);
		APSendBilB12[gun].StopTime.Month = BCDtoHEX(puserinfo->EndTime.Month);
		APSendBilB12[gun].StopTime.Year = BCDtoHEX(puserinfo->EndTime.Year);
		APSendBilB12[gun].ChargeTime = 0;
		APSendBilB12[gun].StartSoc = BMS_BCP_Context[gun].BatterySOC/10;
		APSendBilB12[gun].StopSoc =  BMS_BCS_Context[gun].SOC/10;
		APSendBilB12[gun].ChargeStartKwh = pmeter->StartPower4/100; /*充电前电表读数*/
		APSendBilB12[gun].ChargeStopKwh = pmeter->CurPower4/100;
	}
	else
	{
		APSendBilB12[gun].StartTime.Second = BCDtoHEX(puserinfo->StartTime.Second) * 1000;
		APSendBilB12[gun].StartTime.Day = BCDtoHEX(puserinfo->StartTime.Day);
		APSendBilB12[gun].StartTime.Hour = BCDtoHEX(puserinfo->StartTime.Hour);
		APSendBilB12[gun].StartTime.Minute = BCDtoHEX(puserinfo->StartTime.Minute);
		APSendBilB12[gun].StartTime.Month = BCDtoHEX(puserinfo->StartTime.Month);
		APSendBilB12[gun].StartTime.Year = BCDtoHEX(puserinfo->StartTime.Year);
		
		APSendBilB12[gun].StopTime.Second = BCDtoHEX(puserinfo->EndTime.Second) * 1000;
		APSendBilB12[gun].StopTime.Day = BCDtoHEX(puserinfo->EndTime.Day);
		APSendBilB12[gun].StopTime.Hour = BCDtoHEX(puserinfo->EndTime.Hour);
		APSendBilB12[gun].StopTime.Minute = BCDtoHEX(puserinfo->EndTime.Minute);
		APSendBilB12[gun].StopTime.Month = BCDtoHEX(puserinfo->EndTime.Month);
		APSendBilB12[gun].StopTime.Year = BCDtoHEX(puserinfo->EndTime.Year);
		APSendBilB12[gun].ChargeTime = puserinfo->ChargeTime;
		APSendBilB12[gun].ChargeAllKwh = puserinfo->TotalPower4/100;   //小数点2位
		APSendBilB12[gun].ChargeJKwh = puserinfo->JFPGPower[0]/100; 
		APSendBilB12[gun].ChargeFKwh = puserinfo->JFPGPower[1]/100;
		APSendBilB12[gun].ChargePKwh = puserinfo->JFPGPower[2]/100;
		APSendBilB12[gun].ChargeGKwh = puserinfo->JFPGPower[3]/100;
		APSendBilB12[gun].ChargeStartKwh = pmeter->StartPower4/100; /*充电前电表读数*/
		APSendBilB12[gun].ChargeStopKwh = pmeter->CurPower4/100;
		APSendBilB12[gun].StartSoc = BMS_BCP_Context[gun].BatterySOC/10;
		APSendBilB12[gun].StopSoc = BMS_BCS_Context[gun].SOC/10;
		if(STOP_BSMNORMAL == errcode)
		{
			APSendBilB12[gun].StopReason  = 1; //充满停止
		}
		else if(STOP_ERR_NONE == errcode)
		{
			APSendBilB12[gun].StopReason  = 3; 	//主动停止
		}
		else if(STOP_EMERGENCY == errcode)
		{
			APSendBilB12[gun].StopReason  = 4;		//急停
		}
		else
		{
			APSendBilB12[gun].StopReason  = 6;		//其他原因
		}
		APSendBilB12[gun].ServeFee = puserinfo->TotalServeBill/100;
		APSendBilB12[gun].ChargeFee = (puserinfo->TotalBill - puserinfo->TotalServeBill)/100;
	}

	memcpy(&Sendbuf[15],(INT8U*)&APSendBilB12[gun],sizeof(AP_SEND_B12));
	
	WriterFmBill(gun,1);
	ResendBillControl[gun].CurTime = OSTimeGet();	
	ResendBillControl[gun].LastTime = ResendBillControl[gun].CurTime;
	return ModuleSIM7600_SendData(0, Sendbuf,sizeof(AP_SEND_B12) +15); //发送数据

}


/*****************************************************************************
* Function     : PreAPBill
* Description  : 保存安培快充订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   PreAPBill(_GUN_NUM gun,INT8U *pdata)
{
		INT8U Sendbuf[200] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(gun);
	INT8U errcode;
	_BSPRTC_TIME endtime;
	GetCurTime(&endtime); 
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	memset(APSendBilB12[gun].devnum,0,sizeof(AP_SEND_B12));
	
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0xB1;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x02;
	APSendBilB12[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendBilB12[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendBilB12[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendBilB12[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendBilB12[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendBilB12[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendBilB12[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendBilB12[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendBilB12[gun].gun_index = gun;
	if(APStartType[gun] == _4G_APP_START)
	{
		//memcpy(APSendBilB12[gun].CardNum,APRecvStartCmd[gun].UserNum,sizeof(APRecvStartCmd[gun].UserNum));
		memcpy(APSendBilB12[gun].DrderNum,APRecvStartCmd[gun].DrderNum,sizeof(APRecvStartCmd[gun].UserNum));
		//memcpy(APSendBilB12[gun].CardVin,CcuStartOverContext[GUN_A].vin,17);
	}
	else if(APStartType[gun] == _4G_APP_CARD)
	{
		//卡启动
		memcpy(APSendBilB12[gun].CardNum,APRecvCardCmd[gun].CardNum,sizeof(APRecvCardCmd[gun].CardNum));
		memcpy(APSendBilB12[gun].DrderNum,APRecvCardVinStart[gun].DrderNum,sizeof(APRecvCardVinStart[gun].DrderNum));
		//memcpy(APSendBilB12[gun].CardVin,CcuStartOverContext[GUN_A].vin,17);
	}
	else
	{
		//memcpy(APSendBilB12[gun].CardNum,APRecvCardCmd[gun].CardNum,sizeof(APRecvCardCmd[gun].CardNum));
		memcpy(APSendBilB12[gun].DrderNum,APRecvCardVinStart[gun].DrderNum,sizeof(APRecvCardVinStart[gun].DrderNum));
		memcpy(APSendBilB12[gun].CardVin,BMS_BRM_Context[gun].VIN,17);
//		memset(APSendBilB12[gun].CardVin,0x30,17);
//		APSendBilB12[gun].CardVin[0] = 0x30;
//		APSendBilB12[gun].CardVin[1] = 0x31;
//		APSendBilB12[gun].CardVin[2] = 0x32;
//		APSendBilB12[gun].CardVin[3] = 0x33;
	}
	errcode = APP_GetStopChargeReason(gun);   //获取故障码
	if(errcode & 0x80) //启动失败
	{
		APSendBilB12[gun].StopReason = 6;		//启动失败都为其他原因
		APSendBilB12[gun].StartTime.Second = BCDtoHEX(puserinfo->StartTime.Second) * 1000;
		APSendBilB12[gun].StartTime.Day = BCDtoHEX(puserinfo->StartTime.Day);
		APSendBilB12[gun].StartTime.Hour = BCDtoHEX(puserinfo->StartTime.Hour);
		APSendBilB12[gun].StartTime.Minute = BCDtoHEX(puserinfo->StartTime.Minute);
		APSendBilB12[gun].StartTime.Month = BCDtoHEX(puserinfo->StartTime.Month);
		APSendBilB12[gun].StartTime.Year = BCDtoHEX(puserinfo->StartTime.Year);
		
		APSendBilB12[gun].StopTime.Second = 0;
		APSendBilB12[gun].StopTime.Day = BCDtoHEX(puserinfo->EndTime.Day);
		APSendBilB12[gun].StopTime.Hour = BCDtoHEX(puserinfo->EndTime.Hour);
		APSendBilB12[gun].StopTime.Minute = BCDtoHEX(puserinfo->EndTime.Minute);
		APSendBilB12[gun].StopTime.Month = BCDtoHEX(puserinfo->EndTime.Month);
		APSendBilB12[gun].StopTime.Year = BCDtoHEX(puserinfo->EndTime.Year);
		APSendBilB12[gun].ChargeTime = 0;
		APSendBilB12[gun].StartSoc = BMS_BCP_Context[gun].BatterySOC/10;
		APSendBilB12[gun].StopSoc = BMS_BCS_Context[gun].SOC/10;
		APSendBilB12[gun].ChargeStartKwh = pmeter->StartPower4/100; /*充电前电表读数*/
		APSendBilB12[gun].ChargeStopKwh = pmeter->CurPower4/100;
	}
	else
	{
		APSendBilB12[gun].StartTime.Second = BCDtoHEX(puserinfo->StartTime.Second) * 1000;
		APSendBilB12[gun].StartTime.Day = BCDtoHEX(puserinfo->StartTime.Day);
		APSendBilB12[gun].StartTime.Hour = BCDtoHEX(puserinfo->StartTime.Hour);
		APSendBilB12[gun].StartTime.Minute = BCDtoHEX(puserinfo->StartTime.Minute);
		APSendBilB12[gun].StartTime.Month = BCDtoHEX(puserinfo->StartTime.Month);
		APSendBilB12[gun].StartTime.Year = BCDtoHEX(puserinfo->StartTime.Year);
		
		APSendBilB12[gun].StopTime.Second = BCDtoHEX(endtime.Second) * 1000;
		APSendBilB12[gun].StopTime.Day = BCDtoHEX(endtime.Day);
		APSendBilB12[gun].StopTime.Hour = BCDtoHEX(endtime.Hour);
		APSendBilB12[gun].StopTime.Minute = BCDtoHEX(endtime.Minute);
		APSendBilB12[gun].StopTime.Month = BCDtoHEX(endtime.Month);
		APSendBilB12[gun].StopTime.Year = BCDtoHEX(endtime.Year);
		APSendBilB12[gun].ChargeTime = puserinfo->ChargeTime;
		APSendBilB12[gun].ChargeAllKwh = puserinfo->TotalPower4/100;   //小数点2位
		APSendBilB12[gun].ChargeJKwh = puserinfo->JFPGPower[0]/100;
		APSendBilB12[gun].ChargeFKwh = puserinfo->JFPGPower[1]/100;
		APSendBilB12[gun].ChargePKwh = puserinfo->JFPGPower[2]/100;
		APSendBilB12[gun].ChargeGKwh = puserinfo->JFPGPower[3]/100;
		APSendBilB12[gun].ChargeStartKwh = pmeter->StartPower4/100; /*充电前电表读数*/
		APSendBilB12[gun].ChargeStopKwh = pmeter->CurPower4/100;
		APSendBilB12[gun].StartSoc = BMS_BCP_Context[gun].BatterySOC/10;
		APSendBilB12[gun].StopSoc = BMS_BCS_Context[gun].SOC/10;
		if(STOP_BSMNORMAL == errcode)
		{
			APSendBilB12[gun].StopReason  = 1; //充满停止
		}
		else if(STOP_ERR_NONE == errcode)
		{
			APSendBilB12[gun].StopReason  = 3; 	//主动停止
		}
		else if(STOP_EMERGENCY == errcode)
		{
			APSendBilB12[gun].StopReason  = 4;		//急停
		}
		else
		{
			APSendBilB12[gun].StopReason  = 6;		//其他原因
		}
		APSendBilB12[gun].ServeFee = puserinfo->TotalServeBill/100;
		APSendBilB12[gun].ChargeFee = (puserinfo->TotalBill - puserinfo->TotalServeBill)/100;
	}
	memcpy(&Sendbuf[15],(INT8U*)&APSendBilB12[gun],sizeof(AP_SEND_B12));
	memcpy(pdata,Sendbuf,sizeof(AP_SEND_B12) + 15);
	return TRUE;
}


/*****************************************************************************
* Function     : PreAPOffLineBill
* Description  : 保存离线交易记录
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  PreAPOffLineBill(_GUN_NUM gun,INT8U *pdata)
{
	INT8U Sendbuf[300] = {0};
	INT8U i = 0;
	INT32U CardNum;
	USERCARDINFO * puser_card_info = NULL;
	INT8U * pdevnum = APP_GetDevNum(); 
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(gun);
	_PRICE_SET* ppric =  APP_GetPriceInfo();
	INT8U errcode;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	memset(APSendBilB15[gun].devnum,0,sizeof(AP_SEND_B15));
	puser_card_info = GetGunCardInfo(gun); //获取卡号
	CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
				(puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 0xD3;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x03;
	APSendBilB15[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendBilB15[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendBilB15[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendBilB15[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendBilB15[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendBilB15[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendBilB15[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendBilB15[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendBilB15[gun].gun_index = gun;
	if(APStartType[gun] == _4G_APP_CARD)
	{
		//卡启动
		APSendBilB15[gun].CardNum[7] = HEXtoBCD(CardNum / 100000000);
		APSendBilB15[gun].CardNum[6] = HEXtoBCD(CardNum % 100000000 /1000000);
		APSendBilB15[gun].CardNum[5] = HEXtoBCD(CardNum % 1000000 /10000);
		APSendBilB15[gun].CardNum[4] = HEXtoBCD(CardNum % 10000 /100);
		APSendBilB15[gun].CardNum[3] = HEXtoBCD(CardNum % 100 /1);
		
		//memcpy(APSendBilB12[gun].CardVin,CcuStartOverContext[GUN_A].vin,17);
	}
	else
	{
		//memcpy(APSendBilB12[gun].CardNum,APRecvCardCmd[gun].CardNum,sizeof(APRecvCardCmd[gun].CardNum));
		memcpy(APSendBilB15[gun].CardVin,BMS_BRM_Context[gun].VIN,17);
	}
	//生成一个唯一订单号
	
	//memcpy(APSendBilB15[gun].DrderNum,APRecvCardVinStart[gun].DrderNum,sizeof(APRecvCardVinStart[gun].DrderNum));
	APSendBilB15[gun].DrderNum[9] = 0x20;
	APSendBilB15[gun].DrderNum[8] = puserinfo->StartTime.Year;
	APSendBilB15[gun].DrderNum[7] = puserinfo->StartTime.Month;
	APSendBilB15[gun].DrderNum[6] = puserinfo->StartTime.Day;
	APSendBilB15[gun].DrderNum[5] = puserinfo->StartTime.Hour;
	APSendBilB15[gun].DrderNum[4] = puserinfo->StartTime.Minute;
	APSendBilB15[gun].DrderNum[3] = puserinfo->StartTime.Second;
	APSendBilB15[gun].DrderNum[2] = puserinfo->StartTime.Second;
	APSendBilB15[gun].DrderNum[1] = puserinfo->StartTime.Second;
	APSendBilB15[gun].DrderNum[0] = puserinfo->StartTime.Second;
	errcode = APP_GetStopChargeReason(gun);   //获取故障码
	if(errcode & 0x80) //启动失败
	{
		APSendBilB15[gun].StopReason = 5;		//启动失败都为其他原因
	}
	else
	{
		APSendBilB15[gun].StartTime.Second = BCDtoHEX(puserinfo->StartTime.Second) * 1000;
		APSendBilB15[gun].StartTime.Day = BCDtoHEX(puserinfo->StartTime.Day);
		APSendBilB15[gun].StartTime.Hour = BCDtoHEX(puserinfo->StartTime.Hour);
		APSendBilB15[gun].StartTime.Minute = BCDtoHEX(puserinfo->StartTime.Minute);
		APSendBilB15[gun].StartTime.Month = BCDtoHEX(puserinfo->StartTime.Month);
		APSendBilB15[gun].StartTime.Year = BCDtoHEX(puserinfo->StartTime.Year);
		
		APSendBilB15[gun].StopTime.Second = BCDtoHEX(puserinfo->EndTime.Second) * 1000;
		APSendBilB15[gun].StopTime.Day = BCDtoHEX(puserinfo->EndTime.Day);
		APSendBilB15[gun].StopTime.Hour = BCDtoHEX(puserinfo->EndTime.Hour);
		APSendBilB15[gun].StopTime.Minute = BCDtoHEX(puserinfo->EndTime.Minute);
		APSendBilB15[gun].StopTime.Month = BCDtoHEX(puserinfo->EndTime.Month);
		APSendBilB15[gun].StopTime.Year = BCDtoHEX(puserinfo->EndTime.Year);
		APSendBilB15[gun].ChargeTime = puserinfo->ChargeTime;
		APSendBilB15[gun].ChargeAllKwh = puserinfo->TotalPower4/100;   //小数点2位
		APSendBilB15[gun].ChargeStartKwh = pmeter->StartPower4/100; /*充电前电表读数*/
		APSendBilB15[gun].ChargeStopKwh = pmeter->CurPower4/100;
		APSendBilB15[gun].StartSoc = BMS_BCP_Context[gun].BatterySOC/10;
		APSendBilB15[gun].StopSoc = BMS_BCS_Context[gun].SOC/10;
		if(STOP_BSMNORMAL == errcode)
		{
			APSendBilB15[gun].StopReason  = 1; //充满停止
		}
		else if(STOP_ERR_NONE == errcode)
		{
			APSendBilB15[gun].StopReason  = 3; 	//主动停止
		}
		else if(STOP_EMERGENCY == errcode)
		{
			APSendBilB15[gun].StopReason  = 4;		//急停
		}
		else
		{
			APSendBilB15[gun].StopReason  = 6;		//其他原因
		}
	}
	
	APSendBilB15[gun].ChargeJKwh = puserinfo->JFPGPower[0]/100;
	APSendBilB15[gun].ChargeFKwh = puserinfo->JFPGPower[1]/100;
	APSendBilB15[gun].ChargePKwh = puserinfo->JFPGPower[2]/100;
	APSendBilB15[gun].ChargeGKwh = puserinfo->JFPGPower[3]/100;
	APSendBilB15[gun].JMoney = puserinfo->JFPGBill[0]/100;
	APSendBilB15[gun].FMoney = puserinfo->JFPGBill[1]/100;
	APSendBilB15[gun].PMoney = puserinfo->JFPGBill[2]/100;
	APSendBilB15[gun].GMoney = puserinfo->JFPGBill[3]/100;
	
	INT16U Price[TIME_QUANTUM_MAX];				//电价 	0.01元 	    
	_TIME_QUANTUM CurTimeQuantum[TIME_QUANTUM_MAX];		//当前处于哪个时间段  尖 峰 平 古
	
	for(i = 0;i < TIME_QUANTUM_MAX;i++)
	{
		if(ppric->CurTimeQuantum[i] == TIME_QUANTUM_J)
		{
			APSendBilB15[gun].JBill = ppric->Price[i];
			break;
		}
	}
	for(i = 0;i < TIME_QUANTUM_MAX;i++)
	{
		if(ppric->CurTimeQuantum[i] == TIME_QUANTUM_F)
		{
			APSendBilB15[gun].FBill = ppric->Price[i];
			break;
		}
	}
	for(i = 0;i < TIME_QUANTUM_MAX;i++)
	{
		if(ppric->CurTimeQuantum[i] == TIME_QUANTUM_P)
		{
			APSendBilB15[gun].PBill = ppric->Price[i];
			break;
		}
	}
	
	for(i = 0;i < TIME_QUANTUM_MAX;i++)
	{
		if(ppric->CurTimeQuantum[i] == TIME_QUANTUM_G)
		{
			APSendBilB15[gun].GBill = ppric->Price[i];
			break;
		}
	}
	
	APSendBilB15[gun].ServeFee = puserinfo->TotalServeBill/100;
	APSendBilB15[gun].ChargeFee = (puserinfo->TotalBill - puserinfo->TotalServeBill)/100;
	memcpy(&Sendbuf[15],(INT8U*)&APSendBilB15,sizeof(AP_SEND_B15));
	memcpy(pdata,Sendbuf,0xD5);
	return TRUE;

}


/*****************************************************************************
* Function     : PreAPFSOffLineBill
* Description  : 上传分时交易记录   充电完成后，在B12发送完成后上报。
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  PreAPFSOffLineBill(_GUN_NUM gun,INT8U *pdata)
{
	INT8U Sendbuf[250] = {0};
	INT8U i;
	INT8U * pdevnum = APP_GetDevNum(); 
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	_PRICE_SET* pprice = APP_GetPriceInfo();
		USERINFO * puserinfo  = GetChargingInfo(gun);
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(APSendBilB53[gun].devnum,0,sizeof(AP_SEND_B53));
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 54+sizeof(_PRIC_INFO)*pprice->TimeQuantumNum;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x28;   //应该是B55
	APSendBilB53[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendBilB53[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendBilB53[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendBilB53[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendBilB53[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendBilB53[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendBilB53[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendBilB53[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendBilB53[gun].gun_index = gun;
	memcpy(APSendBilB53[gun].DrderNum,APRecvStartCmd[gun].DrderNum,sizeof(APRecvStartCmd[gun].UserNum));
	memcpy(APSendBilB53[gun].PricTypeID,APRecvB2.PricTypeID,sizeof(APRecvB2.PricTypeID));	//计费模型ID     运营管理系统产生
	memcpy(APSendBilB53[gun].StartTime,APRecvB2.StartTime,sizeof(APRecvB2.StartTime));		//切换时间  CP56Time2a格式
	APSendBilB53[gun].Timenum= pprice->TimeQuantumNum;
	for(i = 0;i <  pprice->TimeQuantumNum;i++)
	{
		APSendBilB53[gun].PricInfo[i].CurTimeQuantum = pprice->CurTimeQuantum[i];  //当前处于哪个时间段
		
		APSendBilB53[gun].PricInfo[i].ServeFee = puserinfo->ServeBill[i]/100;
		APSendBilB53[gun].PricInfo[i].Price = (puserinfo->Bill[i] - puserinfo->ServeBill[i])/100;
		APSendBilB53[gun].PricInfo[i].Power = puserinfo->Power[i];
		
	}
	memcpy(&Sendbuf[15],(INT8U*)&APSendBilB53,sizeof(AP_SEND_B53));
	pdata[0] = Sendbuf[1]+2;	//应为是可变的，所以第一个字节为长度
	memcpy(&pdata[1],Sendbuf,Sendbuf[1]+2);
	return TRUE;
}

/*****************************************************************************
* Function     : AP_SendBillData
* Description  : 发送订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U AP_SendBillData(INT8U * pdata,INT8U len)
{
	return ModuleSIM7600_SendData(0, pdata,MIN(sizeof(AP_SEND_B12) +15,len)); //发送数据
}

/*****************************************************************************
* Function     : AP_SendOffLineBillData
* Description  : 发送离线交易记录订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U AP_SendOffLineBillData(INT8U * pdata,INT16U len)
{
	return ModuleSIM7600_SendData(0, pdata,MIN(0xD5,len)); //发送数据
}

/*****************************************************************************
* Function     : AP_SendOffLineBillData
* Description  : 发送离线交易记录订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U AP_SendOffLineBillFSData(INT8U * pdata,INT16U len)
{
	if(SendB53State == 0)
	{
		return FALSE;
	}
	//第一个字节为数据长度
	return ModuleSIM7600_SendData(0, &pdata[1],MIN(len,pdata[0])); //发送数据
}

/*****************************************************************************
* Function     : AP_SendTimeSharBill
* Description  : 上传分时交易记录   充电完成后，在B12发送完成后上报。
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendTimeSharBill(_GUN_NUM gun)
{
	INT8U Sendbuf[250] = {0};
	INT8U i;
	INT8U * pdevnum = APP_GetDevNum(); 
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	_PRICE_SET* pprice = APP_GetPriceInfo();
		USERINFO * puserinfo  = GetChargingInfo(gun);
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if(SendB53State == 0)   //是否需要发送分时记录
	{
		return FALSE;
	}
	memset(APSendBilB53[gun].devnum,0,sizeof(AP_SEND_B53));
	Sendbuf[0] = 0x68;  //固定
	Sendbuf[1] = 54+sizeof(_PRIC_INFO)*pprice->TimeQuantumNum;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x06;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x27;
	APSendBilB53[gun].devnum[7] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	APSendBilB53[gun].devnum[6] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	APSendBilB53[gun].devnum[5] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	APSendBilB53[gun].devnum[4] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	APSendBilB53[gun].devnum[3] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	APSendBilB53[gun].devnum[2] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	APSendBilB53[gun].devnum[1] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	APSendBilB53[gun].devnum[0] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	APSendBilB53[gun].gun_index = gun;
	memcpy(APSendBilB53[gun].DrderNum,APRecvStartCmd[gun].DrderNum,sizeof(APRecvStartCmd[gun].UserNum));
	memcpy(APSendBilB53[gun].PricTypeID,APRecvB2.PricTypeID,sizeof(APRecvB2.PricTypeID));	//计费模型ID     运营管理系统产生
	memcpy(APSendBilB53[gun].StartTime,APRecvB2.StartTime,sizeof(APRecvB2.StartTime));		//切换时间  CP56Time2a格式
	APSendBilB53[gun].Timenum= pprice->TimeQuantumNum;
	for(i = 0;i <  pprice->TimeQuantumNum;i++)
	{
		APSendBilB53[gun].PricInfo[i].CurTimeQuantum = pprice->CurTimeQuantum[i];  //当前处于哪个时间段
		
		APSendBilB53[gun].PricInfo[i].ServeFee = puserinfo->ServeBill[i]/100;
		APSendBilB53[gun].PricInfo[i].Price = (puserinfo->Bill[i] - puserinfo->ServeBill[i])/100;
		APSendBilB53[gun].PricInfo[i].Power = puserinfo->Power[i];
		
	}
	memcpy(&Sendbuf[15],(INT8U*)&APSendBilB53[gun],sizeof(AP_SEND_B53));
	return ModuleSIM7600_SendData(0, Sendbuf,Sendbuf[1]+2); //发送数据
}



/*****************************************************************************
* Function     : AP_SendSetTimeAck
* Description  : 设置时间应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendSetTimeAck(void)
{
	INT8U Sendbuf[21] = {0};
	Sendbuf[0] = 0x68;
	Sendbuf[1] = 0x13;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x67;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	memcpy(&Sendbuf[14],(INT8U*)&APRecvSetTime,7);
	return ModuleSIM7600_SendData(0, Sendbuf,21); //发送数据
}


/*****************************************************************************
* Function     : AP_SendCardWLAck
* Description  : 发送卡白名单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendCardWLAck(void)
{
	INT8U i = 0;
	
	INT8U Sendbuf[150] = {0};
	Sendbuf[0] = 0x68;
	Sendbuf[1] = 0x91 - (12 - APCardWL.Nnm)*sizeof(_AP_CARD_INFO);
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x14;
	
	
	memcpy(&APSendCardB18,&APCardWL,sizeof(AP_SEND_B18));
	if(APSendCardB18.Nnm > 12)
	{
		return FALSE;
	}
	for(i = 0;i < 12;i++)
	{
		APSendCardB18.CardState[i].ifSuccess = 0x01;		//都显示成功
	}
	memcpy(&Sendbuf[15],(INT8U*)&APSendCardB18,sizeof(APSendCardB18));
	return ModuleSIM7600_SendData(0, Sendbuf,0x91 - (12 - APCardWL.Nnm)*sizeof(_AP_CARD_INFO) + 2); //发送数据
}


/*****************************************************************************
* Function     : AP_SendVinWLAck
* Description  : 发送VIN白名单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendVinWLAck(void)
{
	INT8U i = 0;
	
	INT8U Sendbuf[255] = {0};
	Sendbuf[0] = 0x68;
	Sendbuf[1] = 0xFD - (12 - APVinWL.Nnm)*sizeof(_AP_VIN_INFO);
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x14;

	memcpy(&APSendVinB20,&APVinWL,sizeof(APSendVinB20));
	if(APSendVinB20.Nnm > 12)
	{
		return FALSE;
	}
	for(i = 0;i < 12;i++)
	{
		APSendVinB20.VinState[i].ifSuccess = 0x01;		//都显示成功
	}
	memcpy(&Sendbuf[15],(INT8U*)&APSendVinB20,sizeof(APSendVinB20));
	return ModuleSIM7600_SendData(0, Sendbuf,0xFD - ((12 - APVinWL.Nnm)*sizeof(_AP_VIN_INFO)) + 2); //发送数据
}


/*****************************************************************************
* Function     : AP_SendVinCardResAck
* Description  : 清空白名单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendVinCardResAck(void)
{
	//682500000000820007000000000017
	
	INT8U Sendbuf[50] = {0};
	Sendbuf[0] = 0x68;
	Sendbuf[1] = 0x25;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x17;

	memcpy(&APSendVinCardResB20,&APVinCardRes,sizeof(APSendVinCardResB20));
	
	memcpy(&Sendbuf[15],(INT8U*)&APSendVinCardResB20,sizeof(APSendVinCardResB20));
	return ModuleSIM7600_SendData(0, Sendbuf,Sendbuf[1] + 2); //发送数据
}



/*****************************************************************************
* Function     : AP_SendSetTimeAck
* Description  : 设置费率应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendSetPricB2Ack(void)
{
	INT8U Sendbuf[50] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	
	Sendbuf[0] = 0x68;
	Sendbuf[1] = 0x1E;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x06;
	
	Sendbuf[22] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	Sendbuf[21] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	Sendbuf[20] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	Sendbuf[19] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	Sendbuf[18] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	Sendbuf[17] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	Sendbuf[16] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	Sendbuf[15] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	memcpy(&Sendbuf[23],APRecvB2.PricTypeID,sizeof(APRecvB2.PricTypeID));
	Sendbuf[31] = 0x00; //0表示成功 1表示失败
	
	return ModuleSIM7600_SendData(0, Sendbuf,32); //发送数据
}

/*****************************************************************************
* Function     : AP_SendSetPricB47Ack
* Description  : 设置费率应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U  AP_SendSetPricB47Ack(void)
{
	INT8U Sendbuf[50] = {0};
	INT8U * pdevnum = APP_GetDevNum(); 
	Sendbuf[0] = 0x68;
	Sendbuf[1] = 0x1F;
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x82;
	Sendbuf[7] = 0x00;
	Sendbuf[8] = 0x07;
	Sendbuf[9] = 0x00;
	Sendbuf[10] = 0x00;
	Sendbuf[11] = 0x00;
	Sendbuf[12] = 0x00;
	Sendbuf[13] = 0x00;
	Sendbuf[14] = 0x24;
	
	Sendbuf[22] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	Sendbuf[21] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	Sendbuf[20] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	Sendbuf[19] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	Sendbuf[18] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	Sendbuf[17] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	Sendbuf[16] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	Sendbuf[15] = (pdevnum[14] - '0') *0x10 +  (pdevnum[15] - '0');
	Sendbuf[23] = APRecvB47.gun_index;
	memcpy(&Sendbuf[24],APRecvB47.PricTypeID,sizeof(APRecvB47.PricTypeID));
	Sendbuf[32] = 0x00; //0表示成功 1表示失败
	
	return ModuleSIM7600_SendData(0, Sendbuf,33); //发送数据
}
/***********************接收数据*********************************************/
/*****************************************************************************
* Function     : AP_Registee
* Description  : 注册帧返回（协议返回）
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RegisteeAck(INT8U *pdata,INT16U len)  
{
	if((pdata == NULL) || (len !=10))
	{
		return FALSE;
	}
	return APP_SetAppRegisterState(LINK_NUM,STATE_OK);

}

/*****************************************************************************
* Function     : AP_RecvPricB2
* Description  : 费率设置
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvPricB2(INT8U *pdata,INT16U len)   
{
	static _PRICE_SET price;
	INT8U i = 0,curseg,lastseg,count = 0;	
	_FLASH_OPERATION  FlashOper;
	
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if(len != (sizeof(AP_RECV_B2)) )
	{
		return FALSE;
	}
	memcpy((INT8U*)&APRecvB2,pdata,len);
	
	if((APRecvB2.FricNumber != 4) || (APRecvB2.UserState == 2) )
	{
		return FALSE;			//时间段不为4或者失效
	}
	memset(&price,0,sizeof(price));
	curseg = APRecvB2.FricNum[0];
	lastseg = curseg;
	price.StartTime[0] = 0;			//第一个开始时间肯定为0
	for(i = 1;i < 48;i++)
	{
		curseg = APRecvB2.FricNum[i];
		if(lastseg != curseg)
		{
			if((lastseg == 0) || (lastseg >4) )
			{
				return FALSE;
			}
			price.Price[count] = APRecvB2.Fric[lastseg - 1];		//电价
			price.ServeFee[count] = APRecvB2.ServiceFric*1000;				//服务费
			price.EndTime[count] = i*30;
			price.CurTimeQuantum[count] = lastseg - 1;				//当前处于哪个时间段
			lastseg = curseg;
			count++;
			if(count >= TIME_QUANTUM_MAX)
			{
				break;		//最多为12个时间段
			}
		}
		
	}
	if(count >= TIME_QUANTUM_MAX)
	{
		price.Price[TIME_QUANTUM_MAX - 1] = APRecvB2.Fric[lastseg - 1];		//电价
		price.ServeFee[TIME_QUANTUM_MAX - 1] = APRecvB2.ServiceFric*1000;				//服务费
		price.EndTime[TIME_QUANTUM_MAX - 1] = 0;			//最后一个结束肯定是0
		price.CurTimeQuantum[TIME_QUANTUM_MAX - 1] = lastseg - 1;				//当前处于哪个时间段
	}
	else
	{
		price.Price[count] = APRecvB2.Fric[lastseg - 1];		//电价
		price.ServeFee[count] = APRecvB2.ServiceFric*1000;				//服务费
		price.EndTime[count] = 0;			//最后一个结束肯定是0
		price.CurTimeQuantum[count] = lastseg - 1;				//当前处于哪个时间段
		for(i = 1;i < (count +1);i++)
		{
			price.StartTime[i] = price.EndTime[i - 1];
		}
	}
	price.TimeQuantumNum =  count + 1;
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = PARA_PRICALL_FLLEN;
	FlashOper.ptr = (INT8U *)&price;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);
	
	memcpy(&PriceSet,&price,sizeof(_PRICE_SET));
	
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DataLen = 1;
	Msg.DivNum = APP_RATE_ACK;
	OSQPost(psendevent, &Msg);
	SendB53State = 0;			//不需要发送分时计费记录
	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvPricB47
* Description  : 费率设置1
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvPricB47(INT8U *pdata,INT16U len)  
{
	static _PRICE_SET price;
	_FLASH_OPERATION  FlashOper;
	INT8U i;
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	
	memset(&price,0,sizeof(price));
	if(len < 40)
	{
		return FALSE;
	}
	memcpy((INT8U*)&APRecvB47,pdata,MIN(len,sizeof(APRecvB47)));
	if((APRecvB47.FricNumber == 0) || (APRecvB47.FricNumber > 12))
	{
		return FALSE;
	}
	for(i = 0; i < APRecvB47.FricNumber;i++)
	{
		price.Price[i] = APRecvB47.SetPricInfo[i].Fric;		//电价
		price.ServeFee[i] = APRecvB47.SetPricInfo[i].ServiceFric;				//服务费
		price.StartTime[i] =  BCDtoHEX(APRecvB47.SetPricInfo[i].StartTime[0]) + BCDtoHEX(APRecvB47.SetPricInfo[i].StartTime[1]) * 60;
		price.EndTime[i] =  BCDtoHEX(APRecvB47.SetPricInfo[i].StopTime[0]) + BCDtoHEX(APRecvB47.SetPricInfo[i].StopTime[1]) * 60;;
		price.CurTimeQuantum[i] = (_TIME_QUANTUM)(APRecvB47.SetPricInfo[i].CurTimeQuantum - 1);				//当前处于哪个时间段
	}
	price.EndTime[i - 1] = 0;			//最后一个结尾肯定为0
	price.TimeQuantumNum = APRecvB47.FricNumber;
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = PARA_PRICALL_FLLEN;
	FlashOper.ptr = (INT8U *)&price;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DataLen = 2;
	Msg.DivNum = APP_RATE_ACK;
	
	
	memcpy(&PriceSet,&price,sizeof(_PRICE_SET));
	OSQPost(psendevent, &Msg);
	SendB53State = 1;			//发送分时计费记录	
	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvStartAppAck
* Description  : 启动确认帧
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvStartAppAck(INT8U *pdata,INT16U len)
{
	return TRUE;
}	

/*****************************************************************************
* Function     : AP_RecvHearAck
* Description  : 心跳确认帧
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvHearAck(INT8U *pdata,INT16U len)
{
	return TRUE;
}	


/*****************************************************************************
* Function     : AP_RecvOnlineBillAck
* Description  : 返回在线交易记录
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvOnlineBillAck(INT8U *pdata,INT16U len)
{
	INT8U gun = 0;
	INT8U ifsuccess;
	gun = pdata[8];
	ifsuccess = pdata[9];
	if(ifsuccess == 0)
	{
		ResendBillControl[gun].ResendBillState = FALSE;			//订单确认，不需要重发订单
		ResendBillControl[gun].SendCount = 0;
		WriterFmBill((_GUN_NUM)gun,0);
	}
	return TRUE;
}	

/*****************************************************************************
* Function     : AP_RecvB54BillAck
* Description  : 返回在线分时交易记录
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvB54BillAck(INT8U *pdata,INT16U len)
{
//	INT8U gun = 0;
//	INT8U ifsuccess;
//	gun = pdata[8];
//	ifsuccess = pdata[9];
	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvTime
* Description  : 校准时间
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvTime(INT8U *pdata,INT16U len)
{
	_BSPRTC_TIME SetTime;                       //设定时间
	INT8U times = 3;							//如果设置失败反复设置三次
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	INT8U Second;
	if(len != (sizeof(APRecvSetTime)) )
	{
		return FALSE;
	}
	memcpy((INT8U*)&APRecvSetTime,pdata,7);
	
	Second = APRecvSetTime.Second / 1000;
	//校准时间
	SetTime.Year   = HEXtoBCD(APRecvSetTime.Year);
    SetTime.Month  = HEXtoBCD(APRecvSetTime.Month);
    SetTime.Day    = HEXtoBCD(APRecvSetTime.Day);
    SetTime.Hour   = HEXtoBCD(APRecvSetTime.Hour);
    SetTime.Minute = HEXtoBCD(APRecvSetTime.Minute);
    SetTime.Second = HEXtoBCD(Second);
	
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_STE_TIME;
	OSQPost(psendevent, &Msg);
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
* Function     : AP_RecvCardB7
* Description  : 接收到卡鉴权信息
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvCardB7(INT8U *pdata,INT16U len)
{
	INT8U gun = 0;
	INT16U datalen = 0;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	static _BSP_MESSAGE Msg[GUN_MAX];
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	datalen = sizeof(AP_RECV_B7);
	
	if(len != datalen )
	{
		return FALSE;
	}
	gun = pdata[8];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if(pdisp_conrtol->NetGunState[gun] != GUN_IDLE)
	{
		return FALSE;
	}
	memcpy((INT8U*)&APRecvCardCmd[gun],pdata,len);
	if(APRecvCardCmd[gun].IfSuccess) //鉴权成功，发送
	{
		APStartType[gun] = _4G_APP_CARD;
		StartNetState[gun] = NET_STATE_ONLINE;  //在线充电
		//发送刷卡启动
		Msg[gun].MsgID = BSP_4G_MAIN;
		Msg[gun].DataLen = gun;
		Msg[gun].DivNum = APP_CARDVIN_CHARGE;
		OSQPost(psendevent, &Msg[gun]);
	}
	else
	{
		NOP();
		//通知鉴权失败
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
		OSTimeDly(SYS_DELAY_50ms);	
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	}
	return TRUE;
}


/*****************************************************************************
* Function     : AP_RecvCardB7
* Description  : 接收到卡鉴权信息
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvCardB9(INT8U *pdata,INT16U len)
{
	INT8U gun = 0;
	INT16U datalen = 0;
	static INT8U Vinstate = 0; //0失败 1成功
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	static _BSP_MESSAGE Msg[GUN_MAX];
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	datalen = sizeof(AP_RECV_B9);
	
	if(len != datalen )
	{
		return FALSE;
	}
	gun = pdata[8];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memcpy((INT8U*)&APRecvVinCmd[gun],pdata,len);
	if(APRecvVinCmd[gun].IfSuccess) //鉴权成功，发送
	{
		APStartType[gun] = _4G_APP_VIN;
		StartNetState[gun] = NET_STATE_ONLINE;  //在线充电
		//发送Vin启动
		Msg[gun].MsgID = BSP_4G_MAIN;
		Msg[gun].DataLen = gun;
		Msg[gun].DivNum = APP_CARDVIN_CHARGE;
		OSQPost(psendevent, &Msg[gun]);
	}
	else
	{
		Vinstate = 0;
		//发送启动应答
		Msg[gun].MsgID = BSP_4G_RECV;
		Msg[gun].DataLen = gun;
		Msg[gun].pData = &Vinstate;
		Msg[gun].DivNum = APP_START_VIN;
		OSQPost(psendevent, &Msg[gun]);
		NOP();
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
		OSTimeDly(SYS_DELAY_50ms);	
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
		//通知鉴权失败
	}
	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvCardStartB10
* Description  : 接收到卡Vin启动
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvCardVinStartB11(INT8U *pdata,INT16U len)
{

	INT8U gun = 0;
	static INT8U Vinstate = 0; //0失败 1成功
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	 static _BSP_MESSAGE SendStartMsg[GUN_MAX];
	OS_EVENT* pevent = APP_Get4GMainEvent();
	
	if(len != (sizeof(AP_RECV_B11)) )
	{
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
		OSTimeDly(SYS_DELAY_50ms);	
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
		return FALSE;
	}
	gun = pdata[8];
	if(gun >= GUN_MAX)
	{
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
		OSTimeDly(SYS_DELAY_50ms);	
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
		return FALSE;
	}
	memcpy((INT8U*)&APRecvCardVinStart[gun],pdata,len);
	if(APRecvCardVinStart[gun].ifSuccess) //成功，发送启动成功
	{
		ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
		ResendBillControl[gun].SendCount = 0;
		if(APStartType[gun] == _4G_APP_VIN)
		{
			Vinstate = 1;
			//发送启动应答
			SendStartMsg[gun].MsgID = BSP_4G_RECV;
			SendStartMsg[gun].DataLen = gun;
			SendStartMsg[gun].pData = &Vinstate;
			SendStartMsg[gun].DivNum = APP_START_VIN;
			OSQPost(pevent, &SendStartMsg[gun]);
		}
		else
		{
			//卡启动
					//发送启动应答
			SendStartMsg[gun].MsgID = BSP_4G_RECV;
			SendStartMsg[gun].DataLen = gun;
			SendStartMsg[gun].DivNum = APP_START_CHARGE;
			OSQPost(pevent, &SendStartMsg[gun]);
		}
	}
	else
	{
		if(APStartType[gun] == _4G_APP_VIN)
		{
			Vinstate = 0;
			//发送启动应答
			SendStartMsg[gun].MsgID = BSP_4G_RECV;
			SendStartMsg[gun].DataLen = gun;
			SendStartMsg[gun].pData = &Vinstate;
			SendStartMsg[gun].DivNum = APP_START_VIN;
			OSQPost(pevent, &SendStartMsg[gun]);
		}
		else{
			NOP();
		}
	}
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
	OSTimeDly(SYS_DELAY_50ms);	
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvCardStartB10
* Description  : 接收到卡Vin启动
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvB17CardWL(INT8U *pdata,INT16U len)
{
	INT8U num,i,addnum,delnum,j;
	INT8U buf[96];
	
	
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();

	if(len < 30)
	{
		return FALSE;
	}
	//获取白名单数量
	num = pdata[23];
	//最多十二个	
	if((num == 0) || (num > 12) )
	{
		return FALSE;
	}
	if(len != (sizeof(_AP_CARD_INFO) * num + 24) )
	{
		return FALSE;
	}
	memcpy((INT8U*)&APCardWL,pdata,len);
	//写白名单
	addnum = 0;
	delnum = 0;
	//增加
	for(i = 0;i < num;i++)
	{
		if(APCardWL.CardInfo[i].State == 1)
		{
			
			for(j = 0;j< 8;j++)
			{
				//卡号反过来的
				buf[addnum*8 + j] = APCardWL.CardInfo[i].CardNum[7-j];
			}
			//memcpy(&buf[addnum*8],APCardWL.CardInfo[i].CardNum,8);
			addnum++;
		}
	}
	if(addnum)
	{
		AP_CardWhiteListDispose(buf,addnum,1);
	//减少
	}
	for(i = 0;i < num;i++)
	{
		if(APCardWL.CardInfo[i].State == 2)
		{
			for(j = 0;j< 8;j++)
			{
				//卡号反过来的
				buf[delnum*8 + j] = APCardWL.CardInfo[i].CardNum[7-j];
			}
			delnum++;
		}
	}
	if(delnum)
	{
		AP_CardWhiteListDispose(buf,delnum,0);
	}
	
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_CARD_WL;   //发送卡应答
	OSQPost(psendevent, &Msg);
	return TRUE;
	
}

/*****************************************************************************
* Function     : AP_RecvB19VINWL
* Description  : 接收到Vin白名单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U AP_RecvB19VINWL(INT8U *pdata,INT16U len)
{
	INT8U num,i,addnum,delnum;
	INT8U buf[250];
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if(len < 30)
	{
		return FALSE;
	}
	//获取白名单数量
	num = pdata[23];
	//最多十二个	
	if((num == 0) || (num > 12) )
	{
		return FALSE;
	}
	if(len != (sizeof(_AP_VIN_INFO) * num + 24) )
	{
		return FALSE;
	}
	memcpy((INT8U*)&APVinWL,pdata,len);
	//写白名单
	//写白名单
	addnum = 0;
	delnum = 0;
	//增加
	for(i = 0;i < num;i++)
	{
		if(APVinWL.VinInfo[i].State == 1)
		{
			memcpy(&buf[addnum*17],APVinWL.VinInfo[i].VIN,17);
			addnum++;
		}
	}
	if(addnum)
	{
		AP_VinWhiteListDispose(buf,addnum,1);
	//减少
	}
	for(i = 0;i < num;i++)
	{
		if(APVinWL.VinInfo[i].State == 2)
		{
			memcpy(&buf[delnum*17],APVinWL.VinInfo[i].VIN,17);
			delnum++;
		}
	}
	if(delnum)
	{
		AP_VinWhiteListDispose(buf,delnum,0);
	}
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_VIN_WL;   //发送卡应答
	OSQPost(psendevent, &Msg);
	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvB21VINCardRes
* Description  : 清空白名单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U AP_RecvB21VINCardRes(INT8U *pdata,INT16U len)
{
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	_FLASH_OPERATION  FlashOper;
	 //读取数据
	memset(FlashDisposeBuf,0xff,sizeof(FlashDisposeBuf));
	FlashDisposeBuf[0] = 0;
	
	if(len != sizeof(_AP_RECV_B21) )
	{
		return FALSE;
	}
	memcpy(&APVinCardRes,pdata,len);
	if(APVinCardRes.type == 1)
	{
		FlashOper.DataID = PARA_CARDWHITEL_ID;
		FlashOper.Len = PARA_CARDWHITEL_FLLEN;
		FlashOper.ptr = FlashDisposeBuf;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("res card  err");
		}
	}
	if(APVinCardRes.type == 2)
	{
		FlashOper.DataID = PARA_VINWHITEL_ID;
		FlashOper.Len = PARA_VINWHITEL_FLLEN;
		FlashOper.ptr = FlashDisposeBuf;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("res vin  err");
		}
	}
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_VINCARD_RES;   //清空应答
	OSQPost(psendevent, &Msg);

	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvB16OFFLineRco
* Description  : 离线交易记录应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U AP_RecvB16OFFLineRco(INT8U *pdata,INT16U len)
{
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	static _BSP_MESSAGE Msg;
	INT8U data[300];
	_AP_RECV_B16 *poffline;
	if(len != sizeof(_AP_RECV_B16) )
	{
		return FALSE;
	}
	memcpy(&APOFFLineRec,pdata,len);
	 APP_RWNetOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_READ,data);   //读取离线交易记录
	poffline = (_AP_RECV_B16 *)&data[14];
	if(CmpNBuf(poffline->DrderNum,APOFFLineRec.DrderNum,sizeof(poffline->DrderNum) ) == TRUE)
	{
		//订单号一致
		Msg.MsgID = BSP_4G_MAIN;
		Msg.DivNum = APP_OFFLINE_ACK;   //离线应答
		OSQPost(psendevent, &Msg);
	}

	return TRUE;
}

/*****************************************************************************
* Function     : AP_RecvSTCmd
* Description  : 接收到启动还是停止
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   AP_RecvSTCmd(INT8U *pdata,INT16U len)
{
	INT8U gun = 0;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	 static _BSP_MESSAGE SendStartMsg[GUN_MAX],SendStopMsg[GUN_MAX];
	if(len != (sizeof(AP_RECV_B4)) )
	{
		return FALSE;
	}
	gun = pdata[8];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if(pdata[9] == 1)	//启动充电
	{
		ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
		ResendBillControl[gun].SendCount = 0;
		memcpy((INT8U*)&APRecvStartCmd[gun],pdata,sizeof(AP_RECV_B4));
		//发送启动应答
		APStartType[gun] = _4G_APP_START;
		StartNetState[gun] = NET_STATE_ONLINE;  //在线充电
		SendStartMsg[gun].MsgID = BSP_4G_RECV;
		SendStartMsg[gun].DataLen = gun;
		SendStartMsg[gun].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendStartMsg[gun]);
	}else
	{
		
				//发送停止应答
		Msg[gun].MsgID = BSP_4G_MAIN;
		Msg[gun].DataLen = gun;
		Msg[gun].DivNum = APP_STOP_ACK;
		OSQPost(psendevent, &Msg[gun]);
	//	return TRUE;
		
		memcpy((INT8U*)&APRecvStopCmd[gun],pdata,sizeof(AP_RECV_B4));
		SendStopMsg[gun].MsgID = BSP_4G_RECV;
		SendStopMsg[gun].DataLen = gun;
		SendStopMsg[gun].DivNum = APP_STOP_CHARGE;
		OSQPost(pevent, &SendStopMsg[gun]);
	}
	return TRUE;
}	

/*****************************************************************************
* Function     : AP_RecvFrameDispose
* Description  : 安培快充数据处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   AP_RecvFrameDispose(INT8U * pdata,INT16U len)
{
	INT8U i = 0;
	INT8U num;
	static INT8U buf[300];
	INT8U cmdlen;

	if((pdata == NULL) || (len < 6) )
	{
		return FALSE;
	}
//	if((datalen+2) != len)
//	{
//		return FALSE;
//	}
	for(i = 0;i < AP_RECV_TABLE_NUM;i++)
	{	if(len < APRecvDisposeTable[i].len)
		{
			continue;
		}
		for(num = 0;num < APRecvDisposeTable[i].len;num++)
		{
			if(pdata[num] != APRecvDisposeTable[i].pdata[num])
			{
				break;
			}
		}
		if(num == APRecvDisposeTable[i].len)
		{
			if(APRecvDisposeTable[i].recvfunction != NULL)
			{
				cmdlen = APRecvDisposeTable[i].len;
				//提取数据
				memcpy(buf,&pdata[cmdlen],len - cmdlen);  //真正得数据内容
				APRecvDisposeTable[i].recvfunction(buf,len - cmdlen);
				return TRUE;
			}
		}
		if(APRecvDisposeTable[i].pdata[1] == 0xff)  //表示接收数据可变长度，
		{
			for(num = 0;num < APRecvDisposeTable[i].len - 3;num++)
			{
				if(pdata[num + 3] != APRecvDisposeTable[i].pdata[num + 3])
				{
					break;
				}
			}
			if(num == APRecvDisposeTable[i].len - 3)
			{
				if(APRecvDisposeTable[i].recvfunction != NULL)
				{
					cmdlen = APRecvDisposeTable[i].len;
					//提取数据
					memcpy(buf,&pdata[cmdlen],len - cmdlen);  //真正得数据内容
					APRecvDisposeTable[i].recvfunction(buf,len - cmdlen);
					return TRUE;
				}
			}
		}
	}

	return TRUE;
}


/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/
