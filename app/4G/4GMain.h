/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: GPRSMain.h
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
#ifndef	__4G_MAIN_H_
#define	__4G_MAIN_H_
#include "bsp.h"
#include "DataChangeInterface.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
#define NET_SENDRECV_PRINTF		0   //1需要打印4G发送接收数据
#define  	APP_START_ACK		0	//开始充电应答
#define  	APP_ATIV_STOP		1	//主动停止充电
#define  	APP_STOP_ACK		3	//停止充电应答
#define  	APP_STOP_BILL		17	//停止结算
#define 	APP_RATE_ACK		5   //费率设置应答
#define 	APP_STE_DEVS		6	//查询设备状态
#define 	APP_STE_BILL		7	//查询记录
#define 	APP_STE_RATE		8	//查询费率
#define 	APP_STE_TIME		9	//校准时间
#define     APP_CARDVIN_CHARGE	10	//刷卡vin启动
#define     APP_CARD_INFO		11	//卡鉴权
#define     APP_VIN_INFO		12	//Vin鉴权
#define		APP_CARD_WL			13	//卡白名单
#define		APP_VIN_WL			14	//VIN白名单
#define		APP_VINCARD_RES		15	//白名单清空
#define		APP_OFFLINE_ACK		16	//离线应答
#define     APP_SJDATA_QUERY	18  //查询实时数据
#define		APP_QUERY_RATE		19	//查询费率
#define		APP_RATE_MODE		20	//计费模型
#define    APP_UPDADA_BALANCE   21  //更新余额
#define    APP_SET_ACK   		22  //设置返回
#define    APP_UPDATA_ACK		23  //远程升级应答
#define 	APP_QR_CODE			24	//二维码下发
#define		BSP_4G_SENDNET1		30	//主动读取数据
#define		BSP_4G_SENDNET2		31	//主动读取数据
#define     APP_VIN_INFO_START	32	//汇誉启动的时候发送VIN信息


#define  	APP_START_CHARGE	3	//APP开始充电
#define  	APP_STOP_CHARGE		4	//APP停止充电
#define  	APP_START_VIN		5	//VIN鉴权成功
#define  	APP_START_QRCODE	6	//二维码设置

#define GPRS_IP2   	"114.55.186.206"
#define GPRS_PORT2   9800

//#define GPRS_IP1     "47.96.77.209"    //合肥乾古
//#define GPRS_PORT1   5001



#define GPRS_IP1     "121.199.192.223"    //云快充
#define GPRS_PORT1   8767

//#define GPRS_IP1     "101.37.179.163"    //云快充
//#define GPRS_PORT1   8767

//#define GPRS_IP1     "120.55.83.107"   //汇誉
//#define GPRS_PORT1   44444

//安培快充测试
//#define GPRS_IP1     "120.55.183.164"   
//#define GPRS_PORT1   5738
//安培正式
//#define GPRS_IP1     "114.55.186.206"   
//#define GPRS_PORT1   5738

//#define GPRS_IP1     "120.76.100.197"    
//#define GPRS_PORT1   10002


//#define GPRS_IP1     "122.114.122.174"
//#define GPRS_PORT1   35403


#define LINK_NUM     0 //哪一路网络
#define LINK_NET_NUM 2 // 最多连接2个
typedef enum
{
	XY_HY = 0,
	XY_YKC,
	XY_AP,
//	XY_HFQG,
	XY_XJ,  //小桔
	XY_YL1, //达克云
	XY_YL2, //预留2 --安充
	XY_YL3, //预留3 --星星
	XY_YL4, //预留4 --塑云
	XY_66,
	XY_JG,	//精工
	XY_TT,	//铁塔
	XY_ZSH,	//中石化
	XY_MAX,
}_XY_SELECE;   //协议选择


typedef struct
{
	_XY_SELECE XYSelece;	//协议选择
	INT8U IP[4];
	INT16U port;			//端口
	char  pIp[50];				//IP
	INT8U NetNum;			//网络个数 
}_NET_CONFIG_INFO;			//网络配置信息

extern _NET_CONFIG_INFO  NetConfigInfo[XY_MAX];
#define GPRS_UART 	   		USART3

typedef enum
{
	_4G_APP_START = 0,		//app启动
	_4G_APP_CARD,			//卡启动
	_4G_APP_VIN,			//VIN码启动
	_4G_APP_BCSTART,		//APP并充启动
	_4G_APP_BCCARD,			//卡并充启动
	_4G_APP_BCVIN,			//VIN码并充启动
	_4G_APP_MAX,
}_4G_START_TYPE;


typedef enum
{
    MODE_DATA = 0,			//数据模式
    MODE_HTTP,				//http模式
	MODE_FTP,				//FTP模式
} _4G_MODE;

//启动模式
typedef enum
{
	START_STANDARD = 0,		//标准启动
	START_BCCharge,			//并充
}_4G_START_MODE;

//启动的时候网络是在线还是离线
typedef enum
{
	NET_STATE_ONLINE = 0,		
	NET_STATE_OFFLINE,	
	NET_STATE_MAX,
}_START_NET_STATE;

typedef enum{
	RECV_NOT_DATA = 0,		//没有接收到数据
	RECV_FOUND_DATA			//有数据
}_RECV_DATA_STATUS;

typedef struct
{
	INT32U AllLen;         //总长度为HTTP专门使用
    INT32U DownPackLen;		//下载当前包的长度
	INT8U DataBuf[1500];	//接收数据缓存
	INT16U len;			//接收数据长度
	_RECV_DATA_STATUS RecvStatus;	//接收状态
	INT32U RecvLen;         //接收的总长度
}_RECV_DATA_CONTROL;

typedef enum
{
	STATE_4G_ERR = 0,
	STATE_OK,
}_4G_STATE;     


typedef enum
{
   _4G_RESPOND_ERROR = 0,                    //0:4G无回复或回复异常
   _4G_RESPOND_OK,                           //1:4G回复正常
	_4G_RESPOND_AGAIN,						//再次发送
}_4G_RESPOND_TYPE;

typedef struct
{
    char* ATCode;                              //AT指令
    INT8U (*Fun)(INT8U *pdata, INT16U len);    //对应的处理函数
}_4G_AT_FRAME;

typedef struct{
	INT16U cmd;
	 INT8U  (*recvfunction)(INT8U *,INT16U);
}_4G_RECV_TABLE;

typedef struct
{
    INT32U 		curtime;
    INT32U		lasttime;	
    INT32U    	cycletime;           // 超时时间      0表示不需要周期性发送
	INT8U 		(*Sendfunc)(void);  //发送函数
}_4G_SEND_TABLE;					//周期性发送

typedef struct 
{
	//在线交易记录
	INT8U ResendBillState;  //0表示不需要重发   1表示需要重发
	INT32U CurTime;			//当前发送时间发送时间
	INT32U LastTime;		//上一次发送时间
	//离线交易记录
	//是否发送取决于交易记录个数
	INT8U OffLineNum;			//离线交易记录个数 0
	INT32U OFFLineCurTime;			//当前发送时间发送时间
	INT32U OFFLineLastTime;		//上一次发送时间
	INT8U SendCount;			//发送计数，最多发送三次
}_RESEND_BILL_CONTROL;

typedef struct
{
    char ServerAdd[128];
    char ServerPassword[50];
    uint8_t crc;
} _HTTP_INFO;


__packed typedef struct
{
	INT8U	SerAdd[16];	//升级服务器地址	
	INT16U	Port;			//端口
	INT8U     usename[16];	//用户名  （ASCII 码）不足 16 位补零
    INT8U     password[16];	//密码  （ASCII 码）不足 16 位补零
	INT8U		FilePath[32];	//文件路径   （ASCII 码）不足 32 位补零，文件路径名由平台定义
}_FTP_INFO;

#define OTA_FLASH_LEN   4096
#define OTA_FLASH_NUM	5
typedef struct
{
	INT8U  DataBuf[OTA_FLASH_NUM][OTA_FLASH_LEN];	//接收数据缓存
	INT8U 	num;
	INT16U 	flascount;
	INT32U	Offset;		//
}_RECV_OTA_FLASH_CONTROL;
extern _RECV_OTA_FLASH_CONTROL OTAFlashControl;
extern _HTTP_INFO HttpInfo;
extern _FTP_INFO  FtpInfo;
extern _RESEND_BILL_CONTROL ResendBillControl[GUN_MAX];
extern INT8U	Qrcode[GUN_MAX][256];
/*****************************************************************************
* Function     :Module_HTTPDownload
* Description  :HTTP下载
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Module_HTTPDownload(_HTTP_INFO *info);

/*****************************************************************************
* Function     :Module_FatDownload
* Description  :FTP下载
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t Module_FTPDownload(_FTP_INFO *info);

/*****************************************************************************
* Function     : Send_AT_CIPMODE
* Description  : 数据透传
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U Send_AT_CIPMODE(void);

/*****************************************************************************
* Function     : 4G_RecvFrameDispose
* Description  :4G接收
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U _4G_RecvFrameDispose(INT8U * pdata,INT16U len);

/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   Pre4GBill(_GUN_NUM gun,INT8U *pdata);

/*****************************************************************************
* Function     : APP_GetSIM7600Mode
* Description  : 获取4g当前模式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_MODE APP_GetSIM7600Mode(void);


/*****************************************************************************
* Function     : APP_GetSIM7600Mode
* Description  : 获取4g当前模式
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t APP_SetSIM7600Mode(_4G_MODE mode);

/*****************************************************************************
* Function     : Pre4GBill
* Description  : 保存订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendDevState(_GUN_NUM gun);

/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendBill(_GUN_NUM gun);

/*****************************************************************************
* Function     : YKC_SendBalanceAck
* Description  : 发送更新余额应答 
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendBalanceAck(_GUN_NUM gun);

/*****************************************************************************
* Function     : _66_SendQuerySetAck
* Description  : 查询设置返回
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _66_SendQuerySetAck(void);

/*****************************************************************************
* Function     : _4G_SendStOPtAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _4G_SendStopAck(_GUN_NUM gun);


/*****************************************************************************
* Function     : HY_SendQueryRateAck
* Description  : 查询费率应答 
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U    _4G_SendQueryRate(void);


/*****************************************************************************
* Function     : _4G_SendRateMode
* Description  : 发送计费模型
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U    _4G_SendRateMode(void);

/*****************************************************************************
* Function     : HY_SendFrameDispose
* Description  :4G发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U  _4G_SendFrameDispose(void);


/*****************************************************************************
* Function     : HFQG_SendStartAck
* Description  : 合肥乾古开始充电应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   _4G_SendStartAck(_GUN_NUM gun);

/*****************************************************************************
* Function     :Module_SIM7600Test
* Description  :模块是否存在
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U Module_SIM7600Test(void);

/*****************************************************************************
* Function     :ModuleSIM7600_ConnectServer
* Description  :连接服务器
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U ModuleSIM7600_ConnectServer(INT8U num,INT8U* pIP,INT16U port);

/*****************************************************************************
* Function     :SIM7600_RecvDesposeCmd
* Description  :命令模式下模块接收处理
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U SIM7600_RecvDesposeCmd(INT8U *pdata,INT16U len);

/*****************************************************************************
* Function     : SIM7600Reset
* Description  : 模块复位
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
INT8U SIM7600Reset(void);

/*****************************************************************************
* Function     : SIM7600CloseNet
* Description  : 关闭网络
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
INT8U SIM7600CloseNet(INT8U num);

/*****************************************************************************
* Function     : APP_GetSIM7600Status
* Description  : 获取模块是否存在
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
_4G_STATE APP_GetSIM7600Status(void);

/*****************************************************************************
* Function     : APP_GetModuleConnectState
* Description  : 连接服务器状态
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetModuleConnectState(INT8U num);

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
_4G_STATE APP_GetAppRegisterState(INT8U num);

/*****************************************************************************
* Function     : APP_GetAppRegisterState
* Description  : 注册是否成功,最大可连接10路
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U  APP_SetAppRegisterState(INT8U num,_4G_STATE state);

/*****************************************************************************
* Function     : APP_RecvDataControl
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
_RECV_DATA_CONTROL	* APP_RecvDataControl(INT8U num);

/*****************************************************************************
* Function     : Send_AT_CIPRXGET
* Description  : 读取数据请求
* Input        :num  哪个socket 
				len   数据长度
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U Send_AT_CIPRXGET(INT8U num);

/*****************************************************************************
* Function     : UART_4GWrite
* Description  :串口写入，因多个任务用到了串口写入，因此需要加互斥锁
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2020-11-26     叶喜雨
*****************************************************************************/
INT8U UART_4GWrite(INT8U* const FrameBuf, const INT16U FrameLen);

/*****************************************************************************
* Function     : APP_GetSendTaskEvent
* Description  : 获取发送任务事件
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
OS_EVENT * APP_GetSendTaskEvent(void);

/*****************************************************************************
* Function     :ModuleSIM7600_SendData
* Description  :发送数据
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U ModuleSIM7600_SendData(INT8U num,INT8U* pdata,INT16U len);

/*****************************************************************************
* Function     :APP_SetNetNotConect
* Description  :设置网络未连接
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U APP_SetNetNotConect(INT8U num);

/*****************************************************************************
* Function     : APP_GetGPRSMainEvent
* Description  :获取GPRSMain任务事件
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
OS_EVENT* APP_Get4GMainEvent(void);

/*****************************************************************************
* Function     : APP_GetGPRSMainEvent
* Description  :获取网络状态 
* Input        : 那一路
* Output       : TRUE:表示有网络	FALSE:表示无网络
* Return       : 
* Note(s)      : 
* Contributor  : 2018-6-14      
*****************************************************************************/
INT8U  APP_GetNetState(INT8U num);

/*****************************************************************************
* Function     : Send_AT_CSQ
* Description  : 读取信号强度
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U Send_AT_CSQ(void);

/*****************************************************************************
* Function     : APP_GetCSQNum
* Description  : 获取型号强度值
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
INT8U APP_GetCSQNum(void);



/*****************************************************************************
* Function     : APP_GetBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetBatchNum(INT8U gun);

/*****************************************************************************
* Function     : APP_GetNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetNetMoney(INT8U gun);

/*****************************************************************************
* Function     : APP_GetResendBillState
* Description  : 获取是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U APP_GetResendBillState(INT8U gun);

/*****************************************************************************
* Function     : APP_SetResendBillState
* Description  : 设置是否重发状态
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
void APP_SetResendBillState(INT8U gun,INT8U state);

/*****************************************************************************
* Function     : ReSendBill
* Description  : 重发订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U  ReSendBill(INT8U gun,INT8U* pdata,INT8U ifquery);

/*****************************************************************************
* Function     : _4G_SendRateAck
* Description  : 费率应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendRateAck(INT8U cmd);


/*****************************************************************************
* Function     : _4G_SendSetTimeAck
* Description  : 校时应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendSetTimeAck(void);


/*****************************************************************************
* Function     : ZF_SendFrameDispose
* Description  : 周期性发送政府平台数据
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   ZF_SendFrameDispose(void);

/*****************************************************************************
* Function     : ZF_SendStartCharge
* Description  : 启动帧
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   ZF_SendStartCharge(void);


/*****************************************************************************
* Function     : HY_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U ZF_SendBill(void);

/*****************************************************************************
* Function     : HY_SendQueryRateAck
* Description  : 查询费率应答 
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   HY_SendQueryRateAck(void);


/*****************************************************************************
* Function     : YKC_SendSJDataGunBCmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendSJDataGunACmd13(void);

/*****************************************************************************
* Function     : YKC_SendSJDataGunBCmd13
* Description  : 云快充发送实时数据    上送充电枪实时数据，周期上送时，待机 5 分钟、充电 15 秒
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   完成
*****************************************************************************/
INT8U   YKC_SendSJDataGunBCmd13(void);

/*****************************************************************************
* Function     : _4G_SendCardInfo
* Description  : 发送卡鉴权
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendCardInfo(_GUN_NUM gun);

/*****************************************************************************
* Function     : _4G_SendVinInfo
* Description  : 发送Vin鉴权
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendVinInfo(_GUN_NUM gun);
/*****************************************************************************
* Function     : _4G_SendCardVinCharge
* Description  : 发送卡充电
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U _4G_SendCardVinCharge(_GUN_NUM gun);

/*****************************************************************************
* Function     : AP_SendCardWLAck
* Description  : 发送卡白名单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendCardWLAck(void);

/*****************************************************************************
* Function     : AP_SendVinWLAck
* Description  : 发送VIN白名单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendVinWLAck(void);

/*****************************************************************************
* Function     : AP_SendVinCardResAck
* Description  : 清空白名单应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  AP_SendVinCardResAck(void);

/*****************************************************************************
* Function     : _4G_SetStartType
* Description  : 设置安培快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   _4G_SetStartType(INT8U gun ,_4G_START_TYPE  type);

/*****************************************************************************
* Function     : APP_GetStartNetState
* Description  : 获取启动方式网络状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_GetStartNetState(INT8U gun);

/*****************************************************************************
* Function     : APP_SetStartNetState
* Description  : 设置启动方式网络状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   APP_SetStartNetState(INT8U gun ,_START_NET_STATE  type);

/*****************************************************************************
* Function     : PreAPOffLineBill
* Description  : 保存离线交易记录
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  PreAPOffLineBill(_GUN_NUM gun,INT8U *pdata);

/*****************************************************************************
* Function     : PreAPFSOffLineBill
* Description  : 上传分时交易记录   充电完成后，在B12发送完成后上报。
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U  PreAPFSOffLineBill(_GUN_NUM gun,INT8U *pdata);

/*****************************************************************************
* Function     : ReSendOffLineBill
* Description  : 
* Input        : 发送离线交易记录订单
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U  ReSendOffLineBill(void);

/*****************************************************************************
* Function     : _4G_GetStartType
* Description  : 获取快充启动方式
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
_4G_START_TYPE   _4G_GetStartType(INT8U gun);

/*****************************************************************************
* Function     : YKC_SendUpdataAck
* Description  : 云快充发送远程升级应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U YKC_SendUpdataAck(void);

/*****************************************************************************
* Function     : YKC_SendAPPQR_Ack
* Description  : 二维码应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   YKC_SendAPPQR_Ack(INT8U gun);

/*****************************************************************************
* Function     : _4G_SendUpdataAck
* Description  : 远程升级应答
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   _4G_SendUpdataAck(void);

/*****************************************************************************
* Function     : APP_GetHYCurPower
* Description  :获取当前功率
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT16U APP_GetHYCurPower(void);

/*****************************************************************************
* Function     :Module_FatDownload
* Description  :FTP下载
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t FTPDownloadDispose(uint8_t * pdata,uint16_t lendata);

/*****************************************************************************
* Function     :FTPDownloadWriter
* Description  :写flash
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018年7月11日
*****************************************************************************/
uint8_t  FTPDownloadWriter(uint32_t * offset);
#endif
/************************(C)COPYRIGHT 2021 汇誉科技*****END OF FILE****************************/

