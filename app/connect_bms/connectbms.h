/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: connectbms.h
* Author			: 叶喜雨
* Date First Issued	: 10/29/2013
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2016		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__CONNECT_BMS_H_
#define	__CONNECT_BMS_H_
#include "gbstandard.h"
#include "app_conf.h"
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
#define	BMS_CAN_A	0
#define	BMS_CAN_B	1
//BMS国标定义
#define BMS_OLD_GB2011        (1u)            //2011年老国标
#define BMS_NEW_GB2015        (2u)            //2015年新国标
//当前BMS国标选择，只能二选一
//#define BMS_CUR_GB            BMS_OLD_GB2011 //旧国标
#define BMS_CUR_GB            BMS_NEW_GB2015 //新国标

#define BMSCANOSQ_NUM         (32u)      //连接BMS的CAN消息容量
#define BMSCANRX_BUFF_LEN     (16u)      //接收缓冲长度

#define BMS_RX_LEN            (256u)     //接收到BMS数据的buffer缓存，多页数据全部整理在这里处理

//#define BMS_ERR_HANDLE_LEN    (17u)      //BMS故障错误表长度
#define BMS_ERR_HANDLE_LEN    (4u)      //BMS故障错误表长度

#define BMS_MSG_TABLE_LEN     (1u)       //接收控制任务发送的消息表长度

#define BMS_ERR_RETRY_NUM     (3u)       //BMS故障重试次数
/* Private typedef----------------------------------------------------------------------------*/
typedef enum
{
    BMS_GB_2011,    //2011
    BMS_GB_2015,    //2015
}_BMS_CUR_GB;
//故障处理重试次数
typedef enum
{
    ERR_RETRY_DEC,      //故障处理次数减1
    ERR_RETRY_INC,      //故障处理次数加1      
    ERR_RETRY_CLEAR,    //故障处理次数清0
    ERR_RETRY_SET,      //故障处理次数为BMS_ERR_RETRY_NUM
}_BMS_ERR_RETRY;


typedef enum
{
    ERR_TYPE_NORMAL = 0, //00:正常
    ERR_TYPE_TIMEOUT,    //01:超时
    ERR_TYPE_UNTRUST,    //02:不可信
}_BMS_ERR_STATE_TYPE;

typedef enum
{
	BMS_CHARGE_PAUSE = 0, //00:暂停输出
	BSM_CHARGE_ENABLE,	  //01:允许输出
}_BMS_CHARGE_PAUSE_STATE;

//CEM故障类型
typedef enum
{
    CEM_BRM,
    CEM_BCP,
    CEM_BRO,
    CEM_BCS,
    CEM_BCL,  
    CEM_BST,
    CEM_BSD,
    CEM_ALL,
}_BMS_CEM_TYPE;

//故障等级1 2 3
typedef enum
{
    ERR_LEVEL_DEFAULT = 0,  //故障等级无
    ERR_LEVEL_1 = 1,        //故障等级1
    ERR_LEVEL_2,
    ERR_LEVEL_3,  
}_BMS_ERR_LEVEL;
//充电故障分类
typedef enum
{
    //故障级别为1
    ERR_L1_INSULATION = 0x01,      //绝缘故障
    ERR_L1_LEAKAGE,                //漏电故障
    ERR_L1_EMERGENCY,              //急停故障

    //故障级别为2
    ERR_L2_CONNECT = 0x10,         //连接器故障
    ERR_L2_CONN_TEMP_OVER,         //BMS元件、输出连接器过温
    ERR_L2_BGROUP_TEMP_OVER,       //电池组温度过温
    ERR_L2_BAT_VOL,                //单体电池电压过低、过高
    ERR_L2_BMS_VOL_CUR,            //BMS检测到充电电流过大或充电电压异常
    ERR_L2_CHARGE_VOL_CUR,         //充电机检测到充电电流不匹配或充电电压异常
    ERR_L2_CHARGE_TEMP_OVER,       //充电机内部过温
    ERR_L2_CHARGE_POWER_UNARRIVAL, //充电机电量不能传送
    ERR_L2_CAR_CONN_ADHESION,      //车辆接触器粘连

    //故障级别为3
    ERR_L3_CHAGING_TIMEOUT = 0x20, //充电握手阶段、配置阶段、充电过程超时
    ERR_L3_CHAGING_OVER_TIMEOUT,   //充电结束超时

    ERR_LF_METHOD_A = 0x30,        //直接采用处理方式作为类型
    ERR_LF_METHOD_B,
    ERR_LF_METHOD_C,
}_BMS_ERR_TYPE;

//故障处理方式
typedef enum
{
    ERR_METHOD_UNDEF = 0,          //未定义的方式 
    ERR_METHOD_A = 1,              //方式A:充电机立即停机停用
    ERR_METHOD_B,                  //方式B:停止本次充电，生成交易记录(需重新插拔枪才能进行下一次充电)
    ERR_METHOD_C,                  //方式C:中止充电，待故障现象排除后重新通信握手开始充电
    ERR_METHOD_END,                //直接结束
    ERR_METHOD_BMS,                //BMS发送BEM保文
}_BMS_ERR_MOTHOD;

//故障状态
typedef enum
{
    ERR_STATE_NORMAL = 0,          //无错误
    ERR_STATE_METHOD_A,            //错误方式A
    ERR_STATE_METHOD_B,            //错误方式B
    ERR_STATE_METHOD_C,            //错误方式C
	ERR_STATE_METHOD_END,          //直接结束
    ERR_STATE_BMS,
}_BMS_ERR_STATE;

typedef enum
{
    BMS_CONNECT_DEFAULT =0,
    BMS_RTS_SUCCESS = 1,      //01收到BMS返回的RTS多帧数据报文
    BMS_BHM_SUCCESS,          //02收到BMS返回的BHM车辆握手报文   
    BMS_BRM_SUCCESS,          //03收到BMS返回的BRM车辆辨识报文    
    BMS_BCP_SUCCESS,          //04收到BMS返回的动力蓄电池充电参数报文    
    BMS_BRO_SUCCESS,          //05收到BMS返回的电池充电准备就绪状态报文
    BMS_BCL_SUCCESS,          //06收到BMS返回的电池充电需求报文
    BMS_BCS_SUCCESS,          //07收到BMS返回的电池充电总状态报文
    BMS_BSM_SUCCESS,          //08收到BMS返回的动力蓄电池状态信息报文
    BMS_BMV_SUCCESS,          //09收到BMS返回的单体动力蓄电池电压报文
    BMS_BMT_SUCCESS,          //10收到BMS返回的动力蓄电池温度报文
    BMS_BSP_SUCCESS,          //11收到BMS返回的动力蓄电池预留报文
    BMS_BST_SUCCESS,          //12收到BMS返回的中止充电报文
    BMS_BSD_SUCCESS,          //13收到BMS返回的统计数据报文
    BMS_BEM_SUCCESS,          //14收到BMS返回的错误报文
    //充电阶段的几个特殊情况，
    BMS_BCL_BCS_SUCCESS,
    //BMS_BCL_BCS_BSM_SUCCESS,
    RX_BMS_TIMEOUT,    //BMS超时
}_CONNECT_BMS_STATE;   


//专门开辟一个变量来管理整个流程节点是否合适呢？
typedef struct
{
    _SEND_BMS_STEP		step;
    _CONNECT_BMS_STATE  laststate;     //上一状态
    _CONNECT_BMS_STATE  currentstate;  //当前状态
    //_CONNECT_BMS_STATE  needreconnect; //重新连接   ： RX_BMS_TIMEOUT重新连接        
}_BMS_CONNECT_CONTROL;

extern _BMS_CONNECT_CONTROL BMS_CONNECT_Control[GUN_MAX];

typedef struct
{
    INT8U state;         //管理帧的状态，在处理完整个信息帧后清除,1：表示需要多帧接收
    INT8U nextpagenum;   //后续有多少页，就是总页数
    INT8U currentpagenum;//当前页码
    INT8U pagenum;       //实际获取到页数
    INT8U IDtablepos;    //id在表格中的位置 
    INT32U FrameID;      //帧的ID    
    INT16U datalength;   //数据长度  
}_CANFRAMECONTROL;

extern _CANFRAMECONTROL CanFrameControl[GUN_MAX];

//收到其他任务的消息表结构
typedef struct
{
    INT8U DataID;
    INT8U (*function)(void *pdata);
}_RECEIVE_Q_TABLE;

//充电故障处理结构
typedef struct
{
    _BMS_ERR_TYPE	type;                  	//充电故障类型
    _BMS_ERR_MOTHOD	method;                 //处理方式
    INT8U (*fun)(_BMS_ERR_MOTHOD method);   //故障处理函数
}_BMS_ERR_HANDLE;  

extern const _BMS_ERR_HANDLE BMSErrHandleTable[GUN_MAX][BMS_ERR_HANDLE_LEN];

typedef struct
{
	_BMS_ERR_LEVEL ErrLevle;    //故障等级
    _BMS_ERR_STATE ErrState;    //错误状态
    INT8U MethodCRetry;         //重试次数，在故障C重连三次
    INT8U SelfCheck;            //自检 TRUE---需要重新插拔枪自检
    INT8U GunInput;             //枪连接车
    INT8U ChargeRunning;        //功率模块已经输出
}_BMS_ERR_CONTROL;

extern _BMS_ERR_CONTROL BMSErrControl[GUN_MAX];
typedef enum
{
	VIN_IDLE = 0,
	VIN_SECCSEE = 0x55,
	VIN_FAIL = 0xAA,
}_VIN_STATE;
typedef struct
{
	INT8U	Get_Flag;		//特殊PGN多包收到标志
	INT8U	Count_Delay;	//延迟计数
}_SPECIAL_PGN;				//特殊PGN
typedef struct
{
	INT8U VinState;			//1表示鉴权成功 0表示未鉴权成功
}_VIN_CONTROL;
extern	_SPECIAL_PGN	Special_PGN[GUN_MAX][3];

extern OS_EVENT  	*TaskConnectBMS_peventA;    			//CAN消息队列指针
extern BSPCANTxMsg   BMSCANTxbuffA;      				//发送缓冲
extern BSPCANRxMsg   BMSCANRxbuffA[BMSCANRX_BUFF_LEN];	//接收缓冲
extern BSP_CAN_Set	 bxBMSCANSetA;

extern OS_EVENT  	*TaskConnectBMS_peventB;    			//CAN消息队列指针
extern BSPCANTxMsg   BMSCANTxbuffB;      				//发送缓冲
extern BSPCANRxMsg   BMSCANRxbuffB[BMSCANRX_BUFF_LEN];	//接收缓冲
extern BSP_CAN_Set	 bxBMSCANSetB;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BMS_CONNECT_ControlInit BMS_CONNECT_StateSet
* Description	: 充电流程控制结构体。与设置函数
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
void BMS_CONNECT_ControlInit(INT8U gun);
void BMS_CONNECT_StateSet(INT8U gun,_CONNECT_BMS_STATE state);
void BMS_CONNECT_StepSet(INT8U gun,_SEND_BMS_STEP step);
INT8U ChargeErrHandleA(_BMS_ERR_MOTHOD method);
INT8U ChargeErrHandleB(_BMS_ERR_MOTHOD method);
INT8U APP_ReceiveEndChargeA(void *pdata);
INT8U APP_ReceiveEndChargeB(void *pdata);
_BMS_ERR_STATE GetBMSErrState(INT8U gun);
void SetBMSErrState(INT8U gun ,_BMS_ERR_STATE state);
INT8U ChargeErrDeal(INT8U gun ,_BMS_ERR_TYPE type);
void BMS_FrameContextClear(INT8U gun);
/***********************************************************************************************
* Function      : APP_GetGBType
* Description   : 获取国标类型
* Input         :
* Output        :
* Note(s)       :
* Contributor   : wb 20160922
***********************************************************************************************/
_BMS_CUR_GB APP_GetGBType(INT8U gun);
/***********************************************************************************************
* Function      : APP_SetGBType
* Description   : 设置
* Input         :
* Output        :
* Note(s)       :
* Contributor   : wb 20160922
***********************************************************************************************/
void APP_SetGBType(INT8U gun, _BMS_CUR_GB dat);
/*****************************************************************************
* Function      : APP_GetBMSBHM
* Description   : 获取BHM帧
* Input         : _BMS_BHM_CONTEXT *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月19日  叶喜雨
*****************************************************************************/
INT8U APP_GetBMSBHM(INT8U gun,_BMS_BHM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BSD
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BSD(INT8U gun,_BMS_BSD_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BCP
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BCP(INT8U gun,_BMS_BCP_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_CML
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_CML(INT8U gun,_BMS_CML_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BRM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BRM(INT8U gun,_BMS_BRM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BCL
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BCL(INT8U gun,_BMS_BCL_CONTEXT *pdata);

/***********************************************************************************************
* Function		: APP_GetBMS_BSM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BSM(INT8U gun,_BMS_BSM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BST
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BST(INT8U gun,_BMS_BST_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BEM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BEM(INT8U gun,_BMS_BEM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BEM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 \
***********************************************************************************************/
void APP_GetBMS_CEM(INT8U gun,_BMS_CEM_CONTEXT *pdata);

/***********************************************************************************************
* Function      : Set_VIN_Success
* Description   : VIN是否鉴权成功  1表示成功 在心跳的低
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-07-11 Yxy
***********************************************************************************************/
void Set_VIN_Success(_GUN_NUM gun,_VIN_STATE state);

/***********************************************************************************************
* Function      : Set_VIN_Send
* Description   : VIN 发送
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-07-11 Yxy
***********************************************************************************************/
void Set_VIN_Send(INT8U state);

/***********************************************************************************************
* Function      : Set_VIN_Send
* Description   : VIN 发送
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-07-11 Yxy
***********************************************************************************************/
INT8U Get_VIN_Send(void);

#endif	//__CONNECT_BMS_H_
/************************(C)COPYRIGHT 2016 汇誉科技*****END OF FILE****************************/


