/*****************************************Copyright(H)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: gbstandard.h
* Author			  :      
* Date First Issued	: 10/6/2015
* Version			  : V0.1
* Description		: 大部分驱动都可以在这里初始化
*----------------------------------------历史版本信息-------------------------------------------
* History			  :
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__GBSTANDARD_H_
#define	__GBSTANDARD_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "connectbmsframe.h"
#include "connectbmsinterface.h"
#include "ucos_ii.h"
#include "app_conf.h"
/* Private define-----------------------------------------------------------------------------*/
#define PRI_SendFrame              6     //发送任务优先级  

//辨识符
#define  GB_STANDARD_DEVICE_ID         0x56    // 充电机ID
#define  GB_STANDARD_BMS_ID            0xF4    // BMS的ID

//优先权
#define  GB_PRIO_0                     0u
#define  GB_PRIO_1                     1u
#define  GB_PRIO_2                     2u
#define  GB_PRIO_3                     3u
#define  GB_PRIO_4                     4u
#define  GB_PRIO_5                     5u
#define  GB_PRIO_6                     6u
#define  GB_PRIO_7                     7u

//J1393传输协议连接管理多页传输RTS帧PGN
#define  GB_RTS_PGN_235                60160u   //多帧实际内容相关PGN,如DT
#define  GB_RTS_PGN_236                60416u   //多帧控制相关PGN，如RTS、CTS、EndOfMsgACK、Conn_Abort

//PGN参数组编号
//握手阶段
#define  GB_CHM_PGN_9728               9728u
#define  GB_BHM_PGN_9984               9984u

#define  GB_CRM_PGN_256                256u
#define  GB_BRM_PGN_512                512u
//充电参数配置阶段
#define  GB_BCP_PGN_1536               1536u
#define  GB_CTS_PGN_1792               1792u
#define  GB_CML_PGN_2048               2048u
#define  GB_BRO_PGN_2304               2304u
#define  GB_CRO_PGN_2560               2560u
//充电阶段
#define  GB_BCL_PGN_4096               4096u
#define  GB_BCS_PGN_4352               4352u
#define  GB_CCS_PGN_4608               4608u
#define  GB_BSM_PGN_4864               4864u
#define  GB_BMV_PGN_5376               5376u
#define  GB_BMT_PGN_5632               5632u
#define  GB_BSP_PGN_5888               5888u
#define  GB_BST_PGN_6400               6400u
#define  GB_CST_PGN_6656               6656u

//充电结束阶段
#define  GB_BSD_PGN_7168               7168u
#define  GB_CSD_PGN_7424               7424u
//错误报文
#define  GB_BEM_PGN_7680               7680u
#define  GB_CEM_PGN_7936               7936u


#define  GB_BMSSTATE_NOTREADY          0x00  //BMS未完成
#define  GB_BMSSTATE_STANDBY           0xAA  //BMS完成充电准备
#define  GB_BMSSTATE_INVALID           0xFF  //BMS状态无效

#define  GB_CHARGESTATE_NOTREADY       0x00  //充电机未准备好
#define  GB_CHARGESTATE_STANDBY        0xAA  //充电机完成充电准备
#define  GB_CHARGESTATE_INVALID        0xFF  //充电机状态无效


//本公式只针对国标要求的PDU1格式，即中间8位是PS为目标地址值，第三字节8位位源地址
//计算报文的ID规则:优先权 | R=0 |DFP =0 |  PF = PGN/256  | 目的地址(BMS) | 源地址
//发送组ID公式
#define GB_CONFIGSEND_ID(P,PGN)    (INT32U)(((INT32U)(P&0x07) <<26))| ((((INT32U)PGN>>8)&0x00FF)<<16)\
                                   | ((INT32U)GB_STANDARD_BMS_ID << 8) | GB_STANDARD_DEVICE_ID
//接收组ID公式
#define GB_CONFIGRECEIVE_ID(P,PGN) (INT32U)(((INT32U)(P&0x07) <<26))| ((((INT32U)PGN>>8)&0x00FF)<<16)\
                                   | ((INT32U)GB_STANDARD_DEVICE_ID << 8) | GB_STANDARD_BMS_ID
                                   
                                   
#define CANRECEIVETABLELEN  14u        

#define INT32U_MAX_NUM          (0xFFFFFFFFu)
/* Private typedef----------------------------------------------------------------------------*/
//故障取值选值，跟BMS协议定义一致
typedef enum
{
    STATE_NORMAL = 0,   //正常
    STATE_TROUBLE,      //故障
    STATE_UNTRUST,      //不可信
    //以上取值勿更改
    STATE_LOW = 10,     //过低
    STATE_HIGH,         //过高
}_BMS_ERR_VALUE;

//BMS方的状态
typedef enum
{
    BMS_STEP_DEFAULT =0,
    BMS_RTS_STEP = 1,//01BMS RTS报文  
    BMS_BHM_STEP,    //02BMS充电握手阶段报文
    BMS_BRM_STEP,    //03BMS辨识报文  
    BMS_BCP_STEP,    //04动力蓄电池充电参数  
    BMS_BRO_STEP,    //05电池充电准备就绪状态
    BMS_BCL_STEP,    //06电池充电需求  
    BMS_BCS_STEP,    //07电池充电总状态  
    BMS_BSM_STEP,    //08动力蓄电池状态信息  
    BMS_BMV_STEP,    //09动力蓄电池温度  
    BMS_BMT_STEP,    //10动力蓄电池电压  
    BMS_BSP_STEP,    //11动力蓄电池预留报文
    BMS_BST_STEP,    //12BMS中止充电
    BMS_BSD_STEP,    //13BMS统计数据
    BMS_BEM_STEP,    //14充电机统计数据
    BMS_OTH_STEP,    //15其他超时
    BMS_OTH1_STEP,   //16其他1超时
    BMS_OTH2_STEP,   //17其他2超时		
    BMS_BSD_TIMEOUT, //18LCD获取BSD超时
    BMS_FAULT_SETP,  //19处理故障超时
    BMS_PAUSE_STEP,  //20BMS暂停充电超时
	BMS_BCL_DEMAND_UPDATA,//21BMS需求更新计数
    BMS_MAX_STEP,    //22最大步骤数
    //超时
    BMS_TIME_OUT,
    //参数错误
    BMS_PARA_ERR,
}_BMS_STEP;

//桩本身的状态
typedef enum
{
    BMS_SEND_DEFAULT = 0,
    BMS_CHM_SEND    = 1,  //01充电机充电握手阶段报文 
    BMS_CRM_SEND,         //02充电机辨识报文
    //参数配置阶段
    BMS_PARA_SEND,        //03充电机发送时间同步信息及最大输出能力报文
    BMS_CRO_SEND,         //04充电机输出准备就绪状态(0xAA)
    BMS_CRO_UNREADY_SEND, //05充电机输出准备就绪状态(0x00)
    //充电阶段
    BMS_CCS_SEND,         //06充电机充电状态
    BMS_CST_SEND,         //07充电机中止充电
    BMS_CSD_SEND,         //08充电机统计数据
    //错误信息
    BMS_CEM_SEND,         //09充电机错误报文
    BMS_CEM_CST_SEND,	  //10CEM跟CST一起发送
    BMS_CTS_SEND,         //11发送CTS同步时间
}_SEND_BMS_STEP; 
/* Private macro------------------------------------------------------------------------------*/
//BHM报文内容
__packed typedef struct
{
    INT16U  MaxChargeWholeVol;   //最高允许充电总电压
}_BMS_BHM_CONTEXT;

extern  _BMS_BHM_CONTEXT  BMS_BHM_Context[GUN_MAX];

//BRM报文内容
__packed typedef struct
{
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
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //新国标 
    INT8U  BMSSoftwareVer[8];    //BMS软件版本号（新国标内容）
#endif //#if (BMS_CUR_GB == BMS_NEW_GB2015)

}_BMS_BRM_CONTEXT;

extern  _BMS_BRM_CONTEXT BMS_BRM_Context[GUN_MAX];

//BCP报文内容
//size=13
__packed typedef struct
{
    INT16U UnitBatteryMaxVol;    //单体动力蓄电池最高允许充电电压
    INT16U MaxCurrent;           //最高允许充电电流
    INT16U BatteryWholeEnergy;   //动力蓄电池标称总能量
    INT16U MaxVoltage;           //最高允许充电总电压
    INT8U  MaxTemprature;        //最高允许温度
    INT16U BatterySOC;           //整车蓄电池荷电状态:SOC
    INT16U BatteryPresentVol;    //整车动力蓄电池当前电池电压,0.1V/位
}_BMS_BCP_CONTEXT;

extern _BMS_BCP_CONTEXT BMS_BCP_Context[GUN_MAX];

//BRO报文内容
__packed typedef struct
{
    INT8U StandbyFlag;      //BMS是否已经准备好
}_BMS_BRO_CONTEXT;

extern _BMS_BRO_CONTEXT BMS_BRO_Context[GUN_MAX];

//BCL报文内容
__packed typedef struct
{
    INT16U DemandVol;      //电压需求V
    INT16U DemandCur;      //电流需求A
    INT8U  ChargeMode;     //充电模式  01：恒压充电   02：恒流充电
}_BMS_BCL_CONTEXT;

extern _BMS_BCL_CONTEXT BMS_BCL_Context[GUN_MAX];

//BCS报文内容
__packed typedef struct
{
    INT16U ChargeVolMeasureVal;      //充电电压测量值
    INT16U ChargeCurMeasureVal;      //充电电流测量值
    INT16U MaxUnitVolandNum;         //最高单体动力蓄电池电压以及组号
    INT8U  SOC;                      //当前荷电状态%
    INT16U RemainderTime;            //剩余充电时间min
}_BMS_BCS_CONTEXT;

extern _BMS_BCS_CONTEXT BMS_BCS_Context[GUN_MAX];

//BSM报文内容
__packed typedef struct
{
    INT8U  MaxUnitVolandNum;         //最高单体动力蓄电池电压所在组编号
    INT8U  MaxbatteryTemprature;     //最高动力蓄电池温度
    INT8U  MaxTempMeasurepnum;       //最高温度监测点编号
    INT8U  MinbatteryTemprature;     //最低动力蓄电池问题
    INT8U  MinTempMeasurepnum;       //最低动力蓄电池温度检测点编号
#if 0  
    INT16U UnitVolState;             //单体动力蓄电池电压过高： 00正常 01过高 10过低
    INT16U WholeSOCState;            //整车动力蓄电池荷电状态：00正常 01过高 10过低
    INT16U ChargeCurState;           //动力蓄电池充电过电流 00 正常 01过流  10 不可信状态
    INT16U BatteryTempState;         //动力蓄电池温度：00正常  01过高  10不可信状态
    INT16U BatteryInsulationState;   //动力蓄电池绝缘状态：00 正常 01 不正常 10 不可信状态
    INT16U BatteryConnectState;      //动力蓄电池输出连接器连接状态 00 正常  01不正常  10 不可信状态
    INT16U ChargeAllow;              //充电允许 00 禁止 01允许    
#endif
    __packed union
    {
        INT8U AllBits;          
        __packed struct
        {
            INT8U UnitVolBits:2;      //单体动力蓄电池电压过高： 00正常 01过高 10过低
            INT8U WholeSOCBits:2;     //整车动力蓄电池荷电状态：00正常 01过高 10过低
            INT8U ChargeCurBits:2;    //动力蓄电池充电过电流 00 正常 01过流  10 不可信状态
            INT8U BatteryTempBits:2;  //动力蓄电池温度：00正常  01过高  10不可信状态
        }OneByte;
    }State0;
    
    __packed union
    {
        INT8U AllBits;          
        __packed struct
        {
            INT8U BatteryInsulationBits:2; //动力蓄电池绝缘状态：00 正常 01 不正常 10 不可信状态
            INT8U BatteryConnectBits:2;    //动力蓄电池输出连接器连接状态 00 正常  01不正常  10 不可信状态
            INT8U ChargeAllowBits:2;       //充电允许 00 禁止 01允许   
            INT8U ResverBist:2;            //预留
        }OneByte;
    }State1;

}_BMS_BSM_CONTEXT;

extern _BMS_BSM_CONTEXT BMS_BSM_Context[GUN_MAX];


//BST报文内容
__packed typedef struct
{
    __packed union
    {
        INT8U EndChargeReason;          //BMS中止充电原因
        __packed struct
        {
            INT8U SocTargetBits:2;    //需求SOC目标值，00--未达到  01--达到  10 --不可信
            INT8U TolVolValBits:2;    //总电压设定值，00--未达到   01--达到  10---不可信
            INT8U VolSetValBits:2;    //单体电压设定值，00--未达到   01--达到  10---不可信
            INT8U ChargeStopBits:2;   //充电机主动中止，00--正常   01--充电机中止(收到CST帧)  10---不可信
        }OneByte;
    }State0;

    __packed union
    {
        INT16U  EndChargeTroubleReason;     //BMS中止充电故障原因
        __packed struct
        {
            INT8U InsulationErrBits:2;     //绝缘故障，00--正常  01--故障  10 --不可信
            INT8U ConnectorOverTempBits:2; //输出连接器过温故障，00--正常  01--故障  10 --不可信
            INT8U ElementOverTempBits:2;   //BMS元件过温故障，00--正常  01--故障  10 --不可信
            INT8U ConnectorErrBits:2;      //输出连接器故障，00--正常  01--故障  10 --不可信
            INT8U BatOverTempBits:2;       //电池组过温故障，00--正常  01--故障  10 --不可信
            INT8U HighPresRelayBits:2;     //高压继电器故障，00--正常  01--故障  10 --不可信
            INT8U TPTwoVolErrBits:2;       //检测点2电压检测故障，00--正常  01--故障  10 --不可信
            INT8U OtherErrBits:2;          //其他故障错误，00--正常  01--故障  10 --不可信
        }OneByte;
    }State1;

    __packed union
    {
        INT8U EndChargeErroReason;      //BMS中止充电错误原因
        __packed struct
        {
            INT8U OverCurErrBits:2;     //BMS电流过大，00--电流正常  01--电流超过需求值  10 --不可信
            INT8U OverVolErrBits:2;     //BMS电压过大，00--电压正常  01--电压异常  10 --不可信
            INT8U ResverBits:4;         //预留
        }OneByte;
    }State2;
}_BMS_BST_CONTEXT;

extern _BMS_BST_CONTEXT BMS_BST_Context[GUN_MAX];
//充电结束阶段
//BSD报文内容
__packed typedef struct
{
    INT8U   EndChargeSOC;          //中止荷电状态SOC%
    INT16U  UnitBatteryminVol;     //动力蓄电池单体最低电压
    INT16U  UnitBatteryMaxVol;     //动力蓄电池单体最高电压
    INT8U   BatteryMinTemp;        //动力蓄电池最低温度
    INT8U   BatteryMaxTemp;        //动力蓄电池最高温度  
}_BMS_BSD_CONTEXT;

extern _BMS_BSD_CONTEXT BMS_BSD_Context[GUN_MAX];

//错误报文
//BEM报文内容,元素用报文简称填写，用下划线区分，这样有点不符合编程规范，不够统一，但识别性好些
//size = 4
__packed typedef struct
{
    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CRM00Bits:2;    //接收充电机辨识保文超时(0x00) 00--正常 01--超时 10--不可信
            INT8U CRMAAits:2;     //接收充电机辨识保文超时(0xAA)
            INT8U RevBits:4;      //预留
        }OneByte;
    }State0;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CTS_CMLBits:2; //接收充电机的时间同步和充电机的最大输出能力报文超时
            INT8U CROBits:2;     //接收充电机完成充电准备报文超时
            INT8U RevBits:4;     //预留
        }OneByte;
    }State1;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CCSBits:2;    //接收充电机充电状态超时
            INT8U CSTBits:2;    //接收充电机中止充电保文超时
            INT8U RevBits:4;    //预留
        }OneByte;
    }State2;

    __packed union
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CSDBits:2;    //接收充电机统计报文超时
            INT8U OthBits:6;    //其他错误
        }OneByte;
    }State3;
}_BMS_BEM_CONTEXT;

extern _BMS_BEM_CONTEXT BMS_BEM_Context[GUN_MAX];


//以下是充电机要发送的内容
//CML报文内容
__packed typedef struct
{
    INT16U MaxOutVol;    //最高输出电压
    INT16U MinOutVol;    //最低输出电压
    INT16U MaxOutCur;    //最高输出电流
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //新国标 
    INT16U MinOutCur;    //最低输出电流
#endif   
}_BMS_CML_CONTEXT;

extern _BMS_CML_CONTEXT BMS_CML_Context[GUN_MAX];


//充电机充电状态报文
__packed typedef struct
{
    INT16U OutputVol;       //电压输出值，0.1V/位
    INT16U OutputCur;       //电流输出值，0.1A/位，-400A偏移量
    INT16U TotalChargeTime; //累计充电时间(min)，1min/位
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //新国标 
    INT8U ChargePause;      //充电允许00:暂停 01:允许，只有低2位有效
#endif
}_BMS_CCS_CONTEXT;
extern _BMS_CCS_CONTEXT BMS_CCS_Context[GUN_MAX];

//充电机中止充电报文
__packed typedef struct
{
    __packed union
    {
        INT8U EndChargeReason;          //充电机中止充电原因
        __packed struct
        {
            INT8U ReachConditionBits:2; //达到充电机设定的条件中止，00--正常 01--达到条件 10--不可信
            INT8U UserStopBits:2;       //用户主动中止，00--正常 01--用户中止 10--不可信
            INT8U ErrStopBits:2;        //故障中止，00--正常 01--故障中止 10--不可信
#if (BMS_CUR_GB == BMS_NEW_GB2015)   //新国标 
            INT8U BMSStopBits:2;        //BMS主动中止，00--正常 01--BMS中止(收到BST帧) 10--不可信
#else                                //旧国标
            INT8U ResverBits:2;         //预留
#endif
        }OneByte;
    }State0;

    __packed union
    {
        INT16U  EndChargeTroubleReason;     //充电机中止充电故障原因
        __packed struct
        {
            INT8U ChargeOverTempBits:2;    //充电机过温故障，00--正常  01--故障  10 --不可信
            INT8U ConnectorOverTempBits:2; //输出连接器过温故障，00--正常  01--故障  10 --不可信
            INT8U ChargeOverTempInBits:2;  //充电机内部过温故障，00--正常  01--故障  10 --不可信
            INT8U PowerUnreach:2;          //所需电能不能传输故障，00--正常  01--故障  10 --不可信
            INT8U UrgentStopBits:2;        //充电机急停故障，00--正常  01--故障  10 --不可信
            INT8U OtherErrBits:2;          //其他故障，00--正常  01--故障  10 --不可信
            INT8U ResverBits:4;            //预留
        }OneByte;
    }State1;

    __packed union
    {
        INT8U EndChargeErroReason;         //充电机中止充电错误原因
        __packed struct
        {
            INT8U OverCurErrBits:2;     //充电机电流过大，00--电流正常  01--电流超过需求值  10 --不可信
            INT8U OverVolErrBits:2;     //充电机电压过大，00--电压正常  01--电压异常  10 --不可信
            INT8U ResverBits:4;         //预留
        }OneByte;
    }State2;
    
}_BMS_CST_CONTEXT;

extern _BMS_CST_CONTEXT BMS_CST_Context[GUN_MAX];

__packed typedef struct
{
    INT16U TotalChargeTime; //累计充电时间(min)
    INT16U PowerOut;        //输出电能,0.1kW.h/位
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //新国标 
    INT32U ChargeSN;        //充电机编号,1/位
#endif
}_BMS_CSD_CONTEXT;
extern _BMS_CSD_CONTEXT BMS_CSD_Context[GUN_MAX];

//充电机错误报文CEM
__packed typedef struct
{
    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BRMBits:2;    //接收BMS和车辆的识别辨识超时
            INT8U RevBits:6;    //预留
        }OneByte;
    }State0;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BCPBits:2;    //接收电池充电参数报文超时
            INT8U BROBits:2;    //接收BMS完成充电准备保文超时
            INT8U RevBits:4;    //预留
        }OneByte;
    }State1;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BCSBits:2;    //接收电池充电总状态保文超时
            INT8U BCLBits:2;    //接收电池充电要求报文超时
            INT8U BSTBits:2;    //接收BMS中止充电报文超时
            INT8U RevBits:2;    //预留
        }OneByte;
    }State2;

    __packed union
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BSDBits:2;    //接收BMS充电统计保文超时
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //新国标 
            INT8U OthBits:6;    //其他
#endif 
        }OneByte;
    }State3;
    
}_BMS_CEM_CONTEXT;

extern _BMS_CEM_CONTEXT BMS_CEM_Context[GUN_MAX];


/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
typedef struct
{
	INT32U PGNID;       // 数据标识
    INT32U Timeout;    // 帧长度，包括ID所在的帧,这个帧长是主控过来的帧长度信息,NULL的为主控不会过来的帧
	INT8U (*DealwithDataIDFunction)(CanInterfaceStruct *controlstrcut);  //id对应的操作
	INT8U  (*Updatafunction)(void);//执行完设置等操作后，需要更新的参数执行函数,一些小的参数的更新操作放在这
}_PROTOCOL_CAN_MANAGE_TABLE;

extern const _PROTOCOL_CAN_MANAGE_TABLE  Can_Data_Manage_Table[GUN_MAX][CANRECEIVETABLELEN];


#define CANSENDTALELEN       (12u)     //发送帧管理表格长度  
typedef struct
{
	_SEND_BMS_STEP  step;          // 步骤得重新考虑   
    INT32U cycletime;         // 执行周期，对应国标里面报文周期
	void (*CANSendFrametoBMS)(void);  //id对应的操作
}_PROTOCOL_CAN_SEND_TABLE;


#endif //__GBSTANDARD_H_
/************************(H)COPYRIGHT 2010 汇誉科技*****END OF FILE****************************/
