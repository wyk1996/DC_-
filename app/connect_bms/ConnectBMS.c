/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : connectbms.c
* Author              :
* Date First Issued : 10/6/2015
* Version           : V0.1
* Description       : 
*----------------------------------------历史版本信息-------------------------------------------
* History             :
* //2010            : V
* Description       :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "gbstandard.h"
#include "connectbms.h"
#include "connectbmsframe.h"
#include "bmstimeout.h"
#include "main.h"
#include "controlmain.h"
#include "DisplayMain.h"
#include "4GMain.h"
/* Private define-----------------------------------------------------------------------------*/
typedef enum
{
    APP_TIMEOUT = 0xA0,     //超时
    APP_SELFCHECK_SUCCESS,  //系统自检成功
    APP_RXDEAL_SUCCESS,     //接收处理成功
    APP_END_CHARGING,       //结束充电
    
    APP_DEFAULT = 0xFF,     //缺省状态值

} _APP_CONNECT_BMSSTATE;    //连接BMS过程中的状态监测
//此三个PGN的帧，只响应，咱不解析具体数据
#define BMS_BMV_ID    GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMV_PGN_5376)
#define BMS_BMT_ID    GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMT_PGN_5632)
#define BMS_BSP_ID    GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BSP_PGN_5888)

#define TASK_STK_SIZE_SendFrame    0x100

//重新连接控制
#define TIMEOUT_RECONNECT_NUM      3     //超时重新连接的次数 
/* Private typedef----------------------------------------------------------------------------*/

/* Private macro------------------------------------------------------------------------------*/
BSPCANTxMsg   BMSCANTxbuffA;      //发送缓冲
BSPCANRxMsg   BMSCANRxbuffA[BMSCANRX_BUFF_LEN];//接收缓冲
_BSP_MESSAGE  BMSRxMessageA[BMSCANRX_BUFF_LEN];//消息接收缓冲
BSP_CAN_Set  bxBMSCANSetA;


BSPCANTxMsg   BMSCANTxbuffB;      //发送缓冲
BSPCANRxMsg   BMSCANRxbuffB[BMSCANRX_BUFF_LEN];//接收缓冲
_BSP_MESSAGE  BMSRxMessageB[BMSCANRX_BUFF_LEN];//消息接收缓冲
BSP_CAN_Set  bxBMSCANSetB;

_BMS_RTS_FRAME_INF  BMS_RTS_FrameInf[GUN_MAX];//BMS传输过来的帧信息
/* Private variables--------------------------------------------------------------------------*/

OS_EVENT  *TaskConnectBMS_peventA;    //CAN消息队列指针
void *bxBMSCANOSQA[BMSCANOSQ_NUM];    //指针数组bxCANOSQ[]容纳指向各个消息的指针

OS_EVENT  *TaskConnectBMS_peventB;    //CAN消息队列指针
void *bxBMSCANOSQB[BMSCANOSQ_NUM];    //指针数组bxCANOSQ[]容纳指向各个消息的指针

OS_EVENT  *TaskSendFrame_preventA;    //发送帧任务消息队列
OS_EVENT  *TaskSendFrame_preventB;    //发送帧任务消息队列
void *txBMSCANOSQA[BMSCANOSQ_NUM];    //发送消息缓冲
void *txBMSCANOSQB[BMSCANOSQ_NUM];    //发送消息缓冲

INT8U RxBMSBufferA[256];  //暂定128,可以根据实际最长的帧数来缩减
INT8U RxBMSBufferB[256];  //暂定128,可以根据实际最长的帧数来缩减

_SPECIAL_PGN	Special_PGN[GUN_MAX][3];

_CANFRAMECONTROL		CanFrameControl[GUN_MAX];
_BMS_CONNECT_CONTROL	BMS_CONNECT_Control[GUN_MAX];//管理流程的结构体
_BMS_ERR_CONTROL 		BMSErrControl[GUN_MAX];

INT8U CRMReadyflag[GUN_MAX] = {0};
_BMS_CUR_GB ChargGB_Type[GUN_MAX] = {BMS_GB_2015};     //默认2015新国标
_VIN_CONTROL VinControl[GUN_MAX] = {0};
//充电故障处理
const _BMS_ERR_HANDLE BMSErrHandleTable[GUN_MAX][BMS_ERR_HANDLE_LEN] =
{
	{
		//{ERR_L1_INSULATION              ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //绝缘故障

		//{ERR_L1_LEAKAGE                 ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //漏电故障

		//{ERR_L1_EMERGENCY               ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //急停故障

		//{ERR_L2_CONNECT                 ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //连接器故障

		//{ERR_L2_CONN_TEMP_OVER          ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //BMS元件、输出连接器过温

	   // {ERR_L2_BGROUP_TEMP_OVER        ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //电池组温度过温

	   // {ERR_L2_BAT_VOL                 ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //单体电池电压过低、过高

	   // {ERR_L2_BMS_VOL_CUR             ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //BMS检测到充电电流过大或充电电压异常

	   // {ERR_L2_CHARGE_VOL_CUR          ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //充电机检测到充电电流不匹配或充电电压异常

		//{ERR_L2_CHARGE_TEMP_OVER        ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //充电机内部过温

		//{ERR_L2_CHARGE_POWER_UNARRIVAL  ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //充电机电量不能传送

	   // {ERR_L2_CAR_CONN_ADHESION       ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //车辆接触器粘连

		//{ERR_L3_CHAGING_TIMEOUT         ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //充电握手阶段、配置阶段、充电过程超时

		{ERR_L3_CHAGING_OVER_TIMEOUT    ,   ERR_METHOD_END  ,     ChargeErrHandleA  }, //充电结束超时

		{ERR_LF_METHOD_A                ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //故障A

		{ERR_LF_METHOD_B                ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //故障B

		{ERR_LF_METHOD_C                ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //故障C
	},
	{
		//{ERR_L1_INSULATION              ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //绝缘故障

		//{ERR_L1_LEAKAGE                 ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //漏电故障

		//{ERR_L1_EMERGENCY               ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //急停故障

		//{ERR_L2_CONNECT                 ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //连接器故障

		//{ERR_L2_CONN_TEMP_OVER          ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //BMS元件、输出连接器过温

	   // {ERR_L2_BGROUP_TEMP_OVER        ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //电池组温度过温

	   // {ERR_L2_BAT_VOL                 ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //单体电池电压过低、过高

	   // {ERR_L2_BMS_VOL_CUR             ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //BMS检测到充电电流过大或充电电压异常

	   // {ERR_L2_CHARGE_VOL_CUR          ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //充电机检测到充电电流不匹配或充电电压异常

		//{ERR_L2_CHARGE_TEMP_OVER        ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //充电机内部过温

		//{ERR_L2_CHARGE_POWER_UNARRIVAL  ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //充电机电量不能传送

	   // {ERR_L2_CAR_CONN_ADHESION       ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //车辆接触器粘连

		//{ERR_L3_CHAGING_TIMEOUT         ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //充电握手阶段、配置阶段、充电过程超时

		{ERR_L3_CHAGING_OVER_TIMEOUT    ,   ERR_METHOD_END  ,     ChargeErrHandleB  }, //充电结束超时

		{ERR_LF_METHOD_A                ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //故障A

		{ERR_LF_METHOD_B                ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //故障B

		{ERR_LF_METHOD_C                ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //故障C
	}
	
};

//处理控制任务发送的消息表
const _RECEIVE_Q_TABLE ReceiveMsgHandleTableA[BMS_MSG_TABLE_LEN] =
{
    {APP_CHARGE_END       ,   APP_ReceiveEndChargeA    }, //收到控制任务的结束命令
};

//处理控制任务发送的消息表
const _RECEIVE_Q_TABLE ReceiveMsgHandleTableB[BMS_MSG_TABLE_LEN] =
{
    {APP_CHARGE_END       ,   APP_ReceiveEndChargeB    }, //收到控制任务的结束命令
};

OS_STK TaskSendBMSFrameStkA[TASK_STK_SIZE_SendFrame];     // 任务堆栈
OS_STK TaskSendBMSFrameStkB[TASK_STK_SIZE_SendFrame];     // 任务堆栈




const _PROTOCOL_CAN_MANAGE_TABLE  Can_Data_Manage_Table[GUN_MAX][CANRECEIVETABLELEN] =
{
	{
		//ID          //接收到以后处理  //发送给MC的处理
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236 ),  0, CanProtocolOperationRTSA, NULL}, //J1939传输协议连接管理多页处理帧
		 
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BHM_PGN_9984),  0, CanProtocolOperationBHMA, NULL}, //车辆握手BHM报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BRM_PGN_512 ),  0, CanProtocolOperationBRMA, NULL}, //车辆握手BRM报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCP_PGN_1536),  0, CanProtocolOperationBCPA, NULL}, //动力蓄电池充电参数BCP报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BRO_PGN_2304),  0, CanProtocolOperationBROA, NULL}, //电池充电准备就绪状态BRO报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BCL_PGN_4096),  0, CanProtocolOperationBCLA, NULL}, //电池充电需求BCL报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCS_PGN_4352),  0, CanProtocolOperationBCSA, NULL}, //电池充电总状态BCS报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSM_PGN_4864),  0, CanProtocolOperationBSMA, NULL}, //动力蓄电池状态信息BSM报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMV_PGN_5376),  0, CanProtocolOperationBMVA, NULL}, //单体动力蓄电池电源BMV报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMT_PGN_5632),  0, CanProtocolOperationBMTA, NULL}, //动力蓄电池温度BMT报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BSP_PGN_5888),  0, CanProtocolOperationBSPA, NULL}, //动力蓄电池预留BSP报文

		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BST_PGN_6400),  0, CanProtocolOperationBSTA, NULL}, //BMS中止充电BST报文

		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSD_PGN_7168),  0, CanProtocolOperationBSDA, NULL}, //BMS统计数据BSD报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_2,GB_BEM_PGN_7680),  0, CanProtocolOperationBEMA, NULL}, //BMS错误BEM报文
	},
	{
		//ID          //接收到以后处理  //发送给MC的处理
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236 ),  0, CanProtocolOperationRTSB, NULL}, //J1939传输协议连接管理多页处理帧
		 
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BHM_PGN_9984),  0, CanProtocolOperationBHMB, NULL}, //车辆握手BHM报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BRM_PGN_512 ),  0, CanProtocolOperationBRMB, NULL}, //车辆握手BRM报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCP_PGN_1536),  0, CanProtocolOperationBCPB, NULL}, //动力蓄电池充电参数BCP报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BRO_PGN_2304),  0, CanProtocolOperationBROB, NULL}, //电池充电准备就绪状态BRO报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BCL_PGN_4096),  0, CanProtocolOperationBCLB, NULL}, //电池充电需求BCL报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCS_PGN_4352),  0, CanProtocolOperationBCSB, NULL}, //电池充电总状态BCS报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSM_PGN_4864),  0, CanProtocolOperationBSMB, NULL}, //动力蓄电池状态信息BSM报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMV_PGN_5376),  0, CanProtocolOperationBMVB, NULL}, //单体动力蓄电池电源BMV报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMT_PGN_5632),  0, CanProtocolOperationBMTB, NULL}, //动力蓄电池温度BMT报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BSP_PGN_5888),  0, CanProtocolOperationBSPB, NULL}, //动力蓄电池预留BSP报文

		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BST_PGN_6400),  0, CanProtocolOperationBSTB, NULL}, //BMS中止充电BST报文

		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSD_PGN_7168),  0, CanProtocolOperationBSDB, NULL}, //BMS统计数据BSD报文
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_2,GB_BEM_PGN_7680),  0, CanProtocolOperationBEMB, NULL}, //BMS错误BEM报文
	}
};

_PROTOCOL_CAN_SEND_TABLE  CAN_Send_Table[GUN_MAX][CANSENDTALELEN]=
{
	{
		//充电握手阶段
		{ BMS_CHM_SEND          ,    SYS_DELAY_250ms ,   CHM_SendShakeHandFrameA         }, // 0
		{ BMS_CRM_SEND          ,    SYS_DELAY_250ms ,   CRM_SendShakeHandIdentifyFrameA }, // 1 
		//充电参数配置阶段
		{ BMS_CTS_SEND         ,     SYS_DELAY_500ms ,   CTS_SendSynTimeMessageA         }, // 2充电机发送时间同步信息蔽?
		{ BMS_PARA_SEND         ,    SYS_DELAY_250ms ,   CML_SendMaxVolandCurA           }, // 3充电机最大输出能力报文
		{ BMS_CRO_SEND          ,    SYS_DELAY_250ms ,   CRO_SendChargeDevStandbyA       }, // 4充电机输出准备就绪状态报文(0xAA)
		{ BMS_CRO_UNREADY_SEND  ,    SYS_DELAY_250ms ,   CRO_SendChargeDevUnstandbyA     }, // 5充电机输出准备就绪状态报文(0x00)
		//充电阶段
		{ BMS_CCS_SEND          ,    SYS_DELAY_50ms  ,   CCS_DeviceChargeStateA          }, // 6
		{ BMS_CST_SEND          ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeA            }, // 7
		//充电结束阶段 
		{ BMS_CSD_SEND          ,    SYS_DELAY_250ms ,   CSD_DeviceChargeEndStatisticsA  }, // 8
		//充电机错误报文
		{ BMS_CEM_SEND          ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrA            }, // 9
		//充电机在处理故障ABC阶段
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrA           }, // 10 CEM
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeA            }, // 11 CST
	},
	{
		//充电握手阶段
		{ BMS_CHM_SEND          ,    SYS_DELAY_250ms ,   CHM_SendShakeHandFrameB         }, // 0
		{ BMS_CRM_SEND          ,    SYS_DELAY_250ms ,   CRM_SendShakeHandIdentifyFrameB }, // 1 
		//充电参数配置阶段
		{ BMS_CTS_SEND         ,     SYS_DELAY_500ms ,   CTS_SendSynTimeMessageB         }, // 2充电机发送时间同步信息蔽?
		{ BMS_PARA_SEND         ,    SYS_DELAY_250ms ,   CML_SendMaxVolandCurB           }, // 3充电机最大输出能力报文
		{ BMS_CRO_SEND          ,    SYS_DELAY_250ms ,   CRO_SendChargeDevStandbyB      }, // 4充电机输出准备就绪状态报文(0xAA)
		{ BMS_CRO_UNREADY_SEND  ,    SYS_DELAY_250ms ,   CRO_SendChargeDevUnstandbyB     }, // 5充电机输出准备就绪状态报文(0x00)
		//充电阶段
		{ BMS_CCS_SEND          ,    SYS_DELAY_50ms  ,   CCS_DeviceChargeStateB          }, // 6
		{ BMS_CST_SEND          ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeB            }, // 7
		//充电结束阶段 
		{ BMS_CSD_SEND          ,    SYS_DELAY_250ms ,   CSD_DeviceChargeEndStatisticsB  }, // 8
		//充电机错误报文
		{ BMS_CEM_SEND          ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrB            }, // 9
		//充电机在处理故障ABC阶段
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrB            }, // 10 CEM
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeB            }, // 11 CST
	}
};

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function      : Set_VIN_Success
* Description   : VIN是否鉴权成功  1表示成功 在心跳的低
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-07-11 Yxy
***********************************************************************************************/
void Set_VIN_Success(_GUN_NUM gun,_VIN_STATE state)
{
	if(gun >=GUN_MAX)
	{
		return;
	}
	VinControl[gun].VinState = state;
}


/***********************************************************************************************
* Function      : TaskConnectBMSMain
* Description   :
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
void InitConnectBMSACAN(void)
{
    TaskConnectBMS_peventA = OSQCreate(bxBMSCANOSQA, BMSCANOSQ_NUM); // 建立事件(消息队列)
    /*初始化can收发控制体*/
    memset(&BMSCANTxbuffA, 0x00, sizeof(BMSCANTxbuffA));
    bxBMSCANSetA.TxMessage =  &BMSCANTxbuffA;
    bxBMSCANSetA.RxMessage =  BMSCANRxbuffA;
    bxBMSCANSetA.BspMsg = BMSRxMessageA;
    bxBMSCANSetA.RxMaxNum = BMSCANRX_BUFF_LEN ;
    bxBMSCANSetA.BaundRate = BSP_CAN_BAUNDRATE_250KBPS; //波特率为250K
    BSP_CAN_Init(BMS_CAN_A, &bxBMSCANSetA, TaskConnectBMS_peventA);
}

/***********************************************************************************************
* Function      : TaskConnectBMSMain
* Description   :
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
void InitConnectBMSBCAN(void)
{
    TaskConnectBMS_peventB = OSQCreate(bxBMSCANOSQB, BMSCANOSQ_NUM); // 建立事件(消息队列)
    /*初始化can收发控制体*/
    memset(&BMSCANTxbuffB, 0x00, sizeof(BMSCANTxbuffB));
    bxBMSCANSetB.TxMessage =  &BMSCANTxbuffB;
    bxBMSCANSetB.RxMessage =  BMSCANRxbuffB;
    bxBMSCANSetB.BspMsg = BMSRxMessageB;
    bxBMSCANSetB.RxMaxNum = BMSCANRX_BUFF_LEN ;
    bxBMSCANSetB.BaundRate = BSP_CAN_BAUNDRATE_250KBPS; //波特率为250K
    BSP_CAN_Init(BMS_CAN_B, &bxBMSCANSetB, TaskConnectBMS_peventB);
}
/***********************************************************************************************
* Function      : APP_GetGBType
* Description   : 获取国标类型
* Input         :
* Output        :
* Note(s)       :
* Contributor   : Yxy 20160922
***********************************************************************************************/
_BMS_CUR_GB APP_GetGBType(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return BMS_GB_2015;
	}
    return ChargGB_Type[gun];
}
/***********************************************************************************************
* Function      : APP_SetGBType
* Description   : 设置
* Input         :
* Output        :
* Note(s)       :
* Contributor   : Yxy 20160922
***********************************************************************************************/
void APP_SetGBType(INT8U gun, _BMS_CUR_GB dat)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ChargGB_Type[gun] = dat;
} 
/***********************************************************************************************
* Function      : ClearCanFrameInformation
* Description   : 清楚多帧传输结构体控制各个元素
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 15/1/2015 叶喜雨
***********************************************************************************************/
void ClearCanFrameInformation(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return; 
	}
    memset(&CanFrameControl[gun], 0x00, sizeof(_CANFRAMECONTROL));
}

/*****************************************************************************
* Function      : APP_GetBMSBHM
* Description   : 获取BHM帧
* Input         : _BMS_BHM_CONTEXT *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月19日  叶喜雨
*****************************************************************************/
INT8U APP_GetBMSBHM(INT8U gun,_BMS_BHM_CONTEXT *pdata)
{
    if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return FALSE;
    }
    memcpy(pdata,&BMS_BHM_Context[gun],sizeof(_BMS_BHM_CONTEXT));
    return TRUE;
}
/***********************************************************************************************
* Function		: APP_GetBMS_BSD
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_BSD(INT8U gun,_BMS_BSD_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_BSD_Context[gun],sizeof(BMS_BSD_Context));
}
/***********************************************************************************************
* Function		: APP_GetBMS_BCP
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_BCP(INT8U gun,_BMS_BCP_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_BCP_Context[gun],sizeof(BMS_BCP_Context));
}
/***********************************************************************************************
* Function		: APP_GetBMS_CML
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_CML(INT8U gun,_BMS_CML_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_CML_Context[gun],sizeof(BMS_CML_Context));
}
/***********************************************************************************************
* Function		: APP_GetBMS_BRM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_BRM(INT8U gun,_BMS_BRM_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_BRM_Context[gun],sizeof(BMS_BRM_Context));
}
/***********************************************************************************************
* Function		: APP_GetBMS_BCL
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_BCL(INT8U gun,_BMS_BCL_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_BCL_Context[gun],sizeof(BMS_BCL_Context));
}

/***********************************************************************************************
* Function		: APP_GetBMS_BSM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_BSM(INT8U gun,_BMS_BSM_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_BSM_Context[gun],sizeof(BMS_BSM_Context));
}
/***********************************************************************************************
* Function		: APP_GetBMS_BST
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_BST(INT8U gun,_BMS_BST_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_BST_Context[gun],sizeof(BMS_BST_Context));
}
/***********************************************************************************************
* Function		: APP_GetBMS_BEM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_BEM(INT8U gun,_BMS_BEM_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_BEM_Context[gun],sizeof(BMS_BEM_Context));
}
/***********************************************************************************************
* Function		: APP_GetBMS_BEM
* Description	: 获取BMS数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 Yxy
***********************************************************************************************/
void APP_GetBMS_CEM(INT8U gun,_BMS_CEM_CONTEXT *pdata)
{
	if ((pdata == NULL) || (gun >= GUN_MAX))
    {
        return;
    }
	memcpy(pdata,&BMS_CEM_Context[gun],sizeof(BMS_CEM_Context));
}

/*****************************************************************************
* Function      : GetBMSErrState
* Description   : 获取BMS错误状态
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
_BMS_ERR_STATE GetBMSErrState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return ERR_STATE_NORMAL;
	}
    return BMSErrControl[gun].ErrState;
}

/*****************************************************************************
* Function      : SetBMSErrState
* Description   : 设置BMS错误状态
* Input         : _BMS_ERR_STATE state
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetBMSErrState(INT8U gun ,_BMS_ERR_STATE state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    BMSErrControl[gun].ErrState = state;
}


/*****************************************************************************
* Function      : ChargeErrDeal
* Description   : 查表处理充电故障函数
* Input         : _BMS_ERR_TYPE type
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月2日  叶喜雨
*****************************************************************************/
INT8U SetBMSErrMethodRetry(_GUN_NUM gun,_BMS_ERR_RETRY type)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    switch (type)
    {
        case ERR_RETRY_DEC:
            if (BMSErrControl[gun].MethodCRetry)
            {
                BMSErrControl[gun].MethodCRetry--;
            }
            break;
        case ERR_RETRY_INC:
            if (BMSErrControl[gun].MethodCRetry < BMS_ERR_RETRY_NUM)
            {
                BMSErrControl[gun].MethodCRetry--;
            }
            break;
        case ERR_RETRY_CLEAR:
            BMSErrControl[gun].MethodCRetry = 0;
            break;
        case ERR_RETRY_SET:
            BMSErrControl[gun].MethodCRetry = BMS_ERR_RETRY_NUM;
            break;
        default:
            break;
    }
    return BMSErrControl[gun].MethodCRetry;
}

/*****************************************************************************
* Function      : SetCEMState
* Description   : 设置CEM报文所需要的各个状态
* Input         : _BMS_CEM_TYPE type
             _BMS_ERR_STATE_TYPE state
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月6日  叶喜雨
*****************************************************************************/
INT8U SetCEMState(INT8U gun, _BMS_CEM_TYPE type, _BMS_ERR_STATE_TYPE state)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    switch (type)
    {
        case CEM_BRM:
            BMS_CEM_Context[gun].State0.OneByte.BRMBits = state;
            break;
        case CEM_BCP:
            BMS_CEM_Context[gun].State1.OneByte.BCPBits = state;
            break;
        case CEM_BRO:
            BMS_CEM_Context[gun].State1.OneByte.BROBits = state;
            break;
        case CEM_BCS:
            BMS_CEM_Context[gun].State2.OneByte.BCSBits = state;
            break;
        case CEM_BCL:
            BMS_CEM_Context[gun].State2.OneByte.BCLBits = state;
            break;
        case CEM_BST:
            BMS_CEM_Context[gun].State2.OneByte.BSTBits = state;
            break;
        case CEM_BSD:
            BMS_CEM_Context[gun].State3.OneByte.BSDBits = state;
            break;
        case CEM_ALL: //清空所有错误
            memset(&BMS_CEM_Context[gun], state, sizeof(_BMS_CEM_CONTEXT));
			//BMS发送的帧预留需要设置为1
            BMS_CEM_Context[gun].State0.AllBits = BMS_CEM_Context[gun].State0.AllBits|0xFC;
            BMS_CEM_Context[gun].State1.AllBits = BMS_CEM_Context[gun].State1.AllBits|0xF0;
            BMS_CEM_Context[gun].State2.AllBits = BMS_CEM_Context[gun].State2.AllBits|0xC0;
            BMS_CEM_Context[gun].State3.AllBits = BMS_CEM_Context[gun].State3.AllBits|0xFC;
        default:
            break;
    }
    return TRUE;
}

/*****************************************************************************
* Function      : APP_ReceiveEndChargeA
* Description   : 接收到控制任务的结束命令
* Input         : void *pdata
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_ReceiveEndChargeA(void *pdata)
{
    _SYS_STATE *p;

    if (pdata == NULL)
    {
        return FALSE;
    }
    p = (_SYS_STATE *)pdata;
	//自检错误，对应于启动失败，不算是停止的错误
//    if (p->TotalErrState & ERR_SELFCHECK_TYPE) //自检错误，暂时未处理
//    {

//    }
#if 0   
    if ( (p->TotalErrState & ERR_CHARGESTOP_TYPE) \
      || (p->TotalErrState & ERR_BMSSTOP_TYPE) \
      || (p->TotalErrState & ERR_USER_STOP_TYPE) \
      || (p->TotalErrState & ERR_REACH_CONDITION_TYPE) )  
#endif
    //充电机中止原因
    {
        //复制充电机中止原因到CST报文,发给BMS是在任务里循环扫描发送的
        memcpy(&BMS_CST_Context[GUN_A], &(p->StopReason.ChargeStop), sizeof(_BMS_CST_CONTEXT));
    }
    if (p->TotalErrState & ERR_CHARGERXTIMEOUT_TYPE) //充电机接收超时原因
    {
        //复制充电机错误原因到CEM报文,发给BMS是在任务里循环扫描发送的
        memcpy(&BMS_CEM_Context[GUN_A], &(p->StopReason.ChargeErr), sizeof(_BMS_CEM_CONTEXT));
    }
    return TRUE;
}


/*****************************************************************************
* Function      : APP_ReceiveEndChargeB
* Description   : 接收到控制任务的结束命令
* Input         : void *pdata
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_ReceiveEndChargeB(void *pdata)
{
    _SYS_STATE *p;

    if (pdata == NULL)
    {
        return FALSE;
    }
    p = (_SYS_STATE *)pdata;
	//自检错误，对应于启动失败，不算是停止的错误
//    if (p->TotalErrState & ERR_SELFCHECK_TYPE) //自检错误，暂时未处理
//    {

//    }
#if 0   
    if ( (p->TotalErrState & ERR_CHARGESTOP_TYPE) \
      || (p->TotalErrState & ERR_BMSSTOP_TYPE) \
      || (p->TotalErrState & ERR_USER_STOP_TYPE) \
      || (p->TotalErrState & ERR_REACH_CONDITION_TYPE) )  
#endif
    //充电机中止原因
    {
        //复制充电机中止原因到CST报文,发给BMS是在任务里循环扫描发送的
        memcpy(&BMS_CST_Context[GUN_B], &(p->StopReason.ChargeStop), sizeof(_BMS_CST_CONTEXT));
    }
    if (p->TotalErrState & ERR_CHARGERXTIMEOUT_TYPE) //充电机接收超时原因
    {
        //复制充电机错误原因到CEM报文,发给BMS是在任务里循环扫描发送的
        memcpy(&BMS_CEM_Context[GUN_B], &(p->StopReason.ChargeErr), sizeof(_BMS_CEM_CONTEXT));
    }
    return TRUE;
}


/***********************************************************************************************
* Function      : BMS_CONNECT_ControlInit BMS_CONNECT_StateSet
* Description   : 充电流程控制结构体。与设置函数
* Input         :
* Return        :
* Note(s)       :
* Contributor   : 160523    叶喜雨
***********************************************************************************************/
void BMS_CONNECT_ControlInit(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    BMS_CONNECT_Control[gun].step   = BMS_SEND_DEFAULT;
    BMS_CONNECT_Control[gun].laststate = BMS_CONNECT_DEFAULT;
    BMS_CONNECT_Control[gun].currentstate = BMS_CONNECT_DEFAULT;

    //这里还要添加其他如果需要的结构体初始值，比如下面的
    //后面有用到这个状态来判断，所以初始化为无效状态
    BMS_BRO_Context[gun].StandbyFlag = GB_BMSSTATE_INVALID;

    //暂时直接调用
    BMSErrControl[gun].ErrState = ERR_STATE_NORMAL;  //没有错误状态
    BMSErrControl[gun].MethodCRetry = BMS_ERR_RETRY_NUM; //重试3次
    BMSErrControl[gun].SelfCheck = TRUE; //需要自检

    //CEM所有错误为ERR_TYPE_NORMAL
    SetCEMState(gun,CEM_ALL, ERR_TYPE_NORMAL);
    //BEM所有错误为ERR_TYPE_NORMAL
    memset(&BMS_BEM_Context[gun], ERR_TYPE_NORMAL, sizeof(_BMS_BEM_CONTEXT));

}


/*****************************************************************************
* Function      : BMS_FrameContextClear
* Description   : 清除BMS接收、发送需要的帧内容
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月17日  叶喜雨
*****************************************************************************/
void BMS_FrameContextClear(INT8U gun)
{
    //清BMS发送过来的帧内容
	if(gun >= GUN_MAX)
	{
		return;
	}
    //清BHM内容
    memset((void *)&BMS_BHM_Context[gun].MaxChargeWholeVol, 0, sizeof(_BMS_BHM_CONTEXT));
    //清BRM内容
    memset((void *)BMS_BRM_Context[gun].BMSVersion, 0, sizeof(_BMS_BRM_CONTEXT));
    //清BCP内容
    memset((void *)&BMS_BCP_Context[gun].UnitBatteryMaxVol, 0, sizeof(_BMS_BCP_CONTEXT));
    //清BRO内容
    memset((void *)&BMS_BRO_Context[gun].StandbyFlag, 0, sizeof(_BMS_BRO_CONTEXT));
    //清BCL内容
    memset((void *)&BMS_BCL_Context[gun].DemandVol, 0, sizeof(_BMS_BCL_CONTEXT));
    //清BCS内容
    memset((void *)&BMS_BCS_Context[gun].ChargeVolMeasureVal, 0, sizeof(_BMS_BCS_CONTEXT));
    //清BSM内容
    memset((void *)&BMS_BSM_Context[gun].MaxUnitVolandNum, 0, sizeof(_BMS_BSM_CONTEXT));
    //清BST内容
    memset((void *)&BMS_BST_Context[gun].State0.EndChargeReason, 0, sizeof(_BMS_BST_CONTEXT));
    //清BSD内容
    memset((void *)&BMS_BSD_Context[gun].EndChargeSOC, 0, sizeof(_BMS_BSD_CONTEXT));
    //清BEM内容
    memset((void *)&BMS_BEM_Context[gun].State0.AllBits, 0, sizeof(_BMS_BEM_CONTEXT));
    
    //清充电桩发送的帧内容
    //CML报文内容
    memset((void *)&BMS_CML_Context[gun].MaxOutVol, 0, sizeof(_BMS_CML_CONTEXT));
    //充电机充电状态报文
    memset((void *)&BMS_CCS_Context[gun].OutputVol, 0, sizeof(_BMS_CCS_CONTEXT));
    //充电机中止充电报文
    memset((void *)&BMS_CST_Context[gun].State0.EndChargeReason, 0, sizeof(_BMS_CST_CONTEXT));
    //充电机统计报文内容
    memset((void *)&BMS_CSD_Context[gun].TotalChargeTime, 0, sizeof(_BMS_CSD_CONTEXT));
    //充电机错误原因
    memset((void *)&BMS_CEM_Context[gun].State0.AllBits, 0, sizeof(_BMS_CEM_CONTEXT));
    BMS_CEM_Context[gun].State0.AllBits = BMS_CEM_Context[gun].State0.AllBits|0xFC;
    BMS_CEM_Context[gun].State1.AllBits = BMS_CEM_Context[gun].State1.AllBits|0xF0;
    BMS_CEM_Context[gun].State2.AllBits = BMS_CEM_Context[gun].State2.AllBits|0xC0;
    BMS_CEM_Context[gun].State3.AllBits = BMS_CEM_Context[gun].State3.AllBits|0xFC;
}

/*****************************************************************************
* Function      : BMS_CONNECT_StepSet
* Description   : 通知BMS发送任务发送一帧数据
* Input         : _SEND_BMS_STEP step
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月1日  叶喜雨
*****************************************************************************/
void BMS_CONNECT_StepSet(INT8U gun,_SEND_BMS_STEP step)
{
	static _BSP_MESSAGE send_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return;
	}

    BMS_CONNECT_Control[gun].step = step;
    //进入step先发送一次
    send_message[gun].MsgID = (_BSP_MSGID)(step);
	if(gun == GUN_A)
	{	
		OSQPost(TaskSendFrame_preventA, &send_message[gun]);
	}
	else
	{
		OSQPost(TaskSendFrame_preventB, &send_message[gun]);
	}
	
	OSTimeDly(SYS_DELAY_10ms);
}

void BMS_CONNECT_StateSet(INT8U gun,_CONNECT_BMS_STATE state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    //设置的时候永远都是先把当前的状态备份到上一次状态
    BMS_CONNECT_Control[gun].laststate = BMS_CONNECT_Control[gun].currentstate;
    BMS_CONNECT_Control[gun].currentstate = state;
    //充电阶段的特殊处理
    //取消充电阶段的BCL  BCS控制

}

/*****************************************************************************
* Function      : BMSErrDeal
* Description   : 收到BMS错误报文处理
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月6日  叶喜雨
*****************************************************************************/
void BMSErrDeal(INT8U gun)
{
    static _BSP_MESSAGE send_message[GUN_MAX];
    
	if(gun >= GUN_MAX)
	{
		return ;
	}
    //参考GB/T 27930-2015第37页,收到BEM错误报文
    //(1)发送充电机错误报文CEM跟CST给BMS
    BMS_CONNECT_StepSet(gun,BMS_CEM_CST_SEND);
	
    SetBMSErrState(gun,ERR_STATE_BMS);             //处理BMS发送的错误报文BEM
    
    send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
    send_message[gun].DivNum = APP_BMS_CHARGE_ERR_C;
	send_message[gun].GunNum = gun;
   if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &send_message[gun]);	//给控制任务发送消息
	}
	else
	{
		OSQPost(Control_PeventB, &send_message[gun]);	//给控制任务发送消息
	}
	OSTimeDly(SYS_DELAY_10ms);
}

/*****************************************************************************
* Function      : APP_DealReceiveData
* Description   : 处理从其他任务接收到的消息(非CAN接收)
* Input         : _BSP_MESSAGE *pmsg
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_DealReceiveDataA(_BSP_MESSAGE *pmsg)
{
    INT8U count;
    INT8U allnum = 0;
    const _RECEIVE_Q_TABLE * p = ReceiveMsgHandleTableA;

    if ( (pmsg == NULL) || (pmsg->DataLen == 0) || (pmsg->pData == NULL) )
    {
        return FALSE;
    }

    allnum = BMS_MSG_TABLE_LEN;
    for(count = 0; count < allnum; count++)
    {
        if(pmsg->DivNum && (pmsg->DivNum == p[count].DataID))
        {
            if (p->function != NULL)
            {
                return (p->function((void *)pmsg->pData));
            }
        }
    }
    return FALSE;
}

/*****************************************************************************
* Function      : APP_DealReceiveData
* Description   : 处理从其他任务接收到的消息(非CAN接收)
* Input         : _BSP_MESSAGE *pmsg
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_DealReceiveDataB(_BSP_MESSAGE *pmsg)
{
    INT8U count;
    INT8U allnum = 0;
    const _RECEIVE_Q_TABLE * p = ReceiveMsgHandleTableB;

    if ( (pmsg == NULL) || (pmsg->DataLen == 0) || (pmsg->pData == NULL) )
    {
        return FALSE;
    }

    allnum = BMS_MSG_TABLE_LEN;
    for(count = 0; count < allnum; count++)
    {
        if(pmsg->DivNum && (pmsg->DivNum == p[count].DataID))
        {
            if (p->function != NULL)
            {
                return (p->function((void *)pmsg->pData));
            }
        }
    }
    return FALSE;
}

/***********************************************************************************************
* Function      : CanSearchParaManagementTable
* Description   :  搜索表格返回位置值
* Input         : DataID---搜索的ID号
* Output        : 0xff-----表示没有搜索到
* Note(s)       :
* Contributor   : 23/5/2015 叶喜雨
***********************************************************************************************/
INT8U CanSearchParaManagementTable(INT8U gun,INT32U DataId)
{
	
    INT8U count;
    INT8U allnum = 0;
    const _PROTOCOL_CAN_MANAGE_TABLE *p;

	if(gun >= GUN_MAX)
	{
		return 0xff;
	}
    allnum = CANRECEIVETABLELEN;

    p = &Can_Data_Manage_Table[gun][0];

    for(count = 0; count < allnum; count++)
    {
        if(DataId && (DataId == p[count].PGNID))
        {
            return count;
        }
    }
    return 0xff;
}
/***********************************************************************************************
* Function      : ReciveBMSFrame
* Description   : 解析BMS发送过来的帧
* Input         : ExtenID---接收到的CAN扩展帧ID
                  pdata---CAN的数据内容
                  len---CAN的数据长度
* Output        :
* Note(s)       :
* Contributor   : 02/07/2015    叶喜雨
***********************************************************************************************/
void ReciveBMSFrameA(INT32U ExtenID, INT8U *pdata, INT8U len)
{
    CanInterfaceStruct  parastruct;

    INT32U ID1 = BMS_BMV_ID, ID2 = BMS_BMT_ID , ID3 = BMS_BSP_ID;
    INT32U RTSID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_236) ;//1C EC 56 F4
    //多页传输时的数据接收(TP.DT)的扩展帧
    INT32U pageframeID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_235);//多页传输的帧 1C EB 56 F4

	if (ExtenID == RTSID) //1C EC 56 F4
	{
		if (len < 8)
		{
			return;
		}
		if ( (pdata[0] == 0xff) || (pdata[3] == 0xff) \
		  || ( (pdata[1] == 0xff) && (pdata[2] == 0xff) ) )//取消先前多包
		{
			memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
	        memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
			return;
		}
	}
	#warning "多包接收做得兼容性好一点，   2020"
#if 0
    //分两种帧进行处理，单独的帧与RTS帧，这样好不好呢
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//奇瑞EQ车子需要下面两个清0操作
		memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //如果是RTS帧 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_A][0].DealwithDataIDFunction(&parastruct))//表示下一帧就是具体的多帧的第一帧了
        {
            CanFrameControl[GUN_A].state = 0x01;//表示要进入多帧接收了
            CanFrameControl[GUN_A].nextpagenum = parastruct.pdata[3];//后续有多少页,即总多包总包数
			return;//处理完了RTS即可退出了
        }
    }
    //根据多帧数据ID(1C EB 56 F4)来判断是否是多帧数据内容
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//非多帧传输的接收处理
    {//ExtenID不是EC，也不是EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_A) == BMS_GB_2015))
		{//20170118 荣威车型 屏幕BEM报文
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,ExtenID);//搜索ID号

        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //处理完单帧直接返回
            }
        }
    }
    }
    //多帧的接收，整理数据
    if((CanFrameControl[GUN_A].state) && (ExtenID == pageframeID))
    {//ExtenID为EB，即多包数据部分
        CanFrameControl[GUN_A].currentpagenum = pdata[0];//当前页码

        if((CanFrameControl[GUN_A].currentpagenum <= CanFrameControl[GUN_A].nextpagenum)
           && ((CanFrameControl[GUN_A].pagenum + 1) == CanFrameControl[GUN_A].currentpagenum) )
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//电池单体信息类数据丢弃
			{
                NOP();
			}
            else  //只拷贝有效的数据长度，不要第一个字节的长度信息，最后一帧可能存在的0xff暂时不做特殊处理
			{
                memcpy((RxBMSBufferA + (CanFrameControl[GUN_A].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            CanFrameControl[GUN_A].pagenum += 1;//实际获取到的页数
        }
        else //丢帧了重新接收，CanFrameControl可以不全部清0
        {
            CanFrameControl[GUN_A].state = 0;
        }
    }
    //多帧传输时，最后一帧接收完毕进入，实际处理函数
    if(CanFrameControl[GUN_A].state && (CanFrameControl[GUN_A].pagenum == CanFrameControl[GUN_A].nextpagenum))
    {
        INT32U pgn = 0, contextID = 0;

        //提取多帧的pgn号，在一开始的RTS中已经发送了
        pgn |=  BMS_RTS_FrameInf[GUN_A].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[0] << 16);
        //提取对应的多帧扩展ID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//多页传输的优先级都是7
        //先回复TP.CM_EndofMsgAck，表明多包消息已经全部接收并重组
        Reply_TPCM_EndofMsgAckFrameA();
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,contextID);//搜索ID号
        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferA;
            parastruct.len    = BMS_RTS_FrameInf[GUN_A].messagebytenum ;//RTS里的有效字节信息，实际上不用加1
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
				memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
                memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
            }
        }
    }
#else
	//分两种帧进行处理，单独的帧与RTS帧，这样好不好呢
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//奇瑞EQ车子需要下面两个清0操作
		memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
        memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); 
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //如果是RTS帧 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_A][0].DealwithDataIDFunction(&parastruct))//表示下一帧就是具体的多帧的第一帧了
        {
            //CanFrameControl.state = 0x01;//表示要进入多帧接收了
            CanFrameControl[GUN_A].nextpagenum = parastruct.pdata[3];//后续有多少页,即总多包总包数
			return;//处理完了RTS即可退出了
        }
    }
    //根据多帧数据ID(1C EB 56 F4)来判断是否是多帧数据内容
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//非多帧传输的接收处理
    {//ExtenID不是EC，也不是EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_A) == BMS_GB_2015))
		{//20170118 荣威车型 屏幕BEM报文
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,ExtenID);//搜索ID号

        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //处理完单帧直接返回
            }
        }
    }
    }
    //多帧的接收，整理数据
   // if((CanFrameControl.state) && (ExtenID == pageframeID))
	if(ExtenID == pageframeID)
    {//ExtenID为EB，即多包数据部分
        CanFrameControl[GUN_A].currentpagenum = pdata[0];//当前页码

//        if((CanFrameControl.currentpagenum <= CanFrameControl.nextpagenum)
//           && ((CanFrameControl.pagenum + 1) == CanFrameControl.currentpagenum) )
        if(CanFrameControl[GUN_A].currentpagenum <= CanFrameControl[GUN_A].nextpagenum)
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//电池单体信息类数据丢弃
			{
                NOP();
			}
            else  //只拷贝有效的数据长度，不要第一个字节的长度信息，最后一帧可能存在的0xff暂时不做特殊处理
			{
                memcpy((RxBMSBufferA + (CanFrameControl[GUN_A].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            //CanFrameControl.pagenum += 1;//实际获取到的页数
        }
//        else //丢帧了重新接收，CanFrameControl可以不全部清0
//        {
//            CanFrameControl.state = 0;
//        }
    }
    //多帧传输时，最后一帧接收完毕进入，实际处理函数
   // if(CanFrameControl.state && (CanFrameControl.pagenum == CanFrameControl.nextpagenum))
	if(CanFrameControl[GUN_A].currentpagenum == CanFrameControl[GUN_A].nextpagenum)	
    {
        INT32U pgn = 0, contextID = 0;
		CanFrameControl[GUN_A].currentpagenum = 0;				//多包确认只发一次  20231208
        //提取多帧的pgn号，在一开始的RTS中已经发送了
        pgn |=  BMS_RTS_FrameInf[GUN_A].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[0] << 16);
        //提取对应的多帧扩展ID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//多页传输的优先级都是7
        //先回复TP.CM_EndofMsgAck，表明多包消息已经全部接收并重组
        Reply_TPCM_EndofMsgAckFrameA();
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,contextID);//搜索ID号
        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferA;
            parastruct.len    = BMS_RTS_FrameInf[GUN_A].messagebytenum ;//RTS里的有效字节信息，实际上不用加1
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
//				memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
//                memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
            }
        }
    }
#endif
}


/***********************************************************************************************
* Function      : ReciveBMSFrame
* Description   : 解析BMS发送过来的帧
* Input         : ExtenID---接收到的CAN扩展帧ID
                  pdata---CAN的数据内容
                  len---CAN的数据长度
* Output        :
* Note(s)       :
* Contributor   : 02/07/2015    叶喜雨
***********************************************************************************************/
void ReciveBMSFrameB(INT32U ExtenID, INT8U *pdata, INT8U len)
{
    CanInterfaceStruct  parastruct;

    INT32U ID1 = BMS_BMV_ID, ID2 = BMS_BMT_ID , ID3 = BMS_BSP_ID;
    INT32U RTSID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_236) ;//1C EC 56 F4
    //多页传输时的数据接收(TP.DT)的扩展帧
    INT32U pageframeID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_235);//多页传输的帧 1C EB 56 F4

	if (ExtenID == RTSID) //1C EC 56 F4
	{
		if (len < 8)
		{
			return;
		}
		if ( (pdata[0] == 0xff) || (pdata[3] == 0xff) \
		  || ( (pdata[1] == 0xff) && (pdata[2] == 0xff) ) )//取消先前多包
		{
			memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
	        memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
			return;
		}
	}
	#warning "多包接收做得兼容性好一点，   2020"
#if 0
    //分两种帧进行处理，单独的帧与RTS帧，这样好不好呢
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//奇瑞EQ车子需要下面两个清0操作
		memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //如果是RTS帧 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_B][0].DealwithDataIDFunction(&parastruct))//表示下一帧就是具体的多帧的第一帧了
        {
            CanFrameControl[GUN_B].state = 0x01;//表示要进入多帧接收了
            CanFrameControl[GUN_B].nextpagenum = parastruct.pdata[3];//后续有多少页,即总多包总包数
			return;//处理完了RTS即可退出了
        }
    }
    //根据多帧数据ID(1C EB 56 F4)来判断是否是多帧数据内容
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//非多帧传输的接收处理
    {//ExtenID不是EC，也不是EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_B) == BMS_GB_2015))
		{//20170118 荣威车型 屏幕BEM报文
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,ExtenID);//搜索ID号

        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //处理完单帧直接返回
            }
        }
    }
    }
    //多帧的接收，整理数据
    if((CanFrameControl[GUN_B].state) && (ExtenID == pageframeID))
    {//ExtenID为EB，即多包数据部分
        CanFrameControl[GUN_B].currentpagenum = pdata[0];//当前页码

        if((CanFrameControl[GUN_B].currentpagenum <= CanFrameControl[GUN_B].nextpagenum)
           && ((CanFrameControl[GUN_B].pagenum + 1) == CanFrameControl[GUN_B].currentpagenum) )
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//电池单体信息类数据丢弃
			{
                NOP();
			}
            else  //只拷贝有效的数据长度，不要第一个字节的长度信息，最后一帧可能存在的0xff暂时不做特殊处理
			{
                memcpy((RxBMSBufferB + (CanFrameControl[GUN_B].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            CanFrameControl[GUN_B].pagenum += 1;//实际获取到的页数
        }
        else //丢帧了重新接收，CanFrameControl可以不全部清0
        {
            CanFrameControl[GUN_B].state = 0;
        }
    }
    //多帧传输时，最后一帧接收完毕进入，实际处理函数
    if(CanFrameControl[GUN_B].state && (CanFrameControl[GUN_B].pagenum == CanFrameControl[GUN_B].nextpagenum))
    {
        INT32U pgn = 0, contextID = 0;

        //提取多帧的pgn号，在一开始的RTS中已经发送了
        pgn |=  BMS_RTS_FrameInf[GUN_B].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[0] << 16);
        //提取对应的多帧扩展ID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//多页传输的优先级都是7
        //先回复TP.CM_EndofMsgAck，表明多包消息已经全部接收并重组
        Reply_TPCM_EndofMsgAckFrameB();
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,contextID);//搜索ID号
        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferB;
            parastruct.len    = BMS_RTS_FrameInf[GUN_B].messagebytenum ;//RTS里的有效字节信息，实际上不用加1
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
                memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
            }
        }
    }
#else
	//分两种帧进行处理，单独的帧与RTS帧，这样好不好呢
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//奇瑞EQ车子需要下面两个清0操作
		memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //如果是RTS帧 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_B][0].DealwithDataIDFunction(&parastruct))//表示下一帧就是具体的多帧的第一帧了
        {
            //CanFrameControl.state = 0x01;//表示要进入多帧接收了
            CanFrameControl[GUN_B].nextpagenum = parastruct.pdata[3];//后续有多少页,即总多包总包数
			return;//处理完了RTS即可退出了
        }
    }
    //根据多帧数据ID(1C EB 56 F4)来判断是否是多帧数据内容
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//非多帧传输的接收处理
    {//ExtenID不是EC，也不是EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_B) == BMS_GB_2015))
		{//20170118 荣威车型 屏幕BEM报文
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,ExtenID);//搜索ID号

        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //处理完单帧直接返回
            }
        }
        }
    }
    //多帧的接收，整理数据
   // if((CanFrameControl.state) && (ExtenID == pageframeID))
	if(ExtenID == pageframeID)
    {//ExtenID为EB，即多包数据部分
        CanFrameControl[GUN_B].currentpagenum = pdata[0];//当前页码

//        if((CanFrameControl.currentpagenum <= CanFrameControl.nextpagenum)
//           && ((CanFrameControl.pagenum + 1) == CanFrameControl.currentpagenum) )
        if(CanFrameControl[GUN_B].currentpagenum <= CanFrameControl[GUN_B].nextpagenum)
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//电池单体信息类数据丢弃
			{
                NOP();
			}
            else  //只拷贝有效的数据长度，不要第一个字节的长度信息，最后一帧可能存在的0xff暂时不做特殊处理
			{
                memcpy((RxBMSBufferB + (CanFrameControl[GUN_B].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            //CanFrameControl.pagenum += 1;//实际获取到的页数
        }
//        else //丢帧了重新接收，CanFrameControl可以不全部清0
//        {
//            CanFrameControl.state = 0;
//        }
    }
    //多帧传输时，最后一帧接收完毕进入，实际处理函数
   // if(CanFrameControl.state && (CanFrameControl.pagenum == CanFrameControl.nextpagenum))
	if(CanFrameControl[GUN_B].currentpagenum == CanFrameControl[GUN_B].nextpagenum)	
    {
        INT32U pgn = 0, contextID = 0;
		CanFrameControl[GUN_B].currentpagenum = 0;				//多包确认只发一次  20231208
        //提取多帧的pgn号，在一开始的RTS中已经发送了
        pgn |=  BMS_RTS_FrameInf[GUN_B].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[0] << 16);
        //提取对应的多帧扩展ID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//多页传输的优先级都是7
        //先回复TP.CM_EndofMsgAck，表明多包消息已经全部接收并重组
        Reply_TPCM_EndofMsgAckFrameB();
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,contextID);//搜索ID号
        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferB;
            parastruct.len    = BMS_RTS_FrameInf[GUN_B].messagebytenum ;//RTS里的有效字节信息，实际上不用加1
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
//               memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
//                memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //一轮RTS结束，清除
            }
        }
    }
#endif
}

/******************************************************************************
* Function Name :WaitConnectBMSmessage
* Description   : 等到消息的处理函数，该任务所有的等待消息机制都用此函数处理
* Input         : time, 等待的超时时间，填写1ms的倍数。
* Output        :
* Contributor       :
* Date First Issued :25/09/2008
******************************************************************************/
_APP_CONNECT_BMSSTATE WaitConnectBMSmessageA(INT32U time)
{
    INT8U err = 0;
    INT8U offset = 0;

    _BSP_MESSAGE *pMSG;   // 定义消息指针

    pMSG = OSQPend(TaskConnectBMS_peventA, time, &err);
    if(err == OS_ERR_NONE)
    {
        switch(pMSG->MsgID)
        {
            case BSP_MSGID_CAN_RXOVER:  // 接收完成
				{
					offset = pMSG->DivNum;
					ReciveBMSFrameA(bxBMSCANSetA.RxMessage[offset].ExtId, \
								   bxBMSCANSetA.RxMessage[offset].Data, \
								   bxBMSCANSetA.RxMessage[offset].DLC);
					return APP_RXDEAL_SUCCESS;
				}
            case BSP_MSGID_CONTROL://收到控制任务的消息
                if (APP_DealReceiveDataA(pMSG) == FALSE)
                {
                    return APP_DEFAULT;
                }
                else
                {
                    return APP_END_CHARGING;
                }
            default:
                return APP_TIMEOUT ;
        }
    }
    return APP_TIMEOUT;
}

/******************************************************************************
* Function Name :WaitConnectBMSmessage
* Description   : 等到消息的处理函数，该任务所有的等待消息机制都用此函数处理
* Input         : time, 等待的超时时间，填写1ms的倍数。
* Output        :
* Contributor       :
* Date First Issued :25/09/2008
******************************************************************************/
_APP_CONNECT_BMSSTATE WaitConnectBMSmessageB(INT32U time)
{
    INT8U err = 0;
    INT8U offset = 0;

    _BSP_MESSAGE *pMSG;   // 定义消息指针

    pMSG = OSQPend(TaskConnectBMS_peventB, time, &err);
    if(err == OS_ERR_NONE)
    {
        switch(pMSG->MsgID)
        {
            case BSP_MSGID_CAN_RXOVER:  // 接收完成
				{
					offset = pMSG->DivNum;
					ReciveBMSFrameB(bxBMSCANSetB.RxMessage[offset].ExtId, \
								   bxBMSCANSetB.RxMessage[offset].Data, \
								   bxBMSCANSetB.RxMessage[offset].DLC);
					return APP_RXDEAL_SUCCESS;
				}
            case BSP_MSGID_CONTROL://收到控制任务的消息
                if (APP_DealReceiveDataB(pMSG) == FALSE)
                {
                    return APP_DEFAULT;
                }
                else
                {
                    return APP_END_CHARGING;
                }
            default:
                return APP_TIMEOUT ;
        }
    }
    return APP_TIMEOUT;
}

/***********************************************************************************************
* Function      : CheckHWConnectforBMS
* Description   : 监测物理连接BMS是否成功；成功后进入充电握手阶段。
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
INT8U CheckHWConnectforBMS(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    if  (SysState[gun].SelfCheckState.NoErr == STATE_NO_ERR) 
    {//自检成功
        return TRUE;
    }
    else
    {//自检失败
        SetBMSSelfCheck(gun,TRUE); //需要插拔枪重新自检
        SetBMSInsulationCheck(gun,TRUE); //需要再次进行绝缘检测
        return FALSE;
    }
}

/*****************************************************************************
* Function      : ChargeErrMethodA
* Description   : 故障处理方式A(充电机立即停机停用，等待专业维护人员维修)
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月2日  叶喜雨
*****************************************************************************/
static INT8U ChargeErrMethodA(INT8U gun)
{
	static _BSP_MESSAGE senderr_message[GUN_MAX];
	
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	//充电时序结束，充电故障级别3
	senderr_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	senderr_message[gun].DivNum = APP_BMS_CHARGE_ERR_A;	 //不管具体原因直接发送停止充电一次以复位系统
	senderr_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &senderr_message[gun]);	//给控制任务发送消息
	}
	else
	{
		OSQPost(Control_PeventB, &senderr_message[gun]);	//给控制任务发送消息
	}
	OSTimeDly(SYS_DELAY_20ms);
    return TRUE;
}

/*****************************************************************************
* Function      : ChargeErrMethodB
* Description   : 故障处理方式B(停止本次充电，保存交易记录，需重新插拔枪后，
                  才可以进行下一次充电)
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月2日  叶喜雨
*****************************************************************************/
static INT8U ChargeErrMethodB(INT8U gun)
{
	static _BSP_MESSAGE senderr_message[GUN_MAX];

	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    //非车载充电机控制装置发生3次通讯超时即确认通讯中断，
    //则非车载充电机停止充电，应在10s内断开K1、K2、K3、K4、K5、K6
	if((BMS_CEM_Context[gun].State0.AllBits != 0xFC) || (BMS_CEM_Context[gun].State1.AllBits != 0xF0) || \
		(BMS_CEM_Context[gun].State2.AllBits != 0xC0) || (BMS_CEM_Context[gun].State3.AllBits != 0xFC))
	{
		//数据有超时才发CEM
		BMS_CONNECT_StepSet(gun,BMS_CEM_SEND);    //发送CEM
	}
	SetBMSErrState(gun,ERR_STATE_METHOD_B);   //设置处理B故障状态
	
	//充电时序结束，充电故障级别3
	senderr_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	senderr_message[gun].DivNum = APP_BMS_CHARGE_ERR_B;	 //不管具体原因直接发送停止充电一次以复位系统
	senderr_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &senderr_message[gun]);	//给控制任务发送消息
	}
	else
	{
		OSQPost(Control_PeventB, &senderr_message[gun]);	//给控制任务发送消息
	}
	OSTimeDly(SYS_DELAY_20ms);
	printf("GUN = %d,Error B\r\n",gun);
    return TRUE;
}

/*****************************************************************************
* Function      : ChargeErrMethodC
* Description   : 故障处理方式C，中止充电，待故障现象排除后自动恢复充电，检
                  测到故障状态接触后，需要通信握手开始充电
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月2日  叶喜雨
*****************************************************************************/
static INT8U ChargeErrMethodC(INT8U gun)
{
	static _BSP_MESSAGE senderr_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    //(1)发送充电机错误报文CEM跟CST给BMS
	BMS_CONNECT_StepSet(gun,BMS_CEM_SEND); //发送CEM即可，不需要发送CST
    SetBMSErrState(gun,ERR_STATE_METHOD_C);             //设置处理C故障
    //根据GB/T 18487.1中B.3.7第39页规定，
    //在充电过程中，非车载充电机控制装置如发生通讯超时，则非车载充电机停止充电，
    //应在10s内断开K1、K2、K5、K6;

	//充电控制任务要进入C故障处理方式
	senderr_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	senderr_message[gun].DivNum = APP_BMS_CHARGE_ERR_C;	 //错误C处理方式
	senderr_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &senderr_message[gun]);	//给控制任务发送消息
	}
	else
	{
		OSQPost(Control_PeventB, &senderr_message[gun]);	//给控制任务发送消息
	}
	OSTimeDly(SYS_DELAY_20ms);
	printf("GUN = %d,Error C\r\n",gun);
    return TRUE;
}

INT8U ChargeErrHandleA(_BMS_ERR_MOTHOD method)
{
    switch (method)
    {
        case ERR_METHOD_A:
			SetBMSErrState(GUN_A,ERR_STATE_METHOD_A);             //处理A故障状态
            ChargeErrMethodA(GUN_A);
            break;
        case ERR_METHOD_B: //不再进行重试，需要插拔枪才进行重连
            ChargeErrMethodB(GUN_A);
            break;
        case ERR_METHOD_C: //重试3次变成B
        case ERR_METHOD_BMS: //收到BMS发送的BEM
            //根据GB/T 18487.1中B.3.7第39页规定，
            //在充电过程中，非车载充电机控制装置如发生通讯超时，则非车载充电机停止充电，
            //应在10s内断开K1、K2、K5、K6;
            ChargeErrMethodC(GUN_A);
            //参考GB/T 17930-2015第36页
            break;
        case ERR_METHOD_END:
            SetBMSErrMethodRetry(GUN_A,ERR_RETRY_CLEAR);    //直接结束的故障不再重连3次
            ChargeErrMethodB(GUN_A);
            break;
        default:
            break;
    }
    return TRUE;
}

INT8U ChargeErrHandleB(_BMS_ERR_MOTHOD method)
{
    switch (method)
    {
        case ERR_METHOD_A:
			SetBMSErrState(GUN_B,ERR_STATE_METHOD_A);             //处理A故障状态
            ChargeErrMethodA(GUN_B);
            break;
        case ERR_METHOD_B: //不再进行重试，需要插拔枪才进行重连
            ChargeErrMethodB(GUN_B);
            break;
        case ERR_METHOD_C: //重试3次变成B
        case ERR_METHOD_BMS: //收到BMS发送的BEM
            //根据GB/T 18487.1中B.3.7第39页规定，
            //在充电过程中，非车载充电机控制装置如发生通讯超时，则非车载充电机停止充电，
            //应在10s内断开K1、K2、K5、K6;
            ChargeErrMethodC(GUN_B);
            //参考GB/T 17930-2015第36页
            break;
        case ERR_METHOD_END:
            SetBMSErrMethodRetry(GUN_B,ERR_RETRY_CLEAR);    //直接结束的故障不再重连3次
            ChargeErrMethodB(GUN_B);
            break;
        default:
            break;
    }
    return TRUE;
}


/*****************************************************************************
* Function      : ChargeErrDeal
* Description   : 查表处理充电故障函数
* Input         : _BMS_ERR_TYPE type
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月2日  叶喜雨
*****************************************************************************/
INT8U ChargeErrDeal(INT8U gun ,_BMS_ERR_TYPE type)
{
    INT8U i;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    const _BMS_ERR_HANDLE* p = &BMSErrHandleTable[gun][0];

    for (i = 0; i < BMS_ERR_HANDLE_LEN; i++)
    {
        if (p[i].type == type)
        {
            if (p[i].fun != NULL)
            {
                return (p[i].fun(p[i].method) );
            }
        }
    }
    return FALSE;
}


/*****************************************************************************
* Function      : Dispose_TimeoutErrB
* Description   : 超时进入故障B具体原因处理
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
static void Dispose_TimeoutErrB(_GUN_NUM gun)
{

	if(gun >= GUN_MAX)
	{
		return;
	}
	if(BMS_CEM_Context[gun].State0.OneByte.BRMBits == 0x01)
	{
		SetStartFailType(gun,ENDFAIL_BRMTIMEOUT);
	}
	if( BMS_CEM_Context[gun].State1.OneByte.BCPBits == 0x01)
	{
		SetStartFailType(gun,ENDFAIL_BCPTIMEOUT);
	}
	if(BMS_CEM_Context[gun].State1.OneByte.BROBits == 0x01)
	{
		//目前没有区分00超时 还是AA超时
		SetStartFailType(gun,ENDFAIL_BRORUNTIMEOUT);
	}
	if(BMS_CEM_Context[gun].State2.OneByte.BCSBits == 0x01)
	{
		APP_Set_ERR_Branch(gun,STOP_BCSTIMTOUT);
	}
	if(BMS_CEM_Context[gun].State2.OneByte.BCLBits == 0x01)
	{
		APP_Set_ERR_Branch(gun,STOP_BCLTIMTOUT);
	}
	if(APP_Get_ERR_Branch(gun) == STOP_OTHERERR)  //此处还是其他故障（默认未其他故障），肯定有问题
	{
		APP_Set_ERR_Branch(gun,STOP_CCUBSMERR);
	}


}
/*****************************************************************************
* Function      : ChargeErrSwitch
* Description   : 错误故障方式切换
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void ChargeErrSwitch(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    if (APP_GetGBType(gun) == BMS_GB_2015)  //新国标 
    {
        _BMS_ERR_STATE state;

		SetDealFaultOverState(gun,FALSE);
		state = GetBMSErrState(gun);//判断是否是故障C或者收到BEM错误报文
		if ( (state == ERR_STATE_METHOD_C) || (state == ERR_STATE_BMS) )
		{//是故障C，判断是否超时3次了
			if (SetBMSErrMethodRetry(gun,ERR_RETRY_DEC) == 0)
			{//3次超时，进入故障B处理
				SetBMSConnectStep(gun,BMS_CONNECT_END);
				//CEM细分
				Dispose_TimeoutErrB(gun);		//故障细分
				ChargeErrDeal(gun,ERR_LF_METHOD_B);
				OSTimeDly(SYS_DELAY_1s);
			}
			else
			{
				ChargeErrDeal(gun,ERR_LF_METHOD_C);
			}
		}
		else 
		{//不是故障C
			ChargeErrDeal(gun,ERR_LF_METHOD_C);//第一次出现超时，进入故障C超时处理
        }
    }
    else
    {
        static _BSP_MESSAGE send_message[GUN_MAX];
        SetDealFaultOverState(gun,FALSE);
        BMS_CONNECT_StepSet(gun,BMS_CEM_SEND); //发送CEM即可
		printf("GB2011 Into CEM\r\n");
        OSTimeDly(SYS_DELAY_1s);
		Dispose_TimeoutErrB(gun);    //主要进行故障细分
        send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message[gun].DivNum = APP_CHARGE_END;  //停止电力输出
		NB_WriterReason(gun,"E6",2);
        send_message[gun].GunNum = gun;
        if(gun == GUN_A)
		{
			OSQPost(Control_PeventA, &send_message[gun]);	//给控制任务发送消息
		}
		else
		{
			OSQPost(Control_PeventB, &send_message[gun]);	//给控制任务发送消息
		}
        OSTimeDly(SYS_DELAY_10ms);
		printf("CEM Send\r\n");
    }
    BMS_TIMEOUT_ENTER(gun,BMS_FAULT_SETP, SYS_DELAY_30s); //进入了错误故障的超时判断
    while (GetDealFaultOverState(gun) == FALSE)
    {
        OSTimeDly(SYS_DELAY_20ms);
        if (BMS_TIMEOUT_CHECK(gun,BMS_FAULT_SETP, SYS_DELAY_30s) == BMS_TIME_OUT)
        {
            break;
        }
    }
    SetDealFaultOverState(gun,FALSE);
}

/*****************************************************************************
* Function      : ChargeBMSSelfCheck
* Description   : 自检函数，每隔1秒会检测一次
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U ChargeBMSSelfCheck(INT8U gun)
{
    static _BSP_MESSAGE send_messagedis[GUN_MAX];
	 static _BSP_MESSAGE send_message[GUN_MAX];
	OS_EVENT* pdisevent =  APP_GetDispEvent();
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	// OSTimeDly(SYS_DELAY_100ms); //延时检测
    //判断是否需要自检
    if (GetBMSSelfCheck(gun) == TRUE) 
    {//需要自检
        if (GetGunState(gun) == GUN_CONNECTED) 
        {//检测点1为4V，说明枪已经完全连接上
            //把硬件操作全部留给控制任务去实现，通过消息传递的方式是否有必要
            //暂时不使用消息带数据的方式去锁定/解锁
			//if((BCU_LOCK.LockCMD==ELEC_LOCKED)&&(GetChargeMode() == MODE_AUTO))
			if((GetChargeMode(gun) == MODE_AUTO) || (GetChargeMode(gun) == MODE_VIN))
			{//收到充电指令，并且收到电子锁锁定命令
				BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
				while(1)
				{
					if (GetElecLockState(gun) == ELEC_LOCK_UNLOCKED) //未锁定
					{//5秒超时
						OSTimeDly(SYS_DELAY_250ms);
						if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s) == BMS_TIME_OUT)
						{
							send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
							NB_WriterReason(gun,"F29",3);
							send_message[gun].DivNum = APP_CHARGE_FAIL;		//不管具体原因直接发送停止充电一次以复位系统
							send_message[gun].GunNum = gun;
							if(gun == GUN_A)
							{
								OSQPost(Control_PeventA, &send_message[gun]);	//给控制任务发送消息
							}
							else
							{
								OSQPost(Control_PeventB, &send_message[gun]);	//给控制任务发送消息
							}
							OSTimeDly(SYS_DELAY_50ms);
							SetStartFailType(gun,ENDFAIL_ELECLOCKERR);
							SysState[gun].SelfCheckState.NoErr = STATE_ERR;
							
							send_messagedis[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
							NB_WriterReason(gun,"F28",3);
							send_messagedis[gun].DivNum = APP_CHARGE_FAIL; //启动失败
							send_messagedis[gun].GunNum = gun;
							OSQPost(pdisevent, &send_messagedis[gun]);
							OSTimeDly(SYS_DELAY_10ms);
							printf("2111\r\n");
							//电子锁故障
							SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 1, 1);
							break;
						}
					}
					else
					{
						break;
					}
				}
			}
            //判断电子锁是否就绪
            return ( (GetElecLockState(gun) == ELEC_LOCK_UNLOCKED) ? FALSE : TRUE);
        }
        else //枪未连接上
        {
            return FALSE;
        }
    }
    else //不需要自检
    {
        return TRUE;
    }
}
/***********************************************************************************************
* Function      : ChargeBMSHandshakeA
* Description   : 充电握手阶段，流程参考国标图A.1
* Input         :
* Output        :
* Note(s)       : CRM报文，PGN=256 P=6
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeBMSHandshakeA(void)
{
    static _BSP_MESSAGE send_message9, send_message10;
	OS_EVENT* pvinevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

	SetBMSConnectStep(GUN_A,BMS_CONNECT_HANDSHARE);
//#if (BMS_CUR_GB == BMS_NEW_GB2015)  //新国标 
	//CHM的作用主要是1、区分新旧国标2、绝缘检测需要根据BHM选择输出电压
	//如果绝缘检测不需要做，CHM也就不需要发
	SetBMSInsulationCheck(GUN_A,TRUE);//此语句新国标本身是不需要的，纯粹为了南京测试使用
	if (GetBMSInsulationCheck(GUN_A) == TRUE) //需要绝缘监测
	{
    	BMS_TIMEOUT_ENTER(GUN_A,BMS_BHM_STEP, SYS_DELAY_5s);
    	//处理错误故障C时，直接从握手辨识阶段开始
    	if (GetBMSErrState(GUN_A) != ERR_STATE_METHOD_C)
	    {//设置步骤为，进入握手CHM发送STEP
	        BMS_CONNECT_StepSet(GUN_A,BMS_CHM_SEND); //发送CHM给BMS
	        while(1)
	        {
				if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE)
				{//如果在执行中发生了系统故障，就跳出
					return FALSE;
				}
	            //等待接收250ms
	            result = WaitConnectBMSmessageA(SYS_DELAY_250ms);
	            if(result == APP_RXDEAL_SUCCESS)//接收到数据
	            {
	                if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BHM_SUCCESS)//BHM已经全部接收
	                {
	                    APP_SetGBType(GUN_A,BMS_GB_2015);
						printf("SETGB2015\r\n");
                        break;//跳出当前循环
	                }
	            }
	            //CHM超时后，不需要做超时处理，直接进入握手识别阶段
	            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BHM_STEP, SYS_DELAY_5s) )
	            {
	                APP_SetGBType(GUN_A,BMS_GB_2011);
					printf("SETGB2011\r\n");
                    break;
	            }
	        }
			
//			APP_SetGBType(BMS_GB_2011);//20161004 新旧国标 Test
            if(APP_GetGBType(GUN_A) == BMS_GB_2015)
            {
                //测试发现在某些情况会出现此标志会是FALSE
				printf("GB2015\r\n");
                SetBMSInsulationCheck(GUN_A,TRUE);
                //开始绝缘检测，发送消息给控制任务
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_A;
                OSQPost(Control_PeventA, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_A)) //等待绝缘检测完成
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //绝缘检测正常5秒能完成，这边放宽条件为10秒
                    if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //绝缘检测失败
                        SetBcuRemoteSignalState(GUN_A,(INT8U)REMOTE_SIGNALE_2, 4, 1);
              
                        //充电失败，绝缘检测失败
                        SysState[GUN_A].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_A].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_A,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                        send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                        send_message10.GunNum = GUN_A;
                        send_message10.pData = (INT8U *)&SysState[GUN_A].SelfCheckState.NoErr;
                        OSQPost(Control_PeventA, &send_message10);
				//		NB_WriterReason(GUN_A,"F28",3);
                        OSTimeDly(SYS_DELAY_100ms);
                        SetBMSStartCharge(GUN_A,FALSE);
						printf("SelfCheck Timeout 2015\r\n");
                        return FALSE; 
                    }
                }
                //判断绝缘检测是否成功
				if (CheckHWConnectforBMS(GUN_A) == FALSE)
                {
                    //绝缘检测失败
                    //SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
					//SetStartFailType(ENDFAIL_INSOLUTIONERR);
                    //SysState.SelfCheckState.Insulation = STATE_ERR;
                    //SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                    send_message10.GunNum = GUN_A;
                    send_message10.pData = (INT8U *)&SysState[GUN_A].SelfCheckState.NoErr;
                    OSQPost(Control_PeventA, &send_message10);
                    OSTimeDly(SYS_DELAY_100ms);
                    SetBMSStartCharge(GUN_A,FALSE);
					printf("SelfCheck Fail 2015\r\n");
                    return FALSE;
                }
            }
            else//2011GB
            {//开始绝缘检测，发送消息给控制任务
//				BSP_RLCClose(BSPRLC_BMS_POWER);
				printf("GB2011\r\n");
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_A;
                OSQPost(Control_PeventA, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_A)) //等待绝缘检测完成
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //绝缘检测正常5秒能完成，这边放宽条件为10秒
                    if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //绝缘检测失败
                        SetBcuRemoteSignalState(GUN_A,(INT8U)REMOTE_SIGNALE_2, 4, 1);
                        //充电失败，绝缘检测失败
                        SysState[GUN_A].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_A].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_A,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                        send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                        send_message10.GunNum = GUN_A;
                        send_message10.pData = (INT8U *)&SysState[GUN_A].SelfCheckState.NoErr;
                        OSQPost(Control_PeventA, &send_message10);
						NB_WriterReason(GUN_A,"F27",3);
                        OSTimeDly(SYS_DELAY_100ms);
                        SetBMSStartCharge(GUN_A,FALSE);
						printf("SelfCheck Timeout 2011\r\n");
                        return FALSE;
                    }
                }
                //判断绝缘检测是否成功
                if (CheckHWConnectforBMS(GUN_A) == FALSE)
                {
//                    //绝缘检测失败
//                  SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
//                   
//					SetStartFailType(ENDFAIL_INSOLUTIONERR);
//                  SysState.SelfCheckState.Insulation = STATE_ERR;
//                  SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                    send_message10.GunNum = GUN_A;
                    send_message10.pData = (INT8U *)&SysState[GUN_A].SelfCheckState.NoErr;
                    OSQPost(Control_PeventA, &send_message10);
                    OSTimeDly(SYS_DELAY_100ms);
                    SetBMSStartCharge(GUN_A,FALSE);
					printf("SelfCheck Fail 2011\r\n");
                    return FALSE;
                }
            }
	    }
	}
	//BSP_RLCOpen(BSPRLC_BMS_POWER);;
    CRMReadyflag[GUN_A] = 0;
    result = APP_DEFAULT;
    BMS_CONNECT_StepSet(GUN_A,BMS_CRM_SEND);//发送CRM帧
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BRM_STEP, SYS_DELAY_5s);
	//CEM报文全部正常状态初始化
	SetCEMState(GUN_A,CEM_ALL, ERR_TYPE_NORMAL);
    //收到RTS帧，并回复RTS帧后就进入了等待接收辨识报文的过程，
    while(1)
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE )
		{//如果在执行中发生了系统故障，就跳出
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
        result = WaitConnectBMSmessageA(SYS_DELAY_10ms);//多报文按10ms间隔过来
        if(result == APP_RXDEAL_SUCCESS)//接收到数据
        {
            if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BRM_SUCCESS)//BRM已经全部接收
            {
                BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;
                result = APP_DEFAULT;
				CRMReadyflag[GUN_A] = 1;
				//设置CEM状态为正常
				SetCEMState(GUN_A,CEM_BRM, ERR_TYPE_NORMAL);
                break;//跳出当前循环
            }
            //判断BRM接收是否超时，需要在CRM发送后5秒内接收到
            else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BRM_STEP, SYS_DELAY_5s))
            {
                BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;
                //设置接收BCP超时状态
                SetCEMState(GUN_A,CEM_BRM, ERR_TYPE_TIMEOUT);
                //进入超时机制
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E45",3);
				printf("GUN_A 1\r\n");
                CRMReadyflag[GUN_A] = 0;
                return FALSE;
            }
        }
        //判断BRM接收是否超时，需要在CRM发送后5秒内接收到
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BRM_STEP, SYS_DELAY_5s))
        {
        	//设置接收BCP超时状态
            SetCEMState(GUN_A,CEM_BRM, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E44",3);
			printf("GUN_A 2\r\n");
			CRMReadyflag[GUN_A] = 0;
            return FALSE;
        }
    }
	
	if(GetChargeMode(GUN_A) == MODE_VIN)
	{

		SendMsg.MsgID = BSP_MSGID_DISP;
		SendMsg.DivNum = APP_VIN_INFO;
		SendMsg.DataLen =GUN_A;
		OSQPost(pvinevent, &SendMsg);
	}
    return TRUE;
}


/***********************************************************************************************
* Function      : ChargeBMSHandshakeA
* Description   : 充电握手阶段，流程参考国标图A.1
* Input         :
* Output        :
* Note(s)       : CRM报文，PGN=256 P=6
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeBMSHandshakeB(void)
{
 static _BSP_MESSAGE send_message9, send_message10;
	OS_EVENT* pvinevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

	SetBMSConnectStep(GUN_B,BMS_CONNECT_HANDSHARE);
//#if (BMS_CUR_GB == BMS_NEW_GB2015)  //新国标 
	//CHM的作用主要是1、区分新旧国标2、绝缘检测需要根据BHM选择输出电压
	//如果绝缘检测不需要做，CHM也就不需要发
	SetBMSInsulationCheck(GUN_B,TRUE);//此语句新国标本身是不需要的，纯粹为了南京测试使用
	if (GetBMSInsulationCheck(GUN_B) == TRUE) //需要绝缘监测
	{
    	BMS_TIMEOUT_ENTER(GUN_B,BMS_BHM_STEP, SYS_DELAY_5s);
    	//处理错误故障C时，直接从握手辨识阶段开始
    	if (GetBMSErrState(GUN_B) != ERR_STATE_METHOD_C)
	    {//设置步骤为，进入握手CHM发送STEP
	        BMS_CONNECT_StepSet(GUN_B,BMS_CHM_SEND); //发送CHM给BMS
	        while(1)
	        {
				if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE)
				{//如果在执行中发生了系统故障，就跳出
					return FALSE;
				}
	            //等待接收250ms
	            result = WaitConnectBMSmessageB(SYS_DELAY_250ms);
	            if(result == APP_RXDEAL_SUCCESS)//接收到数据
	            {
	                if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BHM_SUCCESS)//BHM已经全部接收
	                {
	                    APP_SetGBType(GUN_B,BMS_GB_2015);
						printf("SETGB2015\r\n");
                        break;//跳出当前循环
	                }
	            }
	            //CHM超时后，不需要做超时处理，直接进入握手识别阶段
	            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BHM_STEP, SYS_DELAY_5s) )
	            {
	                APP_SetGBType(GUN_B,BMS_GB_2011);
					printf("SETGB2011\r\n");
                    break;
	            }
	        }
			
//			APP_SetGBType(BMS_GB_2011);//20161004 新旧国标 Test
            if(APP_GetGBType(GUN_B) == BMS_GB_2015)
            {
                //测试发现在某些情况会出现此标志会是FALSE
				printf("GB2015\r\n");
                SetBMSInsulationCheck(GUN_B,TRUE);
                //开始绝缘检测，发送消息给控制任务
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_B;
                OSQPost(Control_PeventB, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_B)) //等待绝缘检测完成
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //绝缘检测正常5秒能完成，这边放宽条件为10秒
                    if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //绝缘检测失败
                        SetBcuRemoteSignalState(GUN_B,(INT8U)REMOTE_SIGNALE_2, 4, 1);
              
                        //充电失败，绝缘检测失败
                        SysState[GUN_B].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_B].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_B,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                        send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                        send_message10.GunNum = GUN_B;
                        send_message10.pData = (INT8U *)&SysState[GUN_B].SelfCheckState.NoErr;
                        OSQPost(Control_PeventB, &send_message10);
						NB_WriterReason(GUN_B,"F16",3);
                        OSTimeDly(SYS_DELAY_100ms);
                        SetBMSStartCharge(GUN_B,FALSE);
						printf("SelfCheck Timeout 2015\r\n");
                        return FALSE; 
                    }
                }
                //判断绝缘检测是否成功
				if (CheckHWConnectforBMS(GUN_B) == FALSE)   //内部调试代码其他地方写
                {
                    //绝缘检测失败
                    //SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
					//SetStartFailType(ENDFAIL_INSOLUTIONERR);
                    //SysState.SelfCheckState.Insulation = STATE_ERR;
                    //SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                    send_message10.GunNum = GUN_B;
                    send_message10.pData = (INT8U *)&SysState[GUN_B].SelfCheckState.NoErr;
                    OSQPost(Control_PeventB, &send_message10);
                    OSTimeDly(SYS_DELAY_100ms);
                    SetBMSStartCharge(GUN_B,FALSE);
					printf("SelfCheck Fail 2015\r\n");
                    return FALSE;
                }
            }
            else//2011GB
            {//开始绝缘检测，发送消息给控制任务
//				BSP_RLCClose(BSPRLC_BMS_POWER);
				printf("GB2011\r\n");
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_B;
                OSQPost(Control_PeventB, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_B)) //等待绝缘检测完成
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //绝缘检测正常5秒能完成，这边放宽条件为10秒
                    if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //绝缘检测失败
                        SetBcuRemoteSignalState(GUN_B,(INT8U)REMOTE_SIGNALE_2, 4, 1);
                        //充电失败，绝缘检测失败
                        SysState[GUN_B].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_B].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_B,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
						NB_WriterReason(GUN_B,"F5",2);
                        send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                        send_message10.GunNum = GUN_B;
                        send_message10.pData = (INT8U *)&SysState[GUN_B].SelfCheckState.NoErr;
                        OSQPost(Control_PeventB, &send_message10);
                        OSTimeDly(SYS_DELAY_100ms);
                        SetBMSStartCharge(GUN_B,FALSE);
						printf("SelfCheck Timeout 2011\r\n");
                        return FALSE;
                    }
                }
                //判断绝缘检测是否成功
                if (CheckHWConnectforBMS(GUN_B) == FALSE)    //内部调试代码其他地方写
                {
//                    //绝缘检测失败
//                  SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
//                   
//					SetStartFailType(ENDFAIL_INSOLUTIONERR);
//                  SysState.SelfCheckState.Insulation = STATE_ERR;
//                  SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //启动失败
                    send_message10.GunNum = GUN_B;
                    send_message10.pData = (INT8U *)&SysState[GUN_B].SelfCheckState.NoErr;
                    OSQPost(Control_PeventB, &send_message10);
                    OSTimeDly(SYS_DELAY_100ms);
                    SetBMSStartCharge(GUN_B,FALSE);
					printf("SelfCheck Fail 2011\r\n");
                    return FALSE;
                }
            }
	    }
	}
	//BSP_RLCOpen(BSPRLC_BMS_POWER);;
    CRMReadyflag[GUN_B] = 0;
    result = APP_DEFAULT;
    BMS_CONNECT_StepSet(GUN_B,BMS_CRM_SEND);//发送CRM帧
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BRM_STEP, SYS_DELAY_5s);
	//CEM报文全部正常状态初始化
	SetCEMState(GUN_B,CEM_ALL, ERR_TYPE_NORMAL);
    //收到RTS帧，并回复RTS帧后就进入了等待接收辨识报文的过程，
    while(1)
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE )
		{//如果在执行中发生了系统故障，就跳出
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
        result = WaitConnectBMSmessageB(SYS_DELAY_10ms);//多报文按10ms间隔过来
        if(result == APP_RXDEAL_SUCCESS)//接收到数据
        {
            if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BRM_SUCCESS)//BRM已经全部接收
            {
                BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;
                result = APP_DEFAULT;
				CRMReadyflag[GUN_B] = 1;
				//设置CEM状态为正常
				SetCEMState(GUN_B,CEM_BRM, ERR_TYPE_NORMAL);
                break;//跳出当前循环
            }
            //判断BRM接收是否超时，需要在CRM发送后5秒内接收到
            else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BRM_STEP, SYS_DELAY_5s))
            {
                BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;
                //设置接收BCP超时状态
                SetCEMState(GUN_B,CEM_BRM, ERR_TYPE_TIMEOUT);
                //进入超时机制
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E43",3);
				printf("GUN_B 1\r\n");
                CRMReadyflag[GUN_B] = 0;
                return FALSE;
            }
        }
        //判断BRM接收是否超时，需要在CRM发送后5秒内接收到
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BRM_STEP, SYS_DELAY_5s))
        {
        	//设置接收BCP超时状态
            SetCEMState(GUN_B,CEM_BRM, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E42",3);
			printf("GUN_B 2\r\n");
			CRMReadyflag[GUN_B] = 0;
            return FALSE;
        }
    }
	
	if(GetChargeMode(GUN_B) == MODE_VIN)
	{

		SendMsg.MsgID = BSP_MSGID_DISP;
		SendMsg.DivNum = APP_VIN_INFO;
		SendMsg.DataLen =GUN_B;
		OSQPost(pvinevent, &SendMsg);
	}
    return TRUE;
}
/***********************************************************************************************
* Function      : ChargeParameterConfigStep
* Description   : 充电参数配置阶段
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeParameterConfigStepA(void)
{
    static _BSP_MESSAGE send_message29;
    INT8U firsttime = 0;
	INT8U count;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

	SetBMSConnectStep(GUN_A,BMS_CONNECT_PARACONFIG);
    //等待接收BMS发送的BCP报文
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BCP_STEP, SYS_DELAY_5s); //进入了BCP的超时判断
    while(1)
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE )
		{//如果在执行中发生了系统故障，就跳出
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
        result = WaitConnectBMSmessageA(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)//接收到数据
        {
            if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCP_SUCCESS)//BCP已经全部接收
            {
                //BMS_CONNECT_StepSet(BMS_PARA_SEND);//BCP接收成功了，发送CTS,CML
				//设置接收BCP超时状态
				SetCEMState(GUN_A,CEM_BCP, ERR_TYPE_NORMAL);
				break;//跳出当前循环
            }
            //收到BMS发送的错误报文BEM
            if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BEM_SUCCESS)
            {
                //进入超时机制
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E41",3);
				printf("GUN_A 3\r\n");
                return FALSE;
            }
        }
        //判断是否收到BCP报文，如果没有收到重新发送CRM(0xAA)报文
        if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCP_STEP, SYS_DELAY_5s))
        {
            //设置接收BCP超时状态
            SetCEMState(GUN_A,CEM_BCP, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E40",3);
			printf("GUN_A 4\r\n");
            return FALSE;
        }
    }
    //通知控制任务，判断BCP参数是否适合
    send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
    send_message29.DivNum = APP_BCP_FIT;
	send_message29.GunNum = GUN_A;
    OSQPost(Control_PeventA, &send_message29);
    OSTimeDly(SYS_DELAY_10ms);

    //判断BCP参数是否检测完成
    BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_1s); //纯粹为了保护，协议并未规定此时间
    while (GetBMSBCPFitCheck(GUN_A))
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE )
		{//如果在执行中发生了系统故障，就跳出
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
        OSTimeDly(SYS_DELAY_10ms);
        if (SysState[GUN_A].BMSState.BCPFitState != STATE_NO_ERR)
        {
            //BCP参数不合适,进入充电故障级别3
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E39",3);
			printf("GUN_A 5\r\n");
            return FALSE;
        }
        //超时1秒，也算不合适
        if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_1s) == BMS_TIME_OUT)
        {
            //BCP参数不合适,进入充电故障级别3
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E38",3);
			printf("GUN_A 6\r\n");
            return FALSE;
        }
    }
	BMS_CONNECT_StepSet(GUN_A,BMS_CTS_SEND);//CTS报文添加 吉利帝豪老国标车
	OSTimeDly(SYS_DELAY_5ms);
    //等待BMS返回电池充电准备就绪状态帧，60秒超时
    result = APP_DEFAULT;
    //这里用了5S这个参数，但下面实际分两种情况，1分钟的判断也是公用一个
    BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s);
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BRO_STEP, SYS_DELAY_1M);
	BMS_CONNECT_StepSet(GUN_A,BMS_PARA_SEND);//BCP接收成功了，发送CTS,CML
    while(1)
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE)
		{//如果在执行中发生了系统故障，就跳出
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
	    result = WaitConnectBMSmessageA(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)
        {
            if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BRO_SUCCESS)
            {
                if(BMS_BRO_Context[GUN_A].StandbyFlag == GB_BMSSTATE_STANDBY) //接收到BMS准备好的指令
                {
                	SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_NORMAL);
                    if(APP_GetGBType(GUN_A) == BMS_GB_2015)
                        //接收到BRO,且电池已经准备好了,应该发送CRO(0x00)
                        BMS_CONNECT_StepSet(GUN_A,BMS_CRO_UNREADY_SEND);
                    else
                        //接收到BRO,且电池已经准备好了,应该发送CRO(0xAA)
                        BMS_CONNECT_StepSet(GUN_A,BMS_CRO_SEND);
                    break;//进入下一步
                }
                else if(BMS_BRO_Context[GUN_A].StandbyFlag == GB_BMSSTATE_NOTREADY)
                {
                    firsttime++;
                    if (firsttime == 1)
                    {
                        BMS_TIMEOUT_ENTER(GUN_A,BMS_BRO_STEP, SYS_DELAY_1M); //进入了BRO的超时判断
                    }
                    BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s);
                    //接收到数据，但不是BMS已经准备的指令，继续等待，一分钟内是否都是0x00
                    if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BRO_STEP, SYS_DELAY_1M)) //
                    {
                        //设置接收BRO超时状态
                        SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_TIMEOUT);
                        //进入超时机制
                        ChargeErrSwitch(GUN_A);
						NB_WriterReason(GUN_A,"E37",3);
						printf("GUN_A 7\r\n");
                        return FALSE;
                    }
                }
            }
            else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0就不用了
            {
                //设置接收BRO超时状态
                SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_TIMEOUT);
                //进入超时机制
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E36",3);
				printf("GUN_A 8\r\n");
                return FALSE;
            }
            //收到BMS发送的错误报文BEM
            else if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BEM_SUCCESS)
            {
                //进入超时机制
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E35",3);
				printf("GUN_A 9\r\n");
                return FALSE;
            }
        }
        //没有收到任何BRO的帧，5S超时
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0就不用了
        {
            //设置接收BRO超时状态
            SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E34",3);
			printf("GUN_A 10\r\n");
            return FALSE;
        }
    }
	//VIN鉴权，充电模式就变为自动模式
	if(GetChargeMode(GUN_A) == MODE_VIN)
	{
		count = SYS_DELAY_20s / SYS_DELAY_200ms;
		while(count)
		{
			if(VinControl[GUN_A].VinState == VIN_SECCSEE)
			{
				break;
				//等待鉴权完成
			}
			count--;
			if(count == 0 || (VinControl[GUN_A].VinState == VIN_FAIL))
			{
				VinControl[GUN_A].VinState = VIN_FAIL;
				SetChargeMode(GUN_A,MODE_UNDEF);
				SetStartFailType(GUN_A,ENDFAIL_VIN);
				ChargeErrDeal(GUN_A,ERR_LF_METHOD_B);
				NB_WriterReason(GUN_A,"E49",3);
				OSTimeDly(SYS_DELAY_200ms);
				printf("156\r\n");
				return FALSE;
				//鉴权失败
			}
			OSTimeDly(SYS_DELAY_200ms);
		}
	}
    if (APP_GetGBType(GUN_A) == BMS_GB_2015)  //新国标 
    {
        //自收到BRO(0xAA)后需要检查下桩DC继电器外侧电压等一些硬件操作，相当于预充电
        send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message29.DivNum = APP_CHARGE_READY;
		send_message29.GunNum = GUN_A;
        OSQPost(Control_PeventA, &send_message29);
        OSTimeDly(SYS_DELAY_10ms);
		
        //这个地方为新国标内容，必须先检测DC继电器外侧的电压等
        BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_30s); //纯粹为了保护，协议并未规定此时间
        while (GetChargeReady(GUN_A) == READY_ING)
        {
            OSTimeDly(SYS_DELAY_250ms);
            //超时时间到了
            if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
                //进入超时机制
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E33",3);
				printf("GUN_A 11\r\n");
                return FALSE;
            }
        }
		if(GetChargeReady(GUN_A) == READY_SUCCESS)
		{
			//充电机准备好后，发送CRO报文(0xAA)
			BMS_CONNECT_StepSet(GUN_A,BMS_CRO_SEND);
		}else
		{
			NB_WriterReason(GUN_A,"E48",3);
			ChargeErrDeal(GUN_A,ERR_LF_METHOD_B);
		}
    }
	else
	{
		  BMS_CONNECT_StepSet(GUN_A,BMS_CRO_SEND);
	}
    return TRUE;
}


/***********************************************************************************************
* Function      : ChargeParameterConfigStep
* Description   : 充电参数配置阶段
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeParameterConfigStepB(void)
{
    static _BSP_MESSAGE send_message29;
    INT8U firsttime = 0;
	INT8U count;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

	SetBMSConnectStep(GUN_B,BMS_CONNECT_PARACONFIG);
    //等待接收BMS发送的BCP报文
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BCP_STEP, SYS_DELAY_5s); //进入了BCP的超时判断
    while(1)
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE )
		{//如果在执行中发生了系统故障，就跳出
			printf("sfasldkfj ");
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
        result = WaitConnectBMSmessageB(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)//接收到数据
        {
            if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCP_SUCCESS)//BCP已经全部接收
            {
                //BMS_CONNECT_StepSet(BMS_PARA_SEND);//BCP接收成功了，发送CTS,CML
				//设置接收BCP超时状态
				SetCEMState(GUN_B,CEM_BCP, ERR_TYPE_NORMAL);
				break;//跳出当前循环
            }
            //收到BMS发送的错误报文BEM
            if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BEM_SUCCESS)
            {
                //进入超时机制
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E32",3);
				printf("GUN_B 3\r\n");
                return FALSE;
            }
        }
        //判断是否收到BCP报文，如果没有收到重新发送CRM(0xAA)报文
        if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCP_STEP, SYS_DELAY_5s))
        {
            //设置接收BCP超时状态
            SetCEMState(GUN_B,CEM_BCP, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E31",3);
			printf("GUN_B 4\r\n");
            return FALSE;
        }
    }
    //通知控制任务，判断BCP参数是否适合
    send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
    send_message29.DivNum = APP_BCP_FIT;
	send_message29.GunNum = GUN_B;
    OSQPost(Control_PeventB, &send_message29);
    OSTimeDly(SYS_DELAY_10ms);

    //判断BCP参数是否检测完成
    BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_1s); //纯粹为了保护，协议并未规定此时间
    while (GetBMSBCPFitCheck(GUN_B))
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE )
		{//如果在执行中发生了系统故障，就跳出
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
        OSTimeDly(SYS_DELAY_10ms);
        if (SysState[GUN_B].BMSState.BCPFitState != STATE_NO_ERR)
        {
            //BCP参数不合适,进入充电故障级别3
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E30",3);
			printf("GUN_B 5\r\n");
            return FALSE;
        }
        //超时1秒，也算不合适
        if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_1s) == BMS_TIME_OUT)
        {
            //BCP参数不合适,进入充电故障级别3
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E29",3);
			printf("GUN_B 6\r\n");
            return FALSE;
        }
    }
	BMS_CONNECT_StepSet(GUN_B,BMS_CTS_SEND);//CTS报文添加 吉利帝豪老国标车
	OSTimeDly(SYS_DELAY_5ms);
    //等待BMS返回电池充电准备就绪状态帧，60秒超时
    result = APP_DEFAULT;
    //这里用了5S这个参数，但下面实际分两种情况，1分钟的判断也是公用一个
    BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s);
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BRO_STEP, SYS_DELAY_1M);
	BMS_CONNECT_StepSet(GUN_B,BMS_PARA_SEND);//BCP接收成功了，发送CTS,CML
    while(1)
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE)
		{//如果在执行中发生了系统故障，就跳出
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
	    result = WaitConnectBMSmessageB(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)
        {
            if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BRO_SUCCESS)
            {
                if(BMS_BRO_Context[GUN_B].StandbyFlag == GB_BMSSTATE_STANDBY) //接收到BMS准备好的指令
                {
                	SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_NORMAL);
                    if(APP_GetGBType(GUN_B) == BMS_GB_2015)
                        //接收到BRO,且电池已经准备好了,应该发送CRO(0x00)
                        BMS_CONNECT_StepSet(GUN_B,BMS_CRO_UNREADY_SEND);
                    else
                        //接收到BRO,且电池已经准备好了,应该发送CRO(0xAA)
                        BMS_CONNECT_StepSet(GUN_B,BMS_CRO_SEND);
                    break;//进入下一步
                }
                else if(BMS_BRO_Context[GUN_B].StandbyFlag == GB_BMSSTATE_NOTREADY)
                {
                    firsttime++;
                    if (firsttime == 1)
                    {
                        BMS_TIMEOUT_ENTER(GUN_B,BMS_BRO_STEP, SYS_DELAY_1M); //进入了BRO的超时判断
                    }
                    BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s);
                    //接收到数据，但不是BMS已经准备的指令，继续等待，一分钟内是否都是0x00
                    if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BRO_STEP, SYS_DELAY_1M)) //
                    {
                        //设置接收BRO超时状态
                        SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_TIMEOUT);
                        //进入超时机制
                        ChargeErrSwitch(GUN_B);
						NB_WriterReason(GUN_B,"E28",3);
						printf("GUN_B 7\r\n");
                        return FALSE;
                    }
                }
            }
            else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0就不用了
            {
                //设置接收BRO超时状态
                SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_TIMEOUT);
                //进入超时机制
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E27",3);
				printf("GUN_B 8\r\n");
                return FALSE;
            }
            //收到BMS发送的错误报文BEM
            else if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BEM_SUCCESS)
            {
                //进入超时机制
                ChargeErrSwitch(GUN_B);
				printf("GUN_B 9\r\n");
                return FALSE;
            }
        }
        //没有收到任何BRO的帧，5S超时
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0就不用了
        {
            //设置接收BRO超时状态
            SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E26",3);
			printf("GUN_B 10\r\n");
            return FALSE;
        }
    }
	//VIN鉴权，充电模式就变为自动模式
	if(GetChargeMode(GUN_B) == MODE_VIN)
	{
		count = SYS_DELAY_20s / SYS_DELAY_200ms;
		while(count)
		{
			if(VinControl[GUN_B].VinState == VIN_SECCSEE)
			{
				break;
				//等待鉴权完成
			}
			count--;
			if(count == 0 || (VinControl[GUN_B].VinState == VIN_FAIL))
			{
				VinControl[GUN_B].VinState = VIN_FAIL;
				SetChargeMode(GUN_B,MODE_UNDEF);
				SetStartFailType(GUN_B,ENDFAIL_VIN);
				NB_WriterReason(GUN_B,"E47",3);
				ChargeErrDeal(GUN_B,ERR_LF_METHOD_B);
				OSTimeDly(SYS_DELAY_200ms);
				printf("156\r\n");
				return FALSE;
				//鉴权失败
			}
			OSTimeDly(SYS_DELAY_200ms);
		}
	}
    if (APP_GetGBType(GUN_B) == BMS_GB_2015)  //新国标 
    {
        //自收到BRO(0xAA)后需要检查下桩DC继电器外侧电压等一些硬件操作，相当于预充电
        send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message29.DivNum = APP_CHARGE_READY;
		send_message29.GunNum = GUN_B;
        OSQPost(Control_PeventB, &send_message29);
        OSTimeDly(SYS_DELAY_10ms);
		
        //这个地方为新国标内容，必须先检测DC继电器外侧的电压等
        BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_30s); //纯粹为了保护，协议并未规定此时间
        while (GetChargeReady(GUN_B) == READY_ING)
        {
            OSTimeDly(SYS_DELAY_250ms);
            //超时时间到了
            if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
                //进入超时机制
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E25",3);
				printf("GUN_B 11\r\n");
                return FALSE;
            }
        }
		if(GetChargeReady(GUN_B) == READY_SUCCESS)
		{
			//充电机准备好后，发送CRO报文(0xAA)
			BMS_CONNECT_StepSet(GUN_B,BMS_CRO_SEND);
		}else
		{
			NB_WriterReason(GUN_B,"E46",3);
			ChargeErrDeal(GUN_B,ERR_LF_METHOD_B);
		}
    }
	else
	{
		  BMS_CONNECT_StepSet(GUN_B,BMS_CRO_SEND);
	}
    return TRUE;
}


/***********************************************************************************************
* Function      : ChargeRunningStep
* Description   : 充电阶段
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeRunningStepA(void)
{
    static _BSP_MESSAGE send_message11, send_message12, send_message13;
    static _STOP_TYPE stoptype = STOP_UNDEF;
    _BMS_BCL_CONTEXT BMS_BCL_ContextBak;
	

    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

    INT8U startcharge = FALSE;
	
	SetBMSConnectStep(GUN_A,BMS_CONNECT_RUN);
	//清零下初始值
	memset(&BMS_BCL_ContextBak, 0, sizeof(_BMS_BCL_CONTEXT));
    //GB/T 27930-2015第29页中的"自上次收到报文起Timeout为1s",
    //是否应该是发送CRO报文(0xAA)起Timeout为1s内??
    //暂时按照这个理解去写
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BCL_STEP, SYS_DELAY_1s);
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BCS_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_A,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s);
    while(1)
    {
        result = WaitConnectBMSmessageA(SYS_DELAY_10ms);
		//充电机要发送CCS的条件	，该条件用一次后就不用使用了
        if (((BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCL_SUCCESS)&&(BMS_CONNECT_Control[GUN_A].laststate == BMS_BCS_SUCCESS))
		 ||((BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCS_SUCCESS)&&(BMS_CONNECT_Control[GUN_A].laststate == BMS_BCL_SUCCESS)))//收到了BCL跟BCS，
        {
        	SetCEMState(GUN_A,CEM_BCL, ERR_TYPE_NORMAL);
			SetCEMState(GUN_A,CEM_BCS, ERR_TYPE_NORMAL);

            //发送CCS报文，此处可能会导致不是周期的发送
            if (startcharge == FALSE)
            {
                startcharge = TRUE;
                printf("AAABCPBCP,%x\r\n",BMS_BCP_Context[GUN_A].MaxVoltage);
                BMS_BCL_ContextBak.DemandVol = BMS_BCL_Context[GUN_A].DemandVol;
				BMS_BCL_ContextBak.DemandCur = BMS_BCL_Context[GUN_A].DemandCur;
				//需求参数大于BCP的最高充电电压、电流
				//按照BCP的最高充电电压电流输出
				if (BMS_BCL_Context[GUN_A].DemandVol > BMS_BCP_Context[GUN_A].MaxVoltage)
				{
					BMS_BCL_Context[GUN_A].DemandVol = BMS_BCP_Context[GUN_A].MaxVoltage;
				}
				if (BMS_BCL_Context[GUN_A].DemandCur < BMS_BCP_Context[GUN_A].MaxCurrent) 
				{
					BMS_BCL_Context[GUN_A].DemandCur = BMS_BCP_Context[GUN_A].MaxCurrent;
				}
				
                //发送开始输出电能消息给控制任务
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message11.DivNum = APP_POWER_OUTPUT;
				send_message11.GunNum = GUN_A;
                OSQPost(Control_PeventA, &send_message11);
                OSTimeDly(SYS_DELAY_2ms);               
				//开始发送CCS报文
                BMS_CONNECT_StepSet(GUN_A,BMS_CCS_SEND);
            }
        }
		//下面开始完全各自判断各自的
        //电池充电需求报文BCL充电机超时时间为1秒，超时后应立即结束充电，正常应该为50ms
        if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCL_SUCCESS)//接收到BCL
        {
        	SetCEMState(GUN_A,CEM_BCL, ERR_TYPE_NORMAL);
			if (GetChargePauseState(GUN_A) == FALSE) //充电允许
			{
				//根据BCL的需求实时更新输出,电压电流需求有变化
	            if((BMS_BCL_ContextBak.DemandVol != BMS_BCL_Context[GUN_A].DemandVol)||
	               (BMS_BCL_ContextBak.DemandCur != BMS_BCL_Context[GUN_A].DemandCur))
	            {
	                BMS_BCL_ContextBak.DemandVol = BMS_BCL_Context[GUN_A].DemandVol;
					BMS_BCL_ContextBak.DemandCur = BMS_BCL_Context[GUN_A].DemandCur;
					if (BMS_BCL_Context[GUN_A].DemandVol > BMS_BCP_Context[GUN_A].MaxVoltage)
					{
						BMS_BCL_Context[GUN_A].DemandVol = BMS_BCP_Context[GUN_A].MaxVoltage;
					}
					if (BMS_BCL_Context[GUN_A].DemandCur < BMS_BCP_Context[GUN_A].MaxCurrent) 
					{
						BMS_BCL_Context[GUN_A].DemandCur = BMS_BCP_Context[GUN_A].MaxCurrent;
					}
	                send_message12.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	                send_message12.DivNum = APP_POWER_OUTPUT;
					send_message12.GunNum = GUN_A;
	                OSQPost(Control_PeventA, &send_message12);
//                    if (APP_GetGBType() == BMS_GB_2011)
					{//开始发送CCS报文，有些旧国标车不一定会BCL和BCS都很快发出，但国标规定
						//新国标的BCL和BCS都收到后，才能发出CCS
                        BMS_CONNECT_StepSet(GUN_A,BMS_CCS_SEND);
					}
	            }
				else
				{
					if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s) )
					{//在需求没有变化时，定时5s发送一次当前需求
						if (BMS_BCL_Context[GUN_A].DemandVol > BMS_BCP_Context[GUN_A].MaxVoltage)
						{
							BMS_BCL_Context[GUN_A].DemandVol = BMS_BCP_Context[GUN_A].MaxVoltage;
						}
						if (BMS_BCL_Context[GUN_A].DemandCur < BMS_BCP_Context[GUN_A].MaxCurrent) 
						{
							BMS_BCL_Context[GUN_A].DemandCur = BMS_BCP_Context[GUN_A].MaxCurrent;
						}
						send_message12.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
						send_message12.DivNum = APP_POWER_OUTPUT;
						send_message12.GunNum = GUN_A;
						OSQPost(Control_PeventA, &send_message12);
						BMS_TIMEOUT_ENTER(GUN_A,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s);
					}
				}
			}
			//在规定的时间1s内收到了BCL，重新设置超时时间
            BMS_TIMEOUT_ENTER(GUN_A,BMS_BCL_STEP, SYS_DELAY_1s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;//第一次之后不用管当前状态了
			}
        }
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCL_STEP, SYS_DELAY_1s) )
        {
            //BCL超时
            //发送充电机中止充电报文(CST)给BMS
            //设置接收BCL超时状态
            SetCEMState(GUN_A,CEM_BCL, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E24",3);
			printf("GUN_A 12\r\n");
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCLTIMTOUT);
            return FALSE;
        }

        if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCS_SUCCESS)//接收到BCS
        {
        	SetCEMState(GUN_A,CEM_BCS, ERR_TYPE_NORMAL);
            //在规定的时间5s内收到了BCS，重新设置超时时间
            BMS_TIMEOUT_ENTER(GUN_A,BMS_BCS_STEP, SYS_DELAY_5s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;//第一次之后不用管当前状态了
			}
        }
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCS_STEP, SYS_DELAY_5s))//BCS 5s超时
        {//电池充电总状态报文BCS超时时间为5秒，超时后应立即结束充电，正常应该为250ms
            //超时处理
            //发送充电机中止充电报文(CST)给BMS
            //设置接收BCS超时状态
            SetCEMState(GUN_A,CEM_BCS, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			printf("GUN_A 13\r\n");
			NB_WriterReason(GUN_A,"E23",3);
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCSTIMTOUT);
            return FALSE;
        }

		if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BSM_SUCCESS)
		{//BSM暂停充电
			if (BMS_BSM_Context[GUN_A].State1.OneByte.ChargeAllowBits == BMS_CHARGE_PAUSE)
			{//未暂停输出，需要暂停输出
				if (GetChargePauseState(GUN_A) == FALSE)
				{//暂时充电，只是关闭模块输出
					send_message13.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	                send_message13.DivNum = APP_CHARGE_PAUSE;
					send_message13.GunNum = GUN_A;
	                OSQPost(Control_PeventA, &send_message13);
                    //暂停充电开始计算超时10分钟
                    BMS_TIMEOUT_ENTER(GUN_A,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			else if (BMS_BSM_Context[GUN_A].State1.OneByte.ChargeAllowBits == BSM_CHARGE_ENABLE)
			{//恢复充电
				if (GetChargePauseState(GUN_A) == TRUE)
				{
					SetChargePauseState(GUN_A,FALSE);
					BMS_BCL_ContextBak.DemandVol = 0;
					BMS_BCL_ContextBak.DemandCur = 0;
                    //清除暂停充电超时10分钟
                    BMS_TIMEOUT_ENTER(GUN_A,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			if ( (BMS_BSM_Context[GUN_A].State0.OneByte.UnitVolBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_A].State0.OneByte.WholeSOCBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_A].State0.OneByte.ChargeCurBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_A].State0.OneByte.BatteryTempBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_A].State1.OneByte.BatteryInsulationBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_A].State1.OneByte.BatteryConnectBits == STATE_TROUBLE) )
			{//BSM里有状态不正常
				
				SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //故障停止
				SysState[GUN_A].TotalErrState	|= ERR_BMSSTOP_TYPE;
				
				if(BMS_BSM_Context[GUN_A].State0.OneByte.UnitVolBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMBATVOLTHIGH); //BMS过呀
				}
				else if(BMS_BSM_Context[GUN_A].State0.OneByte.WholeSOCBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMBATVOLTHIGH); //BSM单体电压过高
				}
				else if(BMS_BSM_Context[GUN_A].State0.OneByte.ChargeCurBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMCURRUP);  //BMS过流
				}
				else if(BMS_BSM_Context[GUN_A].State0.OneByte.BatteryTempBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMTEMPUP); //BMS过温度
				}
				else if(BMS_BSM_Context[GUN_A].State1.OneByte.BatteryInsulationBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMINSOLUTION); 
				}
				else if (BMS_BSM_Context[GUN_A].State1.OneByte.BatteryConnectBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMSWERR); 
				}
				else
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMERR);
				}
			
				printf("11127\r\n");
				break;
			}
		}
        if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BST_SUCCESS)
        {//收到BMS的中止充电(BST)报文,正常10ms一个周期，
            //设置中止原因为BMS主动停止
            SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.BMSStopBits = STATE_TROUBLE;
			
			//CheckBSTErr(GUN_A,(INT8U*)&stoptype);   //具体的故障原因填写
			APP_Set_ERR_Branch(GUN_A,STOP_BSMNORMAL);		//无需具体原因，就认为是BMS主动停止（有些车子主动停止也是有问题的）   20211015
            break;
        }
        if (result == APP_END_CHARGING)
        {//收到桩体本身的结束充电命令
            SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.UserStopBits = STATE_TROUBLE;
            stoptype = STOP_USER; //用户主动停止，具体原因，前面已经细分
			SysState[GUN_A].TotalErrState |= ERR_USER_STOP_TYPE;
			printf("User Stop1\r\n");
            break;
        }
        if (GetChargePauseState(GUN_A) == TRUE)
        {//暂停充电状态下
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_PAUSE_STEP, SYS_DELAY_10M) )
            {//CST填写故障中止
                SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //故障停止
				SysState[GUN_A].TotalErrState	|= ERR_CHARGESTOP_TYPE;
				APP_Set_ERR_Branch(GUN_A,STOP_OTHERERR);
				printf("11111\r\n");
                break;
            }
        }

        if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BEM_SUCCESS)
        {//收到BMS发送的错误报文BEM
            //进入超时机制
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E22",3);
			printf("GUN_A 14\r\n");
            return FALSE;
        }
		if(GetBMSStartCharge(GUN_A) == FALSE)    //出现这个现象  20211117
		{
			printf("GUN_A startcharge State Err\r\n");
			break;
		}
    }

	//到此处只能收到BST或者收到屏下发的结束充电帧
    //如果BST超时了，需要发送CEM报文，不要去等待BSD了，结束充电即可
    //如果BST未超时，需要去等待BSD
	SetBMSConnectStep(GUN_A,BMS_CONNECT_END);
    result = APP_DEFAULT;
    //发送充电机中止充电报文(CST)给BMS
    BMS_CONNECT_StepSet(GUN_A,BMS_CST_SEND);
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BST_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_A,BMS_BSD_STEP, SYS_DELAY_10s); //BSD的超时也是从发送CST开始计算
	//可能先收到BST帧
    if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BST_SUCCESS)//BST已经接收
    {
    	SetCEMState(GUN_A,CEM_BST, ERR_TYPE_NORMAL);
        send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message11.DivNum = APP_CHARGE_END;  //停止电力输出
		NB_WriterReason(GUN_A,"E5",2);
        send_message11.DataLen = 1;
		send_message11.GunNum = GUN_A;
        send_message11.pData = (INT8U *)&stoptype;
        OSQPost(Control_PeventA, &send_message11);
        OSTimeDly(SYS_DELAY_10ms);
		printf("Get BST\r\n");
    }
    else//如果CST先发送过去
    {
	    //需要更改下当前状态，发送CST后需要BMS回复一个BST
	    BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;
        while (1)
        {
            result = WaitConnectBMSmessageA(SYS_DELAY_20ms);
            if(result == APP_RXDEAL_SUCCESS)//接收到数据
            {
                if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BST_SUCCESS)//BST已经接收
                {
                	SetCEMState(GUN_A,CEM_BST, ERR_TYPE_NORMAL);
                    send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message11.DivNum = APP_CHARGE_END;  //停止电力输出
//					NB_WriterReason(GUN_A,"E4",2);
                    send_message11.DataLen = 1;
					send_message11.GunNum = GUN_A;
                    send_message11.pData = (INT8U *)&stoptype;
                    OSQPost(Control_PeventA, &send_message11);
                    OSTimeDly(SYS_DELAY_10ms);
					printf("First CST,Get BST\r\n");
                    break;//跳出当前循环
                }             
            }
            if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BST_STEP, SYS_DELAY_5s) )
            {
                //BST超时
                //设置接收BST超时状态
                //此处BST还是要做判断超时的，但不用再做3次重连了
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message11.DivNum = APP_CHARGE_END;  //停止电力输出
//				NB_WriterReason(GUN_A,"E3",2);
                send_message11.DataLen = 1;
				send_message11.GunNum = GUN_A;
                send_message11.pData = (INT8U *)&stoptype;
                OSQPost(Control_PeventA, &send_message11);
                SetCEMState(GUN_A,CEM_BST, ERR_TYPE_TIMEOUT);
//                BMS_CONNECT_StepSet(BMS_CEM_SEND);
                ChargeErrDeal(GUN_A,ERR_L3_CHAGING_OVER_TIMEOUT);//直接结束充电
				printf("BST Timeout\r\n");
                return FALSE;
            }
			//此处判断下CST帧是否被修改了，此处要保证一直在发送CST
			if (BMS_CONNECT_Control[GUN_A].step != BMS_CST_SEND)
			{
            	BMS_CONNECT_StepSet(GUN_A,BMS_CST_SEND);
			}
        }
    }
    return TRUE;
}

/***********************************************************************************************
* Function      : ChargeRunningStep
* Description   : 充电阶段
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeRunningStepB(void)
{
    static _BSP_MESSAGE send_message11, send_message12, send_message13;
    static _STOP_TYPE stoptype = STOP_UNDEF;
    _BMS_BCL_CONTEXT BMS_BCL_ContextBak;
	

    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

    INT8U startcharge = FALSE;
	
	SetBMSConnectStep(GUN_B,BMS_CONNECT_RUN);
	//清零下初始值
	memset(&BMS_BCL_ContextBak, 0, sizeof(_BMS_BCL_CONTEXT));
    //GB/T 27930-2015第29页中的"自上次收到报文起Timeout为1s",
    //是否应该是发送CRO报文(0xAA)起Timeout为1s内??
    //暂时按照这个理解去写
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BCL_STEP, SYS_DELAY_1s);
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BCS_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_B,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s);
    while(1)
    {
        result = WaitConnectBMSmessageB(SYS_DELAY_10ms);
		//充电机要发送CCS的条件	，该条件用一次后就不用使用了
        if (((BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCL_SUCCESS)&&(BMS_CONNECT_Control[GUN_B].laststate == BMS_BCS_SUCCESS))
		 ||((BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCS_SUCCESS)&&(BMS_CONNECT_Control[GUN_B].laststate == BMS_BCL_SUCCESS)))//收到了BCL跟BCS，
        {
        	SetCEMState(GUN_B,CEM_BCL, ERR_TYPE_NORMAL);
			SetCEMState(GUN_B,CEM_BCS, ERR_TYPE_NORMAL);

            //发送CCS报文，此处可能会导致不是周期的发送
            if (startcharge == FALSE)
            {
                startcharge = TRUE;
                printf("AAABCPBCP,%x\r\n",BMS_BCP_Context[GUN_B].MaxVoltage);
                BMS_BCL_ContextBak.DemandVol = BMS_BCL_Context[GUN_B].DemandVol;
				BMS_BCL_ContextBak.DemandCur = BMS_BCL_Context[GUN_B].DemandCur;
				//需求参数大于BCP的最高充电电压、电流
				//按照BCP的最高充电电压电流输出
				if (BMS_BCL_Context[GUN_B].DemandVol > BMS_BCP_Context[GUN_B].MaxVoltage)
				{
					BMS_BCL_Context[GUN_B].DemandVol = BMS_BCP_Context[GUN_B].MaxVoltage;
				}
				if (BMS_BCL_Context[GUN_B].DemandCur < BMS_BCP_Context[GUN_B].MaxCurrent) 
				{
					BMS_BCL_Context[GUN_B].DemandCur = BMS_BCP_Context[GUN_B].MaxCurrent;
				}
				
                //发送开始输出电能消息给控制任务
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message11.DivNum = APP_POWER_OUTPUT;
				send_message11.GunNum = GUN_B;
                OSQPost(Control_PeventB, &send_message11);
                OSTimeDly(SYS_DELAY_2ms);               
				//开始发送CCS报文
                BMS_CONNECT_StepSet(GUN_B,BMS_CCS_SEND);
            }
        }
		//下面开始完全各自判断各自的
        //电池充电需求报文BCL充电机超时时间为1秒，超时后应立即结束充电，正常应该为50ms
        if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCL_SUCCESS)//接收到BCL
        {
        	SetCEMState(GUN_B,CEM_BCL, ERR_TYPE_NORMAL);
			if (GetChargePauseState(GUN_B) == FALSE) //充电允许
			{
				//根据BCL的需求实时更新输出,电压电流需求有变化
	            if((BMS_BCL_ContextBak.DemandVol != BMS_BCL_Context[GUN_B].DemandVol)||
	               (BMS_BCL_ContextBak.DemandCur != BMS_BCL_Context[GUN_B].DemandCur))
	            {
	                BMS_BCL_ContextBak.DemandVol = BMS_BCL_Context[GUN_B].DemandVol;
					BMS_BCL_ContextBak.DemandCur = BMS_BCL_Context[GUN_B].DemandCur;
					if (BMS_BCL_Context[GUN_B].DemandVol > BMS_BCP_Context[GUN_B].MaxVoltage)
					{
						BMS_BCL_Context[GUN_B].DemandVol = BMS_BCP_Context[GUN_B].MaxVoltage;
					}
					if (BMS_BCL_Context[GUN_B].DemandCur < BMS_BCP_Context[GUN_B].MaxCurrent) 
					{
						BMS_BCL_Context[GUN_B].DemandCur = BMS_BCP_Context[GUN_B].MaxCurrent;
					}
	                send_message12.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	                send_message12.DivNum = APP_POWER_OUTPUT;
					send_message12.GunNum = GUN_B;
	                OSQPost(Control_PeventB, &send_message12);
//                    if (APP_GetGBType() == BMS_GB_2011)
					{//开始发送CCS报文，有些旧国标车不一定会BCL和BCS都很快发出，但国标规定
						//新国标的BCL和BCS都收到后，才能发出CCS
                        BMS_CONNECT_StepSet(GUN_B,BMS_CCS_SEND);
					}
	            }
				else
				{
					if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s) )
					{//在需求没有变化时，定时5s发送一次当前需求
						if (BMS_BCL_Context[GUN_B].DemandVol > BMS_BCP_Context[GUN_B].MaxVoltage)
						{
							BMS_BCL_Context[GUN_B].DemandVol = BMS_BCP_Context[GUN_B].MaxVoltage;
						}
						if (BMS_BCL_Context[GUN_B].DemandCur < BMS_BCP_Context[GUN_B].MaxCurrent) 
						{
							BMS_BCL_Context[GUN_B].DemandCur = BMS_BCP_Context[GUN_B].MaxCurrent;
						}
						send_message12.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
						send_message12.DivNum = APP_POWER_OUTPUT;
						send_message12.GunNum = GUN_B;
						OSQPost(Control_PeventB, &send_message12);
						BMS_TIMEOUT_ENTER(GUN_B,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s);
					}
				}
			}
			//在规定的时间1s内收到了BCL，重新设置超时时间
            BMS_TIMEOUT_ENTER(GUN_B,BMS_BCL_STEP, SYS_DELAY_1s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;//第一次之后不用管当前状态了
			}
        }
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCL_STEP, SYS_DELAY_1s) )
        {
            //BCL超时
            //发送充电机中止充电报文(CST)给BMS
            //设置接收BCL超时状态
            SetCEMState(GUN_B,CEM_BCL, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E21",3);
			printf("GUN_B 12\r\n");
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCLTIMTOUT);
            return FALSE;
        }

        if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCS_SUCCESS)//接收到BCS
        {
        	SetCEMState(GUN_B,CEM_BCS, ERR_TYPE_NORMAL);
            //在规定的时间5s内收到了BCS，重新设置超时时间
            BMS_TIMEOUT_ENTER(GUN_B,BMS_BCS_STEP, SYS_DELAY_5s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;//第一次之后不用管当前状态了
			}
        }
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCS_STEP, SYS_DELAY_5s))//BCS 5s超时
        {//电池充电总状态报文BCS超时时间为5秒，超时后应立即结束充电，正常应该为250ms
            //超时处理
            //发送充电机中止充电报文(CST)给BMS
            //设置接收BCS超时状态
            SetCEMState(GUN_B,CEM_BCS, ERR_TYPE_TIMEOUT);
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E20",3);
			printf("GUN_B 13\r\n");
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCSTIMTOUT);
            return FALSE;
        }

		if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BSM_SUCCESS)
		{//BSM暂停充电
			if (BMS_BSM_Context[GUN_B].State1.OneByte.ChargeAllowBits == BMS_CHARGE_PAUSE)
			{//未暂停输出，需要暂停输出
				if (GetChargePauseState(GUN_B) == FALSE)
				{//暂时充电，只是关闭模块输出
					send_message13.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	                send_message13.DivNum = APP_CHARGE_PAUSE;
					send_message13.GunNum = GUN_B;
	                OSQPost(Control_PeventB, &send_message13);
                    //暂停充电开始计算超时10分钟
                    BMS_TIMEOUT_ENTER(GUN_B,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			else if (BMS_BSM_Context[GUN_B].State1.OneByte.ChargeAllowBits == BSM_CHARGE_ENABLE)
			{//恢复充电
				if (GetChargePauseState(GUN_B) == TRUE)
				{
					SetChargePauseState(GUN_B,FALSE);
					BMS_BCL_ContextBak.DemandVol = 0;
					BMS_BCL_ContextBak.DemandCur = 0;
                    //清除暂停充电超时10分钟
                    BMS_TIMEOUT_ENTER(GUN_B,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			if ( (BMS_BSM_Context[GUN_B].State0.OneByte.UnitVolBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_B].State0.OneByte.WholeSOCBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_B].State0.OneByte.ChargeCurBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_B].State0.OneByte.BatteryTempBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_B].State1.OneByte.BatteryInsulationBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_B].State1.OneByte.BatteryConnectBits == STATE_TROUBLE) )
			{//BSM里有状态不正常
				
				SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //故障停止
				SysState[GUN_B].TotalErrState	|= ERR_BMSSTOP_TYPE;
				
				if(BMS_BSM_Context[GUN_B].State0.OneByte.UnitVolBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMBATVOLTHIGH); //BMS过呀
				}
				else if(BMS_BSM_Context[GUN_B].State0.OneByte.WholeSOCBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMBATVOLTHIGH); //BSM单体电压过高
				}
				else if(BMS_BSM_Context[GUN_B].State0.OneByte.ChargeCurBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMCURRUP);  //BMS过流
				}
				else if(BMS_BSM_Context[GUN_B].State0.OneByte.BatteryTempBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMTEMPUP); //BMS过温度
				}
				else if(BMS_BSM_Context[GUN_B].State1.OneByte.BatteryInsulationBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMINSOLUTION); 
				}
				else if (BMS_BSM_Context[GUN_B].State1.OneByte.BatteryConnectBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMSWERR); 
				}
				else
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMERR);
				}
			
				printf("11127\r\n");
				break;
			}
		}
        if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BST_SUCCESS)
        {//收到BMS的中止充电(BST)报文,正常10ms一个周期，
            //设置中止原因为BMS主动停止
            SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.BMSStopBits = STATE_TROUBLE;
			
			//BST中停止原因处理
			//CheckBSTErr(GUN_B,(INT8U*)&stoptype);   //具体的故障原因填写
			APP_Set_ERR_Branch(GUN_B,STOP_BSMNORMAL);		//无需具体原因，就认为是BMS主动停止（有些车子主动停止也是有问题的）   20211015
			printf("11126\r\n");
            break;
        }
        if (result == APP_END_CHARGING)
        {//收到桩体本身的结束充电命令
            SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.UserStopBits = STATE_TROUBLE;
            stoptype = STOP_USER; //用户主动停止，具体原因，前面已经细分
			SysState[GUN_B].TotalErrState |= ERR_USER_STOP_TYPE;
			printf("User Stop1\r\n");
            break;
        }
        if (GetChargePauseState(GUN_B) == TRUE)
        {//暂停充电状态下
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_PAUSE_STEP, SYS_DELAY_10M) )
            {//CST填写故障中止
                SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //故障停止
				SysState[GUN_B].TotalErrState	|= ERR_CHARGESTOP_TYPE;
				APP_Set_ERR_Branch(GUN_B,STOP_OTHERERR);
				printf("11111\r\n");
                break;
            }
        }

        if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BEM_SUCCESS)
        {//收到BMS发送的错误报文BEM
            //进入超时机制
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E19",3);
			printf("GUN_B 14\r\n");
            return FALSE;
        }
		
		if(GetBMSStartCharge(GUN_B) == FALSE)    //出现这个现象  20211117
		{
			printf("GUN_B startcharge State Err\r\n");
			break;
		}
    }

	//到此处只能收到BST或者收到屏下发的结束充电帧
    //如果BST超时了，需要发送CEM报文，不要去等待BSD了，结束充电即可
    //如果BST未超时，需要去等待BSD
	SetBMSConnectStep(GUN_B,BMS_CONNECT_END);
    result = APP_DEFAULT;
    //发送充电机中止充电报文(CST)给BMS
    BMS_CONNECT_StepSet(GUN_B,BMS_CST_SEND);
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BST_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_B,BMS_BSD_STEP, SYS_DELAY_10s); //BSD的超时也是从发送CST开始计算
	//可能先收到BST帧
    if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BST_SUCCESS)//BST已经接收
    {
    	SetCEMState(GUN_B,CEM_BST, ERR_TYPE_NORMAL);
        send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message11.DivNum = APP_CHARGE_END;  //停止电力输出
		NB_WriterReason(GUN_B,"E2",2);
        send_message11.DataLen = 1;
		send_message11.GunNum = GUN_B;
        send_message11.pData = (INT8U *)&stoptype;
        OSQPost(Control_PeventB, &send_message11);
        OSTimeDly(SYS_DELAY_10ms);
		printf("Get BST\r\n");
    }
    else//如果CST先发送过去
    {
	    //需要更改下当前状态，发送CST后需要BMS回复一个BST
	    BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;
        while (1)
        {
            result = WaitConnectBMSmessageB(SYS_DELAY_20ms);
            if(result == APP_RXDEAL_SUCCESS)//接收到数据
            {
                if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BST_SUCCESS)//BST已经接收
                {
                	SetCEMState(GUN_B,CEM_BST, ERR_TYPE_NORMAL);
                    send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message11.DivNum = APP_CHARGE_END;  //停止电力输出
//					NB_WriterReason(GUN_B,"E1",2);
                    send_message11.DataLen = 1;
					send_message11.GunNum = GUN_B;
                    send_message11.pData = (INT8U *)&stoptype;
                    OSQPost(Control_PeventB, &send_message11);
                    OSTimeDly(SYS_DELAY_10ms);
					printf("First CST,Get BST\r\n");
                    break;//跳出当前循环
                }             
            }
            if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BST_STEP, SYS_DELAY_5s) )
            {
                //BST超时
                //设置接收BST超时状态
                //此处BST还是要做判断超时的，但不用再做3次重连了
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
//				NB_WriterReason(GUN_B,"E0",2);
                send_message11.DivNum = APP_CHARGE_END;  //停止电力输出
                send_message11.DataLen = 1;
				send_message11.GunNum = GUN_B;
                send_message11.pData = (INT8U *)&stoptype;
                OSQPost(Control_PeventB, &send_message11);
                SetCEMState(GUN_B,CEM_BST, ERR_TYPE_TIMEOUT);
//                BMS_CONNECT_StepSet(BMS_CEM_SEND);
                ChargeErrDeal(GUN_B,ERR_L3_CHAGING_OVER_TIMEOUT);//直接结束充电
				printf("BST Timeout\r\n");
                return FALSE;
            }
			//此处判断下CST帧是否被修改了，此处要保证一直在发送CST
			if (BMS_CONNECT_Control[GUN_B].step != BMS_CST_SEND)
			{
            	BMS_CONNECT_StepSet(GUN_B,BMS_CST_SEND);
			}
        }
    }
    return TRUE;
}

/***********************************************************************************************
* Function      : ChargeEndStep
* Description   : 充电结束阶段
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeEndStepA(void)
{
    static _BSP_MESSAGE send_message30;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

   BSP_CloseLOCK(GUN_A);
	//BSD的超时是从发送CST开始计算的10秒
	//进入此处就说明已经收到了BST
    if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BSD_SUCCESS)
    {
    	//收到BSD后再发送CSD
    	BMS_CONNECT_StepSet(GUN_A,BMS_CSD_SEND);
    	SetCEMState(GUN_A,CEM_BSD, ERR_TYPE_NORMAL);
        //充电结束,收到BSD报文
        send_message30.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message30.DivNum = APP_CHARGE_BSD;
        send_message30.DataLen = sizeof(_BMS_BSD_CONTEXT);
		send_message30.GunNum =GUN_A;
        send_message30.pData = (INT8U *)&BMS_BSD_Context[GUN_A].EndChargeSOC;
        OSQPost(Control_PeventA, &send_message30);
    }
    else
    {
        while(1)
        {
            result = WaitConnectBMSmessageA(SYS_DELAY_20ms);
            //接收到BMS统计数据(BSD)报文
            if(result == APP_RXDEAL_SUCCESS)
            {
                if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BSD_SUCCESS)
                {
                	//收到BSD后再发送CSD
    				BMS_CONNECT_StepSet(GUN_A,BMS_CSD_SEND);
                	SetCEMState(GUN_A,CEM_BSD, ERR_TYPE_NORMAL);
                    //充电结束,收到BSD报文
                    send_message30.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message30.DivNum = APP_CHARGE_BSD;
                    send_message30.DataLen = sizeof(_BMS_BSD_CONTEXT);
					send_message30.GunNum =GUN_A;
                    send_message30.pData = (INT8U *)&BMS_BSD_Context[GUN_A].EndChargeSOC;
                    OSQPost(Control_PeventA, &send_message30);
                    break;//跳出当前循环
                }
            }
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BSD_STEP, SYS_DELAY_10s))//BSD 10s超时
            {
                //超时处理
                //设置接收BSD超时状态
                SetCEMState(GUN_A,CEM_BSD, ERR_TYPE_TIMEOUT);
                ChargeErrDeal(GUN_A,ERR_L3_CHAGING_OVER_TIMEOUT);//直接结束充电,就是处理方式B
                return FALSE;
            }
			//此处判断下CST帧是否被修改了，此处要保证一直在发送CST
			if (BMS_CONNECT_Control[GUN_A].step != BMS_CST_SEND)
			{
            	BMS_CONNECT_StepSet(GUN_A,BMS_CST_SEND);
			}
        }
    }
    return TRUE;
}


/***********************************************************************************************
* Function      : ChargeEndStep
* Description   : 充电结束阶段
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeEndStepB(void)
{
    static _BSP_MESSAGE send_message30;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

   BSP_CloseLOCK(GUN_B);
	//BSD的超时是从发送CST开始计算的10秒
	//进入此处就说明已经收到了BST
    if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BSD_SUCCESS)
    {
    	//收到BSD后再发送CSD
    	BMS_CONNECT_StepSet(GUN_B,BMS_CSD_SEND);
    	SetCEMState(GUN_B,CEM_BSD, ERR_TYPE_NORMAL);
        //充电结束,收到BSD报文
        send_message30.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message30.DivNum = APP_CHARGE_BSD;
        send_message30.DataLen = sizeof(_BMS_BSD_CONTEXT);
		send_message30.GunNum =GUN_B;
        send_message30.pData = (INT8U *)&BMS_BSD_Context[GUN_B].EndChargeSOC;
        OSQPost(Control_PeventB, &send_message30);
    }
    else
    {
        while(1)
        {
            result = WaitConnectBMSmessageB(SYS_DELAY_20ms);
            //接收到BMS统计数据(BSD)报文
            if(result == APP_RXDEAL_SUCCESS)
            {
                if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BSD_SUCCESS)
                {
                	//收到BSD后再发送CSD
    				BMS_CONNECT_StepSet(GUN_B,BMS_CSD_SEND);
                	SetCEMState(GUN_B,CEM_BSD, ERR_TYPE_NORMAL);
                    //充电结束,收到BSD报文
                    send_message30.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message30.DivNum = APP_CHARGE_BSD;
                    send_message30.DataLen = sizeof(_BMS_BSD_CONTEXT);
					send_message30.GunNum =GUN_B;
                    send_message30.pData = (INT8U *)&BMS_BSD_Context[GUN_B].EndChargeSOC;
                    OSQPost(Control_PeventB, &send_message30);
                    break;//跳出当前循环
                }
            }
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BSD_STEP, SYS_DELAY_10s))//BSD 10s超时
            {
                //超时处理
                //设置接收BSD超时状态
                SetCEMState(GUN_B,CEM_BSD, ERR_TYPE_TIMEOUT);
                ChargeErrDeal(GUN_B,ERR_L3_CHAGING_OVER_TIMEOUT);//直接结束充电,就是处理方式B
                return FALSE;
            }
			//此处判断下CST帧是否被修改了，此处要保证一直在发送CST
			if (BMS_CONNECT_Control[GUN_B].step != BMS_CST_SEND)
			{
            	BMS_CONNECT_StepSet(GUN_B,BMS_CST_SEND);
			}
        }
    }
    return TRUE;
}

/***********************************************************************************************
* Function      : TaskConnectBMSMain
* Description   : 实现跟BMS周期性发送的任务
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
void TaskSendBMSAFrame(void *pdata)
{
    static INT32U oldSysTime = 0;
    static INT32U nowSysTime = 0;
    INT8U err = 0;
    INT8U count ;
	INT8U i=0;
    _BSP_MESSAGE *pMSG;   // 定义消息指针

    TaskSendFrame_preventA = OSQCreate(txBMSCANOSQA, BMSCANOSQ_NUM); // 建立事件(消息队列)

    OSTimeDlyHMSM(0, 0, 0, 500); //延时500MS让其他任务起来
    while(1)
    {
        nowSysTime = OSTimeGet();
		
        pMSG = OSQPend(TaskSendFrame_preventA, SYS_DELAY_10ms, &err);
        if(err == OS_ERR_NONE)
        {
            //充电机跟BMS通讯当前状态
            switch((_SEND_BMS_STEP)pMSG->MsgID)
            {
                case BMS_CHM_SEND: //充电机充电握手阶段报文
                case BMS_CRM_SEND: //充电机辨识报文
                    CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //第一次发送函数CHM或者CRM,
                    break;
                case BMS_PARA_SEND: //充电机发送时间同步信息报文及最大输出能力报文
//                    CAN_Send_Table[(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //第一次发送函数,这里需要错开点么？
                    CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //第一次发送函数,
                    break;
				case BMS_CTS_SEND:
					CAN_Send_Table[GUN_A][2].CANSendFrametoBMS();//使用吉利帝豪的旧国标车，如果没有CTS报文，就会报错，这里临时处理一下 
					break;
                case BMS_CRO_SEND:
                case BMS_CRO_UNREADY_SEND:
                case BMS_CCS_SEND:
                case BMS_CST_SEND:
                case BMS_CSD_SEND:
                case BMS_CEM_SEND:    
                    CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //第一次发送函数,
                    break;
				case BMS_CEM_CST_SEND:
					CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //发送CEM
					CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID) + 1].CANSendFrametoBMS();  //发送CST
                    break;
				default:
					break;
            }
            oldSysTime = nowSysTime;//第一次已经发送，先备份发送后的时间。
        }
        else
        {
            for(count = 0; count < CANSENDTALELEN; count++)
            {
                if(BMS_CONNECT_Control[GUN_A].step  == CAN_Send_Table[GUN_A][count].step)//处于什么阶段属性
                {
                    //大于周期值,OSTimeGet值一般已经很大了，大概49天时间了,逻辑上还是要保护下。
                    if((nowSysTime >= oldSysTime) ? ((nowSysTime - oldSysTime) >= CAN_Send_Table[GUN_A][count].cycletime ) : \
                       ((nowSysTime + (INT32U_MAX_NUM - oldSysTime)) >= CAN_Send_Table[GUN_A][count].cycletime) )
                    {
//                        if(!CanFrameControl.state)
                        CAN_Send_Table[GUN_A][count].CANSendFrametoBMS();//发送函数
                        oldSysTime = nowSysTime;
                    }
                }
            }
        }
		for(i=0;i<3;i++)
		{//特殊处理BMV\BMT\BSP
			if(Special_PGN[GUN_A][i].Get_Flag)
			{
				if(Special_PGN[GUN_A][i].Count_Delay>=50)
				{
					Reply_TPCM_EndofMsgAckFrameA();
					Special_PGN[GUN_A][i].Get_Flag=0x00;
				}
				else
				{
					Special_PGN[GUN_A][i].Count_Delay+=1;
				}
			}
		}
    }
}


/***********************************************************************************************
* Function      : TaskConnectBMSMain
* Description   : 实现跟BMS周期性发送的任务
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
void TaskSendBMSBFrame(void *pdata)
{
    static INT32U oldSysTime = 0;
    static INT32U nowSysTime = 0;
    INT8U err = 0;
    INT8U count ;
	INT8U i=0;
    _BSP_MESSAGE *pMSG;   // 定义消息指针

    TaskSendFrame_preventB = OSQCreate(txBMSCANOSQB, BMSCANOSQ_NUM); // 建立事件(消息队列)

//while(1)
//{
//	OSTimeDlyHMSM(0, 0, 0, 500); //延时500MS让其他任务起来
//	CHM_SendShakeHandFrameB();
//}
	OSTimeDlyHMSM(0, 0, 0, 500); //延时500MS让其他任务起来
    while(1)
    {
        nowSysTime = OSTimeGet();
		
        pMSG = OSQPend(TaskSendFrame_preventB, SYS_DELAY_10ms, &err);
        if(err == OS_ERR_NONE)
        {
            //充电机跟BMS通讯当前状态
            switch((_SEND_BMS_STEP)pMSG->MsgID)
            {
                case BMS_CHM_SEND: //充电机充电握手阶段报文
                case BMS_CRM_SEND: //充电机辨识报文
                    CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //第一次发送函数CHM或者CRM,
                    break;
                case BMS_PARA_SEND: //充电机发送时间同步信息报文及最大输出能力报文
//                    CAN_Send_Table[(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //第一次发送函数,这里需要错开点么？
                    CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //第一次发送函数,
                    break;
				case BMS_CTS_SEND:
					CAN_Send_Table[GUN_B][2].CANSendFrametoBMS();//使用吉利帝豪的旧国标车，如果没有CTS报文，就会报错，这里临时处理一下 
					break;
                case BMS_CRO_SEND:
                case BMS_CRO_UNREADY_SEND:
                case BMS_CCS_SEND:
                case BMS_CST_SEND:
                case BMS_CSD_SEND:
                case BMS_CEM_SEND:    
                    CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //第一次发送函数,
                    break;
				case BMS_CEM_CST_SEND:
					CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //发送CEM
					CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID) + 1].CANSendFrametoBMS();  //发送CST
                    break;
				default:
					break;
            }
            oldSysTime = nowSysTime;//第一次已经发送，先备份发送后的时间。
        }
        else
        {
            for(count = 0; count < CANSENDTALELEN; count++)
            {
                if(BMS_CONNECT_Control[GUN_B].step  == CAN_Send_Table[GUN_B][count].step)//处于什么阶段属性
                {
                    //大于周期值,OSTimeGet值一般已经很大了，大概49天时间了,逻辑上还是要保护下。
                    if((nowSysTime >= oldSysTime) ? ((nowSysTime - oldSysTime) >= CAN_Send_Table[GUN_B][count].cycletime ) : \
                       ((nowSysTime + (INT32U_MAX_NUM - oldSysTime)) >= CAN_Send_Table[GUN_B][count].cycletime) )
                    {
//                        if(!CanFrameControl.state)
                        CAN_Send_Table[GUN_B][count].CANSendFrametoBMS();//发送函数
                        oldSysTime = nowSysTime;
                    }
                }
            }
        }
		for(i=0;i<3;i++)
		{//特殊处理BMV\BMT\BSP
			if(Special_PGN[GUN_B][i].Get_Flag)
			{
				if(Special_PGN[GUN_B][i].Count_Delay>=50)
				{
					Reply_TPCM_EndofMsgAckFrameB();
					Special_PGN[GUN_B][i].Get_Flag=0x00;
				}
				else
				{
					Special_PGN[GUN_B][i].Count_Delay+=1;
				}
			}
		}
    }
}


/***********************************************************************************************
* Function      : TaskConnectBMSMain
* Description   :
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
void TaskConnectBMSAMain(void *pdata)
{
    InitConnectBMSACAN();     //初始化连接BMS管理系统的can
    ClearCanFrameInformation(GUN_A);//初始化清除一下结构体
    OSTimeDlyHMSM(0, 0, 0, SYS_DELAY_500ms); //延时500MS让其他任务起来
    BMS_FrameContextClear(GUN_A);
    BMS_CONNECT_ControlInit(GUN_A);

    OSTaskCreateExt(TaskSendBMSAFrame,
                    (void *)0,
                    &TaskSendBMSFrameStkA[TASK_STK_SIZE_SendFrame - 1],
                    PRI_SEND_BMSA,
                    PRI_SEND_BMSA,
                    &TaskSendBMSFrameStkA[0],
                    TASK_STK_SIZE_SendFrame,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK + OS_TASK_OPT_STK_CLR);
    OSTimeDly(SYS_DELAY_500ms);
    while(1)
    {
        OSTimeDly(SYS_DELAY_100ms);
#if 1       
        if ((GetChargeMode(GUN_A) != MODE_AUTO) && (GetChargeMode(GUN_A) != MODE_VIN))   //非自动模式
        {
            if (BMS_CONNECT_Control[GUN_A].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
            }
            continue;
        }

        if (GetBMSStartCharge(GUN_A) == FALSE) 
        {//收到充电指令
            if (BMS_CONNECT_Control[GUN_A].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
            }
            continue;
        }
		
		if (ChargeBMSSelfCheck(GUN_A) == FALSE)  //枪插入跟电子锁检测
        {
            continue;
        }
#endif

			OSQFlush(TaskConnectBMS_peventA);
			APP_SetGBType(GUN_A,BMS_GB_2015);         //每次默认是新国标
			if (ChargeBMSHandshakeA() == FALSE)//握手
			{//握手超时
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			if (ChargeParameterConfigStepA() == FALSE)//参数配置
			{//参数配置超时
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
		
			if (ChargeRunningStepA() == FALSE) //充电阶段
			{//充电过程超时
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			ChargeEndStepA();            //充电结束阶段
			SetBMSStartCharge(GUN_A,FALSE);
			printf("GUN_A 466656589\r\n");
        //停止发送任何帧
        BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
        BMS_CONNECT_ControlInit(GUN_A);  //重新初始化
    }
}

/***********************************************************************************************
* Function      : TaskConnectBMSMain
* Description   :
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 20101210
***********************************************************************************************/
void TaskConnectBMSBMain(void *pdata)
{
    InitConnectBMSBCAN();     //初始化连接BMS管理系统的can
    ClearCanFrameInformation(GUN_B);//初始化清除一下结构体
    OSTimeDlyHMSM(0, 0, 0, SYS_DELAY_500ms); //延时500MS让其他任务起来
    BMS_FrameContextClear(GUN_B);
    BMS_CONNECT_ControlInit(GUN_B);

    OSTaskCreateExt(TaskSendBMSBFrame,
                    (void *)0,
                    &TaskSendBMSFrameStkB[TASK_STK_SIZE_SendFrame - 1],
                    PRI_SEND_BMSB,
                    PRI_SEND_BMSB,
                    &TaskSendBMSFrameStkB[0],
                    TASK_STK_SIZE_SendFrame,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK + OS_TASK_OPT_STK_CLR);
    OSTimeDly(SYS_DELAY_500ms);
    while(1)
    {
        OSTimeDly(SYS_DELAY_100ms);
#if 1      
        if ((GetChargeMode(GUN_B) != MODE_AUTO) && (GetChargeMode(GUN_B) != MODE_VIN)) 
        {//非自动模式
            if (BMS_CONNECT_Control[GUN_B].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
            }
            continue;
        }
		
        if (GetBMSStartCharge(GUN_B) == FALSE) 
        {//收到充电指令
            if (BMS_CONNECT_Control[GUN_B].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
            }
            continue;
        }
		printf("BMS GUN B Start/r/n");
		if (ChargeBMSSelfCheck(GUN_B) == FALSE)
        {//枪插入跟电子锁检测
            continue;
        }
#endif
		OSQFlush(TaskConnectBMS_peventB);
			APP_SetGBType(GUN_B,BMS_GB_2015);         //每次默认是新国标
			if (ChargeBMSHandshakeB() == FALSE)//握手
			{//握手超时
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			if (ChargeParameterConfigStepB() == FALSE)//参数配置
			{//参数配置超时
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			if (ChargeRunningStepB() == FALSE) //充电阶段
			{//充电过程超时
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			ChargeEndStepB();            //充电结束阶段
			printf("BMS GUN B STOP/r/n");
			SetBMSStartCharge(GUN_B,FALSE);
			        //停止发送任何帧
			BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
			BMS_CONNECT_ControlInit(GUN_B);  //重新初始化
    }
}

/************************(C)COPYRIGHT 2010 汇誉科技*****END OF FILE****************************/
