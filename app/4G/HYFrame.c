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
#include "HYFrame.h"
#include <string.h>
#include "sysconfig.h"
#include "DisplayMain.h"
#include "charging_Interface.h"
#include "RTC_task.h"
#include "PCUMain.h"
#include "DisplayMain.h"
#include "card_cpu.h"
#include "card_Interface.h"
#include "connectbms.h"
/* Private define-----------------------------------------------------------------------------*/
#define HY_SEND_FRAME_LEN   4
#define HY_RECV_FRAME_LEN	14

#define ADD_FS_KWH      1  //不增加   1 增加


typedef enum{
	

	hy_cmd_type_7 	= 7,
	hy_cmd_type_8 	= 8,
	hy_cmd_type_11	= 11,
	hy_cmd_type_12 	= 12,
	hy_cmd_type_33 	= 33,
	hy_cmd_type_34	= 34,
	hy_cmd_type_101	= 101,
	hy_cmd_type_102 = 102,
	hy_cmd_type_103	= 103,
	hy_cmd_type_104	= 104,
	hy_cmd_type_105 = 105,
	hy_cmd_type_106	= 106,
	hy_cmd_type_201	= 201,
	hy_cmd_type_202	= 202,
	hy_cmd_type_107 = 107,
	hy_cmd_type_108	= 108,
	hy_cmd_type_113 = 113,
	hy_cmd_type_114 = 114,	
	hy_cmd_type_117	= 117,
	hy_cmd_type_118	= 118,
	hy_cmd_type_1101	= 1101,
    hy_cmd_type_1102	= 1102,
	hy_cmd_type_1301	= 1301,
    hy_cmd_type_1302	= 1302,
	hy_cmd_type_1309	= 1309,
	hy_cmd_type_1310	= 1310,
	hy_cmd_type_1311	= 1311,
	hy_cmd_type_1312	= 1312,
}hy_mqtt_cmd_enum;

//注册
__packed typedef struct{
 uint8_t      	reg;			//0：无充电桩编号1：成功/
 uint8_t     	time[8];		//同步充电桩时间
}hy_cmd_105;

//106
__packed typedef struct {
  uint16_t	charge_mode_num; 			//该充电桩总共电源模块数
  uint16_t  charge_mode_rate;			//充电桩电源模块总功率大小 0.1kW
  uint8_t   equipment_id[16];			//充电桩编码	
  uint8_t   offline_charge_flag;		//离线/在线允许充电设置 默认值为0x01，其中高位的0表示在线允许充电，低位的1表示离线禁止充电。如果低位设0，则表示离线允许充电
  uint8_t	stake_version[4];			//充电桩软件版本
  uint8_t  stake_type;					//充电桩类型0x00 直流0x01 交流0x02 混合
  uint32_t  stake_start_times;			//启动次数 终端每次启动，计数保存
  uint8_t   data_up_mode;				//终端每次启动，计数保存   1：应答模式2：主动上报模式
  uint16_t  sign_interval;				//签到间隔时间
  uint8_t   gun_index;					//充电枪个数
  uint8_t   heartInterval;				//心跳上报周期
  uint8_t   heart_out_times;			//心跳包检测超时次数
  uint32_t	stake_charge_record_num;	//充电记录数量
  uint8_t   stake_systime[8];			//当前充电桩系统时间
  uint8_t   stake_last_charge_time[8];	//最近一次充电时间
  uint8_t   stake_last_start_time[8];	//最近一次启动时间
  uint8_t   signCode[8];				//签到密码（保留）
  uint8_t  ccu_version[4];				//充电桩CCU软件版本
}hy_cmd_106;


//启停
__packed typedef struct{
	uint32_t	money;						//账户余额 0.01
    uint16_t user_tel;						//用户手机后4位
	uint8_t	gun_index;						//充电枪口  从0开始
	uint8_t	charge_policy;				//0:充满为止（默认）1:时间控制充电2:金额控制充电3:电量控制充电4:按SOC控制充电
	uint32_t		charge_policy_param;	//时间单位为1秒金额单位为0.01元 电量时单位为0.01kw SOC单位为%
	uint8_t	book_time[8];					//预约/定时启动时间
	uint8_t	book_delay_time;				//预约超时时间（保留）单位分钟(预约时有效)
	uint8_t	charge_user_id[32];				//订单号
	uint8_t	allow_offline_charge;			//断网充电标志		0-不允许 1-允许，默认0
	int32_t		allow_offline_charge_kw_amout;//离线可充电电量     0.01kw
	uint8_t	charge_delay_fee;					//是否收取延时费	0-不收取1—收取，默认0
	int32_t		charge_delay_wait_time;			//延时费等待时间  时间单位:分钟min
}hy_cmd_7;

__packed typedef struct{
	uint8_t equipment_id[16];			//充电桩编码
	uint8_t gun_index;					//充电枪口   同服务发送枪口 从0开始
	uint8_t result[4];					//命令执行结果  错误含义见附录：错误码对照表
	uint8_t	charge_user_id[32];			//订单号
}hy_cmd_8;


//服务器终止订单
__packed typedef struct{
	uint8_t equipment_id[16];			//充电桩编码
	uint8_t gun_index;					//充电枪口	从0开始
	uint8_t charge_seq[32];				//订单号
}hy_cmd_11;
// 12
__packed typedef struct{
	uint8_t equipment_id[16];			//充电桩编码
	uint8_t gun_index;					//充电枪口
	uint8_t charge_seq[32];				//订单号 从0开始
} hy_cmd_12;


//充电桩状态
//103
__packed typedef struct{
	uint8_t	gun_index;					//充电口号
	uint8_t IfAck;						//是否查询    0：应答 1查询 2订单不存在
}hy_cmd_103;
//104
__packed typedef struct{				
	uint8_t	equipment_id[16];			//充电桩编码
	uint8_t	gun_cnt;					//充电枪数量
	uint8_t	gun_index;					//充电口号 从0开始
	uint8_t 	gun_type;				//充电枪类型    1=直流； 2=交流；
	uint8_t 	work_stat;				//工作状态0-空闲中 1-已插枪2-充电进行中 3-未拔枪4-预约状态5-自检6-系统故障7-停止中
	uint8_t  soc_percent;				//当前SOC %
	uint8_t  alarm_stat[4];				//告警状态（保留） 0-断开 1-半连接 2-连接直流目前只有0和2状态交流目前有0、1、2三种状态
	uint8_t car_connection_stat;		//车辆连接状态
	uint32_t cumulative_charge_fee;		//本次充电累计充电费用  从本次充电开始到目前的累计充电费用（包括电费与服务费），这里是整型，要乘以0.01才能得到真实的金额
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
	uint32_t cum_charge_money_amount; /*累计充电金额 */
	uint32_t before_charge_meter_kwh_num; /*充电前电表读数 */
	uint32_t now_meter_kwh_num; /* 当前电表读数 */

	uint8_t start_charge_type;				//充电启动方式   0：本地刷卡启动1：后台启动2：本地管理员启动
	uint8_t charge_policy;					//充电策略  0自动充满 1按时间充满  2定金额  3按电量充满 4按SOC充（直流）  
	int32_t charge_policy_param;			//充电策略参数 时间单位为1秒金额单位为0.01元电量时单位为0.01kw SOC为1%
	uint8_t book_flag;						//预约标志  0-无预约（无效）  1-预约有效
	uint8_t charge_user_id[32];				//订单号
	uint8_t book_timeout_min;				//预约超时时间    单位分钟
	uint8_t book_start_charge_time[8];		//预约/开始充电开始时间
	int32_t before_charge_card_account;		//充电前卡余额（保留）   改为充电电量3位小数 20231228
	int32_t charge_power_kw; /* 充电功率 */	//充电功率 0.1kw
} hy_cmd_104;



//心跳
//101
__packed typedef struct{
	uint16_t userkw;		//0.1kw 可使用功率
} hy_cmd_101;
//102
__packed typedef struct{
	uint8_t	equipment_id[16];	//充电桩编码
	uint16_t curkw;		//0.1kw 当前总功率
} hy_cmd_102;

//记录上传
// 201
__packed typedef struct{		
	uint8_t	gun_index;				//充电枪号 从0开始
	uint8_t 	user_id[32];		//充电卡号/用户号
}hy_cmd_201;
//202
__packed typedef struct{
	uint8_t equipment_id[16];		//充电桩编码
	uint8_t gun_type;				//充电枪类型 	1-直流 2-交流
	uint8_t gun_index;				//充电枪口
	uint8_t charge_user_id[32];		//订单号		
	uint8_t charge_start_time[8];	//充电开始时间
	uint8_t charge_end_time[8];		//充电结束时间
	uint32_t charge_time;			//充电时间长度  单位秒
	uint8_t start_soc;				//开始SOC
	uint8_t end_soc;				//结束SOC
	uint8_t err_no[4];				//充电结束原因  错误含义见附录
	
	uint32_t charge_kwh_amount; /*充电电量 */

	uint32_t start_charge_kwh_meter; /*充电前电表读数*/
	uint32_t end_charge_kwh_meter; /* 充电后电表读数*/

	uint32_t total_charge_fee;			//本次充电金额
	
	//卡相关
	uint32_t is_not_stoped_by_card;		//是否不刷卡结束（保留)  这个字段只有在刷卡充电时有效0--否（刷卡结束） 1--是（不刷卡直接拔枪结束）2—用户点击充电桩本地指令
	uint32_t start_card_money;				//改为充电电量 3位小数  20231228
	uint32_t end_card_money;
	uint32_t total_service_fee;
	uint8_t is_paid_by_offline;
	
	uint8_t charge_policy;				//充电策略  0:充满为止1:时间控制充电2:金额控制充电3:电量控制充电4:SOC控制充电
	uint32_t charge_policy_param;		//充电策略参数时间单位为1秒金额单位为0.01元电量时单位为0.01kwSOC单位1%
	uint8_t car_vin[17];				//车辆 VIN	
	uint8_t car_plate_no[8];	
	//车牌号
	#if ADD_FS_KWH
	/* 分时电量 uint16->uint32 */
	uint16_t kwh[48];				//时段电量  0.01
	#else
	uint16_t kwh1;				//时段电量  0.01
	uint16_t kwh2;				//时段电量  0.01
	uint16_t kwh3;				//时段电量  0.01
	uint16_t kwh4;				//时段电量  0.01
	#endif

	uint8_t start_charge_type;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
} hy_cmd_202;

//事件信息上报
//107
__packed typedef struct {
  uint8_t    equipment_id[16];  //充电桩编码
  uint8_t	gun_index;			//充电枪口  从0开始
  uint8_t	errCode[4];			//故障错误码
}hy_cmd_107;


//108
__packed typedef struct {
  uint8_t    equipment_id[16];		//充电桩编码
  uint8_t	gun_index;				//充电枪口 从0开始
  uint8_t	err_code[4];			//故障错误码
  uint8_t	err_status;				//故障状态   0x00-故障发生，0x01-故障已经恢复
  uint8_t charge_user_id[32];		//订单号		
}hy_cmd_108;



//117
__packed typedef struct {
  uint8_t    equipment_id[16];  //充电桩编码
  uint8_t	gun_index;			//充电枪口  从0开始
  uint8_t	errCode[4];			//故障错误码
}hy_cmd_117;


//118
__packed typedef struct {
  uint8_t    equipment_id[16];		//充电桩编码
  uint8_t	gun_index;				//充电枪口  从0开始
  uint8_t	err_code[4];			//故障错误码
  uint8_t	err_status;				//故障状态   0x00-故障发生，0x01-故障已经恢复	
}hy_cmd_118;

//远程升级
__packed typedef struct {
    uint8_t    equipment_id[16];		//充电桩编码
    uint8_t    Devtype;		//桩型号
    uint8_t 	ServerAddr[64];		//服务器地址  （ASCII 码）不足 32 位补零
    uint16_t 	Port;			//端口
    uint8_t     usename[16];	//用户名  （ASCII 码）不足 16 位补零
    uint8_t     password[16];	//密码  （ASCII 码）不足 16 位补零
    uint8_t		FilePath[128];	//文件路径   （ASCII 码）不足 32 位补零，文件路径名由平台定义
    uint8_t		cmd;			//0x01：立即执行 0x02：空闲执行
    uint8_t		downloadtime;	//下载超时时间 min
    uint8_t     crc;			//文件校验
} hy_cmd_1101;

//1310
__packed typedef struct {
    uint8_t   equipment_id[16];		//充电桩编码
    uint8_t		UpdataState;			//升级状态   0x00-成功 0x01-编号错误 0x02-程序与桩型号不符 0x03-下载更新文件超时  0x04-等待升级
    uint8_t   version[5];   //版本号
} hy_cmd_1102 ;

//1309
__packed typedef struct {
	//INT8U PricGtoupNum;			//分组数
	INT8U StartNum;				//起始分段编号
	INT8U Num;					//连续分段个数
	INT16U DMoney;				//电费 0.01
	INT16U FWMoney;				//服务费 0.01
	INT16U YCMoney;				//延时费用
}hy_cmd_1309;


//1310
__packed typedef struct {
	INT8U result;		//0 是成功   1长度错误 2分段数量错误 3分组数错误	
}hy_cmd_1310 ;


//34 充电桩刷卡q鉴权请求
__packed typedef struct {
	uint8_t    equipment_id[16];		//充电桩编码
	uint8_t	gun_index;				//充电枪口 从0开始
	uint8_t card_num[16];            //充电卡卡号   ASCII
	uint8_t card_rad[48];			//充电中随机数	ASCII
	uint32_t card_id;				//物理卡号  
}hy_cmd_34 ;


//33 服务器回复刷卡鉴权结果
__packed typedef struct {
	uint8_t    equipment_id[16];		//充电桩编码
	uint8_t		gun_index;				//充电枪口 从0开始
	uint16_t	card_ifsuccess;			//0成功  非0失败
	uint32_t    card_money;				//卡内余额
}hy_cmd_33 ;




// (CODE=1302)充电桩向后台服务器发送VIN鉴权信息
__packed typedef struct {
	uint8_t    equipment_id[16];		//充电桩编码
	uint8_t	gun_index;				//充电枪口 从0开始
	uint8_t code[16];            //输入密码
	uint8_t card_vin[17];			//充电桩唯一标识
	uint8_t cmd;				//1=启动充电,0非启动充电
}hy_cmd_1302;


// (CODE=1301)后台服务器向充电桩返回VIN鉴权信息
__packed typedef struct {
	uint8_t    equipment_id[16];		//充电桩编码
	uint8_t		gun_index;				//充电枪口 从0开始
	uint8_t card_vin[17];			//充电桩唯一标识
	uint32_t    vin_money;				//账户余额 0.01
	uint8_t	ifsuccess;				//鉴权是否成功
	uint16_t failcause;				//鉴权失败原因
}hy_cmd_1301 ;

//发送数据
hy_cmd_106 	SendHYCmd106;			//注册
hy_cmd_8	SendHYCmd8;				//启动应答
hy_cmd_12	SendHYCmd12;			//停止应答
hy_cmd_102	SendHYCmd102;			//上传心跳
hy_cmd_104	SendHYCmd104[GUN_MAX];			//状态上报	
hy_cmd_202	SendHYCmd202;			//上传充电记录
hy_cmd_108	SendHYCmd108;			//充电桩故障上报（充电过程中产生）
hy_cmd_118	SendHYCmd118;			//充电桩故障上报
hy_cmd_1310 SendHYCmd1310;			//计费设置相应
hy_cmd_34	SendHYCmd34[GUN_MAX];			//卡鉴权回复
hy_cmd_1102 SendHYCmd1102;			//升级回复
hy_cmd_1302	SendHYCmd1302[GUN_MAX];			//发送鉴权信息
//接收函数
hy_cmd_105 	RecvHYCmd105;			//注册
hy_cmd_7	RecvHYCmd7[GUN_MAX];	//启动
hy_cmd_11	RecvHYCmd11[GUN_MAX];	//停止
hy_cmd_101	RecvHYCmd101;			//心跳
hy_cmd_103	RecvHYCmd103[GUN_MAX];	//状态应答
hy_cmd_201	RecvHYCmd201;			//充电记录应答
hy_cmd_107	RecvHYCmd107;			//充电桩故障应答（充电过程中产生）
hy_cmd_117	RecvHYCmd117;			//充电桩故障应答
hy_cmd_1309 RecvHYCmd1309[12];		//费率设置	最多4组
hy_cmd_33	RecvHYCmd33[GUN_MAX];	//发送卡鉴权		
hy_cmd_1101 RecvHYCmd1101;	//接收到升级
hy_cmd_1301 RecvHYCmd1301[GUN_MAX];		//接收失败原因

_4G_START_TYPE HYStartType[GUN_MAX] ={ _4G_APP_START,_4G_APP_START};					//0表示App启动 1表示刷卡启动
INT8U RateGroup = 0;  //费率组数   
static INT8U   HY_SendRegister(void); //注册
static INT8U   HY_SendHear(void)  ;//心跳
INT8U   HY_SendDevStateA(void); //充电桩A状态
INT8U   HY_SendDevStateB(void);  //充电桩B状态
INT8U IfSendUpdataAck = 0;								//是否发送升级成功  0表示不发送   1表示发送  在收到心跳的时候去判断，这样能保证连上网络
extern _VIN_CONTROL VinControl[GUN_MAX];
extern _PRICE_SET PriceSet;							//电价设置 
_4G_SEND_TABLE HYSendTable[HY_SEND_FRAME_LEN] = {
	{0,    0,    SYS_DELAY_10s, 	HY_SendRegister			},  //发送注册帧

	{0,    0,    SYS_DELAY_30s, 	HY_SendHear				},	//心跳
	
	{0,    0,    SYS_DELAY_10s, 	HY_SendDevStateA		},	//充电桩A状态

	{0,    0,    SYS_DELAY_10s, 	HY_SendDevStateB		},	//充电桩B状态

};




static INT8U   HY_RecvStartCharge(INT8U *pdata,INT16U len);
static INT8U   HY_RecvStopCharge(INT8U *pdata,INT16U len);
static INT8U   HY_RecvHearAck(INT8U *pdata,INT16U len);
static INT8U   HY_RecvDevInfoAck(INT8U *pdata,INT16U len);
static INT8U   HY_RecvRegisterAck(INT8U *pdata,INT16U len);
static INT8U   HY_RecvRecordAck(INT8U *pdata,INT16U len);
static INT8U   HY_RecvRateSet(INT8U *pdata,INT16U len);
static INT8U   HY_RecvDevStopAck(INT8U *pdata,INT16U len);
static INT8U   HY_RecvDevFailAck(INT8U *pdata,INT16U len);
static INT8U   HY_RecvQueryBill(INT8U *pdata,INT16U len);
static INT8U   HY_RecvQueryRate(INT8U *pdata,INT16U len);
static INT8U   HY_RecvCardStart(INT8U *pdata,INT16U len);
static uint8_t   HY_RecvUpdata(uint8_t *pdata,uint16_t len);
static uint8_t   HY_RecvVin(uint8_t *pdata,uint16_t len);
_4G_RECV_TABLE HYRecvTable[HY_RECV_FRAME_LEN] = {
	{hy_cmd_type_7				,	HY_RecvStartCharge	}, 		//启动充电

	{hy_cmd_type_11				,  	HY_RecvStopCharge	},		//停止充电

	{hy_cmd_type_101			, 	HY_RecvHearAck		},		//心跳应答

	{hy_cmd_type_103			,  	HY_RecvDevInfoAck	}, 		//状态上报应答

	{hy_cmd_type_105			,  	HY_RecvRegisterAck	},		//注册应答

	{hy_cmd_type_201			,  	HY_RecvRecordAck	},		//记录应答

	{hy_cmd_type_107			,  	HY_RecvDevStopAck	},		//主动停止应答

	{hy_cmd_type_117			,  	HY_RecvDevFailAck	},		//故障上报
	
	{hy_cmd_type_1309			,  	HY_RecvRateSet		},		//费率设置
	
	{hy_cmd_type_113			,  	HY_RecvQueryBill	},		//查询最新一次的充电信息
	
	{hy_cmd_type_1311			,  	HY_RecvQueryRate	},		//查询费率
	
	{hy_cmd_type_33				,   HY_RecvCardStart	},		//接收到卡启动
	
	
    {hy_cmd_type_1101			,   HY_RecvUpdata		},		//远程升级接收
	
	{hy_cmd_type_1301			,   HY_RecvVin		},		//接收到VIN 鉴权

};

__packed typedef struct {
	INT8U Reason;
	INT8U CodeNum[4];
}_FAIL_CODE;		//后台协议与CCU传过来的启动完成帧，停止完成帧对应上
/*******************************启动失败原因*********************************/
#define TABLE_START_FAIL  19
#define TABLE_STOP_REASON  22
const static  _FAIL_CODE StartFailCode[TABLE_START_FAIL] = 
{
	{ENDFAIL_HANDERR ,			{0x31,0x30,0x30,0x44}},		//枪未正确连接  			100D
	
	{ENDFAIL_RECVTIMEOUT ,		{0x33,0x30,0x33,0x32}},		//计费控制单元通讯故障     	3032
	
	{ENDFAIL_EMERGENCY ,		{0x33,0x30,0x30,0x33}},		//急停 						3003
	
	{ENDFAIL_DOORERR ,			{0x33,0x30,0x30,0x32}},		//门禁 						3002
	
	{ENDFAIL_ELECLOCKERR ,		{0x33,0x30,0x31,0x38}},		//充电接口电子锁故障		3018
	
	{ENDFAIL_INSOLUTIONERR ,	{0x35,0x30,0x30,0x44}},		//BMS绝缘故障				500D
	
	{ENDFAIL_BATREVERSE ,		{0x35,0x30,0x31,0x41}},		//电池反接					501A

	{ENDFAIL_OUTSWERR2 ,		{0x33,0x30,0x30,0x46}},		//输出接触器粘连			300F
	
	{ENDFAIL_SAMELEVELSW2 ,		{0x33,0x30,0x32,0x44}},		//母联粘连故障				302D
	
	{ENDFAIL_LEAKOUTTIMEOUT ,	{0x33,0x30,0x31,0x35}},		//泄放回路故障				3015
	
	{ENDFAIL_OUTVOLTVORE ,		{0x34,0x30,0x30,0x31}},		//输出电压过压故障			4001
	
	{ENDFAIL_OUTVOLTUNDER ,		{0x34,0x30,0x30,0x33}},		//输出电压欠压故障			4003
	
	{ENDFAIL_OUTCURROVER ,		{0x34,0x30,0x30,0x32}},		//输出电压过流故障			4002
	
	{ENDFAIL_SHORTCIRCUIT ,		{0x34,0x30,0x30,0x32}},		//输出短路故障				4004
	
	{ENDFAIL_BRMTIMEOUT ,		{0x35,0x30,0x30,0x41}},		//BRM车辆辨识报文超时 		500A
	
	{ENDFAIL_BCPTIMEOUT ,		{0x35,0x30,0x30,0x31}},		//BRM车辆辨识报文超时 		5001
	
	{ENDFAIL_BRORUNTIMEOUT ,	{0x35,0x30,0x30,0x32}},		//BRO充电准备就绪报文超时 	5002
	
	{ENDFAIL_BATVOLTERR2 ,		{0x34,0x30,0x30,0x36}},		//继电器外侧电压大于10v 	4006
	
	{ENDFAIL_OTHERERR ,			{0x33,0x30,0x33,0x42}},		//其他原因 			303B
	
};



/*******************************停止失败原因*********************************/
const static  _FAIL_CODE StopFailCode[TABLE_STOP_REASON] = 
{
	{STOP_ERR_NONE ,			{0x31,0x30,0x30,0x31}},	  	//APP或者微信停止   	1001
	
	{STOP_BSMNORMAL ,			{0x31,0x30,0x30,0x30}},		//直流充满停止 			1000
	
	{STOP_SOC		,			{0x31,0x30,0x30,0x30}},		//达到设定的SOC 		1000
	
	{STOP_HANDERR ,				{0x31,0x30,0x30,0x44}},		//枪未正确连接  		100D
	
	{STOP_EMERGENCY ,			{0x33,0x30,0x30,0x33}},		//急停 					3003
	
	{STOP_ACINERR ,				{0x34,0x30,0x30,0x30}},		//输入电源故障（过压、过流、欠压，跳闸）4000
	
	{STOP_ACSWERR1 ,			{0x34,0x30,0x30,0x35}},		//交流断路器故障 		4005
	
	{STOP_CUPTEMPERATURE ,		{0x33,0x30,0x31,0x36}},		//充电桩过温报警 		3016
	
	{STOP_GUNUPTEMPERATURE ,	{0x33,0x30,0x31,0x37}},		//充电接口过温保护 		3017
	
	{STOP_ELECLOCKERR ,			{0x33,0x30,0x31,0x38}},		//充电接口电子锁故障 	3018
	
	{STOP_LEAKOUTTIMEOUT ,		{0x33,0x30,0x31,0x35}},		//泄放回路故障 			3015
	
	{STOP_BMSPOWERERR ,			{0x33,0x30,0x31,0x33}},		//辅助电源故障 			3013
	
	{STOP_CHARGEMODULEERR ,		{0x33,0x30,0x31,0x41}},		//电源模块故障 			300A
	
	{STOP_OUTVOLTVORE ,			{0x34,0x30,0x30,0x31}},		//输出电压过压故障		4001
	
	{STOP_OUTVOLTUNDER ,		{0x34,0x30,0x30,0x33}},		//输出电压欠压故障		4003
	
	{STOP_OUTCURROVER ,			{0x34,0x30,0x30,0x32}},		//输出电压过流故障		4002
	
	{STOP_BCLTIMTOUT ,			{0x35,0x30,0x30,0x34}},		//BCL电池充电需求报文超时	5004
	
	{STOP_BCSTIMTOUT ,			{0x35,0x30,0x30,0x33}},		//BCS电池充电状态报文超时	5003
	
	{STOP_BSMTIMTOUT ,			{0x35,0x30,0x30,0x37}},		//BSM动力蓄电池状态报文超时	5007
	
	{STOP_BALANCE ,				{0x31,0x30,0x30,0x37}},		//达到设置充电金额停止				1007
	
	{STOP_BSMERR ,				{0x31,0x30,0x30,0x41}},		//BMS异常终止				100A
	
	{STOP_OTHERERR ,			{0x33,0x30,0x33,0x42}},		//其他原因 				303B

};

/*****************************************************************************
* Function     : APP_GetHYCurPower
* Description  :获取当前功率
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT16U APP_GetHYCurPower(void)
{
	return RecvHYCmd101.userkw;
}

/*****************************************************************************
* Function     : APP_GetHYNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetHYNetMoney(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	return RecvHYCmd7[gun].money;
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
INT8U   APP_GetHYStartType(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return _4G_APP_START;
	}
	return (INT8U)HYStartType[gun];
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
INT8U   APP_SetHYStartType(INT8U gun ,_4G_START_TYPE  type)
{
	if((type >=  _4G_APP_MAX) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	
	HYStartType[gun] = type;
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
INT8U *  APP_GetHYBatchNum(INT8U gun)
{
	static INT8U buf[16];		//交易流水号位16个字节，ASICC 因此取后16个数字

	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	memcpy(buf,&RecvHYCmd7[gun].charge_user_id[16],16);		//订单号的后16位唯一
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
INT8U   HY_SendFrameDispose(void)
{
	INT8U i;

	for(i = 0;i < HY_SEND_FRAME_LEN;i++)
	{
		if(HYSendTable[i].cycletime == 0)
		{
			continue;
		}
		HYSendTable[i].curtime = OSTimeGet();
		if((HYSendTable[i].curtime >= HYSendTable[i].lasttime) ? ((HYSendTable[i].curtime - HYSendTable[i].lasttime) >= HYSendTable[i].cycletime) : \
		((HYSendTable[i].curtime + (0xFFFFFFFFu - HYSendTable[i].lasttime)) >= HYSendTable[i].cycletime))
		{
			HYSendTable[i].lasttime = HYSendTable[i].curtime;
			if(HYSendTable[i].Sendfunc != NULL)
			{
				HYSendTable[i].Sendfunc();
			}
		}
		
	}
	return TRUE;
}

/*****************************************************************************
* Function     : HYFreamSend
* Description  : 汇誉帧发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   HYFreamSend(INT16U cmd,INT8U *pdata, INT16U len)
{
	INT8U Sendbuf[500];
	static INT32U count;			//序列号域 
	INT16U datalen = 15+len;
	
	if((pdata == NULL) || (!len) )
	{
		return FALSE;
	}
	//起始域
	Sendbuf[0] = 0x7d;
	Sendbuf[1] = 0xd0;
	//2个字节数据长度
	Sendbuf[2] = datalen& 0x000000ff;
	Sendbuf[3] = (datalen >> 8)& 0x000000ff;
	//4个字节版本号
	Sendbuf[4] = 0x00;
	Sendbuf[5] = 0x00;
	Sendbuf[6] = 0x00;
	Sendbuf[7] = 0x00;
	//序列号
	Sendbuf[8] = count& 0x000000ff;
	Sendbuf[9] = (count >> 8)& 0x000000ff;
	Sendbuf[10] = (count >> 16)& 0x000000ff;
	Sendbuf[11] = (count >> 24)& 0x000000ff;
	//命令代码  大端
	Sendbuf[13] = cmd & 0x00ff;
	Sendbuf[12] = (cmd >> 8) & 0x00ff;
	
	memcpy(&Sendbuf[14],pdata,len);
	//1字节CS
	Sendbuf[14+len] = get_crc_Data(Sendbuf,14+len);
	ModuleSIM7600_SendData(0, Sendbuf,(15+len)); //发送数据
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

/***************************************************************
**Function   :HY_SendVinInfo
**Description:cmd  1 启动充电  0非启动充电
**Input      :gun: [输入/出]
**Output     :
**Return     :
**note(s)    :
**Author     :
**Create_Time:2023-2-17
***************************************************************/
INT8U  HY_SendVinInfo(INT8U gun,INT8U cmd)
{
	INT8U * pdevnum = APP_GetDevNum(); 
	if(gun >= GUN_MAX)
    {
        return FALSE;
    }
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&SendHYCmd1302[gun],0,sizeof(hy_cmd_1302));
	memcpy(SendHYCmd1302[gun].equipment_id,pdevnum,16);	//充电桩编号
	memcpy(SendHYCmd1302[gun].card_vin,BMS_BRM_Context[gun].VIN,sizeof(BMS_BRM_Context[gun].VIN));  //其他方式置零( ASCII码)
	//memset(SendHYCmd1302[gun].card_vin,0x31,sizeof(BMS_BRM_Context[gun].VIN));  //其他方式置零( ASCII码)
	SendHYCmd1302[gun].gun_index = gun;
	SendHYCmd1302[gun].cmd = cmd;	
	return HYFreamSend(hy_cmd_type_1302,(INT8U*)&SendHYCmd1302[gun],sizeof(hy_cmd_1302));
}


/*****************************************************************************
* Function     : HY_SendCardInfo
* Description  :
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  HY_SendCardInfo(_GUN_NUM gun)
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
	memset(SendHYCmd34[gun].equipment_id,0,sizeof(hy_cmd_34));
	memcpy(SendHYCmd34[gun].equipment_id,pdevnum,16);	//充电桩编号
	SendHYCmd34[gun].gun_index = gun;
	SendHYCmd34[gun].card_id = 	CardNum;
	SendHYCmd34[gun].card_num[6] =  (CardNum / 1000000000) + '0';
	CardNum  = 	CardNum % 1000000000;
	SendHYCmd34[gun].card_num[7] = 	(CardNum / 100000000) + '0';
	CardNum  = 	CardNum % 100000000;
	SendHYCmd34[gun].card_num[8] = 	(CardNum / 10000000) + '0';
	CardNum  = 	CardNum % 10000000;
	SendHYCmd34[gun].card_num[9] = 	(CardNum / 1000000) + '0';
	CardNum  = 	CardNum % 1000000;
	SendHYCmd34[gun].card_num[10] = 	(CardNum / 100000) + '0';
	CardNum  = 	CardNum % 100000;
	SendHYCmd34[gun].card_num[11] = 	(CardNum / 10000) + '0';
	CardNum  = 	CardNum % 10000;
	SendHYCmd34[gun].card_num[12] = 	(CardNum / 1000) + '0';
	CardNum  = 	CardNum % 1000;
	SendHYCmd34[gun].card_num[13] = 	(CardNum / 100) + '0';
	CardNum  = 	CardNum % 100;
	SendHYCmd34[gun].card_num[14] = 	(CardNum / 10) + '0';
	CardNum  = 	CardNum % 10;
	SendHYCmd34[gun].card_num[15] = 	(CardNum / 1) + '0';
//	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
//	OSTimeDly(SYS_DELAY_50ms);	
//	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
  	return HYFreamSend(hy_cmd_type_34,SendHYCmd34[gun].equipment_id,sizeof(hy_cmd_34));
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
INT8U HY_SendBillData(INT8U * pdata,INT16U len,INT8U ifquery)
{
	hy_cmd_202 *psend202 = (hy_cmd_202 *)pdata;
	if((pdata == NULL) || (len < sizeof(SendHYCmd202)))
	{
		return FALSE;
	}
	if(ifquery) //查询返回
	{
		return HYFreamSend(hy_cmd_type_114,pdata,sizeof(SendHYCmd202));
	}
	else
	{
		if((psend202->err_no[0] == 0x39) && (psend202->err_no[1] == 0x39) && (psend202->err_no[2] == 0x39) && (psend202->err_no[3] == 0x39))
		{
			//写为正常停止
			psend202->err_no[0] = 0x31;
			psend202->err_no[1] = 0x30;
			psend202->err_no[2] = 0x30;
			psend202->err_no[3] = 0x31;
		}
		return HYFreamSend(hy_cmd_type_202,pdata,sizeof(SendHYCmd202));
	}
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
static INT8U   HY_SendRegister(void)
{
	INT8U * pdevnum = APP_GetDevNum(); 
	
	if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
	{
		return  FALSE;
	}
	SendHYCmd106.stake_version[0] = 0;
	memcmp(&SendHYCmd106.stake_version[1],SYS_APP_VER,3);
	SendHYCmd106.charge_mode_num = 8;
	SendHYCmd106.charge_mode_rate = 1200;		//120kw
	memcpy(SendHYCmd106.equipment_id,pdevnum,16);	//充电桩编号
	SendHYCmd106.offline_charge_flag = 0;			//离线允许充电
	SendHYCmd106.stake_type = 0x00;					//充电桩类型0x00 直流0x01 交流0x02 混合
	SendHYCmd106.gun_index = APP_GetGunNum();					//充电桩枪数量
	return HYFreamSend(hy_cmd_type_106,(INT8U*)&SendHYCmd106,sizeof(SendHYCmd106));
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
INT8U   HY_SendRateAck(INT8U cmd) 
{
	cmd =cmd;
	return HYFreamSend(hy_cmd_type_1310,(INT8U*)&SendHYCmd1310,sizeof(SendHYCmd102));
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
static INT8U   HY_SendHear(void) 
{
	INT32U kw = 0;
	static INT16U count = 0;
	INT8U * pdevnum = APP_GetDevNum(); 
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memcpy(SendHYCmd102.equipment_id,pdevnum,16);	//充电桩编号
	
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		kw += PowerModuleInfo[GUN_A].OutputInfo.Vol * PowerModuleInfo[GUN_A].OutputInfo.Cur / 10000;
		HY_SendVinInfo(GUN_A,0);  //只是发送VIN信息
	}
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		kw += PowerModuleInfo[GUN_B].OutputInfo.Vol * PowerModuleInfo[GUN_B].OutputInfo.Cur / 10000;
		HY_SendVinInfo(GUN_B,0);  //只是发送VIN信息
	}
	SendHYCmd102.curkw =kw;
	//SendHYCmd102.curkw =800;
	return HYFreamSend(hy_cmd_type_102,(INT8U*)&SendHYCmd102,sizeof(SendHYCmd102));
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
INT8U   HY_SendStartAck(_GUN_NUM gun)
{
	//发送启动应答说明已经启动成功了，若启动失败，则不发送启动应答，直接发送订单
	
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if(pdisp_conrtol->NetGunState[gun] == GUN_CHARGEING)
	{
		//表示启动成功
		SendHYCmd8.result[0] = 0x30;
		SendHYCmd8.result[1] = 0x30;
		SendHYCmd8.result[2] = 0x30;
		SendHYCmd8.result[3] = 0x30;
	}else
	{
		//只是表示立即应答
		SendHYCmd8.result[0] = 0x39;
		SendHYCmd8.result[1] = 0x39;
		SendHYCmd8.result[2] = 0x39;
		SendHYCmd8.result[3] = 0x39;
	}
	
	memcpy(SendHYCmd8.equipment_id,pdevnum,16);	//充电桩编号
	SendHYCmd8.gun_index = gun;
	memcpy(SendHYCmd8.charge_user_id,RecvHYCmd7[gun].charge_user_id,32);  //订单号
	return HYFreamSend(hy_cmd_type_8,(INT8U*)&SendHYCmd8,sizeof(SendHYCmd8));
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
INT8U   HY_SendStopAck(_GUN_NUM gun)
{
	INT8U * pdevnum = APP_GetDevNum(); 
	
	memcpy(SendHYCmd12.equipment_id,pdevnum,16);	//充电桩编号
	SendHYCmd12.gun_index = gun;
	memcpy(SendHYCmd12.charge_seq,RecvHYCmd7[gun].charge_user_id,32);  //订单号
	return HYFreamSend(hy_cmd_type_12,(INT8U*)&SendHYCmd12,sizeof(SendHYCmd12));
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
INT8U   HY_SendQueryRateAck(void)
{
	INT8U buf[100];
	INT8U len;
	
	if((RateGroup == 0) || (RateGroup > 4) )
	{
		return FALSE;
	}
	buf[0] = RateGroup;
	memcpy(&buf[1],(INT8U*)RecvHYCmd1309,sizeof(RecvHYCmd1309));
	len = 1 + RateGroup* sizeof(hy_cmd_1309);
	return HYFreamSend(hy_cmd_type_1312,buf,len);
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
INT8U   HY_SendDevStateA(void)
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
	
	memset(&SendHYCmd104[GUN_A],0,sizeof(hy_cmd_104));
	memcpy(SendHYCmd104[GUN_A].equipment_id,pdevnum,16);	//充电桩编号
	SendHYCmd104[GUN_A].gun_cnt = APP_GetGunNum();					//充电枪数量
	SendHYCmd104[GUN_A].gun_index = 0;						//充电口号
	SendHYCmd104[GUN_A].gun_type = 1;						//充电枪类型  1=直流； 2=交流；
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		SendHYCmd104[GUN_A].work_stat = 2;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_UNLINK)
	{
		SendHYCmd104[GUN_A].work_stat = 3;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_STOPING)
	{
		SendHYCmd104[GUN_A].work_stat = 7;
	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)
	{
		if (GetEmergencyState(GUN_A) == EMERGENCY_PRESSED)
		{
			SendHYCmd104[GUN_A].work_stat = 8;
		}
		else
		{
			SendHYCmd104[GUN_A].work_stat = 6;
		}
	}else
	{
		if(GetGunState(GUN_A) == GUN_DISCONNECTED)
		{
			SendHYCmd104[GUN_A].work_stat = 0;		//空闲
		}else
		{
			SendHYCmd104[GUN_A].work_stat = 1;		//已经插枪
		}
	}
	//车辆连接状态
	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
	{
		SendHYCmd104[GUN_A].car_connection_stat = 0;		//空闲
	}else
	{
		SendHYCmd104[GUN_A].car_connection_stat = 2;		//已经插枪
	}	
	
	
	if(SendHYCmd104[GUN_A].work_stat == 2)
	{
		SendHYCmd104[GUN_A].soc_percent = BMS_BCS_Context[GUN_A].SOC; 				//当前spc
		SendHYCmd104[GUN_A].cumulative_charge_fee = puserinfo->TotalBill/100;   			//充电电费
		SendHYCmd104[GUN_A].dc_charge_voltage = PowerModuleInfo[GUN_A].OutputInfo.Vol;			//电压
		SendHYCmd104[GUN_A].dc_charge_current  =  PowerModuleInfo[GUN_A].OutputInfo.Cur;	//电流
		SendHYCmd104[GUN_A].bms_need_voltage = BMS_BCL_Context[GUN_A].DemandVol;			//需求电压
		SendHYCmd104[GUN_A].bms_need_current = BMS_BCL_Context[GUN_A].DemandCur;	//需求电流
		SendHYCmd104[GUN_A].bms_charge_mode = 0;										//自动充满
		SendHYCmd104[GUN_A].charge_full_time_left = BMS_BCS_Context[GUN_A].RemainderTime;
		SendHYCmd104[GUN_A].charged_sec = puserinfo->ChargeTime * 60;
		SendHYCmd104[GUN_A].cum_charge_money_amount =  puserinfo->TotalBill/100;		//累计充电金额
		SendHYCmd104[GUN_A].cum_charge_kwh_amount = puserinfo->TotalPower4/100;			//累计充电电量
		SendHYCmd104[GUN_A].before_charge_meter_kwh_num = pmeter->StartPower4/100;		//开始充电读表
		SendHYCmd104[GUN_A].now_meter_kwh_num = pmeter->CurPower4/100;					//当前充电读表
		SendHYCmd104[GUN_A].charge_power_kw = SendHYCmd104[GUN_A].dc_charge_voltage * SendHYCmd104[GUN_A].dc_charge_current / 10;
		SendHYCmd104[GUN_A].before_charge_card_account	 = 	puserinfo->TotalPower4/10;		//充电电量
		memcpy(SendHYCmd104[GUN_A].charge_user_id,RecvHYCmd7[GUN_A].charge_user_id,32);		//订单号
	}
	else
	{
		;
	}
	return HYFreamSend(hy_cmd_type_104,(INT8U*)&SendHYCmd104[GUN_A],sizeof(hy_cmd_104));
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
INT8U   HY_SendDevStateB(void)
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
	memset(&SendHYCmd104[GUN_B],0,sizeof(hy_cmd_104));
	memcpy(SendHYCmd104[GUN_B].equipment_id,pdevnum,16);	//充电桩编号
	SendHYCmd104[GUN_B].gun_cnt = APP_GetGunNum();					//充电枪数量
	SendHYCmd104[GUN_B].gun_index = 1;						//充电口号
	SendHYCmd104[GUN_B].gun_type = 1;						//充电枪类型  1=直流； 2=交流；
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		SendHYCmd104[GUN_B].work_stat = 2;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_UNLINK)
	{
		SendHYCmd104[GUN_B].work_stat = 3;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_STOPING)
	{
		SendHYCmd104[GUN_B].work_stat = 7;
	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)
	{
		if (GetEmergencyState(GUN_B) == EMERGENCY_PRESSED)
		{
			SendHYCmd104[GUN_B].work_stat = 8;
		}
		else
		{
			SendHYCmd104[GUN_B].work_stat = 6;
		}
	}else
	{
		if(GetGunState(GUN_B) == GUN_DISCONNECTED)
		{
			SendHYCmd104[GUN_B].work_stat = 0;		//空闲
		}else
		{
			SendHYCmd104[GUN_B].work_stat = 1;		//已经插枪
		}
	}
	//车辆连接状态
	if(GetGunState(GUN_B) == GUN_DISCONNECTED)
	{
		SendHYCmd104[GUN_B].car_connection_stat = 0;		//空闲
	}else
	{
		SendHYCmd104[GUN_B].car_connection_stat = 2;		//已经插枪
	}	
	
	
	if(SendHYCmd104[GUN_B].work_stat == 2)
	{
		SendHYCmd104[GUN_B].soc_percent = BMS_BCS_Context[GUN_B].SOC; 				//当前spc
		SendHYCmd104[GUN_B].cumulative_charge_fee = puserinfo->TotalBill/100;   			//充电电费
		SendHYCmd104[GUN_B].dc_charge_voltage = PowerModuleInfo[GUN_B].OutputInfo.Vol;			//电压
		SendHYCmd104[GUN_B].dc_charge_current  =  PowerModuleInfo[GUN_B].OutputInfo.Cur;	//电流
		SendHYCmd104[GUN_B].bms_need_voltage = BMS_BCL_Context[GUN_B].DemandVol;			//需求电压
		SendHYCmd104[GUN_B].bms_need_current = BMS_BCL_Context[GUN_B].DemandCur;	//需求电流
		SendHYCmd104[GUN_B].bms_charge_mode = 0;										//自动充满
		SendHYCmd104[GUN_B].charge_full_time_left = BMS_BCS_Context[GUN_B].RemainderTime;
		SendHYCmd104[GUN_B].charged_sec = puserinfo->ChargeTime * 60;
		SendHYCmd104[GUN_B].cum_charge_money_amount =  puserinfo->TotalBill/100;		//累计充电金额
		SendHYCmd104[GUN_B].cum_charge_kwh_amount = puserinfo->TotalPower4/100;			//累计充电电量
		SendHYCmd104[GUN_B].before_charge_meter_kwh_num = pmeter->StartPower4/100;		//开始充电读表
		SendHYCmd104[GUN_B].now_meter_kwh_num = pmeter->CurPower4/100;					//当前充电读表
		SendHYCmd104[GUN_B].charge_power_kw = SendHYCmd104[GUN_B].dc_charge_voltage * SendHYCmd104[GUN_B].dc_charge_current / 10; 
		memcpy(SendHYCmd104[GUN_B].charge_user_id,RecvHYCmd7[GUN_B].charge_user_id,32);		//订单号
			SendHYCmd104[GUN_B].before_charge_card_account	 = 	puserinfo->TotalPower4/10;;		//充电费用
	}
	else
	{
		;
	}
	return HYFreamSend(hy_cmd_type_104,(INT8U*)&SendHYCmd104[GUN_B],sizeof(hy_cmd_104));
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
INT8U   PreHYBill(_GUN_NUM gun,INT8U *pdata)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	INT8U * pdevnum = APP_GetDevNum(); 
	INT8U errcode,i;			//故障码
	hy_cmd_202 * pbill = (hy_cmd_202 *)pdata;
	_BSPRTC_TIME endtime;
	GetCurTime(&endtime); 

	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if(APP_GetWorkState(gun) == WORK_CHARGE)
	{
		//在充电或者注册不成功
		pbill->err_no[0] = 0x39;
		pbill->err_no[1] = 0x39;
		pbill->err_no[2] = 0x39;
		pbill->err_no[3] = 0x39;
	}
	else
	{
		errcode = APP_GetStopChargeReason(gun);   //获取故障码
		if(errcode & 0x80) //启动失败
		{
			errcode &= 0x7f;
			for(i = 0; i < TABLE_START_FAIL;i++)
			{
				if(StartFailCode[i].Reason == errcode)
				{
					memcpy(pbill->err_no,StartFailCode[i].CodeNum,4);
					break;
				}
			}
			if(i == TABLE_START_FAIL)
			{
				//其他原因
				pbill->err_no[0] = 0x33;
				pbill->err_no[1] = 0x30;
				pbill->err_no[2] = 0x33;
				pbill->err_no[3] = 0x42;
			}
		}
		else //充电停止
		{
			for(i = 0; i < TABLE_STOP_REASON;i++)
			{
				if(StopFailCode[i].Reason == errcode)
				{
					memcpy(pbill->err_no,StopFailCode[i].CodeNum,4);
					break;
				}
			}
			if(i == TABLE_STOP_REASON)
			{
				//其他原因
				pbill->err_no[0] = 0x33;
				pbill->err_no[1] = 0x30;
				pbill->err_no[2] = 0x33;
				pbill->err_no[3] = 0x42;
			}
		}
	}
	memcpy(pbill->equipment_id,pdevnum,16);	//充电桩编号
	pbill->gun_type = 0x01;		//充电枪类型 	1-直流 2-交流
	pbill->gun_index = gun;		//充电枪口
	memcpy(pbill->charge_user_id,RecvHYCmd7[gun].charge_user_id,32);  //订单号
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
	pbill->start_card_money  = pmeter->CurPower4/10 -pmeter->StartPower4/10;
	pbill->charge_kwh_amount  = pmeter->CurPower4/100 -pmeter->StartPower4/100;
	pbill->start_charge_kwh_meter = pmeter->StartPower4/100; /*充电前电表读数*/
	pbill->end_charge_kwh_meter = pmeter->CurPower4/100; /* 充电后电表读数*/
	pbill->total_charge_fee = puserinfo->TotalBill/100;			//本次充电金额
	pbill->total_service_fee = puserinfo->TotalServeBill/100;			//本次充电总服务费
	pbill->charge_policy = RecvHYCmd7[gun].charge_policy;				//充电策略  0:充满为止1:时间控制充电2:金额控制充电3:电量控制充电4:SOC控制充电
	if(HYStartType[gun] == _4G_APP_START)					//0表示App启动 1表示刷卡启动
	{
		pbill->start_charge_type = 1;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
	}
	else
	{
		pbill->start_charge_type = 0;	
	}
	
	#if ADD_FS_KWH
	for(i = 0;i < 48;i++)
	{
		pbill->kwh[i] = puserinfo->kwh[i]/100;
	}
	#endif
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
INT8U HY_SendBill(_GUN_NUM gun)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	INT8U * pdevnum = APP_GetDevNum(); 
	INT8U errcode,i;			//故障码
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	errcode = APP_GetStopChargeReason(gun);   //获取故障码
	if(errcode & 0x80) //启动失败
	{
		errcode &= 0x7f;
		for(i = 0; i < TABLE_START_FAIL;i++)
		{
			if(StartFailCode[i].Reason == errcode)
			{
				memcpy(SendHYCmd202.err_no,StartFailCode[i].CodeNum,4);
				break;
			}
		}
		if(i == TABLE_START_FAIL)
		{
			//其他原因
			SendHYCmd202.err_no[0] = 0x33;
			SendHYCmd202.err_no[1] = 0x30;
			SendHYCmd202.err_no[2] = 0x33;
			SendHYCmd202.err_no[3] = 0x42;
		}
		SendHYCmd202.charge_kwh_amount  = 0;
		SendHYCmd202.start_card_money  = 0;
	}
	else //充电停止
	{
		for(i = 0; i < TABLE_STOP_REASON;i++)
		{
			if(StopFailCode[i].Reason == errcode)
			{
				memcpy(SendHYCmd202.err_no,StopFailCode[i].CodeNum,4);
				break;
			}
		}
		if(i == TABLE_STOP_REASON)
		{
			//其他原因
			SendHYCmd202.err_no[0] = 0x33;
			SendHYCmd202.err_no[1] = 0x30;
			SendHYCmd202.err_no[2] = 0x33;
			SendHYCmd202.err_no[3] = 0x42;
		}
		SendHYCmd202.charge_kwh_amount  = pmeter->CurPower4/100 -pmeter->StartPower4/100;
		SendHYCmd202.start_card_money  = pmeter->CurPower4/10 -pmeter->StartPower4/10;
	}
	memcpy(SendHYCmd202.equipment_id,pdevnum,16);	//充电桩编号
	SendHYCmd202.gun_type = 0x01;		//充电枪类型 	1-直流 2-交流
	SendHYCmd202.gun_index = gun;		//充电枪口
	memcpy(SendHYCmd202.charge_user_id,RecvHYCmd7[gun].charge_user_id,32);  //订单号
	//开始时间
	SendHYCmd202.charge_start_time[0] = 0x20;
	SendHYCmd202.charge_start_time[1] = puserinfo->StartTime.Year;
	SendHYCmd202.charge_start_time[2] = puserinfo->StartTime.Month;
	SendHYCmd202.charge_start_time[3] = puserinfo->StartTime.Day;
	SendHYCmd202.charge_start_time[4] = puserinfo->StartTime.Hour;
	SendHYCmd202.charge_start_time[5] = puserinfo->StartTime.Minute;
	SendHYCmd202.charge_start_time[6] = puserinfo->StartTime.Second;
	//结束时间
	SendHYCmd202.charge_end_time[0] = 0x20;
	SendHYCmd202.charge_end_time[1] = puserinfo->EndTime.Year;
	SendHYCmd202.charge_end_time[2] = puserinfo->EndTime.Month;
	SendHYCmd202.charge_end_time[3] = puserinfo->EndTime.Day;
	SendHYCmd202.charge_end_time[4] = puserinfo->EndTime.Hour;
	SendHYCmd202.charge_end_time[5] = puserinfo->EndTime.Minute;
	SendHYCmd202.charge_end_time[6] = puserinfo->EndTime.Second;
	
	SendHYCmd202.charge_time = puserinfo->ChargeTime * 60;			//充电时间长度  单位秒
	SendHYCmd202.start_soc = BMS_BCP_Context[gun].BatterySOC;
	SendHYCmd202.end_soc = BMS_BCS_Context[gun].SOC;
	//err_no[4];				//充电结束原因  错误含义见附录
	//SendHYCmd202.charge_kwh_amount  = pmeter->CurPower -pmeter->StartPower;
	SendHYCmd202.start_charge_kwh_meter = pmeter->StartPower4/100; /*充电前电表读数*/
	SendHYCmd202.end_charge_kwh_meter = pmeter->CurPower4/100; /* 充电后电表读数*/
	SendHYCmd202.total_charge_fee = puserinfo->TotalBill/100;			//本次充电金额
	SendHYCmd202.total_service_fee = puserinfo->TotalServeBill/100;			//本次充电总服务费
	SendHYCmd202.charge_policy = RecvHYCmd7[gun].charge_policy;				//充电策略  0:充满为止1:时间控制充电2:金额控制充电3:电量控制充电4:SOC控制充电
	if(HYStartType[gun] == _4G_APP_START)					//0表示App启动 1表示刷卡启动
	{
		SendHYCmd202.start_charge_type = 1;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
	}
	else
	{
		SendHYCmd202.start_charge_type = 0;			//启动方式0：本地刷卡启动1：后台启动2：本地管理员启动
	}
	#if ADD_FS_KWH
	for(i = 0;i < 48;i++)
	{
		SendHYCmd202.kwh[i] = puserinfo->kwh[i]/100;
	}
	#endif
	memcpy(SendHYCmd202.car_vin,BMS_BRM_Context[gun].VIN,sizeof(BMS_BRM_Context[gun].VIN));
	//memset(SendHYCmd202.car_vin,0x31,sizeof(BMS_BRM_Context[gun].VIN));
	WriterFmBill(gun,1);
	ResendBillControl[gun].CurTime = OSTimeGet();	
	ResendBillControl[gun].LastTime = ResendBillControl[gun].CurTime;
	return HYFreamSend(hy_cmd_type_202,(INT8U*)&SendHYCmd202,sizeof(hy_cmd_202));
}

/******************************************接收函数*******************************************/
/*****************************************************************************
* Function     : HY_RecvUpdata
* Description  : VIN鉴权
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static uint8_t   HY_RecvVin(uint8_t *pdata,uint16_t len)
{
	static _BSP_MESSAGE SendStartMsg1[GUN_MAX];
	static INT8U Vinstate = 0; //0失败 1成功
	OS_EVENT* pevent = APP_Get4GMainEvent();
	hy_cmd_1301 * p1301 = NULL;
	
    int length;
    if(pdata == NULL)
    {
        return FALSE;
    }
    if(len != sizeof(hy_cmd_1301) )
    {
        return FALSE;
    }
	p1301 = (hy_cmd_1301*)pdata;
	if(p1301->gun_index >= GUN_MAX)
	{
		return FALSE;
	}
    memcpy(&RecvHYCmd1301[p1301->gun_index],pdata,sizeof(hy_cmd_1301));
//	if(RecvHYCmd1301[p1301->gun_index].ifsuccess)  //鉴权成功
//	{
//		_4G_SetStartType(p1301->gun_index,_4G_APP_VIN);			//设置为VIN启动
//		Vinstate = 1;
//		//发送启动应答
//		SendStartMsg1[p1301->gun_index].MsgID = BSP_4G_RECV;
//		SendStartMsg1[p1301->gun_index].DataLen = p1301->gun_index;
//		SendStartMsg1[p1301->gun_index].pData = &Vinstate;
//		SendStartMsg1[p1301->gun_index].DivNum = APP_START_VIN;
//		OSQPost(pevent, &SendStartMsg1[p1301->gun_index]);
//	}
//	else
//	{
//		_4G_SetStartType(p1301->gun_index,_4G_APP_VIN);			//设置为VIN启动
//		Vinstate = 0;
//		//发送启动应答
//		SendStartMsg1[p1301->gun_index].MsgID = BSP_4G_RECV;
//		SendStartMsg1[p1301->gun_index].DataLen = p1301->gun_index;
//		SendStartMsg1[p1301->gun_index].pData = &Vinstate;
//		SendStartMsg1[p1301->gun_index].DivNum = APP_START_VIN;
//		OSQPost(pevent, &SendStartMsg1[p1301->gun_index]);
//	}
    return TRUE;
}


/*****************************************************************************
* Function     : HY_RecvUpdata
* Description  : 远程升级
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static uint8_t   HY_RecvUpdata(uint8_t *pdata,uint16_t len)
{
// 	 uint8_t    equipment_id[16];		//充电桩编码
//	 uint8_t    Devtype;		//桩型号
//	uint8_t 	ServerAddr[32];		//服务器地址  （ASCII 码）不足 32 位补零
//	uint8_t 	Port;			//端口
//	uint8_t     usename[16];	//用户名  （ASCII 码）不足 16 位补零
//	uint8_t     password[16];	//密码  （ASCII 码）不足 16 位补零
//	uint8_t		FilePath[64];	//文件路径   （ASCII 码）不足 32 位补零，文件路径名由平台定义
//	uint8_t		cmd;			//0x01：立即执行0x02：空闲执行
//	uint8_t		downloadtime;	//下载超时时间 min
    int length;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
    if(pdata == NULL)
    {
        return FALSE;
    }
    if(len != sizeof(hy_cmd_1101) )
    {
        return FALSE;
    }

    memcpy(&RecvHYCmd1101,pdata,sizeof(RecvHYCmd1101));
    RecvHYCmd1101.ServerAddr[63] = 0; //IP asicc  最后一个肯定为0， 做保护，
    RecvHYCmd1101.usename[15] = 0;  //用户名 asicc  最后一个肯定为0， 做保护，
    RecvHYCmd1101.password[15]= 0; //密码asicc  最后一个肯定为0， 做保护，
    RecvHYCmd1101.FilePath[127] = 0; //文件路径asicc  最后一个肯定为0， 做保护，
    memset(HttpInfo.ServerAdd,0,sizeof(HttpInfo.ServerAdd));
    memset(HttpInfo.ServerPassword,0,sizeof(HttpInfo.ServerPassword));
    HttpInfo.crc = RecvHYCmd1101.crc;  //CRC校验


//    memcpy(&RecvHYCmd1101,pdata,sizeof(RecvHYCmd1101));
//    RecvHYCmd1101.ServerAddr[31] = 0; //IP asicc  最后一个肯定为0， 做保护，
//    RecvHYCmd1101.usename[15] = 0;  //用户名 asicc  最后一个肯定为0， 做保护，
//    RecvHYCmd1101.password[15]= 0; //密码asicc  最后一个肯定为0， 做保护，
//    RecvHYCmd1101.FilePath[63] = 0; //文件路径asicc  最后一个肯定为0， 做保护，
//    memset(HttpInfo.ServerAdd,0,sizeof(HttpInfo.ServerAdd));
//    memset(HttpInfo.ServerPassword,0,sizeof(HttpInfo.ServerPassword));


    //length = snprintf(HttpInfo.ServerAdd, sizeof(HttpInfo.ServerAdd), "http://%s:%d/%s",RecvHYCmd1101.ServerAddr,RecvHYCmd1101.Port,RecvHYCmd1101.FilePath);
    length = snprintf(HttpInfo.ServerAdd, sizeof(HttpInfo.ServerAdd), "%s",RecvHYCmd1101.FilePath);
    if (length == -1)
    {
        printf("snprintf error, the len is %d", length);
        return FALSE;
    }
    length = snprintf(HttpInfo.ServerPassword, sizeof(HttpInfo.ServerPassword), "%s:%s",RecvHYCmd1101.usename,RecvHYCmd1101.password);
    if (length == -1)
    {
        printf("snprintf error, the len is %d", length);
        return FALSE;
    }
    //APP_SetSIM7600Mode(MODE_HTTP);
    IfSendUpdataAck = 1;   //需要发送升级应答
			#if(USER_GUN != USER_SINGLE_GUN)
    if((IfSendUpdataAck) && ((pdisp_conrtol->NetGunState[GUN_A] == GUN_UNLINK) ||(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)||(pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE)) \
		&&( (pdisp_conrtol->NetGunState[GUN_B] == GUN_UNLINK) ||(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)||(pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE)) )
    {
        //HY_SendUpdataAck(0); //返回一个升级成功
        APP_SetSIM7600Mode(MODE_HTTP);
        IfSendUpdataAck = 0;
    }

	#else
	 if((IfSendUpdataAck) && ((pdisp_conrtol->NetGunState[GUN_A] == GUN_UNLINK) ||(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)||(pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE)) )
    {
        //HY_SendUpdataAck(0); //返回一个升级成功
        APP_SetSIM7600Mode(MODE_HTTP);
        IfSendUpdataAck = 0;
    }

	#endif
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
INT8U   HY_RecvFrameDispose(INT8U * pdata,INT16U len)
{
	INT8U i = 0;
	INT16U cmd;
	INT16U datalen;
	static INT8U buf[300];
	INT8U *pframedata;

	if((pdata == NULL) || (len < 15) )
	{
		return FALSE;
	}
	//数据分帧
	pframedata = pdata;
	while(len > 15)
	{
		//帧头帧尾判断
		if((pframedata[0] != 0x7d) || (pframedata[1] != 0xd0) )
		{
			return FALSE;
		}
		//提取数据长度
		datalen= pframedata[2] | (pframedata[3] << 8);
		if((datalen >300) || (datalen < 15) ) 
		{
			return FALSE;
		}
		if(datalen > len)
		{
			return FALSE;
		}
		cmd = (pframedata[12] << 8) | pframedata[13];  //提取命令字
		datalen = datalen - 15;
		//提取数据
		memcpy(buf,&pframedata[14],datalen);
		
		for(i = 0;i < HY_RECV_FRAME_LEN;i++)
		{
			if(HYRecvTable[i].cmd == cmd)
			{
				if(i == 0)
				{
					__NOP();
				}
				if(HYRecvTable[i].recvfunction != NULL)
				{
					HYRecvTable[i].recvfunction(buf,datalen);
				}
				break;
			}
		}
		len =len -  datalen - 15;
		pframedata = &pdata[datalen+15];
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
static INT8U   HY_RecvStartCharge(INT8U *pdata,INT16U len)
{
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	 static _BSP_MESSAGE SendMsg[GUN_MAX];
	hy_cmd_7 * pcmd7 = (hy_cmd_7 *)pdata;
		static _BSP_MESSAGE SendStartMsg1[GUN_MAX];
	static INT8U Vinstate = 0; //0失败 1成功
	
	if((pdata == NULL) || len != sizeof(hy_cmd_7))
	{
		return FALSE;
	}
	gun = pcmd7->gun_index;
	 ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
	ResendBillControl[gun].SendCount = 0;
	
	if((HYStartType[gun] == _4G_APP_CARD ) || (HYStartType[gun] == _4G_APP_VIN) )
	{
		NOP();
	}
	else
	{
		HYStartType[gun] = _4G_APP_START;
	}
	if(pcmd7->gun_index == 0)
	{
		gun = GUN_A;
		memcpy(&RecvHYCmd7[GUN_A],pcmd7,sizeof(hy_cmd_7));
	
		if (HYStartType[gun] != _4G_APP_VIN)  
		{
			SendMsg[GUN_A].MsgID = BSP_4G_RECV;
			SendMsg[GUN_A].DataLen = GUN_A;
			SendMsg[GUN_A].DivNum = APP_START_CHARGE;
			OSQPost(pevent, &SendMsg[GUN_A]);
		}
		else
		{
			Vinstate = 1;
			//发送启动应答
			SendStartMsg1[GUN_A].MsgID = BSP_4G_RECV;
			SendStartMsg1[GUN_A].DataLen = GUN_A;
			SendStartMsg1[GUN_A].pData = &Vinstate;
			SendStartMsg1[GUN_A].DivNum = APP_START_VIN;
			OSQPost(pevent, &SendStartMsg1[GUN_A]);
		}
	}
	else if(pcmd7->gun_index == 1)
	{
		gun = GUN_B;
		memcpy(&RecvHYCmd7[GUN_B],pcmd7,sizeof(hy_cmd_7));
		if((HYStartType[gun] != _4G_APP_VIN) ) 
		{
			memcpy(&RecvHYCmd7[GUN_B],pcmd7,sizeof(hy_cmd_7));
			SendMsg[GUN_B].MsgID = BSP_4G_RECV;
			SendMsg[GUN_B].DataLen = GUN_B;
			SendMsg[GUN_B].DivNum = APP_START_CHARGE;
			OSQPost(pevent, &SendMsg[GUN_B]);
		}
		else
		{
			Vinstate = 1;
			//发送启动应答
			SendStartMsg1[GUN_B].MsgID = BSP_4G_RECV;
			SendStartMsg1[GUN_B].DataLen = GUN_B;
			SendStartMsg1[GUN_B].pData = &Vinstate;
			SendStartMsg1[GUN_B].DivNum = APP_START_VIN;
			OSQPost(pevent, &SendStartMsg1[GUN_B]);
		}
	}
	
	//发送启动应答
	Msg[gun].MsgID = BSP_4G_MAIN;
	Msg[gun].DataLen = gun;
	Msg[gun].DivNum = APP_START_ACK;
	OSQPost(psendevent, &Msg[gun]);

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
static INT8U   HY_RecvStopCharge(INT8U *pdata,INT16U len)
{
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	 static _BSP_MESSAGE SendMsg[GUN_MAX];
	hy_cmd_11 * pcmd11 = (hy_cmd_11 *)pdata;
	if((pdata == NULL) || len != sizeof(hy_cmd_11))
	{
		return FALSE;
	}
	if(pcmd11->gun_index == 0)
	{
		gun = GUN_A;
		memcpy(&RecvHYCmd11[GUN_A],pcmd11,sizeof(hy_cmd_11));
		SendMsg[GUN_A].MsgID = BSP_4G_RECV;
		SendMsg[GUN_A].DataLen = GUN_A;
		SendMsg[GUN_A].DivNum = APP_STOP_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_A]);
	}
	else if(pcmd11->gun_index == 1)
	{
		gun = GUN_B;
		memcpy(&RecvHYCmd11[GUN_B],pcmd11,sizeof(hy_cmd_11));
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
static INT8U   HY_RecvHearAck(INT8U *pdata,INT16U len)
{    

	
	static uint8_t updatanum,ONENUM = 1;
	
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if((pdata == NULL) || len != sizeof(hy_cmd_101))
	{
		return FALSE;
	}
	
	memcpy(&RecvHYCmd101,pdata,sizeof(hy_cmd_101));
	//什么状态下都可以接收升级更新  等到空闲时，再升级
		#if(USER_GUN != USER_SINGLE_GUN)
    if((IfSendUpdataAck) && ((pdisp_conrtol->NetGunState[GUN_A] == GUN_UNLINK) ||(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)||(pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE)) \
		&&( (pdisp_conrtol->NetGunState[GUN_B] == GUN_UNLINK) ||(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)||(pdisp_conrtol->NetGunState[GUN_B] == GUN_IDLE)) )
    {
        //HY_SendUpdataAck(0); //返回一个升级成功
        APP_SetSIM7600Mode(MODE_HTTP);
        IfSendUpdataAck = 0;
    }
    else if((IfSendUpdataAck) && (updatanum))
    {
        HY_SendUpdataAck(4); //等待升级
        updatanum = 0;
    }
	#else
	 if((IfSendUpdataAck) && ((pdisp_conrtol->NetGunState[GUN_A] == GUN_UNLINK) ||(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)||(pdisp_conrtol->NetGunState[GUN_A] == GUN_IDLE)) )
    {
        //HY_SendUpdataAck(0); //返回一个升级成功
        APP_SetSIM7600Mode(MODE_HTTP);
        IfSendUpdataAck = 0;
    }
    else if((IfSendUpdataAck) && (updatanum))
    {
        HY_SendUpdataAck(4); //等待升级
        updatanum = 0;
    }
	#endif

    if(ONENUM)
    {
		HY_SendUpdataAck(0); //上电重启第一次心跳 返回升级成功和版本号
        ONENUM=0;
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
static INT8U   HY_RecvDevInfoAck(INT8U *pdata,INT16U len)
{
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	hy_cmd_103 * pcmd103;
	if((pdata == NULL) || len != sizeof(hy_cmd_103))
	{
		return FALSE;
	}
	
	pcmd103 = (hy_cmd_103 *)pdata;
	if(pcmd103->gun_index >= GUN_MAX)
	{
		return FALSE;
	}
	if(pcmd103->IfAck == 1) //查询
	{
		gun = pcmd103->gun_index;
		Msg[gun].MsgID = BSP_4G_MAIN;
		Msg[gun].DataLen = gun;
		Msg[gun].DivNum = APP_STE_DEVS;   //发送查询设备状态
		OSQPost(psendevent, &Msg[gun]);
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
static INT8U   HY_RecvRegisterAck(INT8U *pdata,INT16U len)
{
	
	_BSPRTC_TIME SetTime;                       //设定时间
	INT8U times = 3;							//如果设置失败反复设置三次
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if((pdata == NULL) || len != sizeof(hy_cmd_105))
	{
		return FALSE;
	}
	memcpy((INT8U*)&RecvHYCmd105,pdata,len);
	if(RecvHYCmd105.reg == 0)
	{
		//注册失败
		return FALSE;
	}
	APP_SetAppRegisterState(LINK_NUM,STATE_OK);
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING)   //没在充电中读取
	{
		ReadFmBill(GUN_A);  //是否需要发送订单
	}
	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)   //没在充电中读取
	{
		ReadFmBill(GUN_B);  //是否需要发送订单
	}
		//校准时间
	SetTime.Year   = RecvHYCmd105.time[1];//将输入的十进制数转换成BCD
    SetTime.Month  = RecvHYCmd105.time[2];
    SetTime.Day    = RecvHYCmd105.time[3];
    SetTime.Hour   = RecvHYCmd105.time[4];
    SetTime.Minute = RecvHYCmd105.time[5];
    SetTime.Second = RecvHYCmd105.time[6];
	
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
* Function     : HY_RecvRecordAck
* Description  : 订单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   HY_RecvRecordAck(INT8U *pdata,INT16U len)
{
	INT8U gun;
	if((pdata == NULL) || len != sizeof(hy_cmd_201))
	{
		return FALSE;
	}
	memcpy(&RecvHYCmd201,pdata,sizeof(RecvHYCmd201));

	gun = RecvHYCmd201.gun_index;
	
	
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
static INT8U   HY_RecvDevStopAck(INT8U *pdata,INT16U len)
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
static INT8U   HY_RecvDevFailAck(INT8U *pdata,INT16U len)
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
static INT8U   HY_RecvQueryBill(INT8U *pdata,INT16U len)
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
static INT8U   HY_RecvQueryRate(INT8U *pdata,INT16U len)
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
static INT8U   HY_RecvCardStart(INT8U *pdata,INT16U len)
{
	hy_cmd_33 *pcmd33;
	if(pdata == NULL)
	{
		return FALSE;
	}
	if(len != sizeof(hy_cmd_33) )
	{
		return FALSE;
	}
	pcmd33 =(hy_cmd_33 *) pdata;
	if(pcmd33->gun_index >= GUN_MAX)
	{
		return FALSE;
	}
	memcpy(&RecvHYCmd33[pcmd33->gun_index],pdata,sizeof(hy_cmd_33));
	if(RecvHYCmd33[pcmd33->gun_index].card_ifsuccess == 0)
	{
		HYStartType[pcmd33->gun_index] = _4G_APP_CARD;
	}
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
	OSTimeDly(SYS_DELAY_50ms);	
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	return TRUE;
	
}

/*****************************************************************************
* Function     : HY_SendUpdataAck
* Description  : 升级应答
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
uint8_t  HY_SendUpdataAck(uint8_t num)
{
	INT8U * pdevnum = APP_GetDevNum();

//    if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
//    {
//        return  FALSE;
//    }
    memcpy(SendHYCmd1102.equipment_id,pdevnum,16);	//充电桩编号
    SendHYCmd1102.UpdataState = num;	//升级状态   0x00-成功 0x01-编号错误 0x02-程序与桩型号不符 0x03-下载更新文件超时  0x04-等待升级

    memcpy(SendHYCmd1102.version,VERSIONS,MIN(sizeof(SendHYCmd1102.version),strlen(VERSIONS)));
    return HYFreamSend(hy_cmd_type_1102,(uint8_t*)&SendHYCmd1102,sizeof(SendHYCmd1102));
    printf("hy_cmd_type_1102 send_over");
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
static INT8U   HY_RecvRateSet(INT8U *pdata,INT16U len)
{
	INT8U i = 0,num = 0;
	static _PRICE_SET price;
	_FLASH_OPERATION  FlashOper;
	
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	
	if(pdata == NULL)
	{
		return FALSE;
	}
	RateGroup = pdata[0];  //目前最多十二个时间段
	if((RateGroup > TIME_PERIOD_MAX) || ((len - 1) != (sizeof(hy_cmd_1309) *RateGroup)) || (RateGroup == 0))
	{
		return FALSE;
	}
	memcpy(RecvHYCmd1309,&pdata[1],len);
	
	if(RecvHYCmd1309[0].StartNum != 0)
	{
		SendHYCmd1310.result = 3;
		Msg.MsgID = BSP_4G_MAIN;
		Msg.DataLen = 1;
		Msg.DivNum = APP_RATE_ACK;
		OSQPost(psendevent, &Msg);
		return FALSE;
	}
	//首先判断其实分段是否连续
	for(i = 0;i < RateGroup;i++)
	{
		num += RecvHYCmd1309[i].Num;
	}
	if(num != 48)
	{
		SendHYCmd1310.result = 3;
		Msg.MsgID = BSP_4G_MAIN;
		Msg.DataLen = 1;
		Msg.DivNum = APP_RATE_ACK;
		OSQPost(psendevent, &Msg);
		return FALSE;
	}
	SendHYCmd1310.result = 0;		//返回成功
	
	memset(&price,0,sizeof(price));
	price.TimeQuantumNum = RateGroup;
	num = 0;
	for(i = 0;i < RateGroup;i++)
	{
		price.StartTime[i] =  30 * num;   	//开始时间
		num +=  RecvHYCmd1309[i].Num;		//连续分段数量	
		price.EndTime[i] =  30 * num;		  	//结束时间
		#if  ENBLE_JG      //精工下发是的4位
		price.Price[i] = RecvHYCmd1309[i].DMoney*10;	  
		price.ServeFee[i] = RecvHYCmd1309[i].FWMoney*10;	//充电服务费
		#else
		price.Price[i] = RecvHYCmd1309[i].DMoney*1000;	
		price.ServeFee[i] = RecvHYCmd1309[i].FWMoney*1000;	//充电服务费
		#endif
		price.CurTimeQuantum[i] = 	TIME_QUANTUM_P;  //没有说明尖峰评古都是平
	}		
	price.EndTime[RateGroup - 1] = 0;			//最后一个时间段为0，24点就是0点，没有24点
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = PARA_PRICALL_FLLEN;
	FlashOper.ptr = (INT8U *)&price;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);
	
	memcpy(&PriceSet,&price,sizeof(_PRICE_SET));
	//费率设置应答
	Msg.DataLen = 1;
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_RATE_ACK;
	OSQPost(psendevent, &Msg);
	return TRUE;
}
/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

