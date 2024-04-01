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
#ifndef __DISPLAYMAIN_H_
#define __DISPLAYMAIN_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "Display.h"
//#include 	"connectccu_interface.h"
#include "Disinterface.h"
#include "DwinProtocol.h" 
#include "controlmain.h"



#define DISP_NET    	1    	//网络版本 
#define DISP_CARD    	0		//刷卡版本





#define RECODE_DISPOSE1(a)       	((a) == (0) ? (1000): (a)) 
#define RECODE_DISPOSE2(a)       ((a) > (1000) ? ((a) - 1000): (0)) 
/* Private define-----------------------------------------------------------------------------*/


//变量地址地址 0x0000~0x6FFF  0110 1   111 1    （111 1111）      低7位代表当前页面可以使用的地址，一个页面的最大地址为128个，
//一个地址2个字节，最多为256个字节。高9位代码页面ID,最大为0xdf（223）个页面


#define DIS_ADD(id,add)      					 ( ( ( (id) & 0x00ff) << 7 ) | ( (add) & (0x007f) ) )
//显示提示偏移地址为当前页面基地址的 0x30
#define DIS_SHOW(id)            				 ( ( ( (id) & 0x00ff) << 7 ) + 0x30)                
// #define DISP_RECV_TABLE_LEN         (29u)                     //变量接收处理表长度
//各个输入显示变量的地址
#define ADDR_MENU11_TIME            (0x0580)       //界面11输入充电时间地址
#define ADDR_MENU12_CHARGE          (0x0600)       //界面12输入电量地址
#define ADDR_MENU13_MONEY			(0x0680)       //界面13输入金额地址
#define ADDR_MENU14_CODE			(0x0700)	   //界面14输入卡密码地址
#define ADDR_MENU21_CODE			(0x0A80)	   //界面21重新输入卡密码地址
#define ADDR_MENU70_CODE			(0x2300)	   //界面70隐藏按钮输入面界面
#define ADDR_MENU75_CFG             (0x2580)       //界面75读取系统配置1信息
#define ADDR_MENU76_CFG             (0x2600)       //界面76读取系统配置2信息
#define ADDR_MENU80_CODE            (0x2800)	    //界面80密码修改界面
#define ADDR_MENU81_PRC             (0x2880)	    //界面81费率设置界面
#define ADDR_MENU98_NET             (0X3100)        //界面98网络配置1 
#define ADDR_MENU99_NET             (0X3180)        //界面99网络配置2 
#define ADDR_MENU73_HANDA			(0x2480)		//A枪手动模式读取信息
#define ADDR_MENU74_HANDB			(0x2500)		//B枪手动模式读取信息



//提示显示
#define ADDR_MENU70_SHOW            (0x2303)        //界面70输入密码提示地址
#define ADDR_MENU75_SHOW			(0x2598)		//界面75配置1提示地址
#define ADDR_MENU76_SHOW			(0x2608)		//界面76配置2提示地址
#define ADDR_MENU81_SHOW			(0x288E)        //界面81费率设置提示地址
#define ADDR_MENU78_SHOW		    (0x2700)		//界面78解锁
#define ADDR_MENU48_SHOW			(0x1800)		//刷卡结算界面提示
#define ADDR_MENU44_SHOW			(0x1608)		//A枪显示电池类型
#define ADDR_MENU45_SHOW			(0x1688)		//B枪显示电池类型
#define ADDR_MENU7A_SHOW			(0x0390)		//显示A枪故障
#define ADDR_MENU7B_SHOW			(0x03A0)		//显示B枪故障
#define ADDR_MENU71_SHOW			(0x0380)		//提示关闭手动模式,才能退到主界面或上级界面
#define ADDR_MENU58_SHOW            (0x1d12)		//A枪结算界面停止原因显示地址
#define ADDR_MENU59_SHOW            (0x1d92)		//A枪结算界面停止原因显示地址
//#define ADDR_MENU65_SHOW			(0x2080)		//设备信息1基地址
//#define ADDR_MENU66_SHOW			(0x2100)		//设备信息2基地址
//#define ADDR_MENU67_SHOW			(0x2180)		//设备信息3基地址
//#define ADDR_MENU68_SHOW			(0x2200)		//设备信息4基地址


//充电界面电池类型显示
#define ADDR_MENU44_SHOW            (0x1608)			
#define ADDR_MENU45_SHOW            (0x1688)	
//显示当前充电模式
#define ADDR_MENU100_SHOW			(0x3200)
//显示辅助电源地址
#define ADDR_MENU77_SHOW			(0x2680)
//显示21界面密码输入错误剩余次数
#define ADDR_MENU21_SHOW			(0x0A83)

//界面63账户信息页面显示
#define  ADDR_MENU63_SHOW			 (0x1faf)
//界面64	账户信息页面显示
#define  ADDR_MENU64_SHOW			 (0x202f)


//#define ADDR_COUNTDOWN_TIME			(0x0100)		//所有界面的倒计时都用一个地址
//#define ADDR_TIME           	    (0x0200)		          //时间变量地址 "xx:xx:xx"
//#define ADDR_DATE                    (0x0202)		          //日期变量地址2 "xx-xx-xx"


/************************************汇誉屏幕******************************/
#define ADDR_COUNTDOWN_TIME			(0x3050)					//所有界面的倒计时都用一个地址
#define ADDR_TIME           	    (0x3000)		          	//时间变量地址 "xx:xx:xx"
#define ADDR_MENU52_CFG             (0x1A50)      			 	//界面52读取系统配置1信息
#define ADDR_MENU53_CFG             (0x1AD0)      			 	//界面53读取系统配置2信息
#define ADDR_GUNA_T1			(0x1D8A)					//所有界面的倒计时都用一个地址
#define ADDR_GUNA_T2			(0x1D8B)					//所有界面的倒计时都用一个地址
#define ADDR_GUNB_T1			(0x1D8C)					//所有界面的倒计时都用一个地址
#define ADDR_GUNB_T2			(0x1D8D)					//所有界面的倒计时都用一个地址

#define ADDR_DEV_STATUS			    (0x4000)					//设备状态
#define  ADDR_RECODE_NUM			 (0x0470)					//记录个数
/******************************************汇誉界面*******************************/
//界面输入
#define INPUT_MENU21_CODE		0x0A80     //刷卡密码
#define INPUT_MENU23_CODE		0x0B80     //管理员密码
#define INPUT_MENU50_HANDA		(0X1900)	//A枪手动模式
#define INPUT_MENU51_HANDB		(0X1980)	//B枪手动模式
#define INPUT_MENU54_CFG        (0x1B00)    //界面54读取系统配置2信息
/* Private variables--------------------------------------------------------------------------*/
typedef enum
{
	WORK_STANDBY = 0, //待机
	WORK_CHARGE,	  //工作
	WORK_OVER,		  //充电完成
	WORK_PAUSE,		  //充电暂停
	WORK_MAX,	
}_WORK_STAT; //故障状态

 
//STATE_UNOVER 
//STATE_OVER
typedef enum
{
	STATE_UNOVER = 0,  	//未完成执行
	STATE_OVER,			//已经完成执行
}_OVER_STATE; //枪连接状态

typedef struct
{
    INT16U variaddr;                                      //变量地址
    INT8U (*Fun)(INT16U addr, INT8U *pvalue,INT8U len);    //对应的处理函数
}_DISP_ADDR_FRAME;



//网络版本充电枪状态
typedef enum{
	GUN_IDLE = 0,  //空闲  				 无故障而且枪连接
	GUN_UNLINK,	   //枪未连接				 无故障而且枪未连接
	GUN_FAIL,		//故障
	GUN_STARTING,	//启动中
	GUN_STOPING,	//停止中
	GUN_CHARGEING,	//充电中
	GUN_STARTFAIL,	//启动失败					
}_NET_GUN_STATE;


//空闲状态       正在启动       启动失败       启动成功
typedef enum{
	NET_IDLE = 0,  //空闲  	
	NET_STARTING,	//正在启动
	NET_STOPING,	//正在停止
	NET_STARTFAIL,	//启动失败
}_NET_SST_STATE;	   



typedef struct
{
	_GUN_NUM CurUserGun;          			//当前使用枪号
	_GUN_NUM GurLockGun;					//当前解锁枪号
	ST_Menu* ClickGunNextPage[GUN_MAX];		//记录点击A B枪下一页的枪号
	INT16U CountDown;						//记录倒计时时间
	INT8U BmsPowerOnce;						//辅助电源是否一次有效， TRUE（1）, 一次有效，需要在启动后读取flashbms电源
	INT8U CardCode[6];							//卡密码
	//主要用于区别CCU主动停止还是TCU主动停止，可能刷卡完成先执行，也可能停止完成先执行
	_OVER_STATE StopOver[GUN_MAX];							//0表示停止未执行 1表示停止已经执行了
	_OVER_STATE CardOver[GUN_MAX];							//0表示停止未执行 1表示停止已经执行了
	//一次充电只允许处理一次 （CCU可能下发多个启动停止完成帧）,启动完成帧在相应流程内接收有效，无论CCU发送了几次，都只执行了一次 
	_OVER_STATE StopOverFrame[GUN_MAX];	
	//充电桩停止后，必须间隔8s才能再次启动
	INT32U StartIntTime[GUN_MAX];   
	INT8U SendStartCharge[GUN_MAX]; //TREU 需要发送启动充电  FALSE 不需要发送启动充电
	//VIN启动还是自动启动
	INT32U StartType;				//0xBB,表示V启动 ，非bb自动启动

	//故障状态       充电状态       正在启动        正在停止      启动失败              /《二维码界面》     枪未连接状态         空闲状态 
	_NET_GUN_STATE NetGunState[GUN_MAX];      //主要是为了切换页面使用
	_NET_SST_STATE NetSSTState[GUN_MAX];     //网络启停状态
}_DISP_CONTROL;

//充电中显示数据,与界面保持一致
__packed typedef struct
{
	INT16U ChargeSOC;						//SOC1%/位，0%偏移量；数据范围：0%~100%
	INT16U ChargeCur;						//充电电流分辨率0.1A
	INT16U ChargeVol;						//充电电压 分辨率0.1v
	INT16U ChargeTime;						//充电时间 分辨率1分钟
	INT32U ChargeEle;						//充电电量 分辨率0.01度
	INT32U ChargeMoney;						//充电金额 分辨率0.01元
	INT32U CardMoney;						//卡内余额 分辨率0.01元
	INT8U  BatteryType;						//充电电池类型
}_CHARGEING_INFO;

//充电中车辆显示数据，与界面保持一致
__packed typedef struct
{
	INT16U  bmsneedvolt;			//BMS需求电压 0.1v
	INT16U  bmsneedcurr;			//BMS需求电流 0.1A
	INT16U  remaindertime;			//剩余充电时间
	INT16U bmsMaxVoltage;			//最高允许充电总电压0.1
	INT16U bmsMaxcurrent;			//最高允许充电总电流 0.1 -400
	INT16U MaxTemprature;			//最高允许温度 -50 1C/WEI
	INT16U unitbatterymaxvol;		//单体电池最高允许电压0.01
	INT16U  chargevolmeasureval;	//BMS电压测量值 0.1
}_CARD_INFO;

//充电结束数据，与界面保持一致
__packed typedef struct
{
	INT16U StartYear;
	INT16U StartMouth;
	INT16U StartDay;
	INT16U StartHour;
	INT16U StartMinute;
	INT16U StartSecond;
	INT16U StopYear;
	INT16U StopMouth;
	INT16U StopDay;
	INT16U StopHour;
	INT16U StopMinute;
	INT16U StopSecond;
	INT32U TotPower;					//总电量 	 0.01Kwh
	INT32U TotMoney;					//总金额   0.01元
	INT32U BeforeCardBalance;			//扣款后卡内余额 0.01
	INT8U EndChargeReason;				//需要转换为文字显示出来
}_END_CHARGE_INFO;


__packed typedef struct
{
	INT16U outvolt;						//0.1V
	INT16U outcurr;						//0.1A
}HandSet_Info;


extern _SYS_SET SYSSet;
extern _CHARGE_RECODE ChargeRecodeInfo[GUN_MAX];
/* Private typedef----------------------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : NB_WriterReason
* Description  : 内部人员显示故障原因，方便调试使用
* Input        :
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2022年8月24日
*****************************************************************************/
 INT8U NB_WriterReason(INT8U gun,const char * preason,INT8U len);
/*****************************************************************************
* Function     : APP_GetDispControl
* Description  : 获取显示任务控制结构体
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
_DISP_CONTROL* APP_GetDispControl(void);

/*****************************************************************************
* Function     : APP_SetCountDownTime
* Description  : 设置倒计时时间   ，切换页面的时候设置倒计时时间，
				测任务中倒计时每1s -1，直到见到1为止
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
void  APP_SetCountDownTime(INT16U time);

/*****************************************************************************
* Function     : APP_GetChargeingInfo
* Description  : 提供给充电中显示界面显示
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
_CHARGEING_INFO * APP_GetChargeingInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : Display_ShowSet1
* Description  : 初始化显示设置1
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31      
*****************************************************************************/
void Display_ShowSet1(void);

/*****************************************************************************
* Function     : Display_ShowSet2
* Description  : 初始化显示设置2
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31      
*****************************************************************************/
void Display_ShowSet2(void);

/*****************************************************************************
* Function     : Display_ShowSet2
* Description  : 初始化显示设置3
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31      
*****************************************************************************/
void Display_ShowSet3(void);

/*****************************************************************************
* Function     : Display_ShowNet
* Description  : 显示网络协议选择
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31      
*****************************************************************************/
void Display_ShowNet(void);
/*****************************************************************************
* Function     : Display_ShowSet1
* Description  : 显示费率设置 		(flash没写入暂时未作处理)
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31      
*****************************************************************************/
void Display_PricSet(INT8U count);


/*****************************************************************************
* Function     : Display_ShowPowerBms
* Description  : 显示辅助电源
* Input        : TRUE 需要重铁电中读取 FALSE 直接显示SYSSet.SysSet4.bmspower 
				（因为存在一次有效和长期有效）
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31      
*****************************************************************************/
void Display_ShowPowerBms(INT8U state);

/*****************************************************************************
* Function     : APP_SetBmsPowerOnce
* Description  :设置复制电源是否一次有效 
* Input        : TRUE 一次有效
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-8-13     
*****************************************************************************/
void APP_SetBmsPowerOnce(INT8U state);




/*****************************************************************************
* Function     : APP_GetWorkState
* Description  :获取故障状态 
* Input        : 
* Output       :
				00H：待机
				01H：工作
				02H：充电完成
				03H：充电暂停
* Return       : 
* Note(s)      : 
* Contributor  : 2018-8-13     
*****************************************************************************/
_WORK_STAT  APP_GetWorkState(_GUN_NUM gun);

/*****************************************************************************
* Function     : APP_GetErrState
* Description  :获取故障状态 
* Input        : 
* Output       :
				00H：正常
				01H：故障
* Return       : 
* Note(s)      : 
* Contributor  : 2018-8-13     
*****************************************************************************/
INT8U  APP_GetErrState(_GUN_NUM gun);


/*****************************************************************************
* Function     : APP_GetCarInfo
* Description  :提供给车辆信息界面显示界面显示 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
_CARD_INFO * APP_GetCarInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : GetGunState
* Description  :获取枪状态
* Input        : 
* Output       :
				//00H：已连接
				//01H：未连接
* Return       : 
* Note(s)      : 
* Contributor  : 2018-8-13     
*****************************************************************************/
_GUN_STATE  GetGunState(INT8U gun);


/*****************************************************************************
* Function     : SendStartChargeMsg
* Description  : 发送给连接ccu任务发送开始充电(开始充电只发一次)
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendStartChargeMsg(_GUN_NUM gun,_CHARGE_MODE_TYPE charge_mode);


/*****************************************************************************
* Function     : SendStopChargeMsg
* Description  : 发送给连接ccu任务发送停止充电
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendStopChargeMsg(_GUN_NUM gun);


/*****************************************************************************
* Function     : SendCardMsg
* Description  : 发送给卡消息
* Input        : divnum 消息设备号        pdata数据（开始刷卡，需要传入密码） len 密码长度
				 pdata == NULL,说明不需要传递数据
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendCardMsg(_SEND_TO_CARD_CMD divnum, void * pdata,INT8U len);

/*****************************************************************************
* Function     : SendHandSetMsg
* Description  : 发送手动调整输出消息
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U SendHandSetMsg(_GUN_NUM gun);


/*****************************************************************************
* Function     : APP_GetChargeInfo
* Description  : 获取充电类型信息
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
_CHARGE_INFO* APP_GetChargeInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : DispShow_EndChargeDate
* Description  : 结束充电显示数据
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U DispShow_EndChargeDate(_GUN_NUM gun);

/*****************************************************************************
* Function     : APP_SetCardPriceInfo
* Description  : 获取当前记录
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14 
*****************************************************************************/
_CHARGE_RECODE* APP_GetChargeRecodeInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : APP_SelectCurChargeRecode
* Description  :查询当前交易记录，并显示，第一次进入
* Input        : 
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectCurChargeRecode(void);

/*****************************************************************************
* Function     : APP_SelectNextChargeRecode
* Description  :查询下一条交易记录，并显示
* Input        : 
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectNextChargeRecode(void);

/*****************************************************************************
* Function     : APP_SelectUpChargeRecode
* Description  :查询上一条交易记录，并显示
* Input        : 
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectUpChargeRecode(void);

/*****************************************************************************
* Function     : APP_SelectNextNChargeRecode
* Description  :查询下N条交易记录，并显示
* Input        : 
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectNextNChargeRecode(INT16U num);

/*****************************************************************************
* Function     : APP_SelectUpNChargeRecode
* Description  :查询上N条交易记录，并显示
* Input        : 
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_SelectUpNChargeRecode(INT16U num);

/*****************************************************************************
* Function     : APP_GetRecodeCurNum
* Description  : 获取当前交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT32U  APP_GetRecodeCurNum(void);


/*****************************************************************************
* Function     : APP_GetRecodeCurNum
* Description  : 获取当前交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
void  APP_SetRecodeCurNum(INT32U num);

/*****************************************************************************
* Function     : APP_ClearRecodeInfo
* Description  :清除 记录信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_ClearRecodeInfo(void);

/*****************************************************************************
* Function     : DispShow_StatusDivInfo
* Description  : 状态显示
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U DispShow_StatusDivInfo(void);


/*****************************************************************************
* Function     : APP_GetDispUartStatus
* Description  : 获取显示串口状态
* Input        :
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
_DISP_UART_STATUS APP_GetDispUartStatus(void);

/*****************************************************************************
* Function     : APP_SetDispUartStatus
* Description  : 设置显示串口状态
* Input        :
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U  APP_SetDispUartStatus(_DISP_UART_STATUS status);

/*****************************************************************************
* Function     : APP_DWRTCGetTime
* Description  : 获取DWRTC
* Input        : INT8U *pdata
                 INT8U len
* Output       : None
* Return       : 
* Note(s)      :
* Contributor  :2018-6-14 
*****************************************************************************/
INT8U APP_DWRTCGetTime(_BSPRTC_TIME *pTime);

/***********************************************************************************************
* Function		: DisplayGunQRCode
* Description	: 显示二维码
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018年7月18日
***********************************************************************************************/
INT8U DisplayGunQRCode(_GUN_NUM gun);

/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : 周期性检测任务
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
ST_Menu * Disp_NetPageDispos(void);


/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : 周期性检测任务
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U  Net_StartCharge(_GUN_NUM gunnum,INT8U type);


/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : 周期性检测任务
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U  Net_StopCharge(_GUN_NUM gunnum);

/***********************************************************************************************
* Function		: APP_GetDevNum
* Description	: 获取设备号
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 20210111
***********************************************************************************************/
INT8U * APP_GetDevNum( void);

/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  : 在充电中周期性储存记录交易记录   				  billflag:0表示已经结算			billflag：1表示未结算
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U WriterFmBill(_GUN_NUM gun,INT8U billflag);

/*****************************************************************************
* Function     : APP_GetBillInfo
* Description  : 获取枪订单信息
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U *APP_GetBillInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : APP_GetGunNum
* Description  : 获取枪数量
* Input        :
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_GetGunNum(void);
/*****************************************************************************
* Function     : APP_GetStopChargeReason
* Description  : 最高位为1表示启动失败停止；最高位为0表示启动成功停止
* Input        :
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_GetStopChargeReason(_GUN_NUM gun);

/*****************************************************************************
* Function     : ReadFmBill
* Description  : 读取订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月31日
*****************************************************************************/
INT8U ReadFmBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : APP_GetVinWL
* Description  : 获取VIN白名单
* Input        :
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U* APP_GetVinWL(void);

/*****************************************************************************
* Function     : APP_RWOFFLineRe离线交易记录
* Description  : 读写网络离线交易记录
* Input        :
				count  读写在第几条 1 - 100条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_RWNetOFFLineRecode(INT16U count,_FLASH_ORDER RWChoose,INT8U  * pdata);


/*****************************************************************************
* Function     : APP_RWNetFSOFFLineRecode
* Description  : 读写网络离线交易记录
* Input        :
				count  读写在第几条 1 - 100条
                 RWChoose  读写命令
                 precode 缓冲区地址
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_RWNetFSOFFLineRecode(INT16U count,_FLASH_ORDER RWChoose,INT8U  * pdata);
/*****************************************************************************
* Function     : APP_GetNetOFFLineRecodeNum
* Description  : 读写离线交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_GetNetOFFLineRecodeNum(void);

/*****************************************************************************
* Function     : APP_GetNetOFFLineRecodeNum
* Description  : 读写离线交易记录
* Input        :
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_SetNetOFFLineRecodeNum(INT8U num);

#endif //__DISPLAYMAIN_H_
/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/
