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
#include "ZHFrame.h"
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
#define YKC_SEND_FRAME_LEN   9
#define YKC_RECV_FRAME_LEN	16



#define YKC_SEND_CMD_01   		0x01			//充电桩主动注册信息
#define YKC_RECV_CMD_02 		0x02			//pc返回确认信息

#define YKC_SEND_CMD_03   		0x03			//心跳发送
#define YKC_RECV_CMD_04 		0x04			//心跳应答

#define YKC_SEND_CMD_05   		0x05			//计费模型验证请求
#define YKC_RECV_CMD_06 		0x06			//计费模型验证请求应答


#define YKC_SEND_CMD_09   		0x09			//计费模型请求
#define YKC_RECV_CMD_0A 		0x0A			//计费模型请求应答


#define YKC_SEND_CMD_31   		0x31			//发送卡鉴权
#define YKC_RECV_CMD_32 		0x32			//平台确认  下发启动

#define YKC_SEND_CMD_33   		0x33			//启动充电应答
#define YKC_RECV_CMD_34 		0x34			//启动充电

#define YKC_SEND_CMD_A1   		0xA1			//并充发送卡鉴权
#define YKC_RECV_CMD_A2 		0xA2			//并充平台确认  下发启动

#define YKC_SEND_CMD_A3   		0xA3			//并充启动充电应答
#define YKC_RECV_CMD_A4 		0xA4			//并充启动充电

#define YKC_SEND_CMD_35   		0x35			//停止充电应答
#define YKC_RECV_CMD_36 		0x36			//停止充电

#define YKC_SEND_CMD_3B 		0x3B			//上传交易记录

#define YKC_SEND_CMD_13   		0x13			//上传实时监测数据
#define YKC_RECV_CMD_12 		0x12			//读取试试监控数据

#define YKC_SEND_CMD_55   		0x55			//对时应答
#define YKC_RECV_CMD_56 		0x56			//对时

#define YKC_SEND_CMD_57   		0x57			//费率应答
#define YKC_RECV_CMD_58 		0x58			//费率设置

#define YKC_SEND_CMD_41   		0x41			//余额更新
#define YKC_RECV_CMD_42 		0x42			//余额更新应答


#define YKC_SEND_CMD_15   		0x15			//充电握手
#define YKC_SEND_CMD_17   		0x17			//参数配置
#define YKC_SEND_CMD_19 		0x19			//结束充电
#define YKC_SEND_CMD_1B			0x1B			//错误报文
#define YKC_SEND_CMD_1D			0x1D			//充电阶段BMS中止
#define YKC_SEND_CMD_21			0x21			//充电阶段充电机中止
#define YKC_SEND_CMD_23			0x23			//充电过程BMS需求与充电机输出
#define YKC_SEND_CMD_25			0x25			//充电过程BMS信息

#define YKC_RECV_CMD_40 		0x40			//交易记录确认

#define YKC_SEND_CMD_93			0x93			//远程升级
#define YKC_RECV_CMD_94			0x94			//远程升级应答


//=====只供应铁塔=========
#define YKC_RECV_CMD_5A         0x5A       //平台下发充电枪对应的二维码到桩,
#define YKC_SEND_CMD_59         0x59       //平台回应


INT8U YCKDevNum[7] = {0};     //云快充桩编号位7个字节


//注册帧
__packed typedef struct
{
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U Devtype;		//0 表示直流桩，1 表示交流桩
	INT8U GunNum;		//总共枪数量
	INT8U Vsion;		//通信版本
	INT8U ProVersions[8];	//程序版本
	INT8U NetState;		//0 SIM卡  1 LAN     2 WAN   3其他
	INT8U SIM[10];		//SIM卡号
	INT8U Operator;     //运营商 0x00 移动0x02 电信0x03 联通0x04 其他
}_YKC_SEND_CMD01;

//心跳
__packed typedef struct
{
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U GunState;		//枪状态
}_YKC_SEND_CMD03;


//发送卡鉴权
__packed typedef struct
{
    INT8U DevNun[7];     //BCD  不足7位补充0
    INT8U GunNum;		    //枪号，从1开始
    INT8U Startmode;		//启动方式 0x01 表示通过刷卡启动充电  0x03 表示vin码启动充电
    INT8U password;    //是否需要密码 0x00 不需要 0x01 需要
    INT8U card_NUM[8];  //账号或者物理卡号  不足8位补 0，具体见示例
    INT8U Input_password[16];	//输入密码  对用户输入的密码进行16位MD5加密，采用小写上传
    INT8U VIN[17];		//启动方式为vin码启动充电时上送,其他方式置零( ASCII码)，VIN码需要反序上送
} _YKC_SEND_CMD31;

//发送卡鉴权 并充
__packed typedef struct
{
    INT8U DevNun[7];     		//BCD  不足7位补充0
    INT8U GunNum;		    	//枪号，从1开始
    INT8U Startmode;			//启动方式 0x01 表示通过刷卡启动充电  0x03 表示vin码启动充电
    INT8U password;    			//是否需要密码 0x00 不需要 0x01 需要
    INT8U card_NUM[8];  		//账号或者物理卡号  不足8位补 0，具体见示例
    INT8U Input_password[16];	//输入密码  对用户输入的密码进行16位MD5加密，采用小写上传
    INT8U VIN[17];				//启动方式为vin码启动充电时上送,其他方式置零( ASCII码)，VIN码需要反序上送
	INT8U  SMGun;				//主辅枪标记  0x00 主枪 0x01辅枪
	INT8U SerialNumber[6];		//并充序号
} _YKC_SEND_CMDA1;

//启动应答
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U StartState;		//启动是否成功 0失败  1成功
	INT8U FailReason;	//停止原因  0x00无    0x01设备编号不匹配    0x02枪已在充电   0x03设备故障     0x04设备离线     0x05未插枪
}_YKC_SEND_CMD33;


//并充启动应答
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U StartState;		//启动是否成功 0失败  1成功
	INT8U FailReason;	//停止原因  0x00无    0x01设备编号不匹配    0x02枪已在充电   0x03设备故障     0x04设备离线     0x05未插枪
	INT8U  SMGun;				//主辅枪标记  0x00 主枪 0x01辅枪
	INT8U SerialNumber[6];		//并充序号
}_YKC_SEND_CMDA3;

//发送实时数据   周期上传。待机5分钟 充电15秒
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U State;		//状态  0x00：离线 0x01：故障 0x02：空闲 0x03：充电
	INT8U GunReturn;	//枪是否归位   0没归位  1是  2未知
	INT8U GunState;		//是否插枪		0未插枪  1插枪  需做到变位上送
	INT16U OutVol;		//输出电压  0.1
	INT16U OutCur;		//输出电流	0.1
	INT8U GunT;			//枪线维度	偏移 -50
	INT8U GunNb[8];	     //枪线编码	没有置0
	INT8U Soc;
	INT8U BatteryT;		//电池组最高温度
	INT16U ChargeTime;	//充电时间  min
	INT16U RTime;	//剩余充电时间  min
	INT32U ChargeKwh;	//充电读数   精确到小数点4位
	INT32U res;			//计损充电读数
	INT32U ChargeMoney;	//已充金额	精确到小数点4位
//Bit1：急停按钮动作故障；Bit2：无可用整流模块；Bit3：出风口温度过高；Bit4：交流防雷故障；Bit5：交直流模块 DC20 通信中断；Bit6：绝缘检测模块 FC08 通信中断；
//Bit7：电度表通信中断；Bit8：读卡器通信中断；Bit9：RC10 通信中断；Bit10：风扇调速板故障；Bit11：直流熔断器故障；Bit12：高压接触器故障；Bit13：门打开；
	INT8U Fail[2];		
}_YKC_SEND_CMD13;

//充电握手  主动上报一次  BRM
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	
	INT8U  BMSVersion[3];        //BMS通信协议版本
    INT8U  BatteryType;          //电池类型
    INT16U RatedCapacity;        //整车动力蓄电池系统额定容量 /Ah;0.1Ah/位
    INT16U RatedVol;             //整车动力蓄电池系统额定电压/V;0.1V/
    INT8U  BatteryFactory[4];    //电池生产厂商名称
    INT8U  BatterySerialNum[4];  //电池组序号
    INT8U  BatteryProduceYear;   //电池组生产年1年/bit  1985--2235
    INT8U  BatteryProduceMonth;  //电池组生产月1月/bit  
    INT8U  BatteryProduceDay;    //电池组生产日1日/bit  
    INT8U  BatteryChargedFreq[3];//电池组充电次数
    INT8U  BatteryRightFlag;     //电池组产权标示
    INT8U  defaultbyte;          //预留字节
    INT8U  VIN[17];              //车辆识别码
    INT8U  BMSSoftwareVer[8];    //BMS软件版本号（新国标内容）
}_YKC_SEND_CMD15;


//参数配置，主动上报一次  BCP
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	
	INT16U UnitBatteryMaxVol;    //单体动力蓄电池最高允许充电电压
    INT16U MaxCurrent;           //最高允许充电电流
    INT16U BatteryWholeEnergy;   //动力蓄电池标称总能量
    INT16U MaxVoltage;           //最高允许充电总电压
    INT8U  MaxTemprature;        //最高允许温度
    INT16U BatterySOC;           //整车蓄电池荷电状态:SOC
    INT16U BatteryPresentVol;    //整车动力蓄电池当前电池电压,0.1V/位
	INT16U DevHVol;				//0.1设备最高电压
	INT16U DevLVol;				//0.1设备最低电压
	INT16U DevHCur;				//0.1最高电流  	-400A
	INT16U DevLCur;				//0.1最低电流	-400A
}_YKC_SEND_CMD17;


//结束充电，主动上报一次 BSD
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	
    INT8U   EndChargeSOC;          //中止荷电状态SOC%
    INT16U  UnitBatteryminVol;     //动力蓄电池单体最低电压
    INT16U  UnitBatteryMaxVol;     //动力蓄电池单体最高电压
    INT8U   BatteryMinTemp;        //动力蓄电池最低温度
    INT8U   BatteryMaxTemp;        //动力蓄电池最高温度  
	
	INT16U  ChargeTime;				//充电时间 min
	INT16U	ChargeKwh;				//充电输出能量 0.1
	INT8U 	DevNum[4];				//电桩充电机编号 
}_YKC_SEND_CMD19;


//错误报文  BEM CEM 只上报一次
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U BEM[4];
	INT8U CEM[4];
}_YKC_SEND_CMD1B;

//BMS 终止  主动上报  BST
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U BST[4];
}_YKC_SEND_CMD1D;

//用户 终止  主动上报  CST
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U CST[4];
}_YKC_SEND_CMD21;


//15S上报一次
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始

	_BMS_BCL_CONTEXT BCL;
	_BMS_BCS_CONTEXT BCS;
	INT16U OutVol;		//输出电压   0.1
	INT16U OutCur;		//输出电流  0.1 -400A偏移量
	INT16U ChargeTime;	//累计充电时间
}_YKC_SEND_CMD23;

//15S上报一次  BSM
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始

	_BMS_BSM_CONTEXT BSM;
	
}_YKC_SEND_CMD25;

//停止充电应答
__packed typedef struct
{
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始

	INT8U StopResul;	//停止结果  0失败 1成功
	INT8U Fail;			//失败原因    0x00 无
}_YKC_SEND_CMD35;


//升级应答
__packed typedef struct
{
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U UpdataState;		//0x00-成功 0x01-编号错误 0x02-程序与桩型号不符 0x03-下载更新文件超时
}_YKC_SEND_CMD93;

__packed typedef struct{
	//CP56格式
	INT16U Second;  //0.001秒
	INT8U	Minute;
	INT8U	Hour;
	INT8U	Day;
	INT8U	Month;
	INT8U	Year;
}YCK_TIME;

__packed typedef struct{
	INT32U Pric;			//精确到小数点5位
	INT32U PowerKwh;		//电量 精确到小数点4位
	INT32U JSKwh;			//计损电量 精确到小数点4位
	INT32U Money;			//尖金额 精确到小数点4位
}_BILL_FS_INFO;

//发送交易记录
__packed typedef struct
{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];    //BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始

	YCK_TIME	StartTime;				//开始时间  CP56Time2a格式
	YCK_TIME StopTime;				//结束时间  CP56Time2a格式
	
	_BILL_FS_INFO BillFsInfo[4];	//分时电量信息

	INT32U 		ChargeStartKwh; 		//总起示值  精确到小数点后4位
	INT8U      	ChargeStartKwhRes;		//因为是5字节补充
	
	INT32U 		ChargeStopKwh; 			//总止示值  精确到小数点后4位
	INT8U      	ChargeStopKwhRes;		//因为是5字节补充
	
	INT32U 		ChargeAllKwh; 			//总电量  精确到小数点后4位
	INT32U		JSAllKwh;				//计损总电量  精确到小数点后4位
	INT32U 		ChargeMoney;			//充电总金额 电费+服务费  精确到小数点后4位
	INT8U		CarVin[17];				//电动汽车唯一标识
	INT8U		ChargeType;				// 1 app启动  2卡启动   3离线卡启动			5VIN码启动
	YCK_TIME 		JYTime;				//交易时间
	INT8U StopReason;					//停止原因
	INT8U CardNum[8];					//物理卡号
}_YKC_SEND_CMD3B;

__packed typedef struct{
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	YCK_TIME	CurTime;	
}_YKC_SEND_CMD55;   //校时应答


//下发费率 cmd = 0x0a
__packed typedef struct{
	INT32U 	Fric;			//费率     5位小数
	INT32U	ServiceFric;	//服务费	5位小数
}_PRC_INFO;
__packed typedef struct{
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	INT8U PrcNum[2];		//计费模型编号  固定值： 01 00
	_PRC_INFO PrcInfo[4];	//尖峰平古费率服务费
	INT8U Calculate;		//计损比例 目前全部为0
	INT8U			FricNum[48];	//费率号
}_YKC_RECV_CMD0A;

//==启动充电鉴权结果
__packed typedef struct {
    INT8U SerialNum[16];	//交易流水号
    INT8U DevNun[7];   	 	//BCD  不足7位补充0
    INT8U GunNum;			//枪号 从1开始
    INT8U  CardNum1[8];		//逻辑卡号
	INT32U Money;			//余额  保留2位小数
	INT8U Success_flag;  //0x00 失败 0x01 成功
    INT8U fail;		  //失败原因  0x01 账户不存在  0x02 账户冻结  0x03 账户余额不足  0x04 该卡存在未结账记录  0x05 桩停用
} _YKC_RECV_CMD32;

//卡VIN 并充应答
__packed typedef struct {
    INT8U SerialNum[16];	//交易流水号
    INT8U DevNun[7];   	 	//BCD  不足7位补充0
    INT8U GunNum;			//枪号 从1开始
    INT8U  CardNum1[8];		//逻辑卡号
	INT32U Money;			//余额  保留2位小数
	INT8U Success_flag;  //0x00 失败 0x01 成功
    INT8U fail;		  //失败原因  0x01 账户不存在  0x02 账户冻结  0x03 账户余额不足  0x04 该卡存在未结账记录  0x05 桩停用
	INT8U SerialNumber[6];			//0xA1上并充序号
} _YKC_RECV_CMDA2;

__packed typedef struct{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	INT8U GunNum;			//枪号 从1开始
	INT8U CardNum1[8];		//逻辑卡号
	INT8U CardNum2[8];		//物理卡号
	INT32U Money;			//余额  保留2位小数
}_YKC_RECV_CMD34;


__packed typedef struct{
	INT8U SerialNum[16];	//交易流水号
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	INT8U GunNum;			//枪号 从1开始
	INT8U CardNum1[8];		//逻辑卡号
	INT8U CardNum2[8];		//物理卡号
	INT32U Money;			//余额  保留2位小数
	INT8U SerialNumber[6];			//0xA1上并充序号
}_YKC_RECV_CMDA4;

__packed typedef struct{
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	YCK_TIME	CurTime;	
}_YKC_RECV_CMD56;


__packed typedef struct{
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	INT8U PrcNum[2];		//计费模型编号  固定值： 01 00
	_PRC_INFO PrcInfo[4];	//尖峰平古费率服务费
	INT8U Calculate;		//计损比例 目前全部为0
	INT8U			FricNum[48];	//费率号
}_YKC_RECV_CMD58;


__packed typedef struct{
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	INT8U GunNum;		//枪号，从1开始
	INT8U  CardNum[8];	//卡号
	INT32U Balance;		//余额 保留2位小数
}_YKC_RECV_CMD42;

__packed typedef struct{
	INT8U DevNun[7];   	 	//BCD  不足7位补充0
	INT8U DevType;   	 	//桩型号 0x01：直流 0x02：交流
	INT16U DevPower;		//桩功率
	INT8U	SerAdd[16];	//升级服务器地址	
	INT16U	Port;			//端口
	INT8U     usename[16];	//用户名  （ASCII 码）不足 16 位补零
    INT8U     password[16];	//密码  （ASCII 码）不足 16 位补零
	INT8U		FilePath[32];	//文件路径   （ASCII 码）不足 32 位补零，文件路径名由平台定义
	INT8U		cmd;			//0x01：立即执行 0x02：空闲执行
    INT8U		downloadtime;	//下载超时时间 min
}_YKC_RECV_CMD94;


__packed typedef struct {
    uint8_t DevNun[7];   	 	//桩编号 BCD  不足7位补充0
    uint8_t GUNNum;		    //BCD  枪号
	uint8_t QR_code[150];         //二维码内容
} _YKC_RECV_CMD5A;

extern _PRICE_SET PriceSet;							//电价设置 
extern _PRICE_SET A_PriceSet;							//电价设置 
extern _PRICE_SET B_PriceSet;							//电价设置 
//INT8U IfRecvBill = 0;							     //是否收到了电价设置，收到了置1
INT8U IfAPPStop[GUN_MAX] = {0};								//APP是否主动停止    0未主动停止  1主动停止


//发送数据
INT32U Balance[GUN_MAX]; //余额
_YKC_SEND_CMD01 YKCSendCmd1;			//注册帧
_YKC_SEND_CMD03 YKCSendCmd3[GUN_MAX];	//发送枪心跳，（心跳中有各个枪的状态）	
_YKC_SEND_CMD31 YKCSendCmd31[GUN_MAX];	//发送卡鉴权
_YKC_SEND_CMD33 YKCSendCmd33[GUN_MAX];	//启动应答
_YKC_SEND_CMDA1 YKCSendCmdA1[GUN_MAX];	//并充发送卡鉴权
_YKC_SEND_CMDA3 YKCSendCmdA3[GUN_MAX];	//并充启动应答
_YKC_SEND_CMD13 YKCSendCmd13[GUN_MAX];	//发送实时数据
_YKC_SEND_CMD15 YKCSendCmd15[GUN_MAX];	//发送充电握手  BRM
_YKC_SEND_CMD17 YKCSendCmd17[GUN_MAX];  //发送参数配置	BCP
_YKC_SEND_CMD19 YKCSendCmd19[GUN_MAX];	//充电结束      BSD
_YKC_SEND_CMD1B  YKCSendCmd1B[GUN_MAX];	//错误报文   BEM CEM
_YKC_SEND_CMD1D YKCSendCmd1D[GUN_MAX];	//bms终止   BST
_YKC_SEND_CMD21 YKCSendCmd21[GUN_MAX];	//设备终止  CST
_YKC_SEND_CMD23 YKCSendCmd23[GUN_MAX];	//充电中实时数据
_YKC_SEND_CMD25 YKCSendCmd25[GUN_MAX];	//BSM  15秒上报一次
_YKC_SEND_CMD35 YKCSendCmd35[GUN_MAX];	//停止应答
_YKC_SEND_CMD3B YKCSendCmd3B[GUN_MAX];	//上传订单
_YKC_SEND_CMD55 YKCSendCmd55;			//校时应答
_YKC_SEND_CMD93 YKCSendCmd93;			//升级应答
_YKC_RECV_CMD5A YKCRecvCmd5A[GUN_MAX];  //二维码下发
/**************************************接收数据*************************/
_YKC_RECV_CMD0A YKCRecvCmd0A;			//接收倒费率
_YKC_RECV_CMD32	YKCRecvCmd32[GUN_MAX];			//接收卡鉴权结果
_YKC_RECV_CMD34	YKCRecvCmd34[GUN_MAX];			//开关机指令
_YKC_RECV_CMDA2	YKCRecvCmdA2[GUN_MAX];			//并充接收卡鉴权结果
_YKC_RECV_CMDA4	YKCRecvCmdA4[GUN_MAX];			//并充开关机指令
_YKC_RECV_CMD56 YKCRecvCmd56;				//校时放回
_YKC_RECV_CMD58 YKCRecvCmd58;				//费率设置
_YKC_RECV_CMD42 YKCRecvCmd42[GUN_MAX];   //更新余额
_YKC_RECV_CMD94 YKCRecvCmd94;			//远程升级
INT8U   YKC_SendGunAHand(void);   //BRM
INT8U   YKC_SendGunBHand(void); 
INT8U   YKC_SendGunAConf(void); 	//BCP
INT8U   YKC_SendGunBConf(void); 

INT8U   YKC_SendRegisterCmd1(void); 	//云快充送注册帧
INT8U   YKC_SendHearGunACmd3(void);		//发送心跳A
INT8U   YKC_SendHearGunBCmd3(void);		//发送心跳B
INT8U YKC_SendPriodSJDataGunACmd13(void);  	//发送A枪实时数据
INT8U YKC_SendPriodSJDataGunBCmd13(void);  	//发送B枪实时数据
INT8U YKC_SendSJDataGunACmd23(void);  	//发送A枪bms实时数据
INT8U YKC_SendSJDataGunBCmd23(void);  	//发送B枪bms实时数据
INT8U YKC_SendBSMGunACmd25(void);  	//发送A枪BSM数据
INT8U YKC_SendBSMGunBCmd25(void);  	//发送B枪BSM数据

INT8U SerialNum[GUN_MAX][16] = {0};			//交易流水号
_YKC_RECV_CMD0A YKCGunBill[GUN_MAX];			//枪费率
_4G_START_TYPE YKCStartType[GUN_MAX] ={ _4G_APP_START,_4G_APP_START};					//0表示App启动 1表示刷卡启动
_4G_START_MODE YKCStartMode[GUN_MAX] = {START_STANDARD,START_STANDARD}; 

_4G_SEND_TABLE YKCSendTable[YKC_SEND_FRAME_LEN] = {
	{0,    0,    SYS_DELAY_10s, 	YKC_SendRegisterCmd1		},  //发送注册帧

	{0,    0,    SYS_DELAY_10s, 	YKC_SendHearGunACmd3		},  //发送A枪心跳状态

	{0,    0,    SYS_DELAY_10s, 	YKC_SendHearGunBCmd3		},  //发送B枪心跳状态

	{0,    0,    SYS_DELAY_15s, 	YKC_SendPriodSJDataGunACmd13		},  //发送A卡枪实时数据   上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
	
	{0,    0,    SYS_DELAY_15s, 	YKC_SendPriodSJDataGunBCmd13		},  //发送A卡枪实时数据   上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
	
	{0,    0,    SYS_DELAY_15s, 	YKC_SendSJDataGunACmd23		},  //充电 15 秒
	
	{0,    0,    SYS_DELAY_15s, 	YKC_SendSJDataGunBCmd23		},  //充电 15 秒
	
	{0,    0,    SYS_DELAY_15s, 	YKC_SendBSMGunACmd25		},  //充电 15 秒
	
	{0,    0,    SYS_DELAY_15s, 	YKC_SendBSMGunBCmd25		},  //充电 15 秒
	
};



INT8U   YKC_RecvRegisterAck(INT8U *pdata,INT16U len);
INT8U   YKC_RecvHeartAck(INT8U *pdata,INT16U len);
INT8U   YKC_RecvPrimodelAck(INT8U *pdata,INT16U len);
INT8U YKC_RecvPricAck(INT8U *pdata,INT16U len);
INT8U YKC_RecvStart(INT8U *pdata,INT16U len);
INT8U YKC_BCRecvStart(INT8U *pdata,INT16U len);
INT8U YKC_RecvStop(INT8U *pdata,INT16U len);
INT8U YKC_RecvCard(uint8_t *pdata,uint16_t len);
INT8U YKC_BCRecvCard(uint8_t *pdata,uint16_t len);
INT8U YKC_RecvReadData(INT8U *pdata,INT16U len);
static INT8U   YKC_RecvRecordAck(INT8U *pdata,INT16U len);
static INT8U   YKC_RecvTime(INT8U *pdata,INT16U len);
static INT8U   YKC_RecvPricSet(INT8U *pdata,INT16U len);
static INT8U   YKC_RecvBalance(INT8U *pdata,INT16U len);
static INT8U   YKC_RecvUpdate(INT8U *pdata,INT16U len);
static INT8U   YKC_RecvQRcode(INT8U *pdata,INT16U len);
_4G_RECV_TABLE YKCRecvTable[YKC_RECV_FRAME_LEN] = {
	
	{YKC_RECV_CMD_02			,	YKC_RecvRegisterAck		}, 		//pc返回确认信息
	
	{YKC_RECV_CMD_04			,	YKC_RecvHeartAck		}, 		//pc返回确认信息
	
	
	
	{YKC_RECV_CMD_06			,	YKC_RecvPrimodelAck		}, 		//计费模型应答
	
	{YKC_RECV_CMD_0A			,	YKC_RecvPricAck			}, 		//费率接受
	
	{YKC_RECV_CMD_32			,	YKC_RecvCard			}, 		//卡鉴权接收结果
		
	{YKC_RECV_CMD_34			,	YKC_RecvStart			}, 		//接收到启动命令
	
	{YKC_RECV_CMD_A2			,	YKC_BCRecvCard			}, 		//并充卡鉴权接收结果
		
	{YKC_RECV_CMD_A4			,	YKC_BCRecvStart			}, 		//并充接收到启动命令
	
	{YKC_RECV_CMD_36			,	YKC_RecvStop			}, 		//接收到停止充电
	
	{YKC_RECV_CMD_12			,	YKC_RecvReadData		}, 		//读取实时数据
	
	{YKC_RECV_CMD_40			,	YKC_RecvRecordAck		}, 		//交易记录返回
	
	{YKC_RECV_CMD_56			,	YKC_RecvTime			}, 		//接收到校时
	
	{YKC_RECV_CMD_58			,	YKC_RecvPricSet			}, 		//费率设置
	
	
	{YKC_RECV_CMD_42			,	YKC_RecvBalance			}, 		//更新余额
	
	{YKC_RECV_CMD_94			,	YKC_RecvUpdate			}, 		//远程升级
	
	{YKC_RECV_CMD_5A			,	YKC_RecvQRcode			}, 		//下发二维码-铁塔
	
};



__packed typedef struct {
	INT8U Reason;
	INT8U CodeNum;
}_YKC_FAIL_CODE;		//后台协议与CCU传过来的启动完成帧，停止完成帧对应上
/*******************************启动失败原因*********************************/
#define YKC_TABLE_START_FAIL  8
#define YKC_TABLE_STOP_REASON  11
const static _YKC_FAIL_CODE YKCStartFailCode[YKC_TABLE_START_FAIL] = 
{
	{ENDFAIL_HANDERR ,			0x4B},		//枪未正确连接  			100D
	
	{ENDFAIL_EMERGENCY ,		0x50},		//急停 						3003
	
	{ENDFAIL_ELECLOCKERR ,		0x55},		//充电接口电子锁故障		3018
	
	{ENDFAIL_INSOLUTIONERR ,	0X57},		//BMS绝缘故障				500D
	
	{ENDFAIL_BRMTIMEOUT ,		0X5A},		//BRM车辆辨识报文超时 		500A
	
	{ENDFAIL_BCPTIMEOUT ,		0X5B},		//BRM车辆辨识报文超时 		5001
	
	{ENDFAIL_BRORUNTIMEOUT ,	0X5C},		//BRO充电准备就绪报文超时 	5002
	
	{ENDFAIL_OTHERERR ,			0X90},		//其他原因 			303B
	
};



/*******************************停止失败原因*********************************/
const static _YKC_FAIL_CODE YKCStopFailCode[YKC_TABLE_STOP_REASON] = 
{	
	{STOP_BALANCE ,				0X6E},		//达到设置充电金额停止				
	
	{STOP_ERR_NONE ,			0x41},	  	//达到条件停止
	
	{STOP_BSMNORMAL ,			0x41},		//达到条件停止
	
	{STOP_SOC		,			0x41},		//达到条件停止
	
	{STOP_HANDERR ,				0x6B},		//枪未正确连接  		
	
	{STOP_EMERGENCY ,			0X72},		//急停 					
	
	
	{STOP_ELECLOCKERR ,			0X77},		//充电接口电子锁故障 	
	
	
	{STOP_OUTVOLTVORE ,			0X75},		//输出电压过压故障		
	
	{STOP_OUTVOLTUNDER ,		0X75},		//输出电压欠压故障		
	
	{STOP_OUTCURROVER ,			0X75},		//输出电压过流故障		
	
	
	{STOP_OTHERERR ,			0X90},		//其他原因 				

};

INT8U gabyCRCHi[] =
{
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
0x80,0x41,0x00,0xc1,0x81,0x40
};

INT8U gabyCRCLo[] =
{
0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,
0x07,0xc7,0x05,0xc5,0xc4,0x04,0xcc,0x0c,0x0d,0xcd,
0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,
0x08,0xc8,0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,
0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,0x14,0xd4,
0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,
0x11,0xd1,0xd0,0x10,0xf0,0x30,0x31,0xf1,0x33,0xf3,
0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,
0x3b,0xfb,0x39,0xf9,0xf8,0x38,0x28,0xe8,0xe9,0x29,
0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,
0xec,0x2c,0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,
0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,0xa0,0x60,
0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,
0xa5,0x65,0x64,0xa4,0x6c,0xac,0xad,0x6d,0xaf,0x6f,
0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,
0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,0xb4,0x74,0x75,0xb5,
0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,
0x70,0xb0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9c,0x5c,
0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,
0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4b,0x8b,
0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
0x43,0x83,0x41,0x81,0x80,0x40
};

/*****************************************************************************
* Function     : APP_GetAPSta
* Description  : 获取安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_GetYKCStartType(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return _4G_APP_START;
	}
	return (INT8U)YKCStartType[gun];
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
INT8U   APP_SetYKCStartType(INT8U gun ,_4G_START_TYPE  type)
{
	if((type >=  _4G_APP_MAX) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	
	YKCStartType[gun] = type;
	return TRUE;
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
static INT16U ykc_get_crc_Data(INT8U * pData, INT8U len)
{
	INT8U byCRCHi = 0xff;
	INT8U byCRCLo = 0xff;
	INT8U byIdx;
	INT16U crc = 0;
	while(len--)
	{
	byIdx = byCRCHi ^(*(pData++));
	byCRCHi = byCRCLo ^ gabyCRCHi[byIdx];
	byCRCLo = gabyCRCLo[byIdx];
	}
	crc = byCRCHi;
	crc <<= 8;
	crc += byCRCLo;
	return crc;

}


/*****************************************************************************
* Function     : APP_GetYKCNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetYKCNetMoney(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	  return Balance[gun];
}

/*****************************************************************************
* Function     : APP_GetYKCBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetYKCBatchNum(INT8U gun)
{
	NOP();
	return NULL;
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
INT8U   YKC_SendFrameDispose(void)
{
	INT8U i;

	for(i = 0;i < YKC_SEND_FRAME_LEN;i++)
	{
		if(YKCSendTable[i].cycletime == 0)
		{
			continue;
		}
		YKCSendTable[i].curtime = OSTimeGet();
		if((YKCSendTable[i].curtime >= YKCSendTable[i].lasttime) ? ((YKCSendTable[i].curtime - YKCSendTable[i].lasttime) >= YKCSendTable[i].cycletime) : \
		((YKCSendTable[i].curtime + (0xFFFFFFFFu - YKCSendTable[i].lasttime)) >= YKCSendTable[i].cycletime))
		{
			YKCSendTable[i].lasttime = YKCSendTable[i].curtime;
			if(YKCSendTable[i].Sendfunc != NULL)
			{
				YKCSendTable[i].Sendfunc();
				return TRUE;
			}
		}
		
	}
	return TRUE;
}



/*****************************************************************************
* Function     : YKCFreamSend
* Description  : 合肥乾古帧发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   YKCFreamSend(INT8U cmd,INT8U *pdata, INT16U len)
{
	INT16U crc;
	INT8U Sendbuf[200];
	if((pdata == NULL) || (!len)  || (len > 190))
	{
		return FALSE;
	}
	//1字节帧投
	Sendbuf[0] = 0x68;
	//1字节数据长度
	Sendbuf[1] = len+4;
	//2字节序列号
	Sendbuf[2] = 0x00;
	Sendbuf[3] = 0x00;	
	//1个字节加密标志
	Sendbuf[4] = 0x00;	
	//命令
	Sendbuf[5] = cmd;	
	
	memcpy(&Sendbuf[6],pdata,len);
	//2字节crc
	
	

	
	crc = ykc_get_crc_Data(&Sendbuf[2],4+len);
	
	Sendbuf[7+len] = crc & 0x00ff;
	Sendbuf[6+len] = (crc  >> 8)& 0x00ff;
	ModuleSIM7600_SendData(0, Sendbuf,(8+len)); //发送数据
	//OSTimeDly(SYS_DELAY_500ms); 
	return TRUE;
}

/*******************************周期性发送数据*******************************/
/*****************************************************************************
* Function     : YKC_SendRegister
* Description  : 云快充
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日完成
*****************************************************************************/
INT8U   YKC_SendRegisterCmd1(void)
{
	INT8U * pdevnum = APP_GetDevNum(); //设备号 (原始为ASCII)
	if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
	{
		return FALSE;
	}
	memset(&YKCSendCmd1,0,sizeof(_YKC_SEND_CMD01));
	YCKDevNum[0] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
	YCKDevNum[1] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
	YCKDevNum[2] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
	YCKDevNum[3] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
	YCKDevNum[4] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
	YCKDevNum[5] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
	YCKDevNum[6] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	memcpy(YKCSendCmd1.DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd1.Devtype = 0x00;
	YKCSendCmd1.GunNum = APP_GetGunNum();
	YKCSendCmd1.Vsion = 0x10;		//通信版本  V1.6
	YKCSendCmd1.NetState = 0; 		//SIM
	YKCSendCmd1.Operator = 0;		//移动
	return YKCFreamSend(YKC_SEND_CMD_01,(INT8U*)&YKCSendCmd1,sizeof(YKCSendCmd1));
}


//心跳
/*****************************************************************************
* Function     : YKC_SendHearGunACmd3
* Description  : 云快充发送A枪心跳状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendHearGunACmd3(void)
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&YKCSendCmd3[GUN_A],0,sizeof(_YKC_SEND_CMD03));
	memcpy(YKCSendCmd3[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd3[GUN_A].GunNum = 1;
	if(APP_GetErrState(GUN_A) == 0x04)
	{
		YKCSendCmd3[GUN_A].GunState  = 0x01;   //正常
	}		
	else
	{
		YKCSendCmd3[GUN_A].GunState  = 0x00;	//故障
	}
	return YKCFreamSend(YKC_SEND_CMD_03,(INT8U*)&YKCSendCmd3[GUN_A],sizeof(_YKC_SEND_CMD03));
}


/*****************************************************************************
* Function     : YKC_SendHearGunACmd3
* Description  : 云快充发送A枪心跳状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendHearGunBCmd3(void)
{
	if(SYSSet.SysSetNum.UseGun == 1)
	{
		return FALSE;
	}
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&YKCSendCmd3[GUN_B],0,sizeof(_YKC_SEND_CMD03));
	memcpy(YKCSendCmd3[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd3[GUN_B].GunNum = 2;
	if(APP_GetErrState(GUN_B) == 0x04)
	{
		YKCSendCmd3[GUN_B].GunState  = 0x01;   //正常
	}		
	else
	{
		YKCSendCmd3[GUN_B].GunState  = 0x00;	//故障
	}
	return YKCFreamSend(YKC_SEND_CMD_03,(INT8U*)&YKCSendCmd3[GUN_B],sizeof(_YKC_SEND_CMD03));
}

/*****************************************************************************
* Function     : YKC_SendSJDataGunACmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendSJDataGunACmd13(void)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	static _NET_GUN_STATE laststate = GUN_IDLE;		//上一次状态，主要是发送BCP BRM
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	if((pdisp_conrtol->NetGunState[GUN_A] != laststate)  && (pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING) )
	{
		//变为充电状态
		//发送BCP,BRM
		YKC_SendGunAHand();   //BRM
		YKC_SendGunAConf(); 	//BCP
	}
	laststate = pdisp_conrtol->NetGunState[GUN_A];
	
	
	memset(&YKCSendCmd13[GUN_A],0,sizeof(_YKC_SEND_CMD13));
	memcpy(YKCSendCmd13[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd13[GUN_A].GunNum = 1;
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		YKCSendCmd13[GUN_A].State = 3;
	}
	else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)
	{
		YKCSendCmd13[GUN_A].State = 1; //故障
	}else
	{
		YKCSendCmd13[GUN_A].State = 2; //待机
	}
	
	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
	{
		YKCSendCmd13[GUN_A].GunState = 0;
	}
	else
	{
		YKCSendCmd13[GUN_A].GunState = 1;
	}
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		memcpy(YKCSendCmd13[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
		
		YKCSendCmd13[GUN_A].OutVol = PowerModuleInfo[GUN_A].OutputInfo.Vol;		//输出电压  0.1
		YKCSendCmd13[GUN_A].OutCur = PowerModuleInfo[GUN_A].OutputInfo.Cur;		//输出电流	0.1
		YKCSendCmd13[GUN_A].GunT = 100;			//枪线维度	偏移 -50
		YKCSendCmd13[GUN_A].Soc = BMS_BCS_Context[GUN_A].SOC;
		
		YKCSendCmd13[GUN_A].BatteryT =  BMS_BSM_Context[GUN_A].MaxbatteryTemprature;		//电池组最高温度
		YKCSendCmd13[GUN_A].ChargeTime =  puserinfo->ChargeTime;	//充电时间  min
		YKCSendCmd13[GUN_A].RTime = BMS_BCS_Context[GUN_A].RemainderTime;	//剩余充电时间  min
		YKCSendCmd13[GUN_A].ChargeKwh =  puserinfo->TotalPower4;	//充电读数   精确到小数点4位
		YKCSendCmd13[GUN_A].ChargeMoney = puserinfo->TotalBill;	//已充金额	精确到小数点4位
	}
	if(GetStartFailType(GUN_A) == ENDFAIL_EMERGENCY)
	{
		YKCSendCmd13[GUN_A].Fail[0] = 0x01;
	}
	return YKCFreamSend(YKC_SEND_CMD_13,(INT8U*)&YKCSendCmd13[GUN_A],sizeof(_YKC_SEND_CMD13));
}

/*****************************************************************************
* Function     : YKC_SendSJDataGunBCmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendSJDataGunBCmd13(void)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	static _NET_GUN_STATE laststate = GUN_IDLE;		//上一次状态，主要是发送BCP BRM
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	
	if((pdisp_conrtol->NetGunState[GUN_B] != laststate)  && (pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING) )
	{
		//变为充电状态
		//发送BCP,BRM
		YKC_SendGunBHand();   //BRM
		YKC_SendGunBConf(); 	//BCP
	}
	laststate = pdisp_conrtol->NetGunState[GUN_B];
	
	memset(&YKCSendCmd13[GUN_B],0,sizeof(_YKC_SEND_CMD13));
	memcpy(YKCSendCmd13[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd13[GUN_B].GunNum = 2;
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		YKCSendCmd13[GUN_B].State = 3;
	}
	else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)
	{
		YKCSendCmd13[GUN_B].State = 1; //故障
	}else
	{
		YKCSendCmd13[GUN_B].State = 2; //待机
	}
	
	if(GetGunState(GUN_B) == GUN_DISCONNECTED)
	{
		YKCSendCmd13[GUN_B].GunState = 0;
	}
	else
	{
		YKCSendCmd13[GUN_B].GunState = 1;
	}
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		memcpy(YKCSendCmd13[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
		
		YKCSendCmd13[GUN_B].OutVol = PowerModuleInfo[GUN_B].OutputInfo.Vol;		//输出电压  0.1
		YKCSendCmd13[GUN_B].OutCur = PowerModuleInfo[GUN_B].OutputInfo.Cur;		//输出电流	0.1
		YKCSendCmd13[GUN_B].GunT = 100;			//枪线维度	偏移 -50
		YKCSendCmd13[GUN_B].Soc = BMS_BCS_Context[GUN_B].SOC;
		
		YKCSendCmd13[GUN_B].BatteryT =  BMS_BSM_Context[GUN_B].MaxbatteryTemprature;		//电池组最高温度
		YKCSendCmd13[GUN_B].ChargeTime =  puserinfo->ChargeTime;	//充电时间  min
		YKCSendCmd13[GUN_B].RTime = BMS_BCS_Context[GUN_B].RemainderTime;	//剩余充电时间  min
		YKCSendCmd13[GUN_B].ChargeKwh =  puserinfo->TotalPower4;	//充电读数   精确到小数点4位
		YKCSendCmd13[GUN_B].ChargeMoney = puserinfo->TotalBill;	//已充金额	精确到小数点4位
	}
	if(GetStartFailType(GUN_B) == ENDFAIL_EMERGENCY)
	{
		YKCSendCmd13[GUN_B].Fail[0] = 0x01;
	}
	return YKCFreamSend(YKC_SEND_CMD_13,(INT8U*)&YKCSendCmd13[GUN_B],sizeof(_YKC_SEND_CMD13));
}


/*****************************************************************************
* Function     : YKC_SendSJDataGunACmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendPriodSJDataGunACmd13(void)
{
	static INT8U count = 0;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);

	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING)  //未再充电中5分钟发送一次
	{
		//枪状态改变需要立马发送
		if(++count < SYS_DELAY_5M/SYS_DELAY_15s)
		{
			return FALSE;
		}
	}
	count = 0;
	
	
	
	memset(&YKCSendCmd13[GUN_A],0,sizeof(_YKC_SEND_CMD13));
	memcpy(YKCSendCmd13[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd13[GUN_A].GunNum = 1;
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		YKCSendCmd13[GUN_A].State = 3;
	}
	else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)
	{
		YKCSendCmd13[GUN_A].State = 1; //故障
	}else
	{
		YKCSendCmd13[GUN_A].State = 2; //待机
	}
	
	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
	{
		YKCSendCmd13[GUN_A].GunState = 0;
	}
	else
	{
		YKCSendCmd13[GUN_A].GunState = 1;
	}
	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
	{
		memcpy(YKCSendCmd13[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
		
		YKCSendCmd13[GUN_A].OutVol = PowerModuleInfo[GUN_A].OutputInfo.Vol;		//输出电压  0.1
		YKCSendCmd13[GUN_A].OutCur = PowerModuleInfo[GUN_A].OutputInfo.Cur;		//输出电流	0.1
		YKCSendCmd13[GUN_A].GunT = 100;			//枪线维度	偏移 -50
		YKCSendCmd13[GUN_A].Soc = BMS_BCS_Context[GUN_A].SOC;
		
		YKCSendCmd13[GUN_A].BatteryT =  BMS_BSM_Context[GUN_A].MaxbatteryTemprature;		//电池组最高温度
		YKCSendCmd13[GUN_A].ChargeTime =  puserinfo->ChargeTime;	//充电时间  min
		YKCSendCmd13[GUN_A].RTime = BMS_BCS_Context[GUN_A].RemainderTime;	//剩余充电时间  min
		YKCSendCmd13[GUN_A].ChargeKwh =  puserinfo->TotalPower4;	//充电读数   精确到小数点4位
		YKCSendCmd13[GUN_A].ChargeMoney = puserinfo->TotalBill;	//已充金额	精确到小数点4位
	}
	if(GetStartFailType(GUN_A) == ENDFAIL_EMERGENCY)
	{
		YKCSendCmd13[GUN_A].Fail[0] = 0x01;
	}
	return YKCFreamSend(YKC_SEND_CMD_13,(INT8U*)&YKCSendCmd13[GUN_A],sizeof(_YKC_SEND_CMD13));
}

/*****************************************************************************
* Function     : YKC_SendSJDataGunBCmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendPriodSJDataGunBCmd13(void)
{
		static INT8U count = 0;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	if(SYSSet.SysSetNum.UseGun == 1)
	{
		return FALSE;
	}
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}

	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)  //未再充电中5分钟发送一次
	{
		//枪状态改变需要立马发送
		if(++count < SYS_DELAY_5M/SYS_DELAY_15s)  
		{
			return FALSE;
		}
	}
	count = 0;
	
	
	memset(&YKCSendCmd13[GUN_B],0,sizeof(_YKC_SEND_CMD13));
	memcpy(YKCSendCmd13[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd13[GUN_B].GunNum = 2;
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		YKCSendCmd13[GUN_B].State = 3;
	}
	else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)
	{
		YKCSendCmd13[GUN_B].State = 1; //故障
	}else
	{
		YKCSendCmd13[GUN_B].State = 2; //待机
	}
	
	if(GetGunState(GUN_B) == GUN_DISCONNECTED)
	{
		YKCSendCmd13[GUN_B].GunState = 0;
	}
	else
	{
		YKCSendCmd13[GUN_B].GunState = 1;
	}
	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
	{
		memcpy(YKCSendCmd13[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
		
		YKCSendCmd13[GUN_B].OutVol = PowerModuleInfo[GUN_B].OutputInfo.Vol;		//输出电压  0.1
		YKCSendCmd13[GUN_B].OutCur = PowerModuleInfo[GUN_B].OutputInfo.Cur;		//输出电流	0.1
		YKCSendCmd13[GUN_B].GunT = 100;			//枪线维度	偏移 -50
		YKCSendCmd13[GUN_B].Soc = BMS_BCS_Context[GUN_B].SOC;
		
		YKCSendCmd13[GUN_B].BatteryT =  BMS_BSM_Context[GUN_B].MaxbatteryTemprature;		//电池组最高温度
		YKCSendCmd13[GUN_B].ChargeTime =  puserinfo->ChargeTime;	//充电时间  min
		YKCSendCmd13[GUN_B].RTime = BMS_BCS_Context[GUN_B].RemainderTime;	//剩余充电时间  min
		YKCSendCmd13[GUN_B].ChargeKwh =  puserinfo->TotalPower4 ;	//充电读数   精确到小数点4位
		YKCSendCmd13[GUN_B].ChargeMoney = puserinfo->TotalBill;	//已充金额	精确到小数点4位
	}
	if(GetStartFailType(GUN_B) == ENDFAIL_EMERGENCY)
	{
		YKCSendCmd13[GUN_B].Fail[0] = 0x01;
	}
	return YKCFreamSend(YKC_SEND_CMD_13,(INT8U*)&YKCSendCmd13[GUN_B],sizeof(_YKC_SEND_CMD13));
}

/*****************************************************************************
* Function     : YKC_SendSJDataGunACmd23
* Description  : 云快充发送实时数据 充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendSJDataGunACmd23(void)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING)  //未再充电中5分钟发送一次
	{
		//枪状态改变需要立马发送
		return FALSE;
	}

	memset(&YKCSendCmd23[GUN_A],0,sizeof(_YKC_SEND_CMD23));
	memcpy(YKCSendCmd23[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd23[GUN_A].GunNum = 1;
	
	memcpy((INT8U*)&YKCSendCmd23[GUN_A].BCL,(INT8U*)&BMS_BCL_Context[GUN_A],sizeof(_BMS_BCL_CONTEXT));
	memcpy((INT8U*)&YKCSendCmd23[GUN_A].BCS,(INT8U*)&BMS_BCS_Context[GUN_A],sizeof(_BMS_BCS_CONTEXT));
	memcpy(YKCSendCmd23[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	YKCSendCmd23[GUN_A].OutVol = PowerModuleInfo[GUN_A].OutputInfo.Vol;		//输出电压  0.1
	YKCSendCmd23[GUN_A].OutCur = 4000 - PowerModuleInfo[GUN_A].OutputInfo.Cur;		//输出电流	0.1
	
	YKCSendCmd23[GUN_A].ChargeTime =  puserinfo->ChargeTime;	//充电时间  min

	return YKCFreamSend(YKC_SEND_CMD_23,(INT8U*)&YKCSendCmd23[GUN_A],sizeof(_YKC_SEND_CMD23));
}


/*****************************************************************************
* Function     : YKC_SendSJDataGunACmd23
* Description  : 云快充发送实时数据 充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendSJDataGunBCmd23(void)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	if(SYSSet.SysSetNum.UseGun == 1)
	{
		return FALSE;
	}
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)  //未再充电中5分钟发送一次
	{
		//枪状态改变需要立马发送
		return FALSE;
	}

	memset(&YKCSendCmd23[GUN_B],0,sizeof(_YKC_SEND_CMD23));
	memcpy(YKCSendCmd23[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd23[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	YKCSendCmd23[GUN_B].GunNum = 2;
	
	memcpy((INT8U*)&YKCSendCmd23[GUN_B].BCL,(INT8U*)&BMS_BCL_Context[GUN_B],sizeof(_BMS_BCL_CONTEXT));
	memcpy((INT8U*)&YKCSendCmd23[GUN_B].BCS,(INT8U*)&BMS_BCS_Context[GUN_B],sizeof(_BMS_BCS_CONTEXT));
	
	YKCSendCmd23[GUN_B].OutVol = PowerModuleInfo[GUN_B].OutputInfo.Vol;		//输出电压  0.1
	YKCSendCmd23[GUN_B].OutCur = 4000 - PowerModuleInfo[GUN_B].OutputInfo.Cur;		//输出电流	0.1
	
	YKCSendCmd23[GUN_B].ChargeTime =  puserinfo->ChargeTime;	//充电时间  min

	return YKCFreamSend(YKC_SEND_CMD_23,(INT8U*)&YKCSendCmd23[GUN_B],sizeof(_YKC_SEND_CMD23));
}


/*****************************************************************************
* Function     : YKC_SendBSMGunACmd25
* Description  : 云快充发送BSM 充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendBSMGunACmd25(void)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING) 
	{
		return FALSE;
	}

	memset(&YKCSendCmd25[GUN_A],0,sizeof(_YKC_SEND_CMD25));
	memcpy(YKCSendCmd25[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd25[GUN_A].GunNum = 1;
	memcpy(YKCSendCmd25[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	
	memcpy((INT8U*)&YKCSendCmd25[GUN_A].BSM,(INT8U*)&BMS_BSM_Context[GUN_A],sizeof(_BMS_BSM_CONTEXT));

	

	return YKCFreamSend(YKC_SEND_CMD_25,(INT8U*)&YKCSendCmd25[GUN_A],sizeof(_YKC_SEND_CMD25));
}

/*****************************************************************************
* Function     : YKC_SendBSMGunBCmd25
* Description  : 云快充发送BSM 充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendBSMGunBCmd25(void)
{
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(SYSSet.SysSetNum.UseGun == 1)
	{
		return FALSE;
	}
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING)  //未再充电中5分钟发送一次
	{
		//枪状态改变需要立马发送
		return FALSE;
	}

	memset(&YKCSendCmd25[GUN_B],0,sizeof(_YKC_SEND_CMD25));
	memcpy(YKCSendCmd25[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	YKCSendCmd25[GUN_B].GunNum = 2;
	memcpy(YKCSendCmd25[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	memcpy((INT8U*)&YKCSendCmd25[GUN_B].BSM,(INT8U*)&BMS_BSM_Context[GUN_B],sizeof(_BMS_BSM_CONTEXT));

	

	return YKCFreamSend(YKC_SEND_CMD_25,(INT8U*)&YKCSendCmd25[GUN_B],sizeof(_YKC_SEND_CMD25));
}

/*******************************非周期性发送数据*****************************/
/*****************************************************************************
* Function     : YKC_SendPriModel
* Description  : 发送计费模型
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendPriModel(void) 
{
	static INT8U buf[9];
	memset(buf,0,sizeof(buf));
	memcpy(buf,YCKDevNum,sizeof(YCKDevNum));
	//计费模型
	buf[7] = 0x00;
	buf[8] = 0;   
	return YKCFreamSend(YKC_SEND_CMD_05,buf,sizeof(buf));
}

/*****************************************************************************
* Function     : YKC_SendPriReq
* Description  : 发送计费请求
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendPriReq(void) 
{
	static INT8U buf[7];
	memset(buf,0,sizeof(buf));
	memcpy(buf,YCKDevNum,sizeof(YCKDevNum));
	return YKCFreamSend(YKC_SEND_CMD_09,buf,sizeof(buf));
}

/*****************************************************************************
* Function     : YKC_SendGunAHand
* Description  : 发送握手
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunAHand(void) 
{
	//_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&YKCSendCmd15[GUN_A],0,sizeof(_YKC_SEND_CMD15));
	memcpy(YKCSendCmd15[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd15[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	YKCSendCmd15[GUN_A].GunNum = 1;
	memcpy((INT8U*)YKCSendCmd15[GUN_A].BMSVersion,(INT8U*)&BMS_BRM_Context[GUN_A],sizeof(_BMS_BRM_CONTEXT));
	return YKCFreamSend(YKC_SEND_CMD_15,(INT8U*)&YKCSendCmd15[GUN_A],sizeof(_YKC_SEND_CMD15));
}

/*****************************************************************************
* Function     : YKC_SendGunBHand
* Description  : 发送握手
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunBHand(void) 
{
	//_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&YKCSendCmd15[GUN_B],0,sizeof(_YKC_SEND_CMD15));
	memcpy(YKCSendCmd15[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd15[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	YKCSendCmd15[GUN_B].GunNum = 2;
	memcpy((INT8U*)YKCSendCmd15[GUN_B].BMSVersion,(INT8U*)&BMS_BRM_Context[GUN_B],sizeof(_BMS_BRM_CONTEXT));
	return YKCFreamSend(YKC_SEND_CMD_15,(INT8U*)&YKCSendCmd15[GUN_B],sizeof(_YKC_SEND_CMD15));
}


/*****************************************************************************
* Function     : YKC_SendGunAConf
* Description  : 发送参数配置
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunAConf(void) 
{
	//_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&YKCSendCmd17[GUN_A],0,sizeof(_YKC_SEND_CMD17));
	memcpy(YKCSendCmd17[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd17[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	YKCSendCmd17[GUN_A].GunNum = 1;
	memcpy((INT8U*)&YKCSendCmd17[GUN_A].UnitBatteryMaxVol,(INT8U*)&BMS_BCP_Context[GUN_A],sizeof(_BMS_BCP_CONTEXT));
	
	YKCSendCmd17[GUN_A].DevHVol = 7500;				//0.1设备最高电压
	YKCSendCmd17[GUN_A].DevLVol = 1500;				//0.1设备最低电压
	YKCSendCmd17[GUN_A].DevHCur = 1500;				//0.1最高电流  	-400A
	YKCSendCmd17[GUN_A].DevLCur = 3950;				//0.1最低电流	-400A
	return YKCFreamSend(YKC_SEND_CMD_17,(INT8U*)&YKCSendCmd17[GUN_A],sizeof(_YKC_SEND_CMD17));
}

/*****************************************************************************
* Function     : YKC_SendGunBConf
* Description  : 发送参数配置
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunBConf(void) 
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	memset(&YKCSendCmd17[GUN_B],0,sizeof(_YKC_SEND_CMD17));
	memcpy(YKCSendCmd17[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd17[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	YKCSendCmd17[GUN_B].GunNum = 2;
	memcpy((INT8U*)&YKCSendCmd17[GUN_B].UnitBatteryMaxVol,(INT8U*)&BMS_BCP_Context[GUN_B],sizeof(_BMS_BCP_CONTEXT));
	
	YKCSendCmd17[GUN_B].DevHVol = 7500;				//0.1设备最高电压
	YKCSendCmd17[GUN_B].DevLVol = 1500;				//0.1设备最低电压
	YKCSendCmd17[GUN_B].DevHCur = 1500;				//0.1最高电流  	-400A
	YKCSendCmd17[GUN_B].DevLCur = 3950;				//0.1最低电流	-400A
	return YKCFreamSend(YKC_SEND_CMD_17,(INT8U*)&YKCSendCmd17[GUN_B],sizeof(_YKC_SEND_CMD17));
}



/*****************************************************************************
* Function     : YKC_SendGunABSD
* Description  :结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunABSD(void) 
{
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd19[GUN_A],0,sizeof(_YKC_SEND_CMD19));
	memcpy(YKCSendCmd19[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd19[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	YKCSendCmd19[GUN_A].GunNum = 1;
	memcpy((INT8U*)&YKCSendCmd19[GUN_A].EndChargeSOC,(INT8U*)&BMS_BSD_Context[GUN_A],sizeof(_BMS_BSD_CONTEXT));
	
	YKCSendCmd19[GUN_A].ChargeTime = puserinfo->ChargeTime;				//充电时间 min
	YKCSendCmd19[GUN_A].ChargeKwh = puserinfo->TotalPower4/1000;				//充电输出能量 0.1
	YKCSendCmd19[GUN_A].EndChargeSOC = BMS_BCS_Context[GUN_A].SOC;
	return YKCFreamSend(YKC_SEND_CMD_19,(INT8U*)&YKCSendCmd19[GUN_A],sizeof(_YKC_SEND_CMD19));
}

/*****************************************************************************
* Function     : YKC_SendGunBConf
* Description  : 发送参数配置
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunBBSD(void) 
{
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd19[GUN_B],0,sizeof(_YKC_SEND_CMD19));
	memcpy(YKCSendCmd19[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd19[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	YKCSendCmd19[GUN_B].GunNum = 2;
	memcpy((INT8U*)&YKCSendCmd19[GUN_B].EndChargeSOC,(INT8U*)&BMS_BSD_Context[GUN_B],sizeof(_BMS_BSD_CONTEXT));
	
	YKCSendCmd19[GUN_B].ChargeTime = puserinfo->ChargeTime;				//充电时间 min
	YKCSendCmd19[GUN_B].ChargeKwh = puserinfo->TotalPower4/1000;				//充电输出能量 0.1
	YKCSendCmd19[GUN_B].EndChargeSOC = BMS_BCS_Context[GUN_B].SOC;
	return YKCFreamSend(YKC_SEND_CMD_19,(INT8U*)&YKCSendCmd19[GUN_B],sizeof(_YKC_SEND_CMD19));
}


/*****************************************************************************
* Function     : YKC_SendGunABSD
* Description  :结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunABEM_CEM(void) 
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd1B[GUN_A],0,sizeof(_YKC_SEND_CMD1B));
	memcpy(YKCSendCmd1B[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd1B[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	YKCSendCmd1B[GUN_A].GunNum = 1;
	memcpy((INT8U*)YKCSendCmd1B[GUN_A].BEM,(INT8U*)&BMS_BEM_Context[GUN_A],sizeof(YKCSendCmd1B[GUN_A].BEM));
	memcpy((INT8U*)YKCSendCmd1B[GUN_A].CEM,(INT8U*)&BMS_CEM_Context[GUN_A],sizeof(YKCSendCmd1B[GUN_A].CEM));
	
	return YKCFreamSend(YKC_SEND_CMD_1B,(INT8U*)&YKCSendCmd1B[GUN_A],sizeof(_YKC_SEND_CMD1B));
}


/*****************************************************************************
* Function     : YKC_SendGunABSD
* Description  :结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunBBEM_CEM(void) 
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd1B[GUN_B],0,sizeof(_YKC_SEND_CMD1B));
	memcpy(YKCSendCmd1B[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd1B[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	YKCSendCmd1B[GUN_B].GunNum = 2;
	memcpy((INT8U*)YKCSendCmd1B[GUN_B].BEM,(INT8U*)&BMS_BEM_Context[GUN_B],sizeof(YKCSendCmd1B[GUN_B].BEM));
	memcpy((INT8U*)YKCSendCmd1B[GUN_B].CEM,(INT8U*)&BMS_CEM_Context[GUN_B],sizeof(YKCSendCmd1B[GUN_B].CEM));
	
	return YKCFreamSend(YKC_SEND_CMD_1B,(INT8U*)&YKCSendCmd1B[GUN_B],sizeof(_YKC_SEND_CMD1B));
}


/*****************************************************************************
* Function     : YKC_SendGunABST
* Description  :发送枪A结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunABST(void) 
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd1D[GUN_A],0,sizeof(_YKC_SEND_CMD1D));
	memcpy(YKCSendCmd1D[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd1D[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	YKCSendCmd1D[GUN_A].GunNum = 1;
	memcpy((INT8U*)YKCSendCmd1D[GUN_A].BST,(INT8U*)&BMS_BST_Context[GUN_A],sizeof(YKCSendCmd1D[GUN_A].BST));
	
	return YKCFreamSend(YKC_SEND_CMD_1D,(INT8U*)&YKCSendCmd1D[GUN_A],sizeof(_YKC_SEND_CMD1D));
}

/*****************************************************************************
* Function     : YKC_SendGunABST
* Description  :发送枪A结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunBBST(void) 
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd1D[GUN_B],0,sizeof(_YKC_SEND_CMD1D));
	memcpy(YKCSendCmd1D[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd1D[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	YKCSendCmd1D[GUN_B].GunNum = 2;
	memcpy((INT8U*)YKCSendCmd1D[GUN_B].BST,(INT8U*)&BMS_BST_Context[GUN_B],sizeof(YKCSendCmd1D[GUN_B].BST));
	
	return YKCFreamSend(YKC_SEND_CMD_1D,(INT8U*)&YKCSendCmd1D[GUN_B],sizeof(_YKC_SEND_CMD1D));
}

/*****************************************************************************
* Function     : YKC_SendGunABST
* Description  :发送枪A结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunACST(void) 
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd21[GUN_A],0,sizeof(_YKC_SEND_CMD21));
	memcpy(YKCSendCmd21[GUN_A].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd21[GUN_A].SerialNum,&SerialNum[GUN_A][0],16);	//订单号
	YKCSendCmd21[GUN_A].GunNum = 1;
	memcpy((INT8U*)YKCSendCmd21[GUN_A].CST,(INT8U*)&BMS_CST_Context[GUN_A],sizeof(YKCSendCmd21[GUN_A].CST));
	
	return YKCFreamSend(YKC_SEND_CMD_21,(INT8U*)&YKCSendCmd21[GUN_A],sizeof(_YKC_SEND_CMD21));
}

/*****************************************************************************
* Function     : YKC_SendGunBCST
* Description  :发送枪A结束充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendGunBCST(void) 
{
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&YKCSendCmd21[GUN_B],0,sizeof(_YKC_SEND_CMD21));
	memcpy(YKCSendCmd21[GUN_B].DevNun,YCKDevNum,sizeof(YCKDevNum));
	memcpy(YKCSendCmd21[GUN_B].SerialNum,&SerialNum[GUN_B][0],16);	//订单号
	YKCSendCmd21[GUN_B].GunNum = 2;
	memcpy((INT8U*)YKCSendCmd21[GUN_B].CST,(INT8U*)&BMS_CST_Context[GUN_B],sizeof(YKCSendCmd21[GUN_B].CST));
	
	return YKCFreamSend(YKC_SEND_CMD_21,(INT8U*)&YKCSendCmd21[GUN_B],sizeof(_YKC_SEND_CMD21));
}


/***************************************************************
**Function   :YKC_SendCardInfo
**Description: YKC网络卡发送卡鉴权
**Input      :gun: [输入/出]
**Output     :
**Return     :
**note(s)    :
**Author     :
**Create_Time:2023-2-17
***************************************************************/
INT8U  YKC_SendCardInfo(INT8U gun)
{
    INT32U CardNum;
	USERCARDINFO * puser_card_info = NULL;	
		if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)	//注册不成功，返回失败
    {
        return  FALSE;
    }
		
	  memset(&YKCSendCmd31[gun],0,sizeof(_YKC_SEND_CMD31));
	INT8U * pdevnum = APP_GetDevNum(); //设备号 (原始为ASCII)
    YCKDevNum[0] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
    YCKDevNum[1] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
    YCKDevNum[2] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
    YCKDevNum[3] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
    YCKDevNum[4] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
    YCKDevNum[5] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
    YCKDevNum[6] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
		memcpy(YKCSendCmd31[gun].DevNun,YCKDevNum,sizeof(YCKDevNum));  //桩编号BCD码
		YKCSendCmd31[gun].GunNum = gun+1;   //枪号
	
		if(YKCStartType[gun] == _4G_APP_VIN)
		{
			YKCSendCmd31[gun].Startmode = 0x03; //VIN启动
		}
		else
		{
			YKCSendCmd31[gun].Startmode = 0x01; //卡启动
		}
		
		puser_card_info = GetGunCardInfo(gun); //获取卡号
		CardNum = (puser_card_info->CardID[3]) | (puser_card_info->CardID[2] << 8) |\
				(puser_card_info->CardID[1] << 16) | (puser_card_info->CardID[0] << 24);
		
		
		memcpy(&YKCSendCmd31[gun].card_NUM[4],&CardNum,4);   //复制物理卡号
		
		YKCSendCmd31[gun].password = 0;  //不需要密码
		memcpy(&YKCSendCmd31[gun].VIN,BMS_BRM_Context[gun].VIN,sizeof(YKCSendCmd31[gun].VIN));  //其他方式置零( ASCII码)
		
    return YKCFreamSend(YKC_SEND_CMD_31,(uint8_t*)&YKCSendCmd31[gun],sizeof(_YKC_SEND_CMD31));
}


/***************************************************************
**Function   :YKC_SendBCCardInfo
**Description: YKC并充网络卡发送卡鉴权
**Input      :gun: [输入/出]
**Output     :
**Return     :
**note(s)    :
**Author     :
**Create_Time:2023-2-17
***************************************************************/
INT8U  YKC_SendBCCardInfo(INT8U gun)
{
	_BSPRTC_TIME Curtime;
	
    INT32U CardNum;
	USERCARDINFO * puser_card_info = NULL;	
		if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)	//注册不成功，返回失败
    {
        return  FALSE;
    }
	GetCurTime(&Curtime);	
	memset(&YKCSendCmdA1[gun],0,sizeof(_YKC_SEND_CMDA1));
	memset(&YKCRecvCmdA2[gun],0,sizeof(_YKC_RECV_CMDA2));
	INT8U * pdevnum = APP_GetDevNum(); //设备号 (原始为ASCII)
    YCKDevNum[0] = (pdevnum[0] - '0') *0x10 +  (pdevnum[1] - '0');
    YCKDevNum[1] = (pdevnum[2] - '0') *0x10 +  (pdevnum[3] - '0');
    YCKDevNum[2] = (pdevnum[4] - '0') *0x10 +  (pdevnum[5] - '0');
    YCKDevNum[3] = (pdevnum[6] - '0') *0x10 +  (pdevnum[7] - '0');
    YCKDevNum[4] = (pdevnum[8] - '0') *0x10 +  (pdevnum[9] - '0');
    YCKDevNum[5] = (pdevnum[10] - '0') *0x10 +  (pdevnum[11] - '0');
    YCKDevNum[6] = (pdevnum[12] - '0') *0x10 +  (pdevnum[13] - '0');
	memcpy(YKCSendCmdA1[gun].DevNun,YCKDevNum,sizeof(YCKDevNum));  //桩编号BCD码
	YKCSendCmdA1[gun].GunNum = gun+1;   //枪号

	if(YKCStartType[gun] == _4G_APP_BCVIN)
	{
		YKCSendCmdA1[gun].Startmode = 0x03; //VIN启动
	}
	else
	{
		YKCSendCmdA1[gun].Startmode = 0x01; //卡启动
	}
	
	puser_card_info = GetGunCardInfo(gun); //获取卡号
	CardNum = (puser_card_info->CardID[3]) | (puser_card_info->CardID[2] << 8) |\
			(puser_card_info->CardID[1] << 16) | (puser_card_info->CardID[0] << 24);
	
	
	memcpy(&YKCSendCmdA1[gun].card_NUM[4],&CardNum,4);   //复制物理卡号
	
	YKCSendCmdA1[gun].password = 0;  //不需要密码
	memcpy(&YKCSendCmdA1[gun].VIN,BMS_BRM_Context[gun].VIN,sizeof(YKCSendCmdA1[gun].VIN));  //其他方式置零( ASCII码)
	YKCSendCmdA1[gun].SerialNumber[0] = Curtime.Year;
	YKCSendCmdA1[gun].SerialNumber[1] = Curtime.Month;
	YKCSendCmdA1[gun].SerialNumber[2] = Curtime.Day;
	YKCSendCmdA1[gun].SerialNumber[3] = Curtime.Hour;
	YKCSendCmdA1[gun].SerialNumber[4] = Curtime.Minute;
	YKCSendCmdA1[gun].SerialNumber[5] = Curtime.Second;
	YKCSendCmdA1[gun].SMGun = gun;
    return YKCFreamSend(YKC_SEND_CMD_A1,(uint8_t*)&YKCSendCmdA1[gun],sizeof(_YKC_SEND_CMDA1));
}

/*****************************************************************************
* Function     : YKC_SendRateAck
* Description  : 费率设置应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendRateAck(INT8U cmd)  
{
	static INT8U buf[8] = {0};
	memcpy(buf,YCKDevNum,sizeof(YCKDevNum));
	buf[7] = 0x01; //成功
	return YKCFreamSend(YKC_SEND_CMD_57,buf,8);
}




/*****************************************************************************
* Function     : YKC_SendStartAck
* Description  : 启动应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendStartAck(_GUN_NUM gun)
{
	//发送启动应答说明已经启动成功了，若启动失败，则不发送启动应答，直接发送订单
	
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(&YKCSendCmd33[gun],0,sizeof(_YKC_SEND_CMD33));
	if(GetGunState(gun) == GUN_DISCONNECTED)
	{
		YKCSendCmd33[gun].FailReason = 5;  //枪未连接
		YKCSendCmd33[gun].StartState = 0; //启动失败
	}
	else
	{
		//表示启动成功
		YKCSendCmd33[gun].FailReason = 0;
		YKCSendCmd33[gun].StartState = 1; //启动成功
	}
	
	memcpy(YKCSendCmd33[gun].SerialNum,&SerialNum[gun][0],16);	//订单号
	memcpy(YKCSendCmd33[gun].DevNun,YCKDevNum,sizeof(YCKDevNum));       //装编号
	YKCSendCmd33[gun].GunNum = gun+1;
	return YKCFreamSend(YKC_SEND_CMD_33,(INT8U*)&YKCSendCmd33[gun],sizeof(_YKC_SEND_CMD33));
}



/*****************************************************************************
* Function     : YKC_SendStartAck
* Description  : 启动应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendBCStartAck(_GUN_NUM gun)
{
	//发送启动应答说明已经启动成功了，若启动失败，则不发送启动应答，直接发送订单
	
	INT8U * pdevnum = APP_GetDevNum(); 
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(&YKCSendCmdA3[gun],0,sizeof(_YKC_SEND_CMDA3));
	if(GetGunState(gun) == GUN_DISCONNECTED)
	{
		YKCSendCmdA3[gun].FailReason = 5;  //枪未连接
		YKCSendCmdA3[gun].StartState = 0; //启动失败
	}
	else
	{
		//表示启动成功
		YKCSendCmdA3[gun].FailReason = 0;
		YKCSendCmdA3[gun].StartState = 1; //启动成功
	}
	
	memcpy(YKCSendCmdA3[gun].SerialNum,&SerialNum[gun][0],16);	//订单号
	memcpy(YKCSendCmdA3[gun].DevNun,YCKDevNum,sizeof(YCKDevNum));       //装编号
	memcpy(YKCSendCmdA3[gun].SerialNumber,YKCRecvCmdA4[gun].SerialNumber,sizeof(YKCSendCmdA3[gun].SerialNumber));
	YKCSendCmdA3[gun].GunNum = gun+1;
	return YKCFreamSend(YKC_SEND_CMD_A3,(INT8U*)&YKCSendCmdA3[gun],sizeof(_YKC_SEND_CMDA3));
}


/*****************************************************************************
* Function     : ZH_SendStopAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendStopAck(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(&YKCSendCmd35[gun],0,sizeof(_YKC_SEND_CMD35));
	memcpy(YKCSendCmd35[gun].DevNun,YCKDevNum,sizeof(YCKDevNum));       //装编号
	YKCSendCmd35[gun].StopResul = 1;
	YKCSendCmd35[gun].GunNum = gun+1;
	return YKCFreamSend(YKC_SEND_CMD_35,(INT8U*)&YKCSendCmd35[gun],sizeof(_YKC_SEND_CMD35));
}

/*****************************************************************************
* Function     : PreYKCBill
* Description  : 保存汇誉订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   PreYKCBill(_GUN_NUM gun,INT8U *pdata)
{
	INT8U i;
	INT8U errcode;
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	USERINFO * puserinfo  = GetChargingInfo(gun);
	if(gun >= GUN_MAX || pdata == NULL)
	{
		return FALSE;
	}
	_BSPRTC_TIME endtime;
	GetCurTime(&endtime); 
	_YKC_SEND_CMD3B * pbill = (_YKC_SEND_CMD3B *)pdata;
	memset(pbill,0,sizeof(_YKC_SEND_CMD3B));
	memcpy(pbill->DevNun,YCKDevNum,sizeof(YCKDevNum));       //装编号
	memcpy(pbill->SerialNum,&SerialNum[gun][0],16);	//订单号
	pbill->GunNum = gun+1;
	pbill->StartTime.Second = BCDtoHEX(puserinfo->StartTime.Second) * 1000;
	pbill->StartTime.Day = BCDtoHEX(puserinfo->StartTime.Day);
	pbill->StartTime.Hour = BCDtoHEX(puserinfo->StartTime.Hour);
	pbill->StartTime.Minute = BCDtoHEX(puserinfo->StartTime.Minute);
	pbill->StartTime.Month = BCDtoHEX(puserinfo->StartTime.Month);
	pbill->StartTime.Year = BCDtoHEX(puserinfo->StartTime.Year);

	pbill->StopTime.Second = BCDtoHEX(endtime.Second) * 1000;
	pbill->StopTime.Day = BCDtoHEX(endtime.Day);
	pbill->StopTime.Hour = BCDtoHEX(endtime.Hour);
	pbill->StopTime.Minute = BCDtoHEX(endtime.Minute);
	pbill->StopTime.Month = BCDtoHEX(endtime.Month);
	pbill->StopTime.Year = BCDtoHEX(endtime.Year);
	for(i = 0; i <4;i++)
	{
		pbill->BillFsInfo[i].Pric = YKCGunBill[gun].PrcInfo[i].Fric +  YKCGunBill[gun].PrcInfo[i].ServiceFric;
		pbill->BillFsInfo[i].PowerKwh = puserinfo->JFPGPower[i];
		pbill->BillFsInfo[i].Money = puserinfo->JFPGBill[i];
	}
	pbill->ChargeStartKwh = pmeter->StartPower4; /*充电前电表读数*/
	pbill->ChargeStopKwh = pmeter->CurPower4;
	pbill->ChargeAllKwh = puserinfo->TotalPower4;   //小数点2位
	pbill->ChargeMoney = puserinfo->TotalBill;
	memcpy(pbill->CarVin,BMS_BRM_Context[gun].VIN,17);
	
	if((YKCStartType[gun] == _4G_APP_START)  || (YKCStartType[gun] == _4G_APP_BCSTART))
	{
		pbill->ChargeType = 1;				// 1 app启动  2卡启动   3离线卡启动			5VIN码启动		5VIN码启动
	}
	else if ((YKCStartType[gun] == _4G_APP_CARD) || (YKCStartType[gun] == _4G_APP_BCCARD) )
	{
		pbill->ChargeType = 2;
	}
	else
	{
		pbill->ChargeType = 3;
	}

	pbill->JYTime.Second = BCDtoHEX(endtime.Second) * 1000;
	pbill->JYTime.Day = BCDtoHEX(endtime.Day);
	pbill->JYTime.Hour = BCDtoHEX(endtime.Hour);
	pbill->JYTime.Minute = BCDtoHEX(endtime.Minute);
	pbill->JYTime.Month = BCDtoHEX(endtime.Month);
	pbill->JYTime.Year = BCDtoHEX(endtime.Year);
	if(IfAPPStop[gun])
	{
		pbill->StopReason = 0x40;			//APP主动停止
	}
	else
	{
		errcode = APP_GetStopChargeReason(gun);   //获取故障码
		if(errcode & 0x80) //启动失败
		{
			errcode &= 0x7f;
			for(i = 0; i < YKC_TABLE_START_FAIL;i++)
			{
				if(YKCStartFailCode[i].Reason == errcode)
				{
					pbill->StopReason = YKCStartFailCode[i].CodeNum;
					break;
				}
			}
			if(i == YKC_TABLE_START_FAIL)
			{
				//其他原因
				pbill->StopReason = 0X90;
			}
		}
		else //充电停止
		{
			for(i = 0; i < YKC_TABLE_STOP_REASON;i++)
			{
				if(YKCStopFailCode[i].Reason == errcode)
				{
					pbill->StopReason = YKCStopFailCode[i].CodeNum;
					break;
				}
			}
			if(i == YKC_TABLE_STOP_REASON)
			{
				//其他原因
				pbill->StopReason = 0X90;
			}
		}
	}
	//pbill->StopReason = 0x83;  //为了测试通过  异常断电
	return TRUE;
}


/*****************************************************************************
* Function     : YKC_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendBill(_GUN_NUM gun)
{
	INT8U i;
	INT8U errcode;
	INT32U CardNum;
	USERCARDINFO * puser_card_info = NULL;	
	TEMPDATA * pmeter =APP_GetMeterPowerInfo(gun);
	USERINFO * puserinfo  = GetChargingInfo(gun);
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(&YKCSendCmd3B[gun],0,sizeof(_YKC_SEND_CMD3B));
	memcpy(YKCSendCmd3B[gun].DevNun,YCKDevNum,sizeof(YCKDevNum));       //装编号
	memcpy(YKCSendCmd3B[gun].SerialNum,&SerialNum[gun][0],16);	//订单号
	YKCSendCmd3B[gun].GunNum = gun+1;
	YKCSendCmd3B[gun].StartTime.Second = BCDtoHEX(puserinfo->StartTime.Second) * 1000;
	YKCSendCmd3B[gun].StartTime.Day = BCDtoHEX(puserinfo->StartTime.Day);
	YKCSendCmd3B[gun].StartTime.Hour = BCDtoHEX(puserinfo->StartTime.Hour);
	YKCSendCmd3B[gun].StartTime.Minute = BCDtoHEX(puserinfo->StartTime.Minute);
	YKCSendCmd3B[gun].StartTime.Month = BCDtoHEX(puserinfo->StartTime.Month);
	YKCSendCmd3B[gun].StartTime.Year = BCDtoHEX(puserinfo->StartTime.Year);

	YKCSendCmd3B[gun].StopTime.Second = BCDtoHEX(puserinfo->EndTime.Second) * 1000;
	YKCSendCmd3B[gun].StopTime.Day = BCDtoHEX(puserinfo->EndTime.Day);
	YKCSendCmd3B[gun].StopTime.Hour = BCDtoHEX(puserinfo->EndTime.Hour);
	YKCSendCmd3B[gun].StopTime.Minute = BCDtoHEX(puserinfo->EndTime.Minute);
	YKCSendCmd3B[gun].StopTime.Month = BCDtoHEX(puserinfo->EndTime.Month);
	YKCSendCmd3B[gun].StopTime.Year = BCDtoHEX(puserinfo->EndTime.Year);
	for(i = 0; i <4;i++)
	{
		YKCSendCmd3B[gun].BillFsInfo[i].Pric = YKCGunBill[gun].PrcInfo[i].Fric +  YKCGunBill[gun].PrcInfo[i].ServiceFric;
		YKCSendCmd3B[gun].BillFsInfo[i].PowerKwh = puserinfo->JFPGPower[i];
		YKCSendCmd3B[gun].BillFsInfo[i].Money = puserinfo->JFPGBill[i];
	}
	YKCSendCmd3B[gun].ChargeStartKwh = pmeter->StartPower4; /*充电前电表读数*/
	YKCSendCmd3B[gun].ChargeStopKwh = pmeter->CurPower4;
	YKCSendCmd3B[gun].ChargeAllKwh = puserinfo->TotalPower4;   //小数点2位
	YKCSendCmd3B[gun].ChargeMoney = puserinfo->TotalBill;
	memcpy(YKCSendCmd3B[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
	
	//目前都位APP启动
	if((YKCStartType[gun] == _4G_APP_START) || (YKCStartType[gun] == _4G_APP_BCSTART))
	{
		YKCSendCmd3B[gun].ChargeType = 1;				// 1 app启动  2卡启动   3离线卡启动			5VIN码启动
	}
	else if((YKCStartType[gun] == _4G_APP_CARD) || (YKCStartType[gun] == _4G_APP_BCCARD))
	{
		puser_card_info = GetGunCardInfo(gun); //获取卡号
		CardNum = (puser_card_info->CardID[3]) | (puser_card_info->CardID[2] << 8) |\
				(puser_card_info->CardID[1] << 16) | (puser_card_info->CardID[0] << 24);
		
		
		memcpy(&YKCSendCmd3B[gun].CardNum[4],&CardNum,4);   //复制物理卡号
		YKCSendCmd3B[gun].ChargeType = 2;

	}
	else
	{
		YKCSendCmd3B[gun].ChargeType = 3;
	}
	
	YKCSendCmd3B[gun].JYTime.Second = BCDtoHEX(puserinfo->EndTime.Second) * 1000;
	YKCSendCmd3B[gun].JYTime.Day = BCDtoHEX(puserinfo->EndTime.Day);
	YKCSendCmd3B[gun].JYTime.Hour = BCDtoHEX(puserinfo->EndTime.Hour);
	YKCSendCmd3B[gun].JYTime.Minute = BCDtoHEX(puserinfo->EndTime.Minute);
	YKCSendCmd3B[gun].JYTime.Month = BCDtoHEX(puserinfo->EndTime.Month);
	YKCSendCmd3B[gun].JYTime.Year = BCDtoHEX(puserinfo->EndTime.Year);
	if(IfAPPStop[gun]  ||(YKCStartType[gun] == _4G_APP_BCCARD) || (YKCStartType[gun] == _4G_APP_BCVIN) )
	{
		if((YKCStartType[gun] == _4G_APP_BCCARD) || (YKCStartType[gun] == _4G_APP_BCVIN))
		{
			YKCSendCmd3B[gun].StopReason = 0x45;	
		}
		else
		{
			YKCSendCmd3B[gun].StopReason = 0x40;			//APP主动停止
		}
	}
	else
	{
		errcode = APP_GetStopChargeReason(gun);   //获取故障码
		if(errcode & 0x80) //启动失败
		{
			errcode &= 0x7f;
			for(i = 0; i < YKC_TABLE_START_FAIL;i++)
			{
				if(YKCStartFailCode[i].Reason == errcode)
				{
					YKCSendCmd3B[gun].StopReason = YKCStartFailCode[i].CodeNum;
					break;
				}
			}
			if(i == YKC_TABLE_START_FAIL)
			{
				//其他原因
				YKCSendCmd3B[gun].StopReason = 0X90;
			}
		}
		else //充电停止
		{
			for(i = 0; i < YKC_TABLE_STOP_REASON;i++)
			{
				if(YKCStopFailCode[i].Reason == errcode)
				{
					YKCSendCmd3B[gun].StopReason = YKCStopFailCode[i].CodeNum;
					if(i == STOP_ERR_NONE)
					{
						if((YKCStartType[gun] == _4G_APP_CARD) || (YKCStartType[gun] == _4G_APP_BCCARD))
						{
							YKCSendCmd3B[gun].StopReason = 0x45;			//APP主动停止

						}
					}
					break;
				}
			}
			if(i == YKC_TABLE_STOP_REASON)
			{
				//其他原因
				YKCSendCmd3B[gun].StopReason = 0X90;
			}
		}
	}
	WriterFmBill(gun,1);
	ResendBillControl[gun].CurTime = OSTimeGet();	
	ResendBillControl[gun].LastTime = ResendBillControl[gun].CurTime;
	
	if(gun == GUN_A)
	{
		memcpy(&A_PriceSet,&PriceSet,sizeof(_PRICE_SET));   //更新费率
	}
	if(gun == GUN_B)
	{
		memcpy(&B_PriceSet,&PriceSet,sizeof(_PRICE_SET));   //更新费率
	}
	return YKCFreamSend(YKC_SEND_CMD_3B,(INT8U*)&YKCSendCmd3B[gun],sizeof(_YKC_SEND_CMD3B));
	
}

/*****************************************************************************
* Function     : YKC_SendBalanceAck
* Description  : 发送更新余额应答 
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendBalanceAck(_GUN_NUM gun)
{
	static INT8U buf[16];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memset(buf,0,16);
	memcpy(buf,YCKDevNum,sizeof(YCKDevNum));       //装编号
	buf[15] = 0; //修改成功
	return YKCFreamSend(YKC_SEND_CMD_41,buf,16);
}


/*****************************************************************************
* Function     : YKC_SendBillData
* Description  : 云快充重发订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendBillData(INT8U * pdata,INT8U len)
{
	_YKC_SEND_CMD3B *pbill;
	INT8U gun;
	
	if((pdata == NULL) || (len < sizeof(_YKC_SEND_CMD3B)))
	{
		return FALSE;
	}
	pbill = (_YKC_SEND_CMD3B *)pdata;
	gun = pbill->GunNum - 1;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
//	pbill->StopReason = 0x83;   //为了测试通过
	memcpy(&SerialNum[gun][0],pbill->SerialNum,sizeof(pbill->SerialNum));   //交易记录返回需要通过交易流水号判断
	return YKCFreamSend(YKC_SEND_CMD_3B,pdata,sizeof(_YKC_SEND_CMD3B));
}

/*****************************************************************************
* Function     : YKC_SendBillData
* Description  : 云快充重发订单数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendSetTimeAck(void)
{
	_BSPRTC_TIME curtime;
	GetCurTime(&curtime); 
	YKCSendCmd55.CurTime.Second = BCDtoHEX(curtime.Second) * 1000;
	YKCSendCmd55.CurTime.Day = BCDtoHEX(curtime.Day);
	YKCSendCmd55.CurTime.Hour = BCDtoHEX(curtime.Hour);
	YKCSendCmd55.CurTime.Minute = BCDtoHEX(curtime.Minute);
	YKCSendCmd55.CurTime.Month = BCDtoHEX(curtime.Month);
	YKCSendCmd55.CurTime.Year = BCDtoHEX(curtime.Year);
	memcpy(YKCSendCmd55.DevNun,YCKDevNum,sizeof(YCKDevNum));       //装编号
	return YKCFreamSend(YKC_SEND_CMD_55,(INT8U*)&YKCSendCmd55,sizeof(_YKC_SEND_CMD55));
}

/*****************************************************************************
* Function     : YKC_SendUpdataAck
* Description  : 云快充发送远程升级应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendUpdataAck(void)
{
	memcpy(YKCSendCmd93.DevNun,YCKDevNum,sizeof(YCKDevNum));       //装编号
	YKCSendCmd93.UpdataState = 0x00;
	return YKCFreamSend(YKC_SEND_CMD_93,(INT8U*)&YKCSendCmd93,sizeof(YKCSendCmd93));
}

/*****************************************************************************
* Function     : YKC_SendAPPQR_Ack
* Description  : 二维码应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendAPPQR_Ack(INT8U gun)
{
    static INT8U buf[9] = {0};
    memcpy(buf,YCKDevNum,sizeof(YCKDevNum));
    buf[7] = gun + 1; //枪号
	buf[8] = 0x01; //成功
    return YKCFreamSend(YKC_SEND_CMD_59,buf,9);
}

/******************************************接收函数*******************************************/
/*****************************************************************************
* Function     : YKC_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   YKC_RecvFrameDispose(INT8U * pdata,INT16U len)
{
//	INT8U i = 0;
//	INT8U cmd;
//	INT16U datalen;
//	static INT8U buf[100];

//	//帧头帧尾判断
//	if(pdata[0] != 0x68)
//	{
//		return FALSE;
//	}
//	//提取数据长度
//	datalen= pdata[1];
//	if(((datalen+4) != len) ||(datalen >100) )
//	{
//		return FALSE;
//	}
//	cmd = pdata[5];  //提取命令字
//	//提取数据
//	memcpy(buf,&pdata[6],datalen - 4);
//	
//	for(i = 0;i < YKC_RECV_FRAME_LEN;i++)
//	{
//		if(YKCRecvTable[i].cmd == cmd)
//		{
//			if(YKCRecvTable[i].recvfunction != NULL)
//			{
//				YKCRecvTable[i].recvfunction(buf,datalen - 4);
//			}
//			break;
//		}
//	}
//	
	
	//分帧
	INT8U i = 0;
	INT8U cmd;
	INT16U datalen;
	static INT8U buf[200];
	INT8U *pframedata;

	pframedata = pdata;
	
	if((pdata == NULL) || (len < 8) )
	{
		return FALSE;
	}
	while(len >= 8)
	{
		if(pdata == NULL)
		{
			return FALSE;
		}
		//帧头帧尾判断
		if(pframedata[0] != 0x68)
		{
			return FALSE;
		}
		//提取数据长度
		datalen= pframedata[1];
		if(((datalen+4) > len) ||(datalen >200) )
		{
			return FALSE;
		}
		cmd = pframedata[5];  //提取命令字
		//提取数据
		memcpy(buf,&pframedata[6],datalen - 4);
		
		for(i = 0;i < YKC_RECV_FRAME_LEN;i++)
		{
			if(YKCRecvTable[i].cmd == cmd)
			{
				if(YKCRecvTable[i].recvfunction != NULL)
				{
					YKCRecvTable[i].recvfunction(buf,datalen - 4);
				}
				break;
			}
		}
		len =len -  datalen - 4;
		pframedata = &pdata[datalen+4];
	}
	return TRUE;
}
/*****************************************************************************
* Function     : YKC_RecvRegisterAck
* Description  : 注册应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   YKC_RecvRegisterAck(INT8U *pdata,INT16U len)
{
	static _BSP_MESSAGE Msg[3];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	//参照协议
	if((pdata == NULL) || (len != 8) )
	{
		return FALSE;
	}
	if(pdata[7] == 0x00) //0：注册不通过   0x01登录失败
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
		//发送计费模型验证
		Msg[0].MsgID = BSP_4G_MAIN;
		Msg[0].DivNum = APP_RATE_MODE;
		OSQPost(psendevent, &Msg[0]);
		
		
		Msg[1].MsgID = BSP_4G_MAIN;   //发送实时数据
		Msg[1].DataLen =GUN_A;
		Msg[1].DivNum = APP_SJDATA_QUERY;
		OSQPost(psendevent, &Msg[1]);
		
		Msg[2].MsgID = BSP_4G_MAIN;    //发送实时数据
		Msg[2].DataLen =GUN_B;
		Msg[2].DivNum = APP_SJDATA_QUERY;
		OSQPost(psendevent, &Msg[2]);
	}
	
	return TRUE;
}


/*****************************************************************************
* Function     : YKC_RecvHeartAck
* Description  : 心跳应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   YKC_RecvHeartAck(INT8U *pdata,INT16U len)
{
	if(pdata == NULL )
	{
		return FALSE;
	}
	return TRUE;
}


/*****************************************************************************
* Function     : YKC_RecvPrimodelAck
* Description  : 返回计费模型应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   YKC_RecvPrimodelAck(INT8U *pdata,INT16U len)
{
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	//参照协议
	if((pdata == NULL) || (len != 10) )
	{
		return FALSE;
	}
	if(pdata[9] == 0)  //0 桩计费模型与平台一直  1 桩计费模型与平台不一致
	{
		return TRUE;  
	}
	else
	{

		//计费模型不一致，发送 09费率请求
		
		Msg.MsgID = BSP_4G_MAIN;
		Msg.DivNum = APP_QUERY_RATE;   //查新费率
		OSQPost(psendevent, &Msg);
	}
	return FALSE;
}


/*****************************************************************************
* Function     : YKC_RecvPricAck
* Description  : 费率下发
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   YKC_RecvPricAck(INT8U *pdata,INT16U len)   
{
	static _PRICE_SET price;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	INT8U i = 0,curseg,lastseg,count = 0;	
	_FLASH_OPERATION  FlashOper;
	
	if((len != (sizeof(YKCRecvCmd0A))) || (pdata == NULL) )
	{
		return FALSE;
	}
	memcpy((INT8U*)&YKCRecvCmd0A,pdata,len);
	
	
	memset(&price,0,sizeof(price));
	curseg = YKCRecvCmd0A.FricNum[0]  + 1;
	lastseg = curseg;
	price.StartTime[0] = 0;			//第一个开始时间肯定为0
	for(i = 1;i < 48;i++)
	{
		curseg = YKCRecvCmd0A.FricNum[i] + 1;
		if(lastseg != curseg)
		{
			if((lastseg == 0) || (lastseg >4) )
			{
				return FALSE;
			}
			price.Price[count] = YKCRecvCmd0A.PrcInfo[lastseg - 1].Fric;		//电价
			price.ServeFee[count] = YKCRecvCmd0A.PrcInfo[lastseg - 1].ServiceFric;				//服务费
			price.EndTime[count] = i*30;
			price.CurTimeQuantum[count] = (_TIME_QUANTUM)(lastseg - 1);				//当前处于哪个时间段
			lastseg = curseg;
			count++;
			if(count >= TIME_PERIOD_MAX)
			{
				break;		//最多为12个时间段
			}
		}
		
	}
	if(count >= TIME_PERIOD_MAX)
	{
		price.Price[TIME_PERIOD_MAX - 1] = YKCRecvCmd0A.PrcInfo[lastseg - 1].Fric;		//电价
		price.ServeFee[TIME_PERIOD_MAX - 1] =YKCRecvCmd0A.PrcInfo[lastseg - 1].ServiceFric;				//服务费
		price.EndTime[TIME_PERIOD_MAX - 1] = 0;			//最后一个结束肯定是0
		price.CurTimeQuantum[TIME_PERIOD_MAX - 1] = (_TIME_QUANTUM)(lastseg - 1);				//当前处于哪个时间段
	}
	else
	{
		price.Price[count] = YKCRecvCmd0A.PrcInfo[lastseg - 1].Fric;		//电价
		price.ServeFee[count] =YKCRecvCmd0A.PrcInfo[lastseg - 1].ServiceFric;				//服务费
		price.EndTime[count] = 0;			//最后一个结束肯定是0
		price.CurTimeQuantum[count] = (_TIME_QUANTUM)(lastseg - 1);				//当前处于哪个时间段
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
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING) 
	{
		memcpy(&A_PriceSet,&price,sizeof(_PRICE_SET));
		memcpy(&YKCGunBill[GUN_A],&YKCRecvCmd0A,sizeof(YKCRecvCmd0A));
	}
	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING) 
	{
		memcpy(&B_PriceSet,&price,sizeof(_PRICE_SET));
		memcpy(&YKCGunBill[GUN_B],&YKCRecvCmd0A,sizeof(YKCRecvCmd0A));
	}
//	IfRecvBill = 1;
	return TRUE;
}

/*****************************************************************************
* Function     : YKC_RecvPricAck
* Description  : 费率下发
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   YKC_RecvPricSet(INT8U *pdata,INT16U len)   
{
	static _PRICE_SET price;
	INT8U i = 0,curseg,lastseg,count = 0;	
	_FLASH_OPERATION  FlashOper;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if((len != (sizeof(YKCRecvCmd58))) || (pdata == NULL) )
	{
		return FALSE;
	}
	memcpy((INT8U*)&YKCRecvCmd58,pdata,len);
	memcpy((INT8U*)&YKCRecvCmd0A,pdata,len);
	
	memset(&price,0,sizeof(price));
	curseg = YKCRecvCmd58.FricNum[0]  + 1;
	lastseg = curseg;
	price.StartTime[0] = 0;			//第一个开始时间肯定为0
	for(i = 1;i < 48;i++)
	{
		curseg = YKCRecvCmd58.FricNum[i] + 1;
		if(lastseg != curseg)
		{
			if((lastseg == 0) || (lastseg >4) )
			{
				return FALSE;
			}
			price.Price[count] = YKCRecvCmd58.PrcInfo[lastseg - 1].Fric;		//电价
			price.ServeFee[count] = YKCRecvCmd58.PrcInfo[lastseg - 1].ServiceFric ;				//服务费
			price.EndTime[count] = i*30;
			price.CurTimeQuantum[count] = (_TIME_QUANTUM)(lastseg - 1);				//当前处于哪个时间段
			lastseg = curseg;
			count++;
			if(count >= TIME_PERIOD_MAX)
			{
				break;		//最多为12个时间段
			}
		}
		
	}
	if(count >= TIME_PERIOD_MAX)
	{
		price.Price[TIME_PERIOD_MAX - 1] = YKCRecvCmd58.PrcInfo[lastseg - 1].Fric;		//电价
		price.ServeFee[TIME_PERIOD_MAX - 1] =YKCRecvCmd58.PrcInfo[lastseg - 1].ServiceFric;				//服务费
		price.EndTime[TIME_PERIOD_MAX - 1] = 0;			//最后一个结束肯定是0
		price.CurTimeQuantum[TIME_PERIOD_MAX - 1] = (_TIME_QUANTUM)(lastseg - 1);				//当前处于哪个时间段
	}
	else
	{
		price.Price[count] = YKCRecvCmd58.PrcInfo[lastseg - 1].Fric;		//电价
		price.ServeFee[count] =YKCRecvCmd58.PrcInfo[lastseg - 1].ServiceFric;				//服务费
		price.EndTime[count] = 0;			//最后一个结束肯定是0
		price.CurTimeQuantum[count] = (_TIME_QUANTUM)(lastseg - 1);				//当前处于哪个时间段
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
	//不在充电中费率才生效，再充电中费率不生效
	if(pdisp_conrtol->NetGunState[GUN_A] != GUN_CHARGEING) 
	{
		memcpy(&A_PriceSet,&price,sizeof(_PRICE_SET));
		memcpy(&YKCGunBill[GUN_A],&YKCRecvCmd0A,sizeof(YKCRecvCmd0A));
	}
	if(pdisp_conrtol->NetGunState[GUN_B] != GUN_CHARGEING) 
	{
		memcpy(&B_PriceSet,&price,sizeof(_PRICE_SET));
		memcpy(&YKCGunBill[GUN_B],&YKCRecvCmd0A,sizeof(YKCRecvCmd0A));
	}
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DataLen = 1;
	Msg.DivNum = APP_RATE_ACK;
	OSQPost(psendevent, &Msg);
//	IfRecvBill = 1;
	return TRUE;
}

/***************************************************************
**Function   :YKC_RecvCard 
**Description: YKC接收卡鉴权 cmd=0x32
**Input      :pdata: [输入/出] 
**			 len: [输入/出] 
**Output     :
**Return     :
**note(s)    :
**Author     :CSH
**Create_Time:2022-12-13
***************************************************************/
static INT8U  YKC_RecvCard(INT8U *pdata,uint16_t len)
{
    INT8U gun = 0;
	static _BSP_MESSAGE SendStartMsg[GUN_MAX];
	static _BSP_MESSAGE SendStartMsg1[GUN_MAX];
	static INT8U Vinstate = 0; //0失败 1成功
	OS_EVENT* pevent = APP_Get4GMainEvent();
    if((len != (sizeof(_YKC_RECV_CMD32))) || (pdata == NULL) )
    {
        return FALSE;
    }
    gun = pdata[23] - 1; //提取枪号
    if(gun >= GUN_MAX)
    {
        return FALSE;
    }
    memcpy(&YKCRecvCmd32[gun],pdata,len);
		
		Balance[gun] = YKCRecvCmd32[gun].Money;  //余额
	 memcpy(&SerialNum[gun][0],&YKCRecvCmd32[gun].SerialNum,sizeof(YKCRecvCmd32[gun].SerialNum));
		ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
	ResendBillControl[gun].SendCount = 0;
		if(YKCRecvCmd32[gun].Success_flag == 1)   
		{
				//发送启动充电
			SendStartMsg[gun].MsgID = BSP_4G_RECV;
			SendStartMsg[gun].DataLen = gun;
			SendStartMsg[gun].DivNum = APP_START_CHARGE;
			OSQPost(pevent, &SendStartMsg[gun]);
			
			if(YKCSendCmd31[gun].Startmode == 1)
			{
				_4G_SetStartType(gun,_4G_APP_CARD);			//设置为卡启动
			}
			else
			{
				_4G_SetStartType(gun,_4G_APP_VIN);			//设置为VIN启动
				Vinstate = 1;
				//发送启动应答
				SendStartMsg1[gun].MsgID = BSP_4G_RECV;
				SendStartMsg1[gun].DataLen = gun;
				SendStartMsg1[gun].pData = &Vinstate;
				SendStartMsg1[gun].DivNum = APP_START_VIN;
				OSQPost(pevent, &SendStartMsg1[gun]);
			}
		}
		else
		{
			if(YKCSendCmd31[gun].Startmode == 3)
			{
				_4G_SetStartType(gun,_4G_APP_VIN);			//设置为VIN启动
				Vinstate = 0;
				//发送启动应答
				SendStartMsg1[gun].MsgID = BSP_4G_RECV;
				SendStartMsg1[gun].DataLen = gun;
				SendStartMsg1[gun].pData = &Vinstate;
				SendStartMsg1[gun].DivNum = APP_START_VIN;
				OSQPost(pevent, &SendStartMsg1[gun]);
			}
			
		}
		
    return TRUE;
}


/***************************************************************
**Function   :YKC_BCRecvCard 
**Description: 并充YKC接收卡鉴权 cmd=0xA2
**Input      :pdata: [输入/出] 
**			 len: [输入/出] 
**Output     :
**Return     :
**note(s)    :
**Author     :
**Create_Time:2022-12-13
***************************************************************/
static INT8U  YKC_BCRecvCard(INT8U *pdata,uint16_t len)
{
    INT8U gun = 0;
	static _BSP_MESSAGE SendStartMsg[GUN_MAX];
	static _BSP_MESSAGE SendStartMsg1[GUN_MAX];
	static INT8U Vinstate = 0; //0失败 1成功
	OS_EVENT* pevent = APP_Get4GMainEvent();
    if((len != (sizeof(_YKC_RECV_CMDA2))) || (pdata == NULL) )
    {
        return FALSE;
    }
    gun = pdata[23] - 1; //提取枪号
    if(gun >= GUN_MAX)
    {
        return FALSE;
    }

	memcpy(&YKCRecvCmdA2[gun],pdata,len);
		
	Balance[gun] = YKCRecvCmdA2[gun].Money;  //余额
	 memcpy(&SerialNum[gun][0],&YKCRecvCmdA2[gun].SerialNum,sizeof(YKCRecvCmdA2[gun].SerialNum));
	ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
	ResendBillControl[gun].SendCount = 0;
	if((YKCRecvCmdA2[GUN_A].Success_flag == 1)  && (YKCRecvCmdA2[GUN_B].Success_flag == 1))   
	{
			//发送启动充电
		SendStartMsg[GUN_A].MsgID = BSP_4G_RECV;
		SendStartMsg[GUN_A].DataLen = GUN_A;
		SendStartMsg[GUN_A].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendStartMsg[GUN_A]);
		
		SendStartMsg[GUN_B].MsgID = BSP_4G_RECV;
		SendStartMsg[GUN_B].DataLen = GUN_B;
		SendStartMsg[GUN_B].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendStartMsg[GUN_B]);
		
		if(YKCSendCmdA1[gun].Startmode == 1)
		{
			_4G_SetStartType(GUN_A,_4G_APP_BCCARD);			//设置为卡启动
			_4G_SetStartType(GUN_B,_4G_APP_BCCARD);			//设置为卡启动
		}
		else
		{
			_4G_SetStartType(GUN_A,_4G_APP_BCVIN);			//设置为VIN启动
			_4G_SetStartType(GUN_B,_4G_APP_BCVIN);			//设置为VIN启动
			Vinstate = 1;
			//发送启动应答
			SendStartMsg1[GUN_A].MsgID = BSP_4G_RECV;
			SendStartMsg1[GUN_A].DataLen = GUN_A;
			SendStartMsg1[GUN_A].pData = &Vinstate;
			SendStartMsg1[GUN_A].DivNum = APP_START_VIN;
			OSQPost(pevent, &SendStartMsg1[GUN_A]);
			
			SendStartMsg1[GUN_B].MsgID = BSP_4G_RECV;
			SendStartMsg1[GUN_B].DataLen = GUN_B;
			SendStartMsg1[GUN_B].pData = &Vinstate;
			SendStartMsg1[GUN_B].DivNum = APP_START_VIN;
			OSQPost(pevent, &SendStartMsg1[GUN_B]);
		}
	}
	else
	{
		if(YKCRecvCmdA2[gun].Success_flag != 1) 
		{
			if(YKCSendCmdA1[gun].Startmode == 3)
			{
				_4G_SetStartType(GUN_A,_4G_APP_BCVIN);			//设置为VIN启动
				_4G_SetStartType(GUN_B,_4G_APP_BCVIN);			//设置为VIN启动
				Vinstate = 0;
				//发送启动应答
				SendStartMsg1[GUN_A].MsgID = BSP_4G_RECV;
				SendStartMsg1[GUN_A].DataLen = GUN_A;
				SendStartMsg1[GUN_A].pData = &Vinstate;
				SendStartMsg1[GUN_A].DivNum = APP_START_VIN;
				OSQPost(pevent, &SendStartMsg1[GUN_A]);
				
				SendStartMsg1[GUN_B].MsgID = BSP_4G_RECV;
				SendStartMsg1[GUN_B].DataLen = GUN_B;
				SendStartMsg1[GUN_B].pData = &Vinstate;
				SendStartMsg1[GUN_B].DivNum = APP_START_VIN;
				OSQPost(pevent, &SendStartMsg1[GUN_B]);
			}
		}
		
	}
		
    return TRUE;
}


/*****************************************************************************
* Function     : YKC_RecvStart
* Description  : 接收倒启动命令
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   YKC_RecvStart(INT8U *pdata,INT16U len)
{
	INT8U gun = 0;
	static _BSP_MESSAGE Msg[GUN_MAX];
	static _BSP_MESSAGE SendStartMsg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	if((len != (sizeof(_YKC_RECV_CMD34))) || (pdata == NULL) )
	{
		return FALSE;
	}
	gun = pdata[23] - 1; //提取枪号
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	IfAPPStop[gun] = 0;
	memcpy(&YKCRecvCmd34[gun],pdata,len);
	memcpy(&SerialNum[gun][0],&YKCRecvCmd34[gun].SerialNum,sizeof(YKCRecvCmd34[gun].SerialNum));
	ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
	ResendBillControl[gun].SendCount = 0;
	YKCStartType[gun] = _4G_APP_START;			//APP启动
	//发送启动充电
	SendStartMsg[gun].MsgID = BSP_4G_RECV;
	SendStartMsg[gun].DataLen = gun;
	SendStartMsg[gun].DivNum = APP_START_CHARGE;
	OSQPost(pevent, &SendStartMsg[gun]);

	
	
	Msg[gun].MsgID = BSP_4G_MAIN;   //发送启动应答
	Msg[gun].DataLen =gun;
	Msg[gun].DivNum = APP_START_ACK;
	OSQPost(psendevent, &Msg[gun]);
	  Balance[gun] = YKCRecvCmd34[gun].Money;
	//YKCRecvCmd42[gun].Balance = 0;  //可能不会发送余额   发送余额扣到负数 20221112
	
	return TRUE;
}	


/*****************************************************************************
* Function     : YKC_RecvStart
* Description  : 接收倒启动命令
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   YKC_BCRecvStart(INT8U *pdata,INT16U len)
{
	INT8U gun = 0;
	static _BSP_MESSAGE Msg[GUN_MAX];
	static _BSP_MESSAGE SendStartMsg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	if((len != (sizeof(_YKC_RECV_CMDA4))) || (pdata == NULL) )
	{
		return FALSE;
	}
	gun = pdata[23] - 1; //提取枪号
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
//	for(gun = 0;gun < GUN_MAX;gun++)
//	{
		IfAPPStop[gun] = 0;
		memcpy(&YKCRecvCmdA4[gun],pdata,len);
		if(APP_GetErrState(gun) != 0x01)  //不在充电
		{
			memcpy(&SerialNum[gun][0],&YKCRecvCmdA4[gun].SerialNum,sizeof(YKCRecvCmdA4[gun].SerialNum));
		}
		ResendBillControl[gun].ResendBillState = FALSE;	  //之前的订单无需发送了
		ResendBillControl[gun].SendCount = 0;
		YKCStartType[gun] = _4G_APP_BCSTART;			//APP启动
		//发送启动充电
		SendStartMsg[gun].MsgID = BSP_4G_RECV;
		SendStartMsg[gun].DataLen = gun;
		SendStartMsg[gun].DivNum = APP_START_CHARGE;
		OSQPost(pevent, &SendStartMsg[gun]);

		
		YKCStartMode[gun] = START_BCCharge;
		Msg[gun].MsgID = BSP_4G_MAIN;   //发送启动应答
		Msg[gun].DataLen =gun;
		Msg[gun].DivNum = APP_START_ACK;
		OSQPost(psendevent, &Msg[gun]);
		  Balance[gun] = YKCRecvCmdA4[gun].Money;
	//}
	//YKCRecvCmd42[gun].Balance = 0;  //可能不会发送余额   发送余额扣到负数 20221112
	
	return TRUE;
}	

/*****************************************************************************
* Function     : YKC_RecvStop
* Description  : 接收到停止充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年9月15日
*****************************************************************************/
static INT8U   YKC_RecvStop(INT8U *pdata,INT16U len)
{
	INT8U gun = 0;
	static _BSP_MESSAGE Msg[GUN_MAX];
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	if(len != 8 || (pdata == NULL) )
	{
		return FALSE;
	}
	gun = pdata[7] - 1; //提取枪号
	if(gun>=GUN_MAX)
	{
		return FALSE;
	}
	if(YKCStartType[gun] == _4G_APP_BCSTART)
	{
		IfAPPStop[GUN_A] = 1;
		//发送停止充电
		SendMsg[GUN_A].MsgID = BSP_4G_RECV;
		SendMsg[GUN_A].DataLen = GUN_A;
		SendMsg[GUN_A].DivNum = APP_STOP_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_A]);
		
		
		//发送停止充电应答
		Msg[GUN_A].MsgID = BSP_4G_MAIN;
		Msg[GUN_A].DataLen = GUN_A;
		Msg[GUN_A].DivNum = APP_STOP_ACK;
		OSQPost(psendevent, &Msg[GUN_A]);
		
		
		IfAPPStop[GUN_B] = 1;
		//发送停止充电
		SendMsg[GUN_B].MsgID = BSP_4G_RECV;
		SendMsg[GUN_B].DataLen = GUN_B;
		SendMsg[GUN_B].DivNum = APP_STOP_CHARGE;
		OSQPost(pevent, &SendMsg[GUN_B]);
		
		
		//发送停止充电应答
		Msg[GUN_B].MsgID = BSP_4G_MAIN;
		Msg[GUN_B].DataLen = GUN_B;
		Msg[GUN_B].DivNum = APP_STOP_ACK;
		OSQPost(psendevent, &Msg[GUN_B]);
	}
	else
	{
		IfAPPStop[gun] = 1;
		//发送停止充电
		SendMsg[gun].MsgID = BSP_4G_RECV;
		SendMsg[gun].DataLen = gun;
		SendMsg[gun].DivNum = APP_STOP_CHARGE;
		OSQPost(pevent, &SendMsg[gun]);
		
		
		//发送停止充电应答
		Msg[gun].MsgID = BSP_4G_MAIN;
		Msg[gun].DataLen = gun;
		Msg[gun].DivNum = APP_STOP_ACK;
		OSQPost(psendevent, &Msg[gun]);
	}
	return TRUE;
}	

/*****************************************************************************
* Function     : YKC_RecvReadData
* Description  : 接收读取实时数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   YKC_RecvReadData(INT8U *pdata,INT16U len)
{	
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if((pdata == NULL) || (len != 8))
	{
		return FALSE;
	}
	gun = pdata[7] - 1;    //提取枪号
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	Msg[gun].MsgID = BSP_4G_MAIN;
	Msg[gun].DivNum = APP_SJDATA_QUERY;         //查询实时数据
	Msg[gun].DataLen = gun;
	OSQPost(psendevent, &Msg[gun]);
	return TRUE;
}


/*****************************************************************************
* Function     : YKC_RecvReadData
* Description  : 接收读取实时数据
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   YKC_RecvUpdate(INT8U *pdata,INT16U len)
{	
	INT8U gun;
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if((pdata == NULL) || (len != sizeof(_YKC_RECV_CMD94)))
	{
		return FALSE;
	}
	
//	memcpy()
//_YKC_RECV_CMD94 YKCRecvCmd94;			//远程升级
	memcpy(&YKCRecvCmd94,pdata,len);
	#warning "临时直接返回"
	memcpy(&FtpInfo,0,sizeof(FtpInfo));
	memcpy(&FtpInfo,YKCRecvCmd94.SerAdd,sizeof(FtpInfo) );
 	APP_SetSIM7600Mode(MODE_FTP);
	
	Msg.MsgID = BSP_4G_MAIN;
	Msg.DivNum = APP_UPDATA_ACK;         //远程升级应答
	Msg.DataLen = GUN_A;
	OSQPost(psendevent, &Msg);
	return TRUE;
}


/*****************************************************************************
* Function     : YKC_RecvBalance
* Description  : 更新余额
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   YKC_RecvBalance(INT8U *pdata,INT16U len)
{	
	INT8U gun;
	static _BSP_MESSAGE Msg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	if((pdata == NULL) || (len != sizeof(_YKC_RECV_CMD42)))
	{
		return FALSE;
	}
	gun = pdata[7] - 1;    //提取枪号
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	memcpy((INT8U*)&YKCRecvCmd42[gun],pdata,len);
	Balance[gun] = YKCRecvCmd42[gun].Balance;			//更新余额
	
	Msg[gun].MsgID = BSP_4G_MAIN;
	Msg[gun].DivNum = APP_UPDADA_BALANCE;         //查询实时数据
	Msg[gun].DataLen = gun;
	OSQPost(psendevent, &Msg[gun]);
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
static INT8U   YKC_RecvRecordAck(INT8U *pdata,INT16U len)
{
	INT8U i;
	if(len != 17)
	{
		return FALSE;
	}
	//比较交易流水号
	for(i = 0;i < 16;i++)
	{
		if(SerialNum[GUN_A][i] != pdata[i])
		{
			break;
		}
	}
	if(i == 16)
	{
		//与A枪交易流水号一致
		if(pdata[16] == 0)
		{
			ResendBillControl[GUN_A].ResendBillState = FALSE;			//订单确认，不需要重发订单
			ResendBillControl[GUN_A].SendCount = 0;
			WriterFmBill(GUN_A,0);
		}
		return TRUE;
	}

	
	for(i = 0;i < 16;i++)
	{
		if(SerialNum[GUN_B][i] != pdata[i])
		{
			break;
		}
	}
	if(i == 16)
	{
		//与A枪交易流水号一致
		if(pdata[16] == 0)
		{
			ResendBillControl[GUN_B].ResendBillState = FALSE;			//订单确认，不需要重发订单
			ResendBillControl[GUN_B].SendCount = 0;
			WriterFmBill(GUN_B,0);
		}
		return TRUE;
	}
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
static INT8U   YKC_RecvTime(INT8U *pdata,INT16U len)
{
	_BSPRTC_TIME SetTime;                       //设定时间
	INT8U times = 3;							//如果设置失败反复设置三次
	static _BSP_MESSAGE Msg;
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	INT8U Second;
	if(len != (sizeof(YKCRecvCmd56)) )
	{
		return FALSE;
	}
	memcpy((INT8U*)&YKCRecvCmd56,pdata,len);
	
	Second = YKCRecvCmd56.CurTime.Second / 1000;
	//校准时间
	SetTime.Year   = HEXtoBCD(YKCRecvCmd56.CurTime.Year);
    SetTime.Month  = HEXtoBCD(YKCRecvCmd56.CurTime.Month);
    SetTime.Day    = HEXtoBCD(YKCRecvCmd56.CurTime.Day&0x1f);
    SetTime.Hour   = HEXtoBCD(YKCRecvCmd56.CurTime.Hour);
    SetTime.Minute = HEXtoBCD(YKCRecvCmd56.CurTime.Minute);
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


/***************************************************************
**Function   :YKC_RecvUpdate 
**Description:接收铁塔下发二维码
**Input      :pdata: [输入/出] 
**			 len: [输入/出] 
**Output     :
**Return     :
**note(s)    :
**Author     :CSH
**Create_Time:2022-12-13
***************************************************************/
static uint8_t   YKC_RecvQRcode(uint8_t *pdata,uint16_t len)
{ 
	uint8_t gun = 0;
	_YKC_RECV_CMD5A Recvdata;
	 static _BSP_MESSAGE SendMsg[GUN_MAX];
	OS_EVENT* psendevent =  APP_GetSendTaskEvent();
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendStartMsg[GUN_MAX];
	memcpy((uint8_t *)&Recvdata,pdata,MIN(len,sizeof(_YKC_RECV_CMD5A)) ); 
	gun = Recvdata.GUNNum - 1;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	
	memcpy((uint8_t *)&YKCRecvCmd5A[gun],pdata,MIN(len,sizeof(_YKC_RECV_CMD5A)) ); 
	
	SendStartMsg[gun].MsgID = BSP_4G_MAIN;
	SendStartMsg[gun].DivNum = APP_QR_CODE;         //二维码应答
	SendStartMsg[gun].DataLen = gun;
	OSQPost(psendevent, &SendStartMsg[gun]);

	
	SendMsg[gun].pData = YKCRecvCmd5A[gun].QR_code;
	SendMsg[gun].MsgID = BSP_4G_RECV;
	SendMsg[gun].DataLen = gun;
	SendMsg[gun].DivNum = APP_START_QRCODE;
	OSQPost(pevent, &SendMsg[gun]);
    return TRUE;
}

/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

