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
#include "4GMain.h"
#include "4GRecv.h"
#include "_66Frame.h"
#include <string.h>
#include "sysconfig.h"
#include "DisplayMain.h"
#include "charging_Interface.h"
#include "RTC_task.h"
#include "PCUMain.h"
#include "DisplayMain.h"
#include "card_cpu.h"
#include "card_Interface.h"
/* Private define-----------------------------------------------------------------------------*/
#define _66_SEND_FRAME_LEN   4
#define _66_RECV_FRAME_LEN	14




typedef enum{
	
	_66_cmd_type_3 = 3,
	_66_cmd_type_4 = 4,
	_66_cmd_type_5 = 5,
	_66_cmd_type_6 = 6,
	_66_cmd_type_7 	= 7,
	_66_cmd_type_8 	= 8,
	_66_cmd_type_11	= 11,
	_66_cmd_type_12 	= 12,
	_66_cmd_type_33 	= 33,
	_66_cmd_type_34	= 34,
	_66_cmd_type_101	= 101,
	_66_cmd_type_102 = 102,
	_66_cmd_type_103	= 103,
	_66_cmd_type_104	= 104,
	_66_cmd_type_105 = 105,
	_66_cmd_type_106	= 106,
	_66_cmd_type_201	= 201,
	_66_cmd_type_202	= 202,
	_66_cmd_type_107 = 107,
	_66_cmd_type_108	= 108,
	_66_cmd_type_113 = 113,
	_66_cmd_type_114 = 114,	
	_66_cmd_type_117	= 117,
	_66_cmd_type_118	= 118,
	_66_cmd_type_1303	= 1303,
	_66_cmd_type_1304	= 1304,
	_66_cmd_type_1305	= 1305,
	_66_cmd_type_1306	= 1306,
	_66_cmd_type_1309	= 1309,
	_66_cmd_type_1310	= 1310,
	_66_cmd_type_1311	= 1311,
	_66_cmd_type_1312	= 1312,
}_66_mqtt_cmd_enum;

//注册
__packed typedef struct{
 uint8_t      	reg[4];			//0：无充电桩编号1：成功/
}_66_cmd_105;

//106
__packed typedef struct {
 uint16_t	charge_mode_num; 			//该充电桩总共电源模块数
	uint16_t  charge_mode_rate;			//充电桩电源模块总功率大小 0.1kW
	uint8_t   equipment_id[32];			//充电桩编码	
	uint8_t   offline_charge_flag;		//离线/在线允许充电设置 默认值为0x01，其中高位的0表示在线允许充电，低位的1表示离线禁止充电。如果低位设0，则表示离线允许充电
	uint8_t	stake_version[4];			//充电桩软件版本
	uint16_t  stake_type;					//充电桩类型0x00 直流0x01 交流0x02 混合
	uint32_t  stake_start_times;			//启动次数 终端每次启动，计数保存
	uint8_t   data_up_mode;				//终端每次启动，计数保存   1：应答模式2：主动上报模式
	uint16_t  sign_interval;				//签到间隔时间
	uint8_t		res;
	uint8_t   gun_index;					//充电枪个数
	uint8_t   heartInterval;				//心跳上报周期
	uint8_t   heart_out_times;			//心跳包检测超时次数
	uint32_t	stake_charge_record_num;	//充电记录数量
	uint8_t   stake_systime[8];			//当前充电桩系统时间
	uint8_t   stake_last_charge_time[8];	//最近一次充电时间
	uint8_t   stake_last_start_time[8];	//最近一次启动时间
	uint8_t   signCode[8];				//签到密码（保留）
	uint8_t  ccu_version[32];				//充电桩CCU软件版本
}_66_cmd_106;


uint32_t	money;						//账户余额 0.01
    uint16_t user_tel;						//用户手机后4位
//启停
__packed typedef struct{

	uint8_t res0[4];					
	uint8_t	gun_index;						//充电枪口  从0开始
	uint32_t chargetype;				//0立即充电  2预约充电	
	uint32_t	money;						//账户余额 0.01
	uint32_t	charge_policy;				//0:充满为止（默认）1:时间控制充电2:金额控制充电3:电量控制充电4:按SOC控制充电
	uint32_t		charge_policy_param;	//时间单位为1秒金额单位为0.01元 电量时单位为0.01kw SOC单位为%
	uint8_t	book_time[8];					//预约/定时启动时间
	uint8_t	book_delay_time;				//预约超时时间（保留）单位分钟(预约时有效)
	uint8_t	charge_user_app[32];				//用户识别号(App)
	uint8_t	allow_offline_charge;			//断网充电标志		0-不允许 1-允许，默认0
	uint32_t		allow_offline_charge_kw_amout;//离线可充电电量     0.01kw
	uint8_t	charge_user_id[32];				//订单号
	uint8_t	res1[8];
}_66_cmd_7;

__packed typedef struct{
	uint8_t res0[4];	
	uint8_t equipment_id[32];			//充电桩编码
	uint8_t gun_index;					//充电枪口   同服务发送枪口 从0开始
	uint32_t result;					//命令执行结果  错误含义见附录：错误码对照表
	uint8_t	charge_user_id[32];			//订单号
}_66_cmd_8;


//服务器终止订单
__packed typedef struct{
	uint8_t equipment_id[16];			//充电桩编码
	uint8_t gun_index;					//充电枪口	从0开始
	uint8_t charge_seq[32];				//订单号
}_66_cmd_11;
// 12
__packed typedef struct{
	uint8_t equipment_id[16];			//充电桩编码
	uint8_t gun_index;					//充电枪口
	uint8_t charge_seq[32];				//订单号 从0开始
} _66_cmd_12;


//充电桩状态
//103
__packed typedef struct{
	uint8_t res[42];
}_66_cmd_103;
//104
__packed typedef struct{	
	uint8_t res[4];
	uint8_t	equipment_id[32];			//充电桩编码
	uint8_t	gun_cnt;					//充电枪数量
	uint8_t	gun_index;					//充电口号 从0开始
	uint8_t 	gun_type;				//充电枪类型    1=直流； 2=交流；
	uint8_t 	work_stat;				//工作状态0-空闲中 1-已插枪2-充电进行中 3-未拔枪4-预约状态5-自检6-系统故障7-停止中
	uint8_t  soc_percent;				//当前SOC %
	uint8_t  alarm_stat[4];				//告警状态（保留） 0-断开 1-半连接 2-连接直流目前只有0和2状态交流目前有0、1、2三种状态
	uint8_t car_connection_stat;		//车辆连接状态
	uint32_t cumulative_charge_fee;		//本次充电累计充电费用  从本次充电开始到目前的累计充电费用（包括电费与服务费），这里是整型，要乘以0.01才能得到真实的金额
	uint8_t res1[8];
	uint16_t	dc_charge_voltage;		//直流充电电压	
	uint16_t dc_charge_current;			//直流充电电流
	uint16_t bms_need_voltage;			//BMS需求电压
	uint16_t bms_need_current;			//BMS需求电流
	uint8_t bms_charge_mode;			//BMS充电模式  充电有效（直流有效，交流置0）1-恒流 2-恒压
	uint16_t ac_a_vol;					//交流A相充电电压
	uint16_t ac_b_vol;					//流B相充电电压
	uint16_t ac_c_vol;					//交流C相充电电压
	uint16_t ac_a_cur;					//交流A相充电电流
	uint16_t ac_b_cur;					//交流B相充电电流
	uint16_t ac_c_cur;					//交流C相充电电流
	uint16_t charge_full_time_left;		//距离满电剩余充电时间(min)
	uint32_t charged_sec;				//充电时长(秒)
	uint32_t cum_charge_kwh_amount; /*累计充电电量 */
	uint32_t before_charge_meter_kwh_num; /*充电前电表读数 */
	uint32_t now_meter_kwh_num; /* 当前电表读数 */

	uint8_t start_charge_type;				//充电启动方式   0：本地刷卡启动1：后台启动2：本地管理员启动
	uint8_t charge_policy;					//充电策略  0自动充满 1按时间充满  2定金额  3按电量充满 4按SOC充（直流）  
	int32_t charge_policy_param;			//充电策略参数 时间单位为1秒金额单位为0.01元电量时单位为0.01kw SOC为1%
	uint8_t book_flag;						//预约标志  0-无预约（无效）  1-预约有效
	uint8_t charge_user_id[32];				//订单号
	uint8_t book_timeout_min;				//预约超时时间    单位分钟
	uint8_t book_start_charge_time[8];		//预约/开始充电开始时间
	uint32_t before_charge_card_account;		//充电前卡余额（保留）
	uint8_t    res2[4];                       
	uint32_t charge_power_kw; /* 充电功率 */	//充电功率 0.1kw
	uint8_t res3[23];
} _66_cmd_104;



//心跳
//101
__packed typedef struct{
	uint8_t	res[4];
	uint16_t heart_index;		//心跳应答   原样返回收到的102心跳序号字段
} _66_cmd_101;
//102
__packed typedef struct{
	
	uint8_t	res[4];
	uint8_t	equipment_id[32];	//充电桩编码
	uint16_t heart_index;		//心跳序号  省=0，由桩端统一编号，严格递增。达到最大值时，重新从0开始累加
} _66_cmd_102;

//记录上传
// 201
__packed typedef struct{	
	uint8_t res[4];
	uint8_t	gun_index;				//充电枪号 从0开始
	uint8_t 	user_id[32];		//充电卡号/用户号
	uint8_t charge_user_id[32];		//订单号	
}_66_cmd_201;
//202
__packed typedef struct{
	uint8_t res0[4];
	uint8_t equipment_id[32];		//充电桩编码
	uint8_t gun_type;				//充电枪类型 	1-直流 2-交流
	uint8_t gun_index;				//充电枪口
	uint8_t card_id[32];		//卡号	
	uint8_t charge_start_time[8];	//充电开始时间
	uint8_t charge_end_time[8];		//充电结束时间
	uint32_t charge_time;			//充电时间长度  单位秒
	uint8_t start_soc;				//开始SOC
	uint8_t end_soc;				//结束SOC
	uint32_t err_no;				//充电结束原因  错误含义见附录
	
	uint32_t charge_kwh_amount; /*充电电量   0.001*/

	uint32_t start_charge_kwh_meter; /*充电前电表读数  0.001*/
	uint32_t end_charge_kwh_meter; /* 充电后电表读数0.001*/

	uint32_t total_charge_fee;			//本次充电金额
	
	//卡相关
	uint32_t is_not_stoped_by_card;		//是否不刷卡结束（保留)  这个字段只有在刷卡充电时有效0--否（刷卡结束） 1--是（不刷卡直接拔枪结束）2—用户点击充电桩本地指令
	uint32_t start_card_money;
	uint32_t end_card_money;
	uint32_t total_service_fee;
	uint8_t is_paid_by_offline;
	
	uint8_t charge_policy;				//充电策略  0:充满为止1:时间控制充电2:金额控制充电3:电量控制充电4:SOC控制充电
	uint32_t charge_policy_param;		//充电策略参数时间单位为1秒金额单位为0.01元电量时单位为0.01kwSOC单位1%
	uint8_t car_vin[17];				//车辆 VIN	
	uint8_t car_plate_no[8];			//车牌号
	/* 分时电量 uint16->uint32 */
	uint16_t kwh_amount[48];				//时段1电量
	uint8_t start_charge_type;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
	uint8_t charge_user_id[32];		//订单号	
	uint8_t res1[11];
} _66_cmd_202;

//事件信息上报
//107
__packed typedef struct {
  uint8_t    equipment_id[16];  //充电桩编码
  uint8_t	gun_index;			//充电枪口  从0开始
  uint8_t	errCode[4];			//故障错误码
}_66_cmd_107;


//108
__packed typedef struct {
  uint8_t    equipment_id[16];		//充电桩编码
  uint8_t	gun_index;				//充电枪口 从0开始
  uint8_t	err_code[4];			//故障错误码
  uint8_t	err_status;				//故障状态   0x00-故障发生，0x01-故障已经恢复
  uint8_t charge_user_id[32];		//订单号		
}_66_cmd_108;



//117
__packed typedef struct {
  uint8_t    equipment_id[16];  //充电桩编码
  uint8_t	gun_index;			//充电枪口  从0开始
  uint8_t	errCode[4];			//故障错误码
}_66_cmd_117;


//118
__packed typedef struct {
  uint8_t    equipment_id[16];		//充电桩编码
  uint8_t	gun_index;				//充电枪口  从0开始
  uint8_t	err_code[4];			//故障错误码
  uint8_t	err_status;				//故障状态   0x00-故障发生，0x01-故障已经恢复	
}_66_cmd_118;


//1309
__packed typedef struct {
	//INT8U PricGtoupNum;			//分组数
	INT8U StartNum;				//起始分段编号
	INT8U Num;					//连续分段个数
	INT16U DMoney;				//电费 0.01
	INT16U FWMoney;				//服务费 0.01
	INT16U YCMoney;				//延时费用
}_66_cmd_1309;


//1310
__packed typedef struct {
	INT8U result;		//0 是成功   1长度错误 2分段数量错误 3分组数错误	
}_66_cmd_1310 ;


//34 充电桩刷卡q鉴权请求
__packed typedef struct {
	uint8_t    equipment_id[16];		//充电桩编码
	uint8_t	gun_index;				//充电枪口 从0开始
	uint8_t card_num[16];            //充电卡卡号   ASCII
	uint8_t card_rad[48];			//充电中随机数	ASCII
	uint32_t card_id;				//物理卡号  
}_66_cmd_34 ;


//33 服务器回复刷卡鉴权结果
__packed typedef struct {
	uint8_t    equipment_id[16];		//充电桩编码
	uint8_t		gun_index;				//充电枪口 从0开始
	uint16_t	card_ifsuccess;			//0成功  非0失败
	uint32_t    card_money;				//卡内余额
}_66_cmd_33 ;

//3服务器回复刷卡鉴权结果
__packed typedef struct {
	uint8_t    res[4];			//预留
	uint8_t Tpye;				//0 查询  1设置
	uint32_t Sadd;				//查询启始地址
	uint16_t datalen;				//查询启始地址
	uint8_t SetDate[256];		//最长256
}_66_cmd_3;

__packed typedef struct {
	uint8_t    res[4];			//预留
	uint8_t equipment_id[32];		//充电桩编码
	uint8_t Tpye;				//0 查询  1设置
	uint32_t Sadd;				//查询启始地址
	uint8_t Result;				//0表示成功  1表示失败
	uint8_t SetDate[256];		//最长256
}_66_cmd_4;


//3服务器回复刷卡鉴权结果
__packed typedef struct {
	uint8_t    res[4];			//预留
	uint8_t gun;				//枪号
	uint32_t Sadd;				//查询启始地址
	uint8_t cmdnum;				//命令个数
	uint16_t datalen;				//查询启始地址
	uint8_t SetDate[20];		//最长20
}_66_cmd_5;

__packed typedef struct {
	uint8_t    res[4];			//预留
	uint8_t equipment_id[32];		//充电桩编码
	uint8_t gun;				//枪号
	uint32_t Sadd;				//查询启始地址
	uint8_t cmdnum;				//命令个数
	uint8_t Result;				//0表示成功  1表示失败
}_66_cmd_6;

__packed typedef struct {
	INT32U pric[48];		//充电电费，服务费再1305体现	
}_66_cmd_1303 ;

__packed typedef struct {
	INT8U Result;		//0成功 1失败
}_66_cmd_1304 ;

__packed typedef struct {
	INT8U gun;
	INT16U FWpric;		//充电服务电费，服务费再1305体现	
}_66_cmd_1305 ;

__packed typedef struct {
	INT8U Result;		//0成功 1失败
}_66_cmd_1306 ;

//发送数据
_66_cmd_106 	Send66Cmd106;			//注册
_66_cmd_8	Send66Cmd8;				//启动应答
_66_cmd_102	Send66Cmd102;			//上传心跳
_66_cmd_104	Send66Cmd104[GUN_MAX];			//状态上报	
_66_cmd_202	Send66Cmd202;			//上传充电记录
_66_cmd_108	Send66Cmd108;			//充电桩故障上报（充电过程中产生）
_66_cmd_118	Send66Cmd118;			//充电桩故障上报
_66_cmd_1310 Send66Cmd1310;			//计费设置相应
_66_cmd_34	Send66Cmd34[GUN_MAX];			//卡鉴权回复
_66_cmd_4	Send66Cmd4;				//查询设置返回
_66_cmd_6	Send66Cmd6;				//停止充电应答
_66_cmd_1304 Send66Cmd1304;			//费率设置返回
_66_cmd_1306 Send66Cmd1306;			//服务费率设置返回
//接收函数
_66_cmd_105 	Recv66Cmd105 = {0};			//注册
_66_cmd_7	Recv66Cmd7[GUN_MAX] = {0};	//启动
_66_cmd_11	Recv66Cmd11[GUN_MAX] = {0};	//停止
_66_cmd_101	Recv66Cmd101 = {0};			//心跳
_66_cmd_103	Recv66Cmd103[GUN_MAX] = {0};	//状态应答
_66_cmd_201	Recv66Cmd201 = {0};			//充电记录应答
_66_cmd_107	Recv66Cmd107 = {0};			//充电桩故障应答（充电过程中产生）
_66_cmd_117	Recv66Cmd117 = {0};			//充电桩故障应答
_66_cmd_1309 Recv66Cmd1309[12] = {0};		//费率设置	最多4组
_66_cmd_33	Recv66Cmd33[GUN_MAX] = {0};	//发送卡鉴权		
_66_cmd_3	Recv66Cmd3 = {0};				//接收CMD3
_66_cmd_5	Recv66Cmd5 = {0};			//停止充电				
_66_cmd_1303 Recv66Cmd1303;				//费率设置，只有电费，服务费启动充电的时候下发
_66_cmd_1305 Recv66Cmd1305;				//服务费率设置，只有电费，服务费启动充电的时候下发
INT8U	Qrcode[GUN_MAX][256] = {0}; //二维码

_4G_START_TYPE _66StartType[GUN_MAX] ={ _4G_APP_START,_4G_APP_START};					//0表示App启动 1表示刷卡启动
static INT8U RateGroup = 0;  //费率组数   
static INT8U   _66_SendRegister(void); //注册
static INT8U   _66_SendHear(void)  ;//心跳
INT8U   _66_SendDevStateA(void); //充电桩A状态
INT8U   _66_SendDevStateB(void);  //充电桩B状态


extern _PRICE_SET PriceSet;							//电价设置 
_4G_SEND_TABLE _66SendTable[_66_SEND_FRAME_LEN] = {
	{0,    0,    SYS_DELAY_10s, 	_66_SendRegister			},  //发送注册帧

	{0,    0,    SYS_DELAY_30s, 	_66_SendHear				},	//心跳
	
	{0,    0,    SYS_DELAY_10s, 	_66_SendDevStateA		},	//充电桩A状态

	{0,    0,    SYS_DELAY_10s, 	_66_SendDevStateB		},	//充电桩B状态

};




static INT8U   _66_RecvStartCharge(INT8U *pdata,INT16U len);
static INT8U   _66_RecvStopCharge(INT8U *pdata,INT16U len);
static INT8U   _66_RecvHearAck(INT8U *pdata,INT16U len);
static INT8U   _66_RecvDevInfoAck(INT8U *pdata,INT16U len);
static INT8U   _66_RecvRegisterAck(INT8U *pdata,INT16U len);
static INT8U   _66_RecvRecordAck(INT8U *pdata,INT16U len);
static INT8U   _66_RecvRateSet(INT8U *pdata,INT16U len);
static INT8U   _66_RecvRateFWSet(INT8U *pdata,INT16U len);
static INT8U   _66_RecvDevStopAck(INT8U *pdata,INT16U len);
static INT8U   _66_RecvDevFailAck(INT8U *pdata,INT16U len);
static INT8U   _66_RecvQueryBill(INT8U *pdata,INT16U len);
static INT8U   _66_RecvQueryRate(INT8U *pdata,INT16U len);
static INT8U   _66_RecvCardStart(INT8U *pdata,INT16U len);
static INT8U   _66_RecvSetInfo(INT8U *pdata,INT16U len);
_4G_RECV_TABLE _66RecvTable[_66_RECV_FRAME_LEN] = {
	{_66_cmd_type_7				,	_66_RecvStartCharge	}, 		//启动充电

	{_66_cmd_type_5				,  	_66_RecvStopCharge	},		//停止充电

	{_66_cmd_type_101			, 	_66_RecvHearAck		},		//心跳应答

	{_66_cmd_type_103			,  	_66_RecvDevInfoAck	}, 		//状态上报应答

	{_66_cmd_type_105			,  	_66_RecvRegisterAck	},		//注册应答

	{_66_cmd_type_201			,  	_66_RecvRecordAck	},		//记录应答

	{_66_cmd_type_107			,  	_66_RecvDevStopAck	},		//主动停止应答

	{_66_cmd_type_117			,  	_66_RecvDevFailAck	},		//故障上报
	
	{_66_cmd_type_1303			,  	_66_RecvRateSet		},		//费率设置
	
	{_66_cmd_type_113			,  	_66_RecvQueryBill	},		//查询最新一次的充电信息
	
	{_66_cmd_type_1311			,  	_66_RecvQueryRate	},		//查询费率
	
	{_66_cmd_type_33				,   _66_RecvCardStart	},		//接收到卡启动
	
	{_66_cmd_type_3				,   _66_RecvSetInfo	},		//主要是二维码和时间设置
	
	{_66_cmd_type_1305			,   _66_RecvRateFWSet	},		//设置服务费
	

};

__packed typedef struct {
	INT8U Reason;
	INT8U CodeNum[4];
}_FAIL_CODE;		//后台协议与CCU传过来的启动完成帧，停止完成帧对应上
/*******************************启动失败原因*********************************/

/*****************************************************************************
* Function     : APP_GetHYNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_Get66NetMoney(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	return Recv66Cmd7[gun].money;
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
INT8U   APP_Get66StartType(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return _4G_APP_START;
	}
	return (INT8U)_66StartType[gun];
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
INT8U   APP_Set66StartType(INT8U gun ,_4G_START_TYPE  type)
{
	if((type >=  _4G_APP_MAX) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	
	_66StartType[gun] = type;
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
INT8U *  APP_Get66BatchNum(INT8U gun)
{
	static INT8U buf[16];		//交易流水号位16个字节，ASICC 因此取后16个数字

	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	memcpy(buf,&Recv66Cmd7[gun].charge_user_id[16],16);		//订单号的后16位唯一
	return buf;
}

/*****************************************************************************
* Function     : get_crc_Data
* Description  : 累加和
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U get_crc_Data(INT8U *pbuf,INT16U datalong)
{
	INT16U i;
	INT8U crcdata;
	crcdata = 0;
	for(i=0;i<datalong;i++)
	{
		crcdata += pbuf[i];
	}
	return crcdata;
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
INT8U   _66_SendFrameDispose(void)
{
	INT8U i;

	for(i = 0;i < _66_SEND_FRAME_LEN;i++)
	{
		if(_66SendTable[i].cycletime == 0)
		{
			continue;
		}
		_66SendTable[i].curtime = OSTimeGet();
		if((_66SendTable[i].curtime >= _66SendTable[i].lasttime) ? ((_66SendTable[i].curtime - _66SendTable[i].lasttime) >= _66SendTable[i].cycletime) : \
		((_66SendTable[i].curtime + (0xFFFFFFFFu - _66SendTable[i].lasttime)) >= _66SendTable[i].cycletime))
		{
			_66SendTable[i].lasttime = _66SendTable[i].curtime;
			if(_66SendTable[i].Sendfunc != NULL)
			{
				_66SendTable[i].Sendfunc();
			}
		}
		
	}
	return TRUE;
}

/*****************************************************************************
* Function     : _66FreamSend
* Description  : 汇誉帧发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   _66FreamSend(INT16U cmd,INT8U *pdata, INT16U len)
{
	INT8U Sendbuf[500];
	static INT8U count;			//序列号域 
	INT16U datalen = 9+len;
	
	if((pdata == NULL) || (!len) )
	{
		return FALSE;
	}
	//起始域
	Sendbuf[0] = 0xaa;
	Sendbuf[1] = 0xf7;
	//2个字节数据长度
	Sendbuf[2] = datalen& 0x00ff;
	Sendbuf[3] = (datalen >> 8)& 0x00ff;
	//4个字节版本号
	Sendbuf[4] = 0x16;
	//序列号
	Sendbuf[5] = count;

	//命令代码  大端
	Sendbuf[6] = cmd & 0x00ff;
	Sendbuf[7] = (cmd >> 8) & 0x00ff;
	
	memcpy(&Sendbuf[8],pdata,len);
	//1字节CS
	Sendbuf[8+len] = get_crc_Data(&Sendbuf[6],2+len);
	ModuleSIM7600_SendData(0, Sendbuf,(9+len)); //发送数据
	//OSTimeDly(SYS_DELAY_50ms); 
	count++;
	return TRUE;
	
	
}

////34 充电桩刷卡q鉴权请求
//__packed typedef struct {
//	uint8_t    equipment_id[16];		//充电桩编码
//	uint8_t	gun_index;				//充电枪口 从0开始
//	uint8_t card_num[16];            //充电卡卡号   ASCII
//	uint8_t card_rad[48];			//充电中随机数	ASCII
//	uint16_t card_id;				//物理卡号  
//}hy_cmd_34 ;


////33 服务器回复刷卡鉴权结果
//__packed typedef struct {
//	uint8_t    equipment_id[16];		//充电桩编码
//	uint8_t		gun_index;				//充电枪口 从0开始
//	uint16_t	card_ifsuccess;			//0成功  非0失败
//	uint32_t    card_money;				//卡内余额
//}hy_cmd_33 ;

/*****************************************************************************
* Function     : HY_SendCardInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _66_SendCardInfo(_GUN_NUM gun)
{
	INT8U Sendbuf[200] = {0};
	INT32U CardNum;
	USERCARDINFO * puser_card_info = NULL;
	INT8U * pdevnum = APP_GetDevNum(); 
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)	//显示已经注册成功了
	{
		return  FALSE;
	}

	puser_card_info = GetGunCardInfo(gun); //获取卡号
	CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
				(puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
	memset(Send66Cmd34[gun].equipment_id,0,sizeof(_66_cmd_34));
	memcpy(Send66Cmd34[gun].equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd34[gun].gun_index = gun;
	Send66Cmd34[gun].card_id = 	CardNum;
	Send66Cmd34[gun].card_num[6] =  (CardNum / 1000000000) + '0';
	CardNum  = 	CardNum % 1000000000;
	Send66Cmd34[gun].card_num[7] = 	(CardNum / 100000000) + '0';
	CardNum  = 	CardNum % 100000000;
	Send66Cmd34[gun].card_num[8] = 	(CardNum / 10000000) + '0';
	CardNum  = 	CardNum % 10000000;
	Send66Cmd34[gun].card_num[9] = 	(CardNum / 1000000) + '0';
	CardNum  = 	CardNum % 1000000;
	Send66Cmd34[gun].card_num[10] = 	(CardNum / 100000) + '0';
	CardNum  = 	CardNum % 100000;
	Send66Cmd34[gun].card_num[11] = 	(CardNum / 10000) + '0';
	CardNum  = 	CardNum % 10000;
	Send66Cmd34[gun].card_num[12] = 	(CardNum / 1000) + '0';
	CardNum  = 	CardNum % 1000;
	Send66Cmd34[gun].card_num[13] = 	(CardNum / 100) + '0';
	CardNum  = 	CardNum % 100;
	Send66Cmd34[gun].card_num[14] = 	(CardNum / 10) + '0';
	CardNum  = 	CardNum % 10;
	Send66Cmd34[gun].card_num[15] = 	(CardNum / 1) + '0';
//	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
//	OSTimeDly(SYS_DELAY_50ms);	
//	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
  	return _66FreamSend(_66_cmd_type_34,Send66Cmd34[gun].equipment_id,sizeof(_66_cmd_34));
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
INT8U _66_SendBillData(INT8U * pdata,INT16U len,INT8U ifquery)
{
	_66_cmd_202 *psend202 = (_66_cmd_202 *)pdata;
	if((pdata == NULL) || (len < sizeof(Send66Cmd202)))
	{
		return FALSE;
	}
	return _66FreamSend(_66_cmd_type_202,pdata,sizeof(Send66Cmd202));
}


/*****************************************************************************
* Function     : HY_SendRegister
* Description  : 注册
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_SendRegister(void)
{
	INT8U * pdevnum = APP_GetDevNum(); 
	
	if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
	{
		return  FALSE;
	}
	memset(&Send66Cmd106,0,sizeof(Send66Cmd106));
	Send66Cmd106.charge_mode_num = 8;
	Send66Cmd106.charge_mode_rate = 1200;		//120kw
	memcpy(Send66Cmd106.equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd106.offline_charge_flag = 0;			//离线允许充电
	Send66Cmd106.stake_type = 0x00;					//充电桩类型0x00 直流0x01 交流0x02 混合
	Send66Cmd106.gun_index = APP_GetGunNum();					//充电桩枪数量
	return _66FreamSend(_66_cmd_type_106,(INT8U*)&Send66Cmd106,sizeof(Send66Cmd106));

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
INT8U   _66_SendRateAck(INT8U cmd) 
{
	if(cmd == 1)
	{
		Send66Cmd1304.Result = 0;	//表示成功
		return _66FreamSend(_66_cmd_type_1304,(INT8U*)&Send66Cmd1304,sizeof(Send66Cmd1304));
	}
	if(cmd == 0)
	{
		Send66Cmd1306.Result = 0;
		return _66FreamSend(_66_cmd_type_1306,(INT8U*)&Send66Cmd1306,sizeof(Send66Cmd1306));
	}
}

/*****************************************************************************
* Function     : HY_SendHear
* Description  : 心跳
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_SendHear(void) 
{
	static INT16U count = 0;
	INT8U * pdevnum = APP_GetDevNum(); 
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&Send66Cmd102,0,sizeof(Send66Cmd102));
	memcpy(Send66Cmd102.equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd102.heart_index = count++;
	return _66FreamSend(_66_cmd_type_102,(INT8U*)&Send66Cmd102,sizeof(Send66Cmd102));
}

/*****************************************************************************
* Function     : HY_SendStartAck
* Description  : 启动应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _66_SendStartAck(_GUN_NUM gun)
{
	//发送启动应答说明已经启动成功了，若启动失败，则不发送启动应答，直接发送订单
	INT8U errcode;
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(&Send66Cmd8,0,sizeof(Send66Cmd8));
	if(pdisp_conrtol->NetGunState[gun] == GUN_CHARGEING)
	{
		//表示启动成功
		Send66Cmd8.result = 0;
	}else
	{
		errcode = APP_GetStopChargeReason(gun);   //获取故障码
		if(errcode & 0x80) //启动失败
		{
			errcode &= 0x7f;
			if(ENDFAIL_HANDERR == errcode)
			{
				Send66Cmd8.result = 1;
			}
			else if(ENDFAIL_EMERGENCY == errcode)
			{
				Send66Cmd8.result = 5;
			}
			else
			{
				Send66Cmd8.result = 11;
			}

		}
	}
	
	memcpy(Send66Cmd8.equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd8.gun_index = gun;
	memcpy(Send66Cmd8.charge_user_id,Recv66Cmd7[gun].charge_user_id,32);  //订单号
	return _66FreamSend(_66_cmd_type_8,(INT8U*)&Send66Cmd8,sizeof(Send66Cmd8));
}

/*****************************************************************************
* Function     : HY_SendStOPtAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _66_SendStopAck(_GUN_NUM gun)
{
	
//		uint8_t    res[4];			//预留
//	uint8_t equipment_id[32];		//充电桩编码
//	uint8_t gun;				//枪号
//	uint32_t Sadd;				//查询启始地址
//	uint8_t cmdnum;				//命令个数
//	uint8_t Result;				//0表示成功  1表示失败
	INT8U * pdevnum = APP_GetDevNum(); 
	memset(&Send66Cmd6,0,sizeof(Send66Cmd6));
	memcpy(Send66Cmd6.equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd6.gun = gun;
	Send66Cmd6.Sadd = 2;
	Send66Cmd6.cmdnum = 1;
	Send66Cmd6.Result = 0;
	return _66FreamSend(_66_cmd_type_6,(INT8U*)&Send66Cmd6,sizeof(Send66Cmd6));
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
INT8U   _66_SendQueryRateAck(void)
{
	INT8U buf[100];
	INT8U len;
	
	if((RateGroup == 0) || (RateGroup > 4) )
	{
		return FALSE;
	}
	buf[0] = RateGroup;
	memcpy(&buf[1],(INT8U*)Recv66Cmd1309,sizeof(Recv66Cmd1309));
	len = 1 + RateGroup* sizeof(_66_cmd_1309);
	return _66FreamSend(_66_cmd_type_1312,buf,len);
}
/*****************************************************************************
* Function     : HFQG_SendDevStateA
* Description  : 充电桩A状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _66_SendDevStateA(void)
{
	TEMPDATA * pmeter = NULL;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	INT8U * pdevnum = APP_GetDevNum(); 
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	pmeter = APP_GetMeterPowerInfo(GUN_A);
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&Send66Cmd104[GUN_A],0,sizeof(_66_cmd_104));
	memcpy(Send66Cmd104[GUN_A].equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd104[GUN_A].gun_cnt = APP_GetGunNum();					//充电枪数量
	Send66Cmd104[GUN_A].gun_index = 0;						//充电口号
	Send66Cmd104[GUN_A].gun_type = 1;						//充电枪类型  1=直流； 2=交流；
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		Send66Cmd104[GUN_A].work_stat = 2;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_UNLINK)
	{
		Send66Cmd104[GUN_A].work_stat = 3;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_STOPING)
	{
		Send66Cmd104[GUN_A].work_stat = 7;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)
	{
		Send66Cmd104[GUN_A].work_stat = 6;
	}else
	{
		if(GetGunState(GUN_A) == GUN_DISCONNECTED)
		{
			Send66Cmd104[GUN_A].work_stat = 0;		//空闲
		}else
		{
			Send66Cmd104[GUN_A].work_stat = 1;		//已经插枪
		}
	}
	//车辆连接状态
	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
	{
		Send66Cmd104[GUN_A].car_connection_stat = 0;		//空闲
	}else
	{
		Send66Cmd104[GUN_A].car_connection_stat = 2;		//已经插枪
	}	
	
	
	if(Send66Cmd104[GUN_A].work_stat == 2)
	{
		Send66Cmd104[GUN_A].book_start_charge_time[0] = 0x20;
		Send66Cmd104[GUN_A].book_start_charge_time[1] = puserinfo->StartTime.Year;
		Send66Cmd104[GUN_A].book_start_charge_time[2] = puserinfo->StartTime.Month;
		Send66Cmd104[GUN_A].book_start_charge_time[3] = puserinfo->StartTime.Day;
		Send66Cmd104[GUN_A].book_start_charge_time[4] = puserinfo->StartTime.Hour;
		Send66Cmd104[GUN_A].book_start_charge_time[5] = puserinfo->StartTime.Minute;
		Send66Cmd104[GUN_A].book_start_charge_time[6] = puserinfo->StartTime.Second;
		Send66Cmd104[GUN_A].soc_percent = BMS_BCS_Context[GUN_A].SOC; 				//当前spc
		Send66Cmd104[GUN_A].cumulative_charge_fee = puserinfo->TotalBill/100;   			//充电电费
		Send66Cmd104[GUN_A].dc_charge_voltage = PowerModuleInfo[GUN_A].OutputInfo.Vol;			//电压
		Send66Cmd104[GUN_A].dc_charge_current  =  PowerModuleInfo[GUN_A].OutputInfo.Cur;	//电流
		Send66Cmd104[GUN_A].bms_need_voltage = BMS_BCL_Context[GUN_A].DemandVol;			//需求电压
		Send66Cmd104[GUN_A].bms_need_current = 4000 - BMS_BCL_Context[GUN_A].DemandCur;	//需求电流
		Send66Cmd104[GUN_A].bms_charge_mode = 0;										//自动充满
		Send66Cmd104[GUN_A].charge_full_time_left = BMS_BCS_Context[GUN_A].RemainderTime;
		Send66Cmd104[GUN_A].charged_sec = puserinfo->ChargeTime * 60;
		Send66Cmd104[GUN_A].cum_charge_kwh_amount = puserinfo->TotalPower4/10;			//累计充电电量
		Send66Cmd104[GUN_A].before_charge_meter_kwh_num = pmeter->StartPower4/10;		//开始充电读表
		Send66Cmd104[GUN_A].now_meter_kwh_num = pmeter->CurPower4/10;					//当前充电读表
		Send66Cmd104[GUN_A].charge_power_kw = Send66Cmd104[GUN_A].dc_charge_voltage * Send66Cmd104[GUN_A].dc_charge_current / 10000; 
		memcpy(Send66Cmd104[GUN_A].charge_user_id,Recv66Cmd7[GUN_A].charge_user_id,32);		//订单号
	}
	else
	{
		;
	}
	return _66FreamSend(_66_cmd_type_104,(INT8U*)&Send66Cmd104[GUN_A],sizeof(_66_cmd_104));
}

/*****************************************************************************
* Function     : HFQG_SendDevStateB
* Description  : 充电桩B状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _66_SendDevStateB(void)
{
	TEMPDATA * pmeter = NULL;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	INT8U * pdevnum = APP_GetDevNum(); 
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	pmeter = APP_GetMeterPowerInfo(GUN_B);
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	if(APP_GetGunNum() == 1)
	{
		return FALSE;
	}
	memset(&Send66Cmd104[GUN_B],0,sizeof(_66_cmd_104));
	memcpy(Send66Cmd104[GUN_B].equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd104[GUN_B].gun_cnt = APP_GetGunNum();					//充电枪数量
	Send66Cmd104[GUN_B].gun_index = 1;						//充电口号
	Send66Cmd104[GUN_B].gun_type = 1;						//充电枪类型  1=直流； 2=交流；
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		Send66Cmd104[GUN_B].work_stat = 2;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_UNLINK)
	{
		Send66Cmd104[GUN_B].work_stat = 3;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_STOPING)
	{
		Send66Cmd104[GUN_B].work_stat = 7;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)
	{
		Send66Cmd104[GUN_B].work_stat = 6;
	}else
	{
		if(GetGunState(GUN_B) == GUN_DISCONNECTED)
		{
			Send66Cmd104[GUN_B].work_stat = 0;		//空闲
		}else
		{
			Send66Cmd104[GUN_B].work_stat = 1;		//已经插枪
		}
	}
	//车辆连接状态
	if(GetGunState(GUN_B) == GUN_DISCONNECTED)
	{
		Send66Cmd104[GUN_B].car_connection_stat = 0;		//空闲
	}else
	{
		Send66Cmd104[GUN_B].car_connection_stat = 2;		//已经插枪
	}	
	
	
	if(Send66Cmd104[GUN_B].work_stat == 2)
	{
		Send66Cmd104[GUN_B].book_start_charge_time[0] = 0x20;
		Send66Cmd104[GUN_B].book_start_charge_time[1] = puserinfo->StartTime.Year;
		Send66Cmd104[GUN_B].book_start_charge_time[2] = puserinfo->StartTime.Month;
		Send66Cmd104[GUN_B].book_start_charge_time[3] = puserinfo->StartTime.Day;
		Send66Cmd104[GUN_B].book_start_charge_time[4] = puserinfo->StartTime.Hour;
		Send66Cmd104[GUN_B].book_start_charge_time[5] = puserinfo->StartTime.Minute;
		Send66Cmd104[GUN_B].book_start_charge_time[6] = puserinfo->StartTime.Second;
		Send66Cmd104[GUN_B].soc_percent = BMS_BCS_Context[GUN_B].SOC; 				//当前spc
		Send66Cmd104[GUN_B].cumulative_charge_fee = puserinfo->TotalBill/100;   			//充电电费
		Send66Cmd104[GUN_B].dc_charge_voltage = PowerModuleInfo[GUN_B].OutputInfo.Vol;			//电压
		Send66Cmd104[GUN_B].dc_charge_current  =  PowerModuleInfo[GUN_B].OutputInfo.Cur;	//电流
		Send66Cmd104[GUN_B].bms_need_voltage = BMS_BCL_Context[GUN_B].DemandVol;			//需求电压
		Send66Cmd104[GUN_B].bms_need_current = 4000 - BMS_BCL_Context[GUN_B].DemandCur;	//需求电流
		Send66Cmd104[GUN_B].bms_charge_mode = 0;										//自动充满
		Send66Cmd104[GUN_B].charge_full_time_left = BMS_BCS_Context[GUN_B].RemainderTime;
		Send66Cmd104[GUN_B].charged_sec = puserinfo->ChargeTime * 60;
		Send66Cmd104[GUN_B].cum_charge_kwh_amount = puserinfo->TotalPower4/10;			//累计充电电量
		Send66Cmd104[GUN_B].before_charge_meter_kwh_num = pmeter->StartPower4/10;		//开始充电读表
		Send66Cmd104[GUN_B].now_meter_kwh_num = pmeter->CurPower4/10;					//当前充电读表
		Send66Cmd104[GUN_B].charge_power_kw = Send66Cmd104[GUN_B].dc_charge_voltage * Send66Cmd104[GUN_B].dc_charge_current / 10000; 
		memcpy(Send66Cmd104[GUN_B].charge_user_id,Recv66Cmd7[GUN_B].charge_user_id,32);		//订单号
	}
	else
	{
		;
	}
	return _66FreamSend(_66_cmd_type_104,(INT8U*)&Send66Cmd104[GUN_B],sizeof(_66_cmd_104));
}

/*****************************************************************************
* Function     : PreHYBill
* Description  : 保存汇誉订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   Pre66Bill(_GUN_NUM gun,INT8U *pdata)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	INT8U * pdevnum = APP_GetDevNum(); 
	INT8U errcode,i;			//故障码
	_66_cmd_202 * pbill = (_66_cmd_202 *)pdata;
	memset(pbill,0,sizeof(_66_cmd_202));
	_BSPRTC_TIME endtime;
	GetCurTime(&endtime); 

	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	for(i = 0;i < 48;i++)
	{
		pbill->kwh_amount[i] = puserinfo->kwh[i];
	}
	errcode = APP_GetStopChargeReason(gun);   //获取故障码
	if(errcode ==  STOP_SOC)
	{
		
		pbill->err_no = 0;
	}
	else if((errcode == STOP_ERR_NONE) ||(errcode ==  STOP_BSMNORMAL))
	{
		pbill->err_no = 200;  //用户终止
	}
	else if(STOP_HANDERR == errcode)
	{
		pbill->err_no = 200;
	}
	else if(STOP_EMERGENCY == errcode)
	{
		pbill->err_no = 302;
	}
	else
	{
		pbill->err_no = 0;
	}
	memcpy(pbill->equipment_id,pdevnum,16);	//充电桩编号
	pbill->gun_type = 0x01;		//充电枪类型 	1-直流 2-交流
	pbill->gun_index = gun;		//充电枪口
	memcpy(pbill->charge_user_id,Recv66Cmd7[gun].charge_user_id,32);  //订单号
	//开始时间
	pbill->charge_start_time[0] = 0x20;
	pbill->charge_start_time[1] = puserinfo->StartTime.Year;
	pbill->charge_start_time[2] = puserinfo->StartTime.Month;
	pbill->charge_start_time[3] = puserinfo->StartTime.Day;
	pbill->charge_start_time[4] = puserinfo->StartTime.Hour;
	pbill->charge_start_time[5] = puserinfo->StartTime.Minute;
	pbill->charge_start_time[6] = puserinfo->StartTime.Second;
	//结束时间
	pbill->charge_end_time[0] = 0x20;
	pbill->charge_end_time[1] = endtime.Year;
	pbill->charge_end_time[2] = endtime.Month;
	pbill->charge_end_time[3] = endtime.Day;
	pbill->charge_end_time[4] = endtime.Hour;
	pbill->charge_end_time[5] = endtime.Minute;
	pbill->charge_end_time[6] = endtime.Second;
	
	pbill->charge_time = puserinfo->ChargeTime * 60;			//充电时间长度  单位秒
	
	pbill->start_soc = BMS_BCP_Context[gun].BatterySOC;
	pbill->end_soc = BMS_BCS_Context[gun].SOC;
	//err_no[4];				//充电结束原因  错误含义见附录
	pbill->charge_kwh_amount  =  puserinfo->TotalPower4 / 10;
	pbill->start_charge_kwh_meter = pmeter->StartPower4 / 10; /*充电前电表读数*/
	pbill->end_charge_kwh_meter = pmeter->CurPower4 / 10; /* 充电后电表读数*/
	pbill->total_charge_fee = puserinfo->TotalBill/100;			//本次充电金额
	pbill->total_service_fee = puserinfo->TotalServeBill/100;			//本次充电总服务费
	pbill->charge_policy = Recv66Cmd7[gun].charge_policy;				//充电策略  0:充满为止1:时间控制充电2:金额控制充电3:电量控制充电4:SOC控制充电
	if(_66StartType[gun] == _4G_APP_START)					//0表示App启动 1表示刷卡启动
	{
		pbill->start_charge_type = 1;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
	}
	else
	{
		pbill->start_charge_type = 0;	
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
INT8U _66_SendBill(_GUN_NUM gun)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	INT8U * pdevnum = APP_GetDevNum(); 
	INT8U errcode,i;			//故障码
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(&Send66Cmd202,0,sizeof(Send66Cmd202));
	errcode = APP_GetStopChargeReason(gun);   //获取故障码
	if(errcode ==  STOP_SOC)
	{
		
		Send66Cmd202.err_no = 0;
	}
	else if((errcode == STOP_ERR_NONE) ||(errcode ==  STOP_BSMNORMAL))
	{
		Send66Cmd202.err_no = 200;  //用户终止
	}
	else if(STOP_HANDERR == errcode)
	{
		Send66Cmd202.err_no = 200;
	}
	else if(STOP_EMERGENCY == errcode)
	{
		Send66Cmd202.err_no = 302;
	}
	else
	{
		Send66Cmd202.err_no = 0;
	}
	for(i = 0;i < 48;i++)
	{
		Send66Cmd202.kwh_amount[i] = puserinfo->kwh[i];
	}
	memcpy(Send66Cmd202.equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd202.gun_type = 0x01;		//充电枪类型 	1-直流 2-交流
	Send66Cmd202.gun_index = gun;		//充电枪口
	memcpy(Send66Cmd202.charge_user_id,Recv66Cmd7[gun].charge_user_id,32);  //订单号
	//开始时间
	Send66Cmd202.charge_start_time[0] = 0x20;
	Send66Cmd202.charge_start_time[1] = puserinfo->StartTime.Year;
	Send66Cmd202.charge_start_time[2] = puserinfo->StartTime.Month;
	Send66Cmd202.charge_start_time[3] = puserinfo->StartTime.Day;
	Send66Cmd202.charge_start_time[4] = puserinfo->StartTime.Hour;
	Send66Cmd202.charge_start_time[5] = puserinfo->StartTime.Minute;
	Send66Cmd202.charge_start_time[6] = puserinfo->StartTime.Second;
	//结束时间
	Send66Cmd202.charge_end_time[0] = 0x20;
	Send66Cmd202.charge_end_time[1] = puserinfo->EndTime.Year;
	Send66Cmd202.charge_end_time[2] = puserinfo->EndTime.Month;
	Send66Cmd202.charge_end_time[3] = puserinfo->EndTime.Day;
	Send66Cmd202.charge_end_time[4] = puserinfo->EndTime.Hour;
	Send66Cmd202.charge_end_time[5] = puserinfo->EndTime.Minute;
	Send66Cmd202.charge_end_time[6] = puserinfo->EndTime.Second;
	
	Send66Cmd202.charge_time = puserinfo->ChargeTime * 60;			//充电时间长度  单位秒
	Send66Cmd202.start_soc = BMS_BCP_Context[gun].BatterySOC;
	Send66Cmd202.end_soc = BMS_BCS_Context[gun].SOC;
	//err_no[4];				//充电结束原因  错误含义见附录
	Send66Cmd202.charge_kwh_amount  = puserinfo->TotalPower4 / 10;
	Send66Cmd202.start_charge_kwh_meter = pmeter->StartPower4 / 10; /*充电前电表读数*/
	Send66Cmd202.end_charge_kwh_meter = pmeter->CurPower4 / 10; /* 充电后电表读数*/
	Send66Cmd202.total_charge_fee = puserinfo->TotalBill/100;			//本次充电金额
	Send66Cmd202.total_service_fee = puserinfo->TotalServeBill/100;			//本次充电总服务费
	Send66Cmd202.charge_policy = Recv66Cmd7[gun].charge_policy;				//充电策略  0:充满为止1:时间控制充电2:金额控制充电3:电量控制充电4:SOC控制充电
	
	if(_66StartType[gun] == _4G_APP_START)					//0表示App启动 1表示刷卡启动
	{
		Send66Cmd202.start_charge_type = 1;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
	}
	else
	{
		Send66Cmd202.start_charge_type = 0;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
	}
	
	WriterFmBill(gun,1);
	ResendBillControl[gun].CurTime = OSTimeGet();	
	ResendBillControl[gun].LastTime = ResendBillControl[gun].CurTime;
	return _66FreamSend(_66_cmd_type_202,(INT8U*)&Send66Cmd202,sizeof(_66_cmd_202));
}


/*****************************************************************************
* Function     : _66_SendQuerySetAck
* Description  : 查询设置返回
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _66_SendQuerySetAck(void)
{
//		uint8_t    res[4];			//预留
//	uint8_t equipment_id[32];		//充电桩编码
//	uint8_t Tpye;				//0 查询  1设置
//	uint32_t Sadd;				//查询启始地址
//	uint8_t	Snum;				//查询个数
//	uint8_t Result;				//0表示成功  1表示失败
//	uint8_t SetDate[256];		//最长256
	
//	_66_cmd_2	Send66Cmd2;				//查询设置返回
	INT8U * pdevnum = APP_GetDevNum(); 

	memset(&Send66Cmd4,0,sizeof(Send66Cmd4));
	memcpy(Send66Cmd4.equipment_id,pdevnum,16);	//充电桩编号
	Send66Cmd4.Tpye = Recv66Cmd3.Tpye;
	Send66Cmd4.Sadd = Recv66Cmd3.Sadd;
	Send66Cmd4.Result = 0;
	memcpy(Send66Cmd4.SetDate,Recv66Cmd3.SetDate,Recv66Cmd3.datalen);
	return _66FreamSend(_66_cmd_type_4,(INT8U*)&Send66Cmd4,sizeof(Send66Cmd4) + Recv66Cmd3.datalen - 256);
}

/******************************************接收函数*******************************************/
/*****************************************************************************
* Function     : _66_RecvSetInfo
* Description  : 接收设置  主要是二维码
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   _66_RecvSetInfo(INT8U * pdata,INT16U len)
{
	static _BSP_MESSAGE Msg;
		_BSPRTC_TIME SetTime;                       //设定时间
	INT8U times = 3;							//如果设置失败反复设置三次
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if((pdata == NULL) || (len < 11))
	{
		return FALSE;
	}
	
	memcpy(&Recv66Cmd3,pdata,len);
	if(Recv66Cmd3.datalen > 256)
	{
		return FALSE;
	}
	if (Recv66Cmd3.Sadd == 11) 
	{
		memcpy(&Qrcode[GUN_A][0],Recv66Cmd3.SetDate,Recv66Cmd3.datalen);
	}
	if (Recv66Cmd3.Sadd == 12) 
	{
		memcpy(&Qrcode[GUN_B][0],Recv66Cmd3.SetDate,Recv66Cmd3.datalen);
	}
	if(Recv66Cmd3.Sadd == 2) //时间设置
	{
		SetTime.Year   = Recv66Cmd3.SetDate[1];//将输入的十进制数转换成BCD
		SetTime.Month  = Recv66Cmd3.SetDate[2];
		SetTime.Day    = Recv66Cmd3.SetDate[3];
		SetTime.Hour   = Recv66Cmd3.SetDate[4];
		SetTime.Minute = Recv66Cmd3.SetDate[5];
		SetTime.Second = Recv66Cmd3.SetDate[6];
		
		while(times--)
		{
			if(BSP_RTCSetTime(&SetTime) == TRUE)    //设置RTC
			{
				break;
			}                                 
		}
	}
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_SET_ACK;         //查询实时数据
	
	OSQPost(psendevent, &Msg);
	return TRUE;
}

/*****************************************************************************
* Function     : HFQG_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   _66_RecvFrameDispose(INT8U * pdata,INT16U len)
{
	INT8U i = 0;
	INT16U cmd;
	INT16U datalen;
	static INT8U buf[500];
	INT8U *pframedata;

	if((pdata == NULL) || (len < 9) )
	{
		return FALSE;
	}
	//数据分帧
	pframedata = pdata;
	while(len > 9)
	{
		//帧头帧尾判断
		if((pframedata[0] != 0xaa) || (pframedata[1] != 0xf7) )
		{
			return FALSE;
		}
		//提取数据长度
		datalen= pframedata[2] | (pframedata[3] << 8);
		if((datalen >500) || (datalen < 9) ) 
		{
			return FALSE;
		}
		if(datalen > len)
		{
			return FALSE;
		}
		cmd = (pframedata[7] << 8) | pframedata[6];  //提取命令字
		datalen = datalen - 9;
		//提取数据
		memcpy(buf,&pframedata[8],datalen);
		
		for(i = 0;i < _66_RECV_FRAME_LEN;i++)
		{
			if(_66RecvTable[i].cmd == cmd)
			{
				if(_66RecvTable[i].recvfunction != NULL)
				{
					_66RecvTable[i].recvfunction(buf,datalen);
				}
				break;
			}
		}
		len =len -  datalen - 9;
		pframedata = &pdata[datalen+9];
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvStartCharge
* Description  : 启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_RecvStartCharge(INT8U *pdata,INT16U len)
{
	INT8U gun;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	 static _BSP_MESSAGE SendMsg[GUN_MAX];
	_66_cmd_7 * pcmd7 = (_66_cmd_7 *)pdata;
	if(pdata == NULL) 
	{
		return FALSE;
	}
	gun = pcmd7->gun_index;
	 ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
	ResendBillControl[gun].SendCount = 0;
	if(pcmd7->gun_index == 0)
	{
		gun = GUN_A;
		memcpy(&Recv66Cmd7[GUN_A],pcmd7,sizeof(_66_cmd_7));
		SendMsg[GUN_A].MsgID = BSP_4G_RECV;
		SendMsg[GUN_A].DataLen = GUN_A;
		SendMsg[GUN_A].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_A]);
	}
	else if(pcmd7->gun_index == 1)
	{
		gun = GUN_B;
		memcpy(&Recv66Cmd7[GUN_B],pcmd7,sizeof(_66_cmd_7));
		SendMsg[GUN_B].MsgID = BSP_4G_RECV;
		SendMsg[GUN_B].DataLen = GUN_B;
		SendMsg[GUN_B].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_B]);
	}
	

	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvStopCharge
* Description  : 停止应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_RecvStopCharge(INT8U *pdata,INT16U len)
{
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	 static _BSP_MESSAGE SendMsg[GUN_MAX];
	_66_cmd_5 * pcmd5 = (_66_cmd_5 *)pdata;
	if(pdata == NULL)
	{
		return FALSE;
	}
	if(pcmd5->Sadd != 2)   //其实地址必须为2
	{
		return FALSE;
	}
	
	if(pcmd5->gun == 0)
	{
		gun = GUN_A;
		SendMsg[GUN_A].MsgID = BSP_4G_RECV;
		SendMsg[GUN_A].DataLen = GUN_A;
		SendMsg[GUN_A].DivNum = APP_STOP_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_A]);
	}
	else if(pcmd5->gun == 1)
	{
		gun = GUN_B;
		SendMsg[GUN_B].MsgID = BSP_4G_RECV;
		SendMsg[GUN_B].DataLen = GUN_B;
		SendMsg[GUN_B].DivNum = APP_STOP_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_B]);
	}
	else
	{
		return FALSE;
	}
	
	//发送结算
	Msg[gun].MsgID = BSP_4G_MAIN;
	Msg[gun].DataLen = gun;
	Msg[gun].DivNum = APP_STOP_ACK;
	OSQPost(psendevent, &Msg[gun]);

	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvHearAck
* Description  : 心跳应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_RecvHearAck(INT8U *pdata,INT16U len)
{
	if((pdata == NULL) || len != sizeof(_66_cmd_101))
	{
		return FALSE;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvDevInfoAck
* Description  : 设备信息应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_RecvDevInfoAck(INT8U *pdata,INT16U len)
{
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	_66_cmd_103 * pcmd103;
	if((pdata == NULL) || len != sizeof(_66_cmd_103))
	{
		return FALSE;
	}
	
//	pcmd103 = (_66_cmd_103 *)pdata;
//	if(pcmd103->gun_index >= GUN_MAX)
//	{
//		return FALSE;
//	}
//	if(pcmd103->IfAck == 1) //查询
//	{
//		gun = pcmd103->gun_index;
//		Msg[gun].MsgID = BSP_4G_MAIN;
//		Msg[gun].DataLen = gun;
//		Msg[gun].DivNum = APP_STE_DEVS;   //发送查询设备状态
//		OSQPost(psendevent, &Msg[gun]);
//	}
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvStartCharge
* Description  : 启动应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_RecvRegisterAck(INT8U *pdata,INT16U len)
{
	
	_BSPRTC_TIME SetTime;                       //设定时间
	INT8U times = 3;							//如果设置失败反复设置三次
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if((pdata == NULL) || len != sizeof(_66_cmd_105))
	{
		return FALSE;
	}
	memcpy((INT8U*)&Recv66Cmd105,pdata,len);
	
	APP_SetAppRegisterState(LINK_NUM,STATE_OK);
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING)   //没在充电中读取
	{
		ReadFmBill(GUN_A);  //是否需要发送订单
	}
	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)   //没在充电中读取
	{
		ReadFmBill(GUN_B);  //是否需要发送订单
	}
//		//校准时间
//	SetTime.Year   = Recv66Cmd105.time[1];//将输入的十进制数转换成BCD
//    SetTime.Month  = Recv66Cmd105.time[2];
//    SetTime.Day    = Recv66Cmd105.time[3];
//    SetTime.Hour   = Recv66Cmd105.time[4];
//    SetTime.Minute = Recv66Cmd105.time[5];
//    SetTime.Second = Recv66Cmd105.time[6];
//	
//    while(times--)
//    {
//        if(BSP_RTCSetTime(&SetTime) == TRUE)    //设置RTC
//        {
//            return TRUE;
//        }                                 
//    }
//	
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvRecordAck
* Description  : 订单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   _66_RecvRecordAck(INT8U *pdata,INT16U len)
{
	INT8U gun;
	if((pdata == NULL) || (len != sizeof(Recv66Cmd201)))
	{
		return FALSE;
	}
	memcpy(&Recv66Cmd201,pdata,sizeof(Recv66Cmd201));

	gun = Recv66Cmd201.gun_index;
	
	
	ResendBillControl[gun].ResendBillState = FALSE;			//订单确认，不需要重发订单
	ResendBillControl[gun].SendCount = 0;
	WriterFmBill((_GUN_NUM)gun,0);
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvDevStopAck
* Description  : 设备主动停止应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U    _66_RecvDevStopAck(INT8U *pdata,INT16U len)
{
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvDevFailAck
* Description  : 故障状态上报应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U    _66_RecvDevFailAck(INT8U *pdata,INT16U len)
{
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvQueryBill
* Description  : 查询最新的一次记录
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U    _66_RecvQueryBill(INT8U *pdata,INT16U len)
{
	//充电桩编码 16   充电口号  1
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if(len != 17 )
	{
		return FALSE;
	}
	gun = pdata[16];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	Msg[gun].MsgID = BSP_4G_MAIN;
	Msg[gun].DataLen = gun;
	Msg[gun].DivNum = APP_STE_BILL;   //发送查询订单
	OSQPost(psendevent, &Msg[gun]);
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvQueryRate
* Description  : 查询费率
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U    _66_RecvQueryRate(INT8U *pdata,INT16U len)
{
	//充电桩编码 16   充电口号  1
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if(len != 16 )
	{
		return FALSE;
	}

	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_STE_RATE;   //发送查询订单
	OSQPost(psendevent, &Msg);
	return TRUE;
}





/*****************************************************************************
* Function     : HY_RecvCardStart
* Description  : 开启动返回
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U    _66_RecvCardStart(INT8U *pdata,INT16U len)
{
	 _66_cmd_33 *pcmd33;
	if(pdata == NULL)
	{
		return FALSE;
	}
	if(len != sizeof( _66_cmd_33) )
	{
		return FALSE;
	}
	pcmd33 =( _66_cmd_33 *) pdata;
	if(pcmd33->gun_index >= GUN_MAX)
	{
		return FALSE;
	}
	memcpy(&Recv66Cmd33[pcmd33->gun_index],pdata,sizeof( _66_cmd_33));
	if(Recv66Cmd33[pcmd33->gun_index].card_ifsuccess == 0)
	{
		 _66StartType[pcmd33->gun_index] = _4G_APP_CARD;
	}
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
	OSTimeDly(SYS_DELAY_50ms);	
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	return TRUE;
	
}

/*****************************************************************************
* Function     : HY_RecvRateSet
* Description  : 费率设置
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U    _66_RecvRateSet(INT8U *pdata,INT16U len)
{
	INT8U i = 0;
	_FLASH_OPERATION  FlashOper;
	USERINFO * puserinfoA  = GetChargingInfo(GUN_A);
	USERINFO * puserinfoB  = GetChargingInfo(GUN_B);
	
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	
	if((pdata == NULL) || (len != sizeof(_66_cmd_1303)))
	{
		return FALSE;
	}
	memcpy(&Recv66Cmd1303,pdata,len);
	
	

	for(i = 0;i < 48;i++)
	{
		puserinfoA->pric[i] = Recv66Cmd1303.pric[i] * 100;
		puserinfoB->pric[i] = Recv66Cmd1303.pric[i] * 100;
	}		
	
	//费率设置应答
	Msg.DataLen = 1;
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_RATE_ACK;
	OSQPost(psendevent, &Msg);
	return TRUE;
}

/*****************************************************************************
* Function     : HY_RecvRateSet
* Description  : 费率设置
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U    _66_RecvRateFWSet(INT8U *pdata,INT16U len)
{
	INT8U i = 0;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	static _BSP_MESSAGE Msg;
	USERINFO * puserinfo;
	if((pdata == NULL) || (len != sizeof(_66_cmd_1305)))
	{
		return FALSE;
	}
	memcpy(&Recv66Cmd1305,pdata,len);
	
	if(Recv66Cmd1305.gun >= GUN_MAX)
	{
		return FALSE;
	}
	puserinfo  = GetChargingInfo(Recv66Cmd1305.gun);

	for(i = 0;i < 48;i++)
	{
		puserinfo->fwpric[i] = Recv66Cmd1305.FWpric * 100;
	}		
	
	//费率设置应答
	Msg.DataLen = 0;
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_RATE_ACK;
	OSQPost(psendevent, &Msg);
	return TRUE;
}

/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

