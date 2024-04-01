/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName          : connectbms.c
* Author              :
* Date First Issued : 10/6/2015
* Version           : V0.1
* Description       : 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
    APP_TIMEOUT = 0xA0,     //��ʱ
    APP_SELFCHECK_SUCCESS,  //ϵͳ�Լ�ɹ�
    APP_RXDEAL_SUCCESS,     //���մ���ɹ�
    APP_END_CHARGING,       //�������
    
    APP_DEFAULT = 0xFF,     //ȱʡ״ֵ̬

} _APP_CONNECT_BMSSTATE;    //����BMS�����е�״̬���
//������PGN��֡��ֻ��Ӧ���۲�������������
#define BMS_BMV_ID    GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMV_PGN_5376)
#define BMS_BMT_ID    GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMT_PGN_5632)
#define BMS_BSP_ID    GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BSP_PGN_5888)

#define TASK_STK_SIZE_SendFrame    0x100

//�������ӿ���
#define TIMEOUT_RECONNECT_NUM      3     //��ʱ�������ӵĴ��� 
/* Private typedef----------------------------------------------------------------------------*/

/* Private macro------------------------------------------------------------------------------*/
BSPCANTxMsg   BMSCANTxbuffA;      //���ͻ���
BSPCANRxMsg   BMSCANRxbuffA[BMSCANRX_BUFF_LEN];//���ջ���
_BSP_MESSAGE  BMSRxMessageA[BMSCANRX_BUFF_LEN];//��Ϣ���ջ���
BSP_CAN_Set  bxBMSCANSetA;


BSPCANTxMsg   BMSCANTxbuffB;      //���ͻ���
BSPCANRxMsg   BMSCANRxbuffB[BMSCANRX_BUFF_LEN];//���ջ���
_BSP_MESSAGE  BMSRxMessageB[BMSCANRX_BUFF_LEN];//��Ϣ���ջ���
BSP_CAN_Set  bxBMSCANSetB;

_BMS_RTS_FRAME_INF  BMS_RTS_FrameInf[GUN_MAX];//BMS���������֡��Ϣ
/* Private variables--------------------------------------------------------------------------*/

OS_EVENT  *TaskConnectBMS_peventA;    //CAN��Ϣ����ָ��
void *bxBMSCANOSQA[BMSCANOSQ_NUM];    //ָ������bxCANOSQ[]����ָ�������Ϣ��ָ��

OS_EVENT  *TaskConnectBMS_peventB;    //CAN��Ϣ����ָ��
void *bxBMSCANOSQB[BMSCANOSQ_NUM];    //ָ������bxCANOSQ[]����ָ�������Ϣ��ָ��

OS_EVENT  *TaskSendFrame_preventA;    //����֡������Ϣ����
OS_EVENT  *TaskSendFrame_preventB;    //����֡������Ϣ����
void *txBMSCANOSQA[BMSCANOSQ_NUM];    //������Ϣ����
void *txBMSCANOSQB[BMSCANOSQ_NUM];    //������Ϣ����

INT8U RxBMSBufferA[256];  //�ݶ�128,���Ը���ʵ�����֡��������
INT8U RxBMSBufferB[256];  //�ݶ�128,���Ը���ʵ�����֡��������

_SPECIAL_PGN	Special_PGN[GUN_MAX][3];

_CANFRAMECONTROL		CanFrameControl[GUN_MAX];
_BMS_CONNECT_CONTROL	BMS_CONNECT_Control[GUN_MAX];//�������̵Ľṹ��
_BMS_ERR_CONTROL 		BMSErrControl[GUN_MAX];

INT8U CRMReadyflag[GUN_MAX] = {0};
_BMS_CUR_GB ChargGB_Type[GUN_MAX] = {BMS_GB_2015};     //Ĭ��2015�¹���
_VIN_CONTROL VinControl[GUN_MAX] = {0};
//�����ϴ���
const _BMS_ERR_HANDLE BMSErrHandleTable[GUN_MAX][BMS_ERR_HANDLE_LEN] =
{
	{
		//{ERR_L1_INSULATION              ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //��Ե����

		//{ERR_L1_LEAKAGE                 ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //©�����

		//{ERR_L1_EMERGENCY               ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //��ͣ����

		//{ERR_L2_CONNECT                 ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //����������

		//{ERR_L2_CONN_TEMP_OVER          ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //BMSԪ�����������������

	   // {ERR_L2_BGROUP_TEMP_OVER        ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //������¶ȹ���

	   // {ERR_L2_BAT_VOL                 ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //�����ص�ѹ���͡�����

	   // {ERR_L2_BMS_VOL_CUR             ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //BMS��⵽��������������ѹ�쳣

	   // {ERR_L2_CHARGE_VOL_CUR          ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //������⵽��������ƥ������ѹ�쳣

		//{ERR_L2_CHARGE_TEMP_OVER        ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //�����ڲ�����

		//{ERR_L2_CHARGE_POWER_UNARRIVAL  ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //�����������ܴ���

	   // {ERR_L2_CAR_CONN_ADHESION       ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //�����Ӵ���ճ��

		//{ERR_L3_CHAGING_TIMEOUT         ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //������ֽ׶Ρ����ý׶Ρ������̳�ʱ

		{ERR_L3_CHAGING_OVER_TIMEOUT    ,   ERR_METHOD_END  ,     ChargeErrHandleA  }, //��������ʱ

		{ERR_LF_METHOD_A                ,   ERR_METHOD_A    ,     ChargeErrHandleA  }, //����A

		{ERR_LF_METHOD_B                ,   ERR_METHOD_B    ,     ChargeErrHandleA  }, //����B

		{ERR_LF_METHOD_C                ,   ERR_METHOD_C    ,     ChargeErrHandleA  }, //����C
	},
	{
		//{ERR_L1_INSULATION              ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //��Ե����

		//{ERR_L1_LEAKAGE                 ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //©�����

		//{ERR_L1_EMERGENCY               ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //��ͣ����

		//{ERR_L2_CONNECT                 ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //����������

		//{ERR_L2_CONN_TEMP_OVER          ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //BMSԪ�����������������

	   // {ERR_L2_BGROUP_TEMP_OVER        ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //������¶ȹ���

	   // {ERR_L2_BAT_VOL                 ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //�����ص�ѹ���͡�����

	   // {ERR_L2_BMS_VOL_CUR             ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //BMS��⵽��������������ѹ�쳣

	   // {ERR_L2_CHARGE_VOL_CUR          ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //������⵽��������ƥ������ѹ�쳣

		//{ERR_L2_CHARGE_TEMP_OVER        ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //�����ڲ�����

		//{ERR_L2_CHARGE_POWER_UNARRIVAL  ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //�����������ܴ���

	   // {ERR_L2_CAR_CONN_ADHESION       ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //�����Ӵ���ճ��

		//{ERR_L3_CHAGING_TIMEOUT         ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //������ֽ׶Ρ����ý׶Ρ������̳�ʱ

		{ERR_L3_CHAGING_OVER_TIMEOUT    ,   ERR_METHOD_END  ,     ChargeErrHandleB  }, //��������ʱ

		{ERR_LF_METHOD_A                ,   ERR_METHOD_A    ,     ChargeErrHandleB  }, //����A

		{ERR_LF_METHOD_B                ,   ERR_METHOD_B    ,     ChargeErrHandleB  }, //����B

		{ERR_LF_METHOD_C                ,   ERR_METHOD_C    ,     ChargeErrHandleB  }, //����C
	}
	
};

//������������͵���Ϣ��
const _RECEIVE_Q_TABLE ReceiveMsgHandleTableA[BMS_MSG_TABLE_LEN] =
{
    {APP_CHARGE_END       ,   APP_ReceiveEndChargeA    }, //�յ���������Ľ�������
};

//������������͵���Ϣ��
const _RECEIVE_Q_TABLE ReceiveMsgHandleTableB[BMS_MSG_TABLE_LEN] =
{
    {APP_CHARGE_END       ,   APP_ReceiveEndChargeB    }, //�յ���������Ľ�������
};

OS_STK TaskSendBMSFrameStkA[TASK_STK_SIZE_SendFrame];     // �����ջ
OS_STK TaskSendBMSFrameStkB[TASK_STK_SIZE_SendFrame];     // �����ջ




const _PROTOCOL_CAN_MANAGE_TABLE  Can_Data_Manage_Table[GUN_MAX][CANRECEIVETABLELEN] =
{
	{
		//ID          //���յ��Ժ���  //���͸�MC�Ĵ���
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236 ),  0, CanProtocolOperationRTSA, NULL}, //J1939����Э�����ӹ����ҳ����֡
		 
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BHM_PGN_9984),  0, CanProtocolOperationBHMA, NULL}, //��������BHM����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BRM_PGN_512 ),  0, CanProtocolOperationBRMA, NULL}, //��������BRM����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCP_PGN_1536),  0, CanProtocolOperationBCPA, NULL}, //�������س�����BCP����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BRO_PGN_2304),  0, CanProtocolOperationBROA, NULL}, //��س��׼������״̬BRO����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BCL_PGN_4096),  0, CanProtocolOperationBCLA, NULL}, //��س������BCL����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCS_PGN_4352),  0, CanProtocolOperationBCSA, NULL}, //��س����״̬BCS����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSM_PGN_4864),  0, CanProtocolOperationBSMA, NULL}, //��������״̬��ϢBSM����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMV_PGN_5376),  0, CanProtocolOperationBMVA, NULL}, //���嶯�����ص�ԴBMV����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMT_PGN_5632),  0, CanProtocolOperationBMTA, NULL}, //���������¶�BMT����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BSP_PGN_5888),  0, CanProtocolOperationBSPA, NULL}, //��������Ԥ��BSP����

		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BST_PGN_6400),  0, CanProtocolOperationBSTA, NULL}, //BMS��ֹ���BST����

		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSD_PGN_7168),  0, CanProtocolOperationBSDA, NULL}, //BMSͳ������BSD����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_2,GB_BEM_PGN_7680),  0, CanProtocolOperationBEMA, NULL}, //BMS����BEM����
	},
	{
		//ID          //���յ��Ժ���  //���͸�MC�Ĵ���
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236 ),  0, CanProtocolOperationRTSB, NULL}, //J1939����Э�����ӹ����ҳ����֡
		 
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BHM_PGN_9984),  0, CanProtocolOperationBHMB, NULL}, //��������BHM����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BRM_PGN_512 ),  0, CanProtocolOperationBRMB, NULL}, //��������BRM����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCP_PGN_1536),  0, CanProtocolOperationBCPB, NULL}, //�������س�����BCP����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BRO_PGN_2304),  0, CanProtocolOperationBROB, NULL}, //��س��׼������״̬BRO����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BCL_PGN_4096),  0, CanProtocolOperationBCLB, NULL}, //��س������BCL����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BCS_PGN_4352),  0, CanProtocolOperationBCSB, NULL}, //��س����״̬BCS����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSM_PGN_4864),  0, CanProtocolOperationBSMB, NULL}, //��������״̬��ϢBSM����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMV_PGN_5376),  0, CanProtocolOperationBMVB, NULL}, //���嶯�����ص�ԴBMV����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BMT_PGN_5632),  0, CanProtocolOperationBMTB, NULL}, //���������¶�BMT����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_BSP_PGN_5888),  0, CanProtocolOperationBSPB, NULL}, //��������Ԥ��BSP����

		{GB_CONFIGRECEIVE_ID(GB_PRIO_4,GB_BST_PGN_6400),  0, CanProtocolOperationBSTB, NULL}, //BMS��ֹ���BST����

		{GB_CONFIGRECEIVE_ID(GB_PRIO_6,GB_BSD_PGN_7168),  0, CanProtocolOperationBSDB, NULL}, //BMSͳ������BSD����
		
		{GB_CONFIGRECEIVE_ID(GB_PRIO_2,GB_BEM_PGN_7680),  0, CanProtocolOperationBEMB, NULL}, //BMS����BEM����
	}
};

_PROTOCOL_CAN_SEND_TABLE  CAN_Send_Table[GUN_MAX][CANSENDTALELEN]=
{
	{
		//������ֽ׶�
		{ BMS_CHM_SEND          ,    SYS_DELAY_250ms ,   CHM_SendShakeHandFrameA         }, // 0
		{ BMS_CRM_SEND          ,    SYS_DELAY_250ms ,   CRM_SendShakeHandIdentifyFrameA }, // 1 
		//���������ý׶�
		{ BMS_CTS_SEND         ,     SYS_DELAY_500ms ,   CTS_SendSynTimeMessageA         }, // 2��������ʱ��ͬ����Ϣ��?
		{ BMS_PARA_SEND         ,    SYS_DELAY_250ms ,   CML_SendMaxVolandCurA           }, // 3������������������
		{ BMS_CRO_SEND          ,    SYS_DELAY_250ms ,   CRO_SendChargeDevStandbyA       }, // 4�������׼������״̬����(0xAA)
		{ BMS_CRO_UNREADY_SEND  ,    SYS_DELAY_250ms ,   CRO_SendChargeDevUnstandbyA     }, // 5�������׼������״̬����(0x00)
		//���׶�
		{ BMS_CCS_SEND          ,    SYS_DELAY_50ms  ,   CCS_DeviceChargeStateA          }, // 6
		{ BMS_CST_SEND          ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeA            }, // 7
		//�������׶� 
		{ BMS_CSD_SEND          ,    SYS_DELAY_250ms ,   CSD_DeviceChargeEndStatisticsA  }, // 8
		//����������
		{ BMS_CEM_SEND          ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrA            }, // 9
		//�����ڴ������ABC�׶�
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrA           }, // 10 CEM
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeA            }, // 11 CST
	},
	{
		//������ֽ׶�
		{ BMS_CHM_SEND          ,    SYS_DELAY_250ms ,   CHM_SendShakeHandFrameB         }, // 0
		{ BMS_CRM_SEND          ,    SYS_DELAY_250ms ,   CRM_SendShakeHandIdentifyFrameB }, // 1 
		//���������ý׶�
		{ BMS_CTS_SEND         ,     SYS_DELAY_500ms ,   CTS_SendSynTimeMessageB         }, // 2��������ʱ��ͬ����Ϣ��?
		{ BMS_PARA_SEND         ,    SYS_DELAY_250ms ,   CML_SendMaxVolandCurB           }, // 3������������������
		{ BMS_CRO_SEND          ,    SYS_DELAY_250ms ,   CRO_SendChargeDevStandbyB      }, // 4�������׼������״̬����(0xAA)
		{ BMS_CRO_UNREADY_SEND  ,    SYS_DELAY_250ms ,   CRO_SendChargeDevUnstandbyB     }, // 5�������׼������״̬����(0x00)
		//���׶�
		{ BMS_CCS_SEND          ,    SYS_DELAY_50ms  ,   CCS_DeviceChargeStateB          }, // 6
		{ BMS_CST_SEND          ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeB            }, // 7
		//�������׶� 
		{ BMS_CSD_SEND          ,    SYS_DELAY_250ms ,   CSD_DeviceChargeEndStatisticsB  }, // 8
		//����������
		{ BMS_CEM_SEND          ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrB            }, // 9
		//�����ڴ������ABC�׶�
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_250ms ,   CEM_DeviceChargeErrB            }, // 10 CEM
		{ BMS_CEM_CST_SEND      ,    SYS_DELAY_10ms  ,   CST_DeviceEndChargeB            }, // 11 CST
	}
};

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function      : Set_VIN_Success
* Description   : VIN�Ƿ��Ȩ�ɹ�  1��ʾ�ɹ� �������ĵ�
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
    TaskConnectBMS_peventA = OSQCreate(bxBMSCANOSQA, BMSCANOSQ_NUM); // �����¼�(��Ϣ����)
    /*��ʼ��can�շ�������*/
    memset(&BMSCANTxbuffA, 0x00, sizeof(BMSCANTxbuffA));
    bxBMSCANSetA.TxMessage =  &BMSCANTxbuffA;
    bxBMSCANSetA.RxMessage =  BMSCANRxbuffA;
    bxBMSCANSetA.BspMsg = BMSRxMessageA;
    bxBMSCANSetA.RxMaxNum = BMSCANRX_BUFF_LEN ;
    bxBMSCANSetA.BaundRate = BSP_CAN_BAUNDRATE_250KBPS; //������Ϊ250K
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
    TaskConnectBMS_peventB = OSQCreate(bxBMSCANOSQB, BMSCANOSQ_NUM); // �����¼�(��Ϣ����)
    /*��ʼ��can�շ�������*/
    memset(&BMSCANTxbuffB, 0x00, sizeof(BMSCANTxbuffB));
    bxBMSCANSetB.TxMessage =  &BMSCANTxbuffB;
    bxBMSCANSetB.RxMessage =  BMSCANRxbuffB;
    bxBMSCANSetB.BspMsg = BMSRxMessageB;
    bxBMSCANSetB.RxMaxNum = BMSCANRX_BUFF_LEN ;
    bxBMSCANSetB.BaundRate = BSP_CAN_BAUNDRATE_250KBPS; //������Ϊ250K
    BSP_CAN_Init(BMS_CAN_B, &bxBMSCANSetB, TaskConnectBMS_peventB);
}
/***********************************************************************************************
* Function      : APP_GetGBType
* Description   : ��ȡ��������
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
* Description   : ����
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
* Description   : �����֡����ṹ����Ƹ���Ԫ��
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 15/1/2015 Ҷϲ��
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
* Description   : ��ȡBHM֡
* Input         : _BMS_BHM_CONTEXT *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016��7��19��  Ҷϲ��
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description	: ��ȡBMS����
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
* Description   : ��ȡBMS����״̬
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��3��  Ҷϲ��
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
* Description   : ����BMS����״̬
* Input         : _BMS_ERR_STATE state
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��3��  Ҷϲ��
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
* Description   : ���������Ϻ���
* Input         : _BMS_ERR_TYPE type
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��2��  Ҷϲ��
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
* Description   : ����CEM��������Ҫ�ĸ���״̬
* Input         : _BMS_CEM_TYPE type
             _BMS_ERR_STATE_TYPE state
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��6��  Ҷϲ��
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
        case CEM_ALL: //������д���
            memset(&BMS_CEM_Context[gun], state, sizeof(_BMS_CEM_CONTEXT));
			//BMS���͵�֡Ԥ����Ҫ����Ϊ1
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
* Description   : ���յ���������Ľ�������
* Input         : void *pdata
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��14��  Ҷϲ��
*****************************************************************************/
INT8U APP_ReceiveEndChargeA(void *pdata)
{
    _SYS_STATE *p;

    if (pdata == NULL)
    {
        return FALSE;
    }
    p = (_SYS_STATE *)pdata;
	//�Լ���󣬶�Ӧ������ʧ�ܣ�������ֹͣ�Ĵ���
//    if (p->TotalErrState & ERR_SELFCHECK_TYPE) //�Լ������ʱδ����
//    {

//    }
#if 0   
    if ( (p->TotalErrState & ERR_CHARGESTOP_TYPE) \
      || (p->TotalErrState & ERR_BMSSTOP_TYPE) \
      || (p->TotalErrState & ERR_USER_STOP_TYPE) \
      || (p->TotalErrState & ERR_REACH_CONDITION_TYPE) )  
#endif
    //������ֹԭ��
    {
        //���Ƴ�����ֹԭ��CST����,����BMS����������ѭ��ɨ�跢�͵�
        memcpy(&BMS_CST_Context[GUN_A], &(p->StopReason.ChargeStop), sizeof(_BMS_CST_CONTEXT));
    }
    if (p->TotalErrState & ERR_CHARGERXTIMEOUT_TYPE) //�������ճ�ʱԭ��
    {
        //���Ƴ�������ԭ��CEM����,����BMS����������ѭ��ɨ�跢�͵�
        memcpy(&BMS_CEM_Context[GUN_A], &(p->StopReason.ChargeErr), sizeof(_BMS_CEM_CONTEXT));
    }
    return TRUE;
}


/*****************************************************************************
* Function      : APP_ReceiveEndChargeB
* Description   : ���յ���������Ľ�������
* Input         : void *pdata
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��14��  Ҷϲ��
*****************************************************************************/
INT8U APP_ReceiveEndChargeB(void *pdata)
{
    _SYS_STATE *p;

    if (pdata == NULL)
    {
        return FALSE;
    }
    p = (_SYS_STATE *)pdata;
	//�Լ���󣬶�Ӧ������ʧ�ܣ�������ֹͣ�Ĵ���
//    if (p->TotalErrState & ERR_SELFCHECK_TYPE) //�Լ������ʱδ����
//    {

//    }
#if 0   
    if ( (p->TotalErrState & ERR_CHARGESTOP_TYPE) \
      || (p->TotalErrState & ERR_BMSSTOP_TYPE) \
      || (p->TotalErrState & ERR_USER_STOP_TYPE) \
      || (p->TotalErrState & ERR_REACH_CONDITION_TYPE) )  
#endif
    //������ֹԭ��
    {
        //���Ƴ�����ֹԭ��CST����,����BMS����������ѭ��ɨ�跢�͵�
        memcpy(&BMS_CST_Context[GUN_B], &(p->StopReason.ChargeStop), sizeof(_BMS_CST_CONTEXT));
    }
    if (p->TotalErrState & ERR_CHARGERXTIMEOUT_TYPE) //�������ճ�ʱԭ��
    {
        //���Ƴ�������ԭ��CEM����,����BMS����������ѭ��ɨ�跢�͵�
        memcpy(&BMS_CEM_Context[GUN_B], &(p->StopReason.ChargeErr), sizeof(_BMS_CEM_CONTEXT));
    }
    return TRUE;
}


/***********************************************************************************************
* Function      : BMS_CONNECT_ControlInit BMS_CONNECT_StateSet
* Description   : ������̿��ƽṹ�塣�����ú���
* Input         :
* Return        :
* Note(s)       :
* Contributor   : 160523    Ҷϲ��
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

    //���ﻹҪ������������Ҫ�Ľṹ���ʼֵ�����������
    //�������õ����״̬���жϣ����Գ�ʼ��Ϊ��Ч״̬
    BMS_BRO_Context[gun].StandbyFlag = GB_BMSSTATE_INVALID;

    //��ʱֱ�ӵ���
    BMSErrControl[gun].ErrState = ERR_STATE_NORMAL;  //û�д���״̬
    BMSErrControl[gun].MethodCRetry = BMS_ERR_RETRY_NUM; //����3��
    BMSErrControl[gun].SelfCheck = TRUE; //��Ҫ�Լ�

    //CEM���д���ΪERR_TYPE_NORMAL
    SetCEMState(gun,CEM_ALL, ERR_TYPE_NORMAL);
    //BEM���д���ΪERR_TYPE_NORMAL
    memset(&BMS_BEM_Context[gun], ERR_TYPE_NORMAL, sizeof(_BMS_BEM_CONTEXT));

}


/*****************************************************************************
* Function      : BMS_FrameContextClear
* Description   : ���BMS���ա�������Ҫ��֡����
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016��7��17��  Ҷϲ��
*****************************************************************************/
void BMS_FrameContextClear(INT8U gun)
{
    //��BMS���͹�����֡����
	if(gun >= GUN_MAX)
	{
		return;
	}
    //��BHM����
    memset((void *)&BMS_BHM_Context[gun].MaxChargeWholeVol, 0, sizeof(_BMS_BHM_CONTEXT));
    //��BRM����
    memset((void *)BMS_BRM_Context[gun].BMSVersion, 0, sizeof(_BMS_BRM_CONTEXT));
    //��BCP����
    memset((void *)&BMS_BCP_Context[gun].UnitBatteryMaxVol, 0, sizeof(_BMS_BCP_CONTEXT));
    //��BRO����
    memset((void *)&BMS_BRO_Context[gun].StandbyFlag, 0, sizeof(_BMS_BRO_CONTEXT));
    //��BCL����
    memset((void *)&BMS_BCL_Context[gun].DemandVol, 0, sizeof(_BMS_BCL_CONTEXT));
    //��BCS����
    memset((void *)&BMS_BCS_Context[gun].ChargeVolMeasureVal, 0, sizeof(_BMS_BCS_CONTEXT));
    //��BSM����
    memset((void *)&BMS_BSM_Context[gun].MaxUnitVolandNum, 0, sizeof(_BMS_BSM_CONTEXT));
    //��BST����
    memset((void *)&BMS_BST_Context[gun].State0.EndChargeReason, 0, sizeof(_BMS_BST_CONTEXT));
    //��BSD����
    memset((void *)&BMS_BSD_Context[gun].EndChargeSOC, 0, sizeof(_BMS_BSD_CONTEXT));
    //��BEM����
    memset((void *)&BMS_BEM_Context[gun].State0.AllBits, 0, sizeof(_BMS_BEM_CONTEXT));
    
    //����׮���͵�֡����
    //CML��������
    memset((void *)&BMS_CML_Context[gun].MaxOutVol, 0, sizeof(_BMS_CML_CONTEXT));
    //�������״̬����
    memset((void *)&BMS_CCS_Context[gun].OutputVol, 0, sizeof(_BMS_CCS_CONTEXT));
    //������ֹ��籨��
    memset((void *)&BMS_CST_Context[gun].State0.EndChargeReason, 0, sizeof(_BMS_CST_CONTEXT));
    //����ͳ�Ʊ�������
    memset((void *)&BMS_CSD_Context[gun].TotalChargeTime, 0, sizeof(_BMS_CSD_CONTEXT));
    //��������ԭ��
    memset((void *)&BMS_CEM_Context[gun].State0.AllBits, 0, sizeof(_BMS_CEM_CONTEXT));
    BMS_CEM_Context[gun].State0.AllBits = BMS_CEM_Context[gun].State0.AllBits|0xFC;
    BMS_CEM_Context[gun].State1.AllBits = BMS_CEM_Context[gun].State1.AllBits|0xF0;
    BMS_CEM_Context[gun].State2.AllBits = BMS_CEM_Context[gun].State2.AllBits|0xC0;
    BMS_CEM_Context[gun].State3.AllBits = BMS_CEM_Context[gun].State3.AllBits|0xFC;
}

/*****************************************************************************
* Function      : BMS_CONNECT_StepSet
* Description   : ֪ͨBMS����������һ֡����
* Input         : _SEND_BMS_STEP step
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��1��  Ҷϲ��
*****************************************************************************/
void BMS_CONNECT_StepSet(INT8U gun,_SEND_BMS_STEP step)
{
	static _BSP_MESSAGE send_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return;
	}

    BMS_CONNECT_Control[gun].step = step;
    //����step�ȷ���һ��
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
    //���õ�ʱ����Զ�����Ȱѵ�ǰ��״̬���ݵ���һ��״̬
    BMS_CONNECT_Control[gun].laststate = BMS_CONNECT_Control[gun].currentstate;
    BMS_CONNECT_Control[gun].currentstate = state;
    //���׶ε����⴦��
    //ȡ�����׶ε�BCL  BCS����

}

/*****************************************************************************
* Function      : BMSErrDeal
* Description   : �յ�BMS�����Ĵ���
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��6��  Ҷϲ��
*****************************************************************************/
void BMSErrDeal(INT8U gun)
{
    static _BSP_MESSAGE send_message[GUN_MAX];
    
	if(gun >= GUN_MAX)
	{
		return ;
	}
    //�ο�GB/T 27930-2015��37ҳ,�յ�BEM������
    //(1)���ͳ���������CEM��CST��BMS
    BMS_CONNECT_StepSet(gun,BMS_CEM_CST_SEND);
	
    SetBMSErrState(gun,ERR_STATE_BMS);             //����BMS���͵Ĵ�����BEM
    
    send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
    send_message[gun].DivNum = APP_BMS_CHARGE_ERR_C;
	send_message[gun].GunNum = gun;
   if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &send_message[gun]);	//��������������Ϣ
	}
	else
	{
		OSQPost(Control_PeventB, &send_message[gun]);	//��������������Ϣ
	}
	OSTimeDly(SYS_DELAY_10ms);
}

/*****************************************************************************
* Function      : APP_DealReceiveData
* Description   : ���������������յ�����Ϣ(��CAN����)
* Input         : _BSP_MESSAGE *pmsg
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��14��  Ҷϲ��
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
* Description   : ���������������յ�����Ϣ(��CAN����)
* Input         : _BSP_MESSAGE *pmsg
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��14��  Ҷϲ��
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
* Description   :  ������񷵻�λ��ֵ
* Input         : DataID---������ID��
* Output        : 0xff-----��ʾû��������
* Note(s)       :
* Contributor   : 23/5/2015 Ҷϲ��
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
* Description   : ����BMS���͹�����֡
* Input         : ExtenID---���յ���CAN��չ֡ID
                  pdata---CAN����������
                  len---CAN�����ݳ���
* Output        :
* Note(s)       :
* Contributor   : 02/07/2015    Ҷϲ��
***********************************************************************************************/
void ReciveBMSFrameA(INT32U ExtenID, INT8U *pdata, INT8U len)
{
    CanInterfaceStruct  parastruct;

    INT32U ID1 = BMS_BMV_ID, ID2 = BMS_BMT_ID , ID3 = BMS_BSP_ID;
    INT32U RTSID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_236) ;//1C EC 56 F4
    //��ҳ����ʱ�����ݽ���(TP.DT)����չ֡
    INT32U pageframeID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_235);//��ҳ�����֡ 1C EB 56 F4

	if (ExtenID == RTSID) //1C EC 56 F4
	{
		if (len < 8)
		{
			return;
		}
		if ( (pdata[0] == 0xff) || (pdata[3] == 0xff) \
		  || ( (pdata[1] == 0xff) && (pdata[2] == 0xff) ) )//ȡ����ǰ���
		{
			memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
	        memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
			return;
		}
	}
	#warning "����������ü����Ժ�һ�㣬   2020"
#if 0
    //������֡���д���������֡��RTS֡�������ò�����
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//����EQ������Ҫ����������0����
		memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //�����RTS֡ 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_A][0].DealwithDataIDFunction(&parastruct))//��ʾ��һ֡���Ǿ���Ķ�֡�ĵ�һ֡��
        {
            CanFrameControl[GUN_A].state = 0x01;//��ʾҪ�����֡������
            CanFrameControl[GUN_A].nextpagenum = parastruct.pdata[3];//�����ж���ҳ,���ܶ���ܰ���
			return;//��������RTS�����˳���
        }
    }
    //���ݶ�֡����ID(1C EB 56 F4)���ж��Ƿ��Ƕ�֡��������
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//�Ƕ�֡����Ľ��մ���
    {//ExtenID����EC��Ҳ����EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_A) == BMS_GB_2015))
		{//20170118 �������� ��ĻBEM����
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,ExtenID);//����ID��

        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //�����굥ֱ֡�ӷ���
            }
        }
    }
    }
    //��֡�Ľ��գ���������
    if((CanFrameControl[GUN_A].state) && (ExtenID == pageframeID))
    {//ExtenIDΪEB����������ݲ���
        CanFrameControl[GUN_A].currentpagenum = pdata[0];//��ǰҳ��

        if((CanFrameControl[GUN_A].currentpagenum <= CanFrameControl[GUN_A].nextpagenum)
           && ((CanFrameControl[GUN_A].pagenum + 1) == CanFrameControl[GUN_A].currentpagenum) )
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//��ص�����Ϣ�����ݶ���
			{
                NOP();
			}
            else  //ֻ������Ч�����ݳ��ȣ���Ҫ��һ���ֽڵĳ�����Ϣ�����һ֡���ܴ��ڵ�0xff��ʱ�������⴦��
			{
                memcpy((RxBMSBufferA + (CanFrameControl[GUN_A].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            CanFrameControl[GUN_A].pagenum += 1;//ʵ�ʻ�ȡ����ҳ��
        }
        else //��֡�����½��գ�CanFrameControl���Բ�ȫ����0
        {
            CanFrameControl[GUN_A].state = 0;
        }
    }
    //��֡����ʱ�����һ֡������Ͻ��룬ʵ�ʴ�����
    if(CanFrameControl[GUN_A].state && (CanFrameControl[GUN_A].pagenum == CanFrameControl[GUN_A].nextpagenum))
    {
        INT32U pgn = 0, contextID = 0;

        //��ȡ��֡��pgn�ţ���һ��ʼ��RTS���Ѿ�������
        pgn |=  BMS_RTS_FrameInf[GUN_A].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[0] << 16);
        //��ȡ��Ӧ�Ķ�֡��չID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//��ҳ��������ȼ�����7
        //�Ȼظ�TP.CM_EndofMsgAck�����������Ϣ�Ѿ�ȫ�����ղ�����
        Reply_TPCM_EndofMsgAckFrameA();
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,contextID);//����ID��
        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferA;
            parastruct.len    = BMS_RTS_FrameInf[GUN_A].messagebytenum ;//RTS�����Ч�ֽ���Ϣ��ʵ���ϲ��ü�1
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
				memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
                memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
            }
        }
    }
#else
	//������֡���д���������֡��RTS֡�������ò�����
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//����EQ������Ҫ����������0����
		memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
        memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); 
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //�����RTS֡ 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_A][0].DealwithDataIDFunction(&parastruct))//��ʾ��һ֡���Ǿ���Ķ�֡�ĵ�һ֡��
        {
            //CanFrameControl.state = 0x01;//��ʾҪ�����֡������
            CanFrameControl[GUN_A].nextpagenum = parastruct.pdata[3];//�����ж���ҳ,���ܶ���ܰ���
			return;//��������RTS�����˳���
        }
    }
    //���ݶ�֡����ID(1C EB 56 F4)���ж��Ƿ��Ƕ�֡��������
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//�Ƕ�֡����Ľ��մ���
    {//ExtenID����EC��Ҳ����EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_A) == BMS_GB_2015))
		{//20170118 �������� ��ĻBEM����
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,ExtenID);//����ID��

        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //�����굥ֱ֡�ӷ���
            }
        }
    }
    }
    //��֡�Ľ��գ���������
   // if((CanFrameControl.state) && (ExtenID == pageframeID))
	if(ExtenID == pageframeID)
    {//ExtenIDΪEB����������ݲ���
        CanFrameControl[GUN_A].currentpagenum = pdata[0];//��ǰҳ��

//        if((CanFrameControl.currentpagenum <= CanFrameControl.nextpagenum)
//           && ((CanFrameControl.pagenum + 1) == CanFrameControl.currentpagenum) )
        if(CanFrameControl[GUN_A].currentpagenum <= CanFrameControl[GUN_A].nextpagenum)
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//��ص�����Ϣ�����ݶ���
			{
                NOP();
			}
            else  //ֻ������Ч�����ݳ��ȣ���Ҫ��һ���ֽڵĳ�����Ϣ�����һ֡���ܴ��ڵ�0xff��ʱ�������⴦��
			{
                memcpy((RxBMSBufferA + (CanFrameControl[GUN_A].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            //CanFrameControl.pagenum += 1;//ʵ�ʻ�ȡ����ҳ��
        }
//        else //��֡�����½��գ�CanFrameControl���Բ�ȫ����0
//        {
//            CanFrameControl.state = 0;
//        }
    }
    //��֡����ʱ�����һ֡������Ͻ��룬ʵ�ʴ�����
   // if(CanFrameControl.state && (CanFrameControl.pagenum == CanFrameControl.nextpagenum))
	if(CanFrameControl[GUN_A].currentpagenum == CanFrameControl[GUN_A].nextpagenum)	
    {
        INT32U pgn = 0, contextID = 0;
		CanFrameControl[GUN_A].currentpagenum = 0;				//���ȷ��ֻ��һ��  20231208
        //��ȡ��֡��pgn�ţ���һ��ʼ��RTS���Ѿ�������
        pgn |=  BMS_RTS_FrameInf[GUN_A].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_A].paranumber[0] << 16);
        //��ȡ��Ӧ�Ķ�֡��չID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//��ҳ��������ȼ�����7
        //�Ȼظ�TP.CM_EndofMsgAck�����������Ϣ�Ѿ�ȫ�����ղ�����
        Reply_TPCM_EndofMsgAckFrameA();
        CanFrameControl[GUN_A].IDtablepos = CanSearchParaManagementTable(GUN_A,contextID);//����ID��
        if(CanFrameControl[GUN_A].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferA;
            parastruct.len    = BMS_RTS_FrameInf[GUN_A].messagebytenum ;//RTS�����Ч�ֽ���Ϣ��ʵ���ϲ��ü�1
            if(Can_Data_Manage_Table[GUN_A][CanFrameControl[GUN_A].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
//				memset(&CanFrameControl[GUN_A], 0x00, sizeof(_CANFRAMECONTROL));
//                memset(&BMS_RTS_FrameInf[GUN_A], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
            }
        }
    }
#endif
}


/***********************************************************************************************
* Function      : ReciveBMSFrame
* Description   : ����BMS���͹�����֡
* Input         : ExtenID---���յ���CAN��չ֡ID
                  pdata---CAN����������
                  len---CAN�����ݳ���
* Output        :
* Note(s)       :
* Contributor   : 02/07/2015    Ҷϲ��
***********************************************************************************************/
void ReciveBMSFrameB(INT32U ExtenID, INT8U *pdata, INT8U len)
{
    CanInterfaceStruct  parastruct;

    INT32U ID1 = BMS_BMV_ID, ID2 = BMS_BMT_ID , ID3 = BMS_BSP_ID;
    INT32U RTSID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_236) ;//1C EC 56 F4
    //��ҳ����ʱ�����ݽ���(TP.DT)����չ֡
    INT32U pageframeID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, GB_RTS_PGN_235);//��ҳ�����֡ 1C EB 56 F4

	if (ExtenID == RTSID) //1C EC 56 F4
	{
		if (len < 8)
		{
			return;
		}
		if ( (pdata[0] == 0xff) || (pdata[3] == 0xff) \
		  || ( (pdata[1] == 0xff) && (pdata[2] == 0xff) ) )//ȡ����ǰ���
		{
			memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
	        memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
			return;
		}
	}
	#warning "����������ü����Ժ�һ�㣬   2020"
#if 0
    //������֡���д���������֡��RTS֡�������ò�����
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//����EQ������Ҫ����������0����
		memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //�����RTS֡ 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_B][0].DealwithDataIDFunction(&parastruct))//��ʾ��һ֡���Ǿ���Ķ�֡�ĵ�һ֡��
        {
            CanFrameControl[GUN_B].state = 0x01;//��ʾҪ�����֡������
            CanFrameControl[GUN_B].nextpagenum = parastruct.pdata[3];//�����ж���ҳ,���ܶ���ܰ���
			return;//��������RTS�����˳���
        }
    }
    //���ݶ�֡����ID(1C EB 56 F4)���ж��Ƿ��Ƕ�֡��������
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//�Ƕ�֡����Ľ��մ���
    {//ExtenID����EC��Ҳ����EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_B) == BMS_GB_2015))
		{//20170118 �������� ��ĻBEM����
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,ExtenID);//����ID��

        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //�����굥ֱ֡�ӷ���
            }
        }
    }
    }
    //��֡�Ľ��գ���������
    if((CanFrameControl[GUN_B].state) && (ExtenID == pageframeID))
    {//ExtenIDΪEB����������ݲ���
        CanFrameControl[GUN_B].currentpagenum = pdata[0];//��ǰҳ��

        if((CanFrameControl[GUN_B].currentpagenum <= CanFrameControl[GUN_B].nextpagenum)
           && ((CanFrameControl[GUN_B].pagenum + 1) == CanFrameControl[GUN_B].currentpagenum) )
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//��ص�����Ϣ�����ݶ���
			{
                NOP();
			}
            else  //ֻ������Ч�����ݳ��ȣ���Ҫ��һ���ֽڵĳ�����Ϣ�����һ֡���ܴ��ڵ�0xff��ʱ�������⴦��
			{
                memcpy((RxBMSBufferB + (CanFrameControl[GUN_B].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            CanFrameControl[GUN_B].pagenum += 1;//ʵ�ʻ�ȡ����ҳ��
        }
        else //��֡�����½��գ�CanFrameControl���Բ�ȫ����0
        {
            CanFrameControl[GUN_B].state = 0;
        }
    }
    //��֡����ʱ�����һ֡������Ͻ��룬ʵ�ʴ�����
    if(CanFrameControl[GUN_B].state && (CanFrameControl[GUN_B].pagenum == CanFrameControl[GUN_B].nextpagenum))
    {
        INT32U pgn = 0, contextID = 0;

        //��ȡ��֡��pgn�ţ���һ��ʼ��RTS���Ѿ�������
        pgn |=  BMS_RTS_FrameInf[GUN_B].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[0] << 16);
        //��ȡ��Ӧ�Ķ�֡��չID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//��ҳ��������ȼ�����7
        //�Ȼظ�TP.CM_EndofMsgAck�����������Ϣ�Ѿ�ȫ�����ղ�����
        Reply_TPCM_EndofMsgAckFrameB();
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,contextID);//����ID��
        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferB;
            parastruct.len    = BMS_RTS_FrameInf[GUN_B].messagebytenum ;//RTS�����Ч�ֽ���Ϣ��ʵ���ϲ��ü�1
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
                memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
            }
        }
    }
#else
	//������֡���д���������֡��RTS֡�������ò�����
    //if ( (!CanFrameControl.state) && (ExtenID == RTSID))
	if ( ExtenID == RTSID )    //1C EC 56 F4
    {
		//����EQ������Ҫ����������0����
		memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
 //       memset(&RxBMSBuffer, 0x00, sizeof(RxBMSBuffer));
		
        //�����RTS֡ 1C EC 56 F4
        parastruct.DataID = ExtenID;
        parastruct.pdata = pdata;
        parastruct.len = len;
        if(Can_Data_Manage_Table[GUN_B][0].DealwithDataIDFunction(&parastruct))//��ʾ��һ֡���Ǿ���Ķ�֡�ĵ�һ֡��
        {
            //CanFrameControl.state = 0x01;//��ʾҪ�����֡������
            CanFrameControl[GUN_B].nextpagenum = parastruct.pdata[3];//�����ж���ҳ,���ܶ���ܰ���
			return;//��������RTS�����˳���
        }
    }
    //���ݶ�֡����ID(1C EB 56 F4)���ж��Ƿ��Ƕ�֡��������
    else if( (pageframeID != ExtenID) && (RTSID != ExtenID) )//�Ƕ�֡����Ľ��մ���
    {//ExtenID����EC��Ҳ����EB
		if((ExtenID!=0x081E56F4)||(APP_GetGBType(GUN_B) == BMS_GB_2015))
		{//20170118 �������� ��ĻBEM����
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,ExtenID);//����ID��

        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = pdata;
            parastruct.len    = len;
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
                return; //�����굥ֱ֡�ӷ���
            }
        }
        }
    }
    //��֡�Ľ��գ���������
   // if((CanFrameControl.state) && (ExtenID == pageframeID))
	if(ExtenID == pageframeID)
    {//ExtenIDΪEB����������ݲ���
        CanFrameControl[GUN_B].currentpagenum = pdata[0];//��ǰҳ��

//        if((CanFrameControl.currentpagenum <= CanFrameControl.nextpagenum)
//           && ((CanFrameControl.pagenum + 1) == CanFrameControl.currentpagenum) )
        if(CanFrameControl[GUN_B].currentpagenum <= CanFrameControl[GUN_B].nextpagenum)
        {
            if((ExtenID == ID1) || (ExtenID == ID2) || (ExtenID == ID3))//��ص�����Ϣ�����ݶ���
			{
                NOP();
			}
            else  //ֻ������Ч�����ݳ��ȣ���Ҫ��һ���ֽڵĳ�����Ϣ�����һ֡���ܴ��ڵ�0xff��ʱ�������⴦��
			{
                memcpy((RxBMSBufferB + (CanFrameControl[GUN_B].currentpagenum - 1) * (len - 1) ), &pdata[1], (len - 1));
			}
            //CanFrameControl.pagenum += 1;//ʵ�ʻ�ȡ����ҳ��
        }
//        else //��֡�����½��գ�CanFrameControl���Բ�ȫ����0
//        {
//            CanFrameControl.state = 0;
//        }
    }
    //��֡����ʱ�����һ֡������Ͻ��룬ʵ�ʴ�����
   // if(CanFrameControl.state && (CanFrameControl.pagenum == CanFrameControl.nextpagenum))
	if(CanFrameControl[GUN_B].currentpagenum == CanFrameControl[GUN_B].nextpagenum)	
    {
        INT32U pgn = 0, contextID = 0;
		CanFrameControl[GUN_B].currentpagenum = 0;				//���ȷ��ֻ��һ��  20231208
        //��ȡ��֡��pgn�ţ���һ��ʼ��RTS���Ѿ�������
        pgn |=  BMS_RTS_FrameInf[GUN_B].paranumber[2] ;
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[1] << 8 );
        pgn |=  (BMS_RTS_FrameInf[GUN_B].paranumber[0] << 16);
        //��ȡ��Ӧ�Ķ�֡��չID
        contextID = GB_CONFIGRECEIVE_ID(GB_PRIO_7, pgn);//��ҳ��������ȼ�����7
        //�Ȼظ�TP.CM_EndofMsgAck�����������Ϣ�Ѿ�ȫ�����ղ�����
        Reply_TPCM_EndofMsgAckFrameB();
        CanFrameControl[GUN_B].IDtablepos = CanSearchParaManagementTable(GUN_B,contextID);//����ID��
        if(CanFrameControl[GUN_B].IDtablepos < CANRECEIVETABLELEN)
        {
            parastruct.DataID = ExtenID;
            parastruct.pdata  = RxBMSBufferB;
            parastruct.len    = BMS_RTS_FrameInf[GUN_B].messagebytenum ;//RTS�����Ч�ֽ���Ϣ��ʵ���ϲ��ü�1
            if(Can_Data_Manage_Table[GUN_B][CanFrameControl[GUN_B].IDtablepos].DealwithDataIDFunction(&parastruct))
            {
//               memset(&CanFrameControl[GUN_B], 0x00, sizeof(_CANFRAMECONTROL));
//                memset(&BMS_RTS_FrameInf[GUN_B], 0x00, sizeof(_BMS_RTS_FRAME_INF)); //һ��RTS���������
            }
        }
    }
#endif
}

/******************************************************************************
* Function Name :WaitConnectBMSmessage
* Description   : �ȵ���Ϣ�Ĵ����������������еĵȴ���Ϣ���ƶ��ô˺�������
* Input         : time, �ȴ��ĳ�ʱʱ�䣬��д1ms�ı�����
* Output        :
* Contributor       :
* Date First Issued :25/09/2008
******************************************************************************/
_APP_CONNECT_BMSSTATE WaitConnectBMSmessageA(INT32U time)
{
    INT8U err = 0;
    INT8U offset = 0;

    _BSP_MESSAGE *pMSG;   // ������Ϣָ��

    pMSG = OSQPend(TaskConnectBMS_peventA, time, &err);
    if(err == OS_ERR_NONE)
    {
        switch(pMSG->MsgID)
        {
            case BSP_MSGID_CAN_RXOVER:  // �������
				{
					offset = pMSG->DivNum;
					ReciveBMSFrameA(bxBMSCANSetA.RxMessage[offset].ExtId, \
								   bxBMSCANSetA.RxMessage[offset].Data, \
								   bxBMSCANSetA.RxMessage[offset].DLC);
					return APP_RXDEAL_SUCCESS;
				}
            case BSP_MSGID_CONTROL://�յ������������Ϣ
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
* Description   : �ȵ���Ϣ�Ĵ����������������еĵȴ���Ϣ���ƶ��ô˺�������
* Input         : time, �ȴ��ĳ�ʱʱ�䣬��д1ms�ı�����
* Output        :
* Contributor       :
* Date First Issued :25/09/2008
******************************************************************************/
_APP_CONNECT_BMSSTATE WaitConnectBMSmessageB(INT32U time)
{
    INT8U err = 0;
    INT8U offset = 0;

    _BSP_MESSAGE *pMSG;   // ������Ϣָ��

    pMSG = OSQPend(TaskConnectBMS_peventB, time, &err);
    if(err == OS_ERR_NONE)
    {
        switch(pMSG->MsgID)
        {
            case BSP_MSGID_CAN_RXOVER:  // �������
				{
					offset = pMSG->DivNum;
					ReciveBMSFrameB(bxBMSCANSetB.RxMessage[offset].ExtId, \
								   bxBMSCANSetB.RxMessage[offset].Data, \
								   bxBMSCANSetB.RxMessage[offset].DLC);
					return APP_RXDEAL_SUCCESS;
				}
            case BSP_MSGID_CONTROL://�յ������������Ϣ
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
* Description   : �����������BMS�Ƿ�ɹ����ɹ�����������ֽ׶Ρ�
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
    {//�Լ�ɹ�
        return TRUE;
    }
    else
    {//�Լ�ʧ��
        SetBMSSelfCheck(gun,TRUE); //��Ҫ���ǹ�����Լ�
        SetBMSInsulationCheck(gun,TRUE); //��Ҫ�ٴν��о�Ե���
        return FALSE;
    }
}

/*****************************************************************************
* Function      : ChargeErrMethodA
* Description   : ���ϴ���ʽA(��������ͣ��ͣ�ã��ȴ�רҵά����Աά��)
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��2��  Ҷϲ��
*****************************************************************************/
static INT8U ChargeErrMethodA(INT8U gun)
{
	static _BSP_MESSAGE senderr_message[GUN_MAX];
	
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	//���ʱ������������ϼ���3
	senderr_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	senderr_message[gun].DivNum = APP_BMS_CHARGE_ERR_A;	 //���ܾ���ԭ��ֱ�ӷ���ֹͣ���һ���Ը�λϵͳ
	senderr_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &senderr_message[gun]);	//��������������Ϣ
	}
	else
	{
		OSQPost(Control_PeventB, &senderr_message[gun]);	//��������������Ϣ
	}
	OSTimeDly(SYS_DELAY_20ms);
    return TRUE;
}

/*****************************************************************************
* Function      : ChargeErrMethodB
* Description   : ���ϴ���ʽB(ֹͣ���γ�磬���潻�׼�¼�������²��ǹ��
                  �ſ��Խ�����һ�γ��)
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��2��  Ҷϲ��
*****************************************************************************/
static INT8U ChargeErrMethodB(INT8U gun)
{
	static _BSP_MESSAGE senderr_message[GUN_MAX];

	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    //�ǳ��س�������װ�÷���3��ͨѶ��ʱ��ȷ��ͨѶ�жϣ�
    //��ǳ��س���ֹͣ��磬Ӧ��10s�ڶϿ�K1��K2��K3��K4��K5��K6
	if((BMS_CEM_Context[gun].State0.AllBits != 0xFC) || (BMS_CEM_Context[gun].State1.AllBits != 0xF0) || \
		(BMS_CEM_Context[gun].State2.AllBits != 0xC0) || (BMS_CEM_Context[gun].State3.AllBits != 0xFC))
	{
		//�����г�ʱ�ŷ�CEM
		BMS_CONNECT_StepSet(gun,BMS_CEM_SEND);    //����CEM
	}
	SetBMSErrState(gun,ERR_STATE_METHOD_B);   //���ô���B����״̬
	
	//���ʱ������������ϼ���3
	senderr_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	senderr_message[gun].DivNum = APP_BMS_CHARGE_ERR_B;	 //���ܾ���ԭ��ֱ�ӷ���ֹͣ���һ���Ը�λϵͳ
	senderr_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &senderr_message[gun]);	//��������������Ϣ
	}
	else
	{
		OSQPost(Control_PeventB, &senderr_message[gun]);	//��������������Ϣ
	}
	OSTimeDly(SYS_DELAY_20ms);
	printf("GUN = %d,Error B\r\n",gun);
    return TRUE;
}

/*****************************************************************************
* Function      : ChargeErrMethodC
* Description   : ���ϴ���ʽC����ֹ��磬�����������ų����Զ��ָ���磬��
                  �⵽����״̬�Ӵ�����Ҫͨ�����ֿ�ʼ���
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��2��  Ҷϲ��
*****************************************************************************/
static INT8U ChargeErrMethodC(INT8U gun)
{
	static _BSP_MESSAGE senderr_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    //(1)���ͳ���������CEM��CST��BMS
	BMS_CONNECT_StepSet(gun,BMS_CEM_SEND); //����CEM���ɣ�����Ҫ����CST
    SetBMSErrState(gun,ERR_STATE_METHOD_C);             //���ô���C����
    //����GB/T 18487.1��B.3.7��39ҳ�涨��
    //�ڳ������У��ǳ��س�������װ���緢��ͨѶ��ʱ����ǳ��س���ֹͣ��磬
    //Ӧ��10s�ڶϿ�K1��K2��K5��K6;

	//����������Ҫ����C���ϴ���ʽ
	senderr_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	senderr_message[gun].DivNum = APP_BMS_CHARGE_ERR_C;	 //����C����ʽ
	senderr_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &senderr_message[gun]);	//��������������Ϣ
	}
	else
	{
		OSQPost(Control_PeventB, &senderr_message[gun]);	//��������������Ϣ
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
			SetBMSErrState(GUN_A,ERR_STATE_METHOD_A);             //����A����״̬
            ChargeErrMethodA(GUN_A);
            break;
        case ERR_METHOD_B: //���ٽ������ԣ���Ҫ���ǹ�Ž�������
            ChargeErrMethodB(GUN_A);
            break;
        case ERR_METHOD_C: //����3�α��B
        case ERR_METHOD_BMS: //�յ�BMS���͵�BEM
            //����GB/T 18487.1��B.3.7��39ҳ�涨��
            //�ڳ������У��ǳ��س�������װ���緢��ͨѶ��ʱ����ǳ��س���ֹͣ��磬
            //Ӧ��10s�ڶϿ�K1��K2��K5��K6;
            ChargeErrMethodC(GUN_A);
            //�ο�GB/T 17930-2015��36ҳ
            break;
        case ERR_METHOD_END:
            SetBMSErrMethodRetry(GUN_A,ERR_RETRY_CLEAR);    //ֱ�ӽ����Ĺ��ϲ�������3��
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
			SetBMSErrState(GUN_B,ERR_STATE_METHOD_A);             //����A����״̬
            ChargeErrMethodA(GUN_B);
            break;
        case ERR_METHOD_B: //���ٽ������ԣ���Ҫ���ǹ�Ž�������
            ChargeErrMethodB(GUN_B);
            break;
        case ERR_METHOD_C: //����3�α��B
        case ERR_METHOD_BMS: //�յ�BMS���͵�BEM
            //����GB/T 18487.1��B.3.7��39ҳ�涨��
            //�ڳ������У��ǳ��س�������װ���緢��ͨѶ��ʱ����ǳ��س���ֹͣ��磬
            //Ӧ��10s�ڶϿ�K1��K2��K5��K6;
            ChargeErrMethodC(GUN_B);
            //�ο�GB/T 17930-2015��36ҳ
            break;
        case ERR_METHOD_END:
            SetBMSErrMethodRetry(GUN_B,ERR_RETRY_CLEAR);    //ֱ�ӽ����Ĺ��ϲ�������3��
            ChargeErrMethodB(GUN_B);
            break;
        default:
            break;
    }
    return TRUE;
}


/*****************************************************************************
* Function      : ChargeErrDeal
* Description   : ���������Ϻ���
* Input         : _BMS_ERR_TYPE type
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��2��  Ҷϲ��
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
* Description   : ��ʱ�������B����ԭ����
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��3��  Ҷϲ��
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
		//Ŀǰû������00��ʱ ����AA��ʱ
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
	if(APP_Get_ERR_Branch(gun) == STOP_OTHERERR)  //�˴������������ϣ�Ĭ��δ�������ϣ����϶�������
	{
		APP_Set_ERR_Branch(gun,STOP_CCUBSMERR);
	}


}
/*****************************************************************************
* Function      : ChargeErrSwitch
* Description   : ������Ϸ�ʽ�л�
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��3��  Ҷϲ��
*****************************************************************************/
void ChargeErrSwitch(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    if (APP_GetGBType(gun) == BMS_GB_2015)  //�¹��� 
    {
        _BMS_ERR_STATE state;

		SetDealFaultOverState(gun,FALSE);
		state = GetBMSErrState(gun);//�ж��Ƿ��ǹ���C�����յ�BEM������
		if ( (state == ERR_STATE_METHOD_C) || (state == ERR_STATE_BMS) )
		{//�ǹ���C���ж��Ƿ�ʱ3����
			if (SetBMSErrMethodRetry(gun,ERR_RETRY_DEC) == 0)
			{//3�γ�ʱ���������B����
				SetBMSConnectStep(gun,BMS_CONNECT_END);
				//CEMϸ��
				Dispose_TimeoutErrB(gun);		//����ϸ��
				ChargeErrDeal(gun,ERR_LF_METHOD_B);
				OSTimeDly(SYS_DELAY_1s);
			}
			else
			{
				ChargeErrDeal(gun,ERR_LF_METHOD_C);
			}
		}
		else 
		{//���ǹ���C
			ChargeErrDeal(gun,ERR_LF_METHOD_C);//��һ�γ��ֳ�ʱ���������C��ʱ����
        }
    }
    else
    {
        static _BSP_MESSAGE send_message[GUN_MAX];
        SetDealFaultOverState(gun,FALSE);
        BMS_CONNECT_StepSet(gun,BMS_CEM_SEND); //����CEM����
		printf("GB2011 Into CEM\r\n");
        OSTimeDly(SYS_DELAY_1s);
		Dispose_TimeoutErrB(gun);    //��Ҫ���й���ϸ��
        send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message[gun].DivNum = APP_CHARGE_END;  //ֹͣ�������
		NB_WriterReason(gun,"E6",2);
        send_message[gun].GunNum = gun;
        if(gun == GUN_A)
		{
			OSQPost(Control_PeventA, &send_message[gun]);	//��������������Ϣ
		}
		else
		{
			OSQPost(Control_PeventB, &send_message[gun]);	//��������������Ϣ
		}
        OSTimeDly(SYS_DELAY_10ms);
		printf("CEM Send\r\n");
    }
    BMS_TIMEOUT_ENTER(gun,BMS_FAULT_SETP, SYS_DELAY_30s); //�����˴�����ϵĳ�ʱ�ж�
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
* Description   : �Լ캯����ÿ��1�����һ��
* Input         : void
* Output        : None
* Note(s)       :
* Contributor   : 2016��6��3��  Ҷϲ��
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
	// OSTimeDly(SYS_DELAY_100ms); //��ʱ���
    //�ж��Ƿ���Ҫ�Լ�
    if (GetBMSSelfCheck(gun) == TRUE) 
    {//��Ҫ�Լ�
        if (GetGunState(gun) == GUN_CONNECTED) 
        {//����1Ϊ4V��˵��ǹ�Ѿ���ȫ������
            //��Ӳ������ȫ��������������ȥʵ�֣�ͨ����Ϣ���ݵķ�ʽ�Ƿ��б�Ҫ
            //��ʱ��ʹ����Ϣ�����ݵķ�ʽȥ����/����
			//if((BCU_LOCK.LockCMD==ELEC_LOCKED)&&(GetChargeMode() == MODE_AUTO))
			if((GetChargeMode(gun) == MODE_AUTO) || (GetChargeMode(gun) == MODE_VIN))
			{//�յ����ָ������յ���������������
				BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
				while(1)
				{
					if (GetElecLockState(gun) == ELEC_LOCK_UNLOCKED) //δ����
					{//5�볬ʱ
						OSTimeDly(SYS_DELAY_250ms);
						if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s) == BMS_TIME_OUT)
						{
							send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
							NB_WriterReason(gun,"F29",3);
							send_message[gun].DivNum = APP_CHARGE_FAIL;		//���ܾ���ԭ��ֱ�ӷ���ֹͣ���һ���Ը�λϵͳ
							send_message[gun].GunNum = gun;
							if(gun == GUN_A)
							{
								OSQPost(Control_PeventA, &send_message[gun]);	//��������������Ϣ
							}
							else
							{
								OSQPost(Control_PeventB, &send_message[gun]);	//��������������Ϣ
							}
							OSTimeDly(SYS_DELAY_50ms);
							SetStartFailType(gun,ENDFAIL_ELECLOCKERR);
							SysState[gun].SelfCheckState.NoErr = STATE_ERR;
							
							send_messagedis[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
							NB_WriterReason(gun,"F28",3);
							send_messagedis[gun].DivNum = APP_CHARGE_FAIL; //����ʧ��
							send_messagedis[gun].GunNum = gun;
							OSQPost(pdisevent, &send_messagedis[gun]);
							OSTimeDly(SYS_DELAY_10ms);
							printf("2111\r\n");
							//����������
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
            //�жϵ������Ƿ����
            return ( (GetElecLockState(gun) == ELEC_LOCK_UNLOCKED) ? FALSE : TRUE);
        }
        else //ǹδ������
        {
            return FALSE;
        }
    }
    else //����Ҫ�Լ�
    {
        return TRUE;
    }
}
/***********************************************************************************************
* Function      : ChargeBMSHandshakeA
* Description   : ������ֽ׶Σ����̲ο�����ͼA.1
* Input         :
* Output        :
* Note(s)       : CRM���ģ�PGN=256 P=6
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeBMSHandshakeA(void)
{
    static _BSP_MESSAGE send_message9, send_message10;
	OS_EVENT* pvinevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

	SetBMSConnectStep(GUN_A,BMS_CONNECT_HANDSHARE);
//#if (BMS_CUR_GB == BMS_NEW_GB2015)  //�¹��� 
	//CHM��������Ҫ��1�������¾ɹ���2����Ե�����Ҫ����BHMѡ�������ѹ
	//�����Ե��ⲻ��Ҫ����CHMҲ�Ͳ���Ҫ��
	SetBMSInsulationCheck(GUN_A,TRUE);//������¹��걾���ǲ���Ҫ�ģ�����Ϊ���Ͼ�����ʹ��
	if (GetBMSInsulationCheck(GUN_A) == TRUE) //��Ҫ��Ե���
	{
    	BMS_TIMEOUT_ENTER(GUN_A,BMS_BHM_STEP, SYS_DELAY_5s);
    	//����������Cʱ��ֱ�Ӵ����ֱ�ʶ�׶ο�ʼ
    	if (GetBMSErrState(GUN_A) != ERR_STATE_METHOD_C)
	    {//���ò���Ϊ����������CHM����STEP
	        BMS_CONNECT_StepSet(GUN_A,BMS_CHM_SEND); //����CHM��BMS
	        while(1)
	        {
				if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE)
				{//�����ִ���з�����ϵͳ���ϣ�������
					return FALSE;
				}
	            //�ȴ�����250ms
	            result = WaitConnectBMSmessageA(SYS_DELAY_250ms);
	            if(result == APP_RXDEAL_SUCCESS)//���յ�����
	            {
	                if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BHM_SUCCESS)//BHM�Ѿ�ȫ������
	                {
	                    APP_SetGBType(GUN_A,BMS_GB_2015);
						printf("SETGB2015\r\n");
                        break;//������ǰѭ��
	                }
	            }
	            //CHM��ʱ�󣬲���Ҫ����ʱ����ֱ�ӽ�������ʶ��׶�
	            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BHM_STEP, SYS_DELAY_5s) )
	            {
	                APP_SetGBType(GUN_A,BMS_GB_2011);
					printf("SETGB2011\r\n");
                    break;
	            }
	        }
			
//			APP_SetGBType(BMS_GB_2011);//20161004 �¾ɹ��� Test
            if(APP_GetGBType(GUN_A) == BMS_GB_2015)
            {
                //���Է�����ĳЩ�������ִ˱�־����FALSE
				printf("GB2015\r\n");
                SetBMSInsulationCheck(GUN_A,TRUE);
                //��ʼ��Ե��⣬������Ϣ����������
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_A;
                OSQPost(Control_PeventA, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_A)) //�ȴ���Ե������
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //��Ե�������5������ɣ���߷ſ�����Ϊ10��
                    if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //��Ե���ʧ��
                        SetBcuRemoteSignalState(GUN_A,(INT8U)REMOTE_SIGNALE_2, 4, 1);
              
                        //���ʧ�ܣ���Ե���ʧ��
                        SysState[GUN_A].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_A].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_A,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                        send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
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
                //�жϾ�Ե����Ƿ�ɹ�
				if (CheckHWConnectforBMS(GUN_A) == FALSE)
                {
                    //��Ե���ʧ��
                    //SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
					//SetStartFailType(ENDFAIL_INSOLUTIONERR);
                    //SysState.SelfCheckState.Insulation = STATE_ERR;
                    //SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
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
            {//��ʼ��Ե��⣬������Ϣ����������
//				BSP_RLCClose(BSPRLC_BMS_POWER);
				printf("GB2011\r\n");
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_A;
                OSQPost(Control_PeventA, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_A)) //�ȴ���Ե������
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //��Ե�������5������ɣ���߷ſ�����Ϊ10��
                    if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //��Ե���ʧ��
                        SetBcuRemoteSignalState(GUN_A,(INT8U)REMOTE_SIGNALE_2, 4, 1);
                        //���ʧ�ܣ���Ե���ʧ��
                        SysState[GUN_A].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_A].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_A,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                        send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
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
                //�жϾ�Ե����Ƿ�ɹ�
                if (CheckHWConnectforBMS(GUN_A) == FALSE)
                {
//                    //��Ե���ʧ��
//                  SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
//                   
//					SetStartFailType(ENDFAIL_INSOLUTIONERR);
//                  SysState.SelfCheckState.Insulation = STATE_ERR;
//                  SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
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
    BMS_CONNECT_StepSet(GUN_A,BMS_CRM_SEND);//����CRM֡
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BRM_STEP, SYS_DELAY_5s);
	//CEM����ȫ������״̬��ʼ��
	SetCEMState(GUN_A,CEM_ALL, ERR_TYPE_NORMAL);
    //�յ�RTS֡�����ظ�RTS֡��ͽ����˵ȴ����ձ�ʶ���ĵĹ��̣�
    while(1)
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE )
		{//�����ִ���з�����ϵͳ���ϣ�������
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
        result = WaitConnectBMSmessageA(SYS_DELAY_10ms);//�౨�İ�10ms�������
        if(result == APP_RXDEAL_SUCCESS)//���յ�����
        {
            if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BRM_SUCCESS)//BRM�Ѿ�ȫ������
            {
                BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;
                result = APP_DEFAULT;
				CRMReadyflag[GUN_A] = 1;
				//����CEM״̬Ϊ����
				SetCEMState(GUN_A,CEM_BRM, ERR_TYPE_NORMAL);
                break;//������ǰѭ��
            }
            //�ж�BRM�����Ƿ�ʱ����Ҫ��CRM���ͺ�5���ڽ��յ�
            else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BRM_STEP, SYS_DELAY_5s))
            {
                BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;
                //���ý���BCP��ʱ״̬
                SetCEMState(GUN_A,CEM_BRM, ERR_TYPE_TIMEOUT);
                //���볬ʱ����
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E45",3);
				printf("GUN_A 1\r\n");
                CRMReadyflag[GUN_A] = 0;
                return FALSE;
            }
        }
        //�ж�BRM�����Ƿ�ʱ����Ҫ��CRM���ͺ�5���ڽ��յ�
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BRM_STEP, SYS_DELAY_5s))
        {
        	//���ý���BCP��ʱ״̬
            SetCEMState(GUN_A,CEM_BRM, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
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
* Description   : ������ֽ׶Σ����̲ο�����ͼA.1
* Input         :
* Output        :
* Note(s)       : CRM���ģ�PGN=256 P=6
* Contributor   : 20101210
***********************************************************************************************/
INT8U ChargeBMSHandshakeB(void)
{
 static _BSP_MESSAGE send_message9, send_message10;
	OS_EVENT* pvinevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg;
    _APP_CONNECT_BMSSTATE result = APP_DEFAULT;

	SetBMSConnectStep(GUN_B,BMS_CONNECT_HANDSHARE);
//#if (BMS_CUR_GB == BMS_NEW_GB2015)  //�¹��� 
	//CHM��������Ҫ��1�������¾ɹ���2����Ե�����Ҫ����BHMѡ�������ѹ
	//�����Ե��ⲻ��Ҫ����CHMҲ�Ͳ���Ҫ��
	SetBMSInsulationCheck(GUN_B,TRUE);//������¹��걾���ǲ���Ҫ�ģ�����Ϊ���Ͼ�����ʹ��
	if (GetBMSInsulationCheck(GUN_B) == TRUE) //��Ҫ��Ե���
	{
    	BMS_TIMEOUT_ENTER(GUN_B,BMS_BHM_STEP, SYS_DELAY_5s);
    	//����������Cʱ��ֱ�Ӵ����ֱ�ʶ�׶ο�ʼ
    	if (GetBMSErrState(GUN_B) != ERR_STATE_METHOD_C)
	    {//���ò���Ϊ����������CHM����STEP
	        BMS_CONNECT_StepSet(GUN_B,BMS_CHM_SEND); //����CHM��BMS
	        while(1)
	        {
				if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE)
				{//�����ִ���з�����ϵͳ���ϣ�������
					return FALSE;
				}
	            //�ȴ�����250ms
	            result = WaitConnectBMSmessageB(SYS_DELAY_250ms);
	            if(result == APP_RXDEAL_SUCCESS)//���յ�����
	            {
	                if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BHM_SUCCESS)//BHM�Ѿ�ȫ������
	                {
	                    APP_SetGBType(GUN_B,BMS_GB_2015);
						printf("SETGB2015\r\n");
                        break;//������ǰѭ��
	                }
	            }
	            //CHM��ʱ�󣬲���Ҫ����ʱ����ֱ�ӽ�������ʶ��׶�
	            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BHM_STEP, SYS_DELAY_5s) )
	            {
	                APP_SetGBType(GUN_B,BMS_GB_2011);
					printf("SETGB2011\r\n");
                    break;
	            }
	        }
			
//			APP_SetGBType(BMS_GB_2011);//20161004 �¾ɹ��� Test
            if(APP_GetGBType(GUN_B) == BMS_GB_2015)
            {
                //���Է�����ĳЩ�������ִ˱�־����FALSE
				printf("GB2015\r\n");
                SetBMSInsulationCheck(GUN_B,TRUE);
                //��ʼ��Ե��⣬������Ϣ����������
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_B;
                OSQPost(Control_PeventB, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_B)) //�ȴ���Ե������
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //��Ե�������5������ɣ���߷ſ�����Ϊ10��
                    if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //��Ե���ʧ��
                        SetBcuRemoteSignalState(GUN_B,(INT8U)REMOTE_SIGNALE_2, 4, 1);
              
                        //���ʧ�ܣ���Ե���ʧ��
                        SysState[GUN_B].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_B].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_B,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                        send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
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
                //�жϾ�Ե����Ƿ�ɹ�
				if (CheckHWConnectforBMS(GUN_B) == FALSE)   //�ڲ����Դ��������ط�д
                {
                    //��Ե���ʧ��
                    //SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
					//SetStartFailType(ENDFAIL_INSOLUTIONERR);
                    //SysState.SelfCheckState.Insulation = STATE_ERR;
                    //SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
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
            {//��ʼ��Ե��⣬������Ϣ����������
//				BSP_RLCClose(BSPRLC_BMS_POWER);
				printf("GB2011\r\n");
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
                send_message9.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message9.DivNum = APP_INSULATIONG_CHECK;
				send_message9.GunNum = GUN_B;
                OSQPost(Control_PeventB, &send_message9);
                OSTimeDly(SYS_DELAY_1s);
                BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s);
                while (GetBMSInsulationCheck(GUN_B)) //�ȴ���Ե������
                {
                    OSTimeDly(SYS_DELAY_1s);
                    //��Ե�������5������ɣ���߷ſ�����Ϊ10��
                    if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_40s) == BMS_TIME_OUT)
                    {
                        //��Ե���ʧ��
                        SetBcuRemoteSignalState(GUN_B,(INT8U)REMOTE_SIGNALE_2, 4, 1);
                        //���ʧ�ܣ���Ե���ʧ��
                        SysState[GUN_B].SelfCheckState.Insulation = STATE_ERR;
                        SysState[GUN_B].SelfCheckState.NoErr = STATE_ERR;
						SetStartFailType(GUN_B,ENDFAIL_INSOLUTIONERR);
                        send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
						NB_WriterReason(GUN_B,"F5",2);
                        send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
                        send_message10.GunNum = GUN_B;
                        send_message10.pData = (INT8U *)&SysState[GUN_B].SelfCheckState.NoErr;
                        OSQPost(Control_PeventB, &send_message10);
                        OSTimeDly(SYS_DELAY_100ms);
                        SetBMSStartCharge(GUN_B,FALSE);
						printf("SelfCheck Timeout 2011\r\n");
                        return FALSE;
                    }
                }
                //�жϾ�Ե����Ƿ�ɹ�
                if (CheckHWConnectforBMS(GUN_B) == FALSE)    //�ڲ����Դ��������ط�д
                {
//                    //��Ե���ʧ��
//                  SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 1);
//                   
//					SetStartFailType(ENDFAIL_INSOLUTIONERR);
//                  SysState.SelfCheckState.Insulation = STATE_ERR;
//                  SysState.SelfCheckState.NoErr = STATE_ERR;
                    send_message10.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message10.DivNum = APP_CHARGE_FAIL; //����ʧ��
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
    BMS_CONNECT_StepSet(GUN_B,BMS_CRM_SEND);//����CRM֡
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BRM_STEP, SYS_DELAY_5s);
	//CEM����ȫ������״̬��ʼ��
	SetCEMState(GUN_B,CEM_ALL, ERR_TYPE_NORMAL);
    //�յ�RTS֡�����ظ�RTS֡��ͽ����˵ȴ����ձ�ʶ���ĵĹ��̣�
    while(1)
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE )
		{//�����ִ���з�����ϵͳ���ϣ�������
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
        result = WaitConnectBMSmessageB(SYS_DELAY_10ms);//�౨�İ�10ms�������
        if(result == APP_RXDEAL_SUCCESS)//���յ�����
        {
            if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BRM_SUCCESS)//BRM�Ѿ�ȫ������
            {
                BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;
                result = APP_DEFAULT;
				CRMReadyflag[GUN_B] = 1;
				//����CEM״̬Ϊ����
				SetCEMState(GUN_B,CEM_BRM, ERR_TYPE_NORMAL);
                break;//������ǰѭ��
            }
            //�ж�BRM�����Ƿ�ʱ����Ҫ��CRM���ͺ�5���ڽ��յ�
            else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BRM_STEP, SYS_DELAY_5s))
            {
                BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;
                //���ý���BCP��ʱ״̬
                SetCEMState(GUN_B,CEM_BRM, ERR_TYPE_TIMEOUT);
                //���볬ʱ����
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E43",3);
				printf("GUN_B 1\r\n");
                CRMReadyflag[GUN_B] = 0;
                return FALSE;
            }
        }
        //�ж�BRM�����Ƿ�ʱ����Ҫ��CRM���ͺ�5���ڽ��յ�
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BRM_STEP, SYS_DELAY_5s))
        {
        	//���ý���BCP��ʱ״̬
            SetCEMState(GUN_B,CEM_BRM, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
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
* Description   : ���������ý׶�
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
    //�ȴ�����BMS���͵�BCP����
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BCP_STEP, SYS_DELAY_5s); //������BCP�ĳ�ʱ�ж�
    while(1)
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE )
		{//�����ִ���з�����ϵͳ���ϣ�������
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
        result = WaitConnectBMSmessageA(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)//���յ�����
        {
            if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCP_SUCCESS)//BCP�Ѿ�ȫ������
            {
                //BMS_CONNECT_StepSet(BMS_PARA_SEND);//BCP���ճɹ��ˣ�����CTS,CML
				//���ý���BCP��ʱ״̬
				SetCEMState(GUN_A,CEM_BCP, ERR_TYPE_NORMAL);
				break;//������ǰѭ��
            }
            //�յ�BMS���͵Ĵ�����BEM
            if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BEM_SUCCESS)
            {
                //���볬ʱ����
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E41",3);
				printf("GUN_A 3\r\n");
                return FALSE;
            }
        }
        //�ж��Ƿ��յ�BCP���ģ����û���յ����·���CRM(0xAA)����
        if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCP_STEP, SYS_DELAY_5s))
        {
            //���ý���BCP��ʱ״̬
            SetCEMState(GUN_A,CEM_BCP, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E40",3);
			printf("GUN_A 4\r\n");
            return FALSE;
        }
    }
    //֪ͨ���������ж�BCP�����Ƿ��ʺ�
    send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
    send_message29.DivNum = APP_BCP_FIT;
	send_message29.GunNum = GUN_A;
    OSQPost(Control_PeventA, &send_message29);
    OSTimeDly(SYS_DELAY_10ms);

    //�ж�BCP�����Ƿ������
    BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_1s); //����Ϊ�˱�����Э�鲢δ�涨��ʱ��
    while (GetBMSBCPFitCheck(GUN_A))
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE )
		{//�����ִ���з�����ϵͳ���ϣ�������
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
        OSTimeDly(SYS_DELAY_10ms);
        if (SysState[GUN_A].BMSState.BCPFitState != STATE_NO_ERR)
        {
            //BCP����������,��������ϼ���3
            //���볬ʱ����
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E39",3);
			printf("GUN_A 5\r\n");
            return FALSE;
        }
        //��ʱ1�룬Ҳ�㲻����
        if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_1s) == BMS_TIME_OUT)
        {
            //BCP����������,��������ϼ���3
            //���볬ʱ����
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E38",3);
			printf("GUN_A 6\r\n");
            return FALSE;
        }
    }
	BMS_CONNECT_StepSet(GUN_A,BMS_CTS_SEND);//CTS������� �����ۺ��Ϲ��공
	OSTimeDly(SYS_DELAY_5ms);
    //�ȴ�BMS���ص�س��׼������״̬֡��60�볬ʱ
    result = APP_DEFAULT;
    //��������5S���������������ʵ�ʷ����������1���ӵ��ж�Ҳ�ǹ���һ��
    BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s);
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BRO_STEP, SYS_DELAY_1M);
	BMS_CONNECT_StepSet(GUN_A,BMS_PARA_SEND);//BCP���ճɹ��ˣ�����CTS,CML
    while(1)
    {
		if(SysState[GUN_A].TotalErrState!=ERR_NORMAL_TYPE)
		{//�����ִ���з�����ϵͳ���ϣ�������
			BSP_CloseBMSPower(GUN_A);
			return FALSE;
		}
	    result = WaitConnectBMSmessageA(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)
        {
            if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BRO_SUCCESS)
            {
                if(BMS_BRO_Context[GUN_A].StandbyFlag == GB_BMSSTATE_STANDBY) //���յ�BMS׼���õ�ָ��
                {
                	SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_NORMAL);
                    if(APP_GetGBType(GUN_A) == BMS_GB_2015)
                        //���յ�BRO,�ҵ���Ѿ�׼������,Ӧ�÷���CRO(0x00)
                        BMS_CONNECT_StepSet(GUN_A,BMS_CRO_UNREADY_SEND);
                    else
                        //���յ�BRO,�ҵ���Ѿ�׼������,Ӧ�÷���CRO(0xAA)
                        BMS_CONNECT_StepSet(GUN_A,BMS_CRO_SEND);
                    break;//������һ��
                }
                else if(BMS_BRO_Context[GUN_A].StandbyFlag == GB_BMSSTATE_NOTREADY)
                {
                    firsttime++;
                    if (firsttime == 1)
                    {
                        BMS_TIMEOUT_ENTER(GUN_A,BMS_BRO_STEP, SYS_DELAY_1M); //������BRO�ĳ�ʱ�ж�
                    }
                    BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s);
                    //���յ����ݣ�������BMS�Ѿ�׼����ָ������ȴ���һ�������Ƿ���0x00
                    if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BRO_STEP, SYS_DELAY_1M)) //
                    {
                        //���ý���BRO��ʱ״̬
                        SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_TIMEOUT);
                        //���볬ʱ����
                        ChargeErrSwitch(GUN_A);
						NB_WriterReason(GUN_A,"E37",3);
						printf("GUN_A 7\r\n");
                        return FALSE;
                    }
                }
            }
            else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0�Ͳ�����
            {
                //���ý���BRO��ʱ״̬
                SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_TIMEOUT);
                //���볬ʱ����
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E36",3);
				printf("GUN_A 8\r\n");
                return FALSE;
            }
            //�յ�BMS���͵Ĵ�����BEM
            else if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BEM_SUCCESS)
            {
                //���볬ʱ����
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E35",3);
				printf("GUN_A 9\r\n");
                return FALSE;
            }
        }
        //û���յ��κ�BRO��֡��5S��ʱ
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0�Ͳ�����
        {
            //���ý���BRO��ʱ״̬
            SetCEMState(GUN_A,CEM_BRO, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E34",3);
			printf("GUN_A 10\r\n");
            return FALSE;
        }
    }
	//VIN��Ȩ�����ģʽ�ͱ�Ϊ�Զ�ģʽ
	if(GetChargeMode(GUN_A) == MODE_VIN)
	{
		count = SYS_DELAY_20s / SYS_DELAY_200ms;
		while(count)
		{
			if(VinControl[GUN_A].VinState == VIN_SECCSEE)
			{
				break;
				//�ȴ���Ȩ���
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
				//��Ȩʧ��
			}
			OSTimeDly(SYS_DELAY_200ms);
		}
	}
    if (APP_GetGBType(GUN_A) == BMS_GB_2015)  //�¹��� 
    {
        //���յ�BRO(0xAA)����Ҫ�����׮DC�̵�������ѹ��һЩӲ���������൱��Ԥ���
        send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message29.DivNum = APP_CHARGE_READY;
		send_message29.GunNum = GUN_A;
        OSQPost(Control_PeventA, &send_message29);
        OSTimeDly(SYS_DELAY_10ms);
		
        //����ط�Ϊ�¹������ݣ������ȼ��DC�̵������ĵ�ѹ��
        BMS_TIMEOUT_ENTER(GUN_A,BMS_OTH_STEP, SYS_DELAY_30s); //����Ϊ�˱�����Э�鲢δ�涨��ʱ��
        while (GetChargeReady(GUN_A) == READY_ING)
        {
            OSTimeDly(SYS_DELAY_250ms);
            //��ʱʱ�䵽��
            if (BMS_TIMEOUT_CHECK(GUN_A,BMS_OTH_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
                //���볬ʱ����
                ChargeErrSwitch(GUN_A);
				NB_WriterReason(GUN_A,"E33",3);
				printf("GUN_A 11\r\n");
                return FALSE;
            }
        }
		if(GetChargeReady(GUN_A) == READY_SUCCESS)
		{
			//����׼���ú󣬷���CRO����(0xAA)
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
* Description   : ���������ý׶�
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
    //�ȴ�����BMS���͵�BCP����
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BCP_STEP, SYS_DELAY_5s); //������BCP�ĳ�ʱ�ж�
    while(1)
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE )
		{//�����ִ���з�����ϵͳ���ϣ�������
			printf("sfasldkfj ");
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
        result = WaitConnectBMSmessageB(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)//���յ�����
        {
            if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCP_SUCCESS)//BCP�Ѿ�ȫ������
            {
                //BMS_CONNECT_StepSet(BMS_PARA_SEND);//BCP���ճɹ��ˣ�����CTS,CML
				//���ý���BCP��ʱ״̬
				SetCEMState(GUN_B,CEM_BCP, ERR_TYPE_NORMAL);
				break;//������ǰѭ��
            }
            //�յ�BMS���͵Ĵ�����BEM
            if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BEM_SUCCESS)
            {
                //���볬ʱ����
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E32",3);
				printf("GUN_B 3\r\n");
                return FALSE;
            }
        }
        //�ж��Ƿ��յ�BCP���ģ����û���յ����·���CRM(0xAA)����
        if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCP_STEP, SYS_DELAY_5s))
        {
            //���ý���BCP��ʱ״̬
            SetCEMState(GUN_B,CEM_BCP, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E31",3);
			printf("GUN_B 4\r\n");
            return FALSE;
        }
    }
    //֪ͨ���������ж�BCP�����Ƿ��ʺ�
    send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
    send_message29.DivNum = APP_BCP_FIT;
	send_message29.GunNum = GUN_B;
    OSQPost(Control_PeventB, &send_message29);
    OSTimeDly(SYS_DELAY_10ms);

    //�ж�BCP�����Ƿ������
    BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_1s); //����Ϊ�˱�����Э�鲢δ�涨��ʱ��
    while (GetBMSBCPFitCheck(GUN_B))
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE )
		{//�����ִ���з�����ϵͳ���ϣ�������
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
        OSTimeDly(SYS_DELAY_10ms);
        if (SysState[GUN_B].BMSState.BCPFitState != STATE_NO_ERR)
        {
            //BCP����������,��������ϼ���3
            //���볬ʱ����
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E30",3);
			printf("GUN_B 5\r\n");
            return FALSE;
        }
        //��ʱ1�룬Ҳ�㲻����
        if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_1s) == BMS_TIME_OUT)
        {
            //BCP����������,��������ϼ���3
            //���볬ʱ����
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E29",3);
			printf("GUN_B 6\r\n");
            return FALSE;
        }
    }
	BMS_CONNECT_StepSet(GUN_B,BMS_CTS_SEND);//CTS������� �����ۺ��Ϲ��공
	OSTimeDly(SYS_DELAY_5ms);
    //�ȴ�BMS���ص�س��׼������״̬֡��60�볬ʱ
    result = APP_DEFAULT;
    //��������5S���������������ʵ�ʷ����������1���ӵ��ж�Ҳ�ǹ���һ��
    BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s);
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BRO_STEP, SYS_DELAY_1M);
	BMS_CONNECT_StepSet(GUN_B,BMS_PARA_SEND);//BCP���ճɹ��ˣ�����CTS,CML
    while(1)
    {
		if(SysState[GUN_B].TotalErrState!=ERR_NORMAL_TYPE)
		{//�����ִ���з�����ϵͳ���ϣ�������
			BSP_CloseBMSPower(GUN_B);
			return FALSE;
		}
	    result = WaitConnectBMSmessageB(SYS_DELAY_250ms);
        if(result == APP_RXDEAL_SUCCESS)
        {
            if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BRO_SUCCESS)
            {
                if(BMS_BRO_Context[GUN_B].StandbyFlag == GB_BMSSTATE_STANDBY) //���յ�BMS׼���õ�ָ��
                {
                	SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_NORMAL);
                    if(APP_GetGBType(GUN_B) == BMS_GB_2015)
                        //���յ�BRO,�ҵ���Ѿ�׼������,Ӧ�÷���CRO(0x00)
                        BMS_CONNECT_StepSet(GUN_B,BMS_CRO_UNREADY_SEND);
                    else
                        //���յ�BRO,�ҵ���Ѿ�׼������,Ӧ�÷���CRO(0xAA)
                        BMS_CONNECT_StepSet(GUN_B,BMS_CRO_SEND);
                    break;//������һ��
                }
                else if(BMS_BRO_Context[GUN_B].StandbyFlag == GB_BMSSTATE_NOTREADY)
                {
                    firsttime++;
                    if (firsttime == 1)
                    {
                        BMS_TIMEOUT_ENTER(GUN_B,BMS_BRO_STEP, SYS_DELAY_1M); //������BRO�ĳ�ʱ�ж�
                    }
                    BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s);
                    //���յ����ݣ�������BMS�Ѿ�׼����ָ������ȴ���һ�������Ƿ���0x00
                    if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BRO_STEP, SYS_DELAY_1M)) //
                    {
                        //���ý���BRO��ʱ״̬
                        SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_TIMEOUT);
                        //���볬ʱ����
                        ChargeErrSwitch(GUN_B);
						NB_WriterReason(GUN_B,"E28",3);
						printf("GUN_B 7\r\n");
                        return FALSE;
                    }
                }
            }
            else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0�Ͳ�����
            {
                //���ý���BRO��ʱ״̬
                SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_TIMEOUT);
                //���볬ʱ����
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E27",3);
				printf("GUN_B 8\r\n");
                return FALSE;
            }
            //�յ�BMS���͵Ĵ�����BEM
            else if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BEM_SUCCESS)
            {
                //���볬ʱ����
                ChargeErrSwitch(GUN_B);
				printf("GUN_B 9\r\n");
                return FALSE;
            }
        }
        //û���յ��κ�BRO��֡��5S��ʱ
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_5s) ) //reconneccount=0�Ͳ�����
        {
            //���ý���BRO��ʱ״̬
            SetCEMState(GUN_B,CEM_BRO, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E26",3);
			printf("GUN_B 10\r\n");
            return FALSE;
        }
    }
	//VIN��Ȩ�����ģʽ�ͱ�Ϊ�Զ�ģʽ
	if(GetChargeMode(GUN_B) == MODE_VIN)
	{
		count = SYS_DELAY_20s / SYS_DELAY_200ms;
		while(count)
		{
			if(VinControl[GUN_B].VinState == VIN_SECCSEE)
			{
				break;
				//�ȴ���Ȩ���
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
				//��Ȩʧ��
			}
			OSTimeDly(SYS_DELAY_200ms);
		}
	}
    if (APP_GetGBType(GUN_B) == BMS_GB_2015)  //�¹��� 
    {
        //���յ�BRO(0xAA)����Ҫ�����׮DC�̵�������ѹ��һЩӲ���������൱��Ԥ���
        send_message29.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message29.DivNum = APP_CHARGE_READY;
		send_message29.GunNum = GUN_B;
        OSQPost(Control_PeventB, &send_message29);
        OSTimeDly(SYS_DELAY_10ms);
		
        //����ط�Ϊ�¹������ݣ������ȼ��DC�̵������ĵ�ѹ��
        BMS_TIMEOUT_ENTER(GUN_B,BMS_OTH_STEP, SYS_DELAY_30s); //����Ϊ�˱�����Э�鲢δ�涨��ʱ��
        while (GetChargeReady(GUN_B) == READY_ING)
        {
            OSTimeDly(SYS_DELAY_250ms);
            //��ʱʱ�䵽��
            if (BMS_TIMEOUT_CHECK(GUN_B,BMS_OTH_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
                //���볬ʱ����
                ChargeErrSwitch(GUN_B);
				NB_WriterReason(GUN_B,"E25",3);
				printf("GUN_B 11\r\n");
                return FALSE;
            }
        }
		if(GetChargeReady(GUN_B) == READY_SUCCESS)
		{
			//����׼���ú󣬷���CRO����(0xAA)
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
* Description   : ���׶�
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
	//�����³�ʼֵ
	memset(&BMS_BCL_ContextBak, 0, sizeof(_BMS_BCL_CONTEXT));
    //GB/T 27930-2015��29ҳ�е�"���ϴ��յ�������TimeoutΪ1s",
    //�Ƿ�Ӧ���Ƿ���CRO����(0xAA)��TimeoutΪ1s��??
    //��ʱ����������ȥд
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BCL_STEP, SYS_DELAY_1s);
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BCS_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_A,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s);
    while(1)
    {
        result = WaitConnectBMSmessageA(SYS_DELAY_10ms);
		//����Ҫ����CCS������	����������һ�κ�Ͳ���ʹ����
        if (((BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCL_SUCCESS)&&(BMS_CONNECT_Control[GUN_A].laststate == BMS_BCS_SUCCESS))
		 ||((BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCS_SUCCESS)&&(BMS_CONNECT_Control[GUN_A].laststate == BMS_BCL_SUCCESS)))//�յ���BCL��BCS��
        {
        	SetCEMState(GUN_A,CEM_BCL, ERR_TYPE_NORMAL);
			SetCEMState(GUN_A,CEM_BCS, ERR_TYPE_NORMAL);

            //����CCS���ģ��˴����ܻᵼ�²������ڵķ���
            if (startcharge == FALSE)
            {
                startcharge = TRUE;
                printf("AAABCPBCP,%x\r\n",BMS_BCP_Context[GUN_A].MaxVoltage);
                BMS_BCL_ContextBak.DemandVol = BMS_BCL_Context[GUN_A].DemandVol;
				BMS_BCL_ContextBak.DemandCur = BMS_BCL_Context[GUN_A].DemandCur;
				//�����������BCP����߳���ѹ������
				//����BCP����߳���ѹ�������
				if (BMS_BCL_Context[GUN_A].DemandVol > BMS_BCP_Context[GUN_A].MaxVoltage)
				{
					BMS_BCL_Context[GUN_A].DemandVol = BMS_BCP_Context[GUN_A].MaxVoltage;
				}
				if (BMS_BCL_Context[GUN_A].DemandCur < BMS_BCP_Context[GUN_A].MaxCurrent) 
				{
					BMS_BCL_Context[GUN_A].DemandCur = BMS_BCP_Context[GUN_A].MaxCurrent;
				}
				
                //���Ϳ�ʼ���������Ϣ����������
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message11.DivNum = APP_POWER_OUTPUT;
				send_message11.GunNum = GUN_A;
                OSQPost(Control_PeventA, &send_message11);
                OSTimeDly(SYS_DELAY_2ms);               
				//��ʼ����CCS����
                BMS_CONNECT_StepSet(GUN_A,BMS_CCS_SEND);
            }
        }
		//���濪ʼ��ȫ�����жϸ��Ե�
        //��س��������BCL������ʱʱ��Ϊ1�룬��ʱ��Ӧ����������磬����Ӧ��Ϊ50ms
        if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCL_SUCCESS)//���յ�BCL
        {
        	SetCEMState(GUN_A,CEM_BCL, ERR_TYPE_NORMAL);
			if (GetChargePauseState(GUN_A) == FALSE) //�������
			{
				//����BCL������ʵʱ�������,��ѹ���������б仯
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
					{//��ʼ����CCS���ģ���Щ�ɹ��공��һ����BCL��BCS���ܿ췢����������涨
						//�¹����BCL��BCS���յ��󣬲��ܷ���CCS
                        BMS_CONNECT_StepSet(GUN_A,BMS_CCS_SEND);
					}
	            }
				else
				{
					if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s) )
					{//������û�б仯ʱ����ʱ5s����һ�ε�ǰ����
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
			//�ڹ涨��ʱ��1s���յ���BCL���������ó�ʱʱ��
            BMS_TIMEOUT_ENTER(GUN_A,BMS_BCL_STEP, SYS_DELAY_1s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;//��һ��֮���ùܵ�ǰ״̬��
			}
        }
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCL_STEP, SYS_DELAY_1s) )
        {
            //BCL��ʱ
            //���ͳ�����ֹ��籨��(CST)��BMS
            //���ý���BCL��ʱ״̬
            SetCEMState(GUN_A,CEM_BCL, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E24",3);
			printf("GUN_A 12\r\n");
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCLTIMTOUT);
            return FALSE;
        }

        if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BCS_SUCCESS)//���յ�BCS
        {
        	SetCEMState(GUN_A,CEM_BCS, ERR_TYPE_NORMAL);
            //�ڹ涨��ʱ��5s���յ���BCS���������ó�ʱʱ��
            BMS_TIMEOUT_ENTER(GUN_A,BMS_BCS_STEP, SYS_DELAY_5s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;//��һ��֮���ùܵ�ǰ״̬��
			}
        }
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BCS_STEP, SYS_DELAY_5s))//BCS 5s��ʱ
        {//��س����״̬����BCS��ʱʱ��Ϊ5�룬��ʱ��Ӧ����������磬����Ӧ��Ϊ250ms
            //��ʱ����
            //���ͳ�����ֹ��籨��(CST)��BMS
            //���ý���BCS��ʱ״̬
            SetCEMState(GUN_A,CEM_BCS, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_A);
			printf("GUN_A 13\r\n");
			NB_WriterReason(GUN_A,"E23",3);
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCSTIMTOUT);
            return FALSE;
        }

		if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BSM_SUCCESS)
		{//BSM��ͣ���
			if (BMS_BSM_Context[GUN_A].State1.OneByte.ChargeAllowBits == BMS_CHARGE_PAUSE)
			{//δ��ͣ�������Ҫ��ͣ���
				if (GetChargePauseState(GUN_A) == FALSE)
				{//��ʱ��磬ֻ�ǹر�ģ�����
					send_message13.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	                send_message13.DivNum = APP_CHARGE_PAUSE;
					send_message13.GunNum = GUN_A;
	                OSQPost(Control_PeventA, &send_message13);
                    //��ͣ��翪ʼ���㳬ʱ10����
                    BMS_TIMEOUT_ENTER(GUN_A,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			else if (BMS_BSM_Context[GUN_A].State1.OneByte.ChargeAllowBits == BSM_CHARGE_ENABLE)
			{//�ָ����
				if (GetChargePauseState(GUN_A) == TRUE)
				{
					SetChargePauseState(GUN_A,FALSE);
					BMS_BCL_ContextBak.DemandVol = 0;
					BMS_BCL_ContextBak.DemandCur = 0;
                    //�����ͣ��糬ʱ10����
                    BMS_TIMEOUT_ENTER(GUN_A,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			if ( (BMS_BSM_Context[GUN_A].State0.OneByte.UnitVolBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_A].State0.OneByte.WholeSOCBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_A].State0.OneByte.ChargeCurBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_A].State0.OneByte.BatteryTempBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_A].State1.OneByte.BatteryInsulationBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_A].State1.OneByte.BatteryConnectBits == STATE_TROUBLE) )
			{//BSM����״̬������
				
				SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //����ֹͣ
				SysState[GUN_A].TotalErrState	|= ERR_BMSSTOP_TYPE;
				
				if(BMS_BSM_Context[GUN_A].State0.OneByte.UnitVolBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMBATVOLTHIGH); //BMS��ѽ
				}
				else if(BMS_BSM_Context[GUN_A].State0.OneByte.WholeSOCBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMBATVOLTHIGH); //BSM�����ѹ����
				}
				else if(BMS_BSM_Context[GUN_A].State0.OneByte.ChargeCurBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMCURRUP);  //BMS����
				}
				else if(BMS_BSM_Context[GUN_A].State0.OneByte.BatteryTempBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_A,STOP_BSMTEMPUP); //BMS���¶�
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
        {//�յ�BMS����ֹ���(BST)����,����10msһ�����ڣ�
            //������ֹԭ��ΪBMS����ֹͣ
            SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.BMSStopBits = STATE_TROUBLE;
			
			//CheckBSTErr(GUN_A,(INT8U*)&stoptype);   //����Ĺ���ԭ����д
			APP_Set_ERR_Branch(GUN_A,STOP_BSMNORMAL);		//�������ԭ�򣬾���Ϊ��BMS����ֹͣ����Щ��������ֹͣҲ��������ģ�   20211015
            break;
        }
        if (result == APP_END_CHARGING)
        {//�յ�׮�屾��Ľ����������
            SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.UserStopBits = STATE_TROUBLE;
            stoptype = STOP_USER; //�û�����ֹͣ������ԭ��ǰ���Ѿ�ϸ��
			SysState[GUN_A].TotalErrState |= ERR_USER_STOP_TYPE;
			printf("User Stop1\r\n");
            break;
        }
        if (GetChargePauseState(GUN_A) == TRUE)
        {//��ͣ���״̬��
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_PAUSE_STEP, SYS_DELAY_10M) )
            {//CST��д������ֹ
                SysState[GUN_A].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //����ֹͣ
				SysState[GUN_A].TotalErrState	|= ERR_CHARGESTOP_TYPE;
				APP_Set_ERR_Branch(GUN_A,STOP_OTHERERR);
				printf("11111\r\n");
                break;
            }
        }

        if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BEM_SUCCESS)
        {//�յ�BMS���͵Ĵ�����BEM
            //���볬ʱ����
            ChargeErrSwitch(GUN_A);
			NB_WriterReason(GUN_A,"E22",3);
			printf("GUN_A 14\r\n");
            return FALSE;
        }
		if(GetBMSStartCharge(GUN_A) == FALSE)    //�����������  20211117
		{
			printf("GUN_A startcharge State Err\r\n");
			break;
		}
    }

	//���˴�ֻ���յ�BST�����յ����·��Ľ������֡
    //���BST��ʱ�ˣ���Ҫ����CEM���ģ���Ҫȥ�ȴ�BSD�ˣ�������缴��
    //���BSTδ��ʱ����Ҫȥ�ȴ�BSD
	SetBMSConnectStep(GUN_A,BMS_CONNECT_END);
    result = APP_DEFAULT;
    //���ͳ�����ֹ��籨��(CST)��BMS
    BMS_CONNECT_StepSet(GUN_A,BMS_CST_SEND);
    BMS_TIMEOUT_ENTER(GUN_A,BMS_BST_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_A,BMS_BSD_STEP, SYS_DELAY_10s); //BSD�ĳ�ʱҲ�Ǵӷ���CST��ʼ����
	//�������յ�BST֡
    if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BST_SUCCESS)//BST�Ѿ�����
    {
    	SetCEMState(GUN_A,CEM_BST, ERR_TYPE_NORMAL);
        send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message11.DivNum = APP_CHARGE_END;  //ֹͣ�������
		NB_WriterReason(GUN_A,"E5",2);
        send_message11.DataLen = 1;
		send_message11.GunNum = GUN_A;
        send_message11.pData = (INT8U *)&stoptype;
        OSQPost(Control_PeventA, &send_message11);
        OSTimeDly(SYS_DELAY_10ms);
		printf("Get BST\r\n");
    }
    else//���CST�ȷ��͹�ȥ
    {
	    //��Ҫ�����µ�ǰ״̬������CST����ҪBMS�ظ�һ��BST
	    BMS_CONNECT_Control[GUN_A].currentstate = BMS_CONNECT_DEFAULT;
        while (1)
        {
            result = WaitConnectBMSmessageA(SYS_DELAY_20ms);
            if(result == APP_RXDEAL_SUCCESS)//���յ�����
            {
                if(BMS_CONNECT_Control[GUN_A].currentstate == BMS_BST_SUCCESS)//BST�Ѿ�����
                {
                	SetCEMState(GUN_A,CEM_BST, ERR_TYPE_NORMAL);
                    send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message11.DivNum = APP_CHARGE_END;  //ֹͣ�������
//					NB_WriterReason(GUN_A,"E4",2);
                    send_message11.DataLen = 1;
					send_message11.GunNum = GUN_A;
                    send_message11.pData = (INT8U *)&stoptype;
                    OSQPost(Control_PeventA, &send_message11);
                    OSTimeDly(SYS_DELAY_10ms);
					printf("First CST,Get BST\r\n");
                    break;//������ǰѭ��
                }             
            }
            if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BST_STEP, SYS_DELAY_5s) )
            {
                //BST��ʱ
                //���ý���BST��ʱ״̬
                //�˴�BST����Ҫ���жϳ�ʱ�ģ�����������3��������
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message11.DivNum = APP_CHARGE_END;  //ֹͣ�������
//				NB_WriterReason(GUN_A,"E3",2);
                send_message11.DataLen = 1;
				send_message11.GunNum = GUN_A;
                send_message11.pData = (INT8U *)&stoptype;
                OSQPost(Control_PeventA, &send_message11);
                SetCEMState(GUN_A,CEM_BST, ERR_TYPE_TIMEOUT);
//                BMS_CONNECT_StepSet(BMS_CEM_SEND);
                ChargeErrDeal(GUN_A,ERR_L3_CHAGING_OVER_TIMEOUT);//ֱ�ӽ������
				printf("BST Timeout\r\n");
                return FALSE;
            }
			//�˴��ж���CST֡�Ƿ��޸��ˣ��˴�Ҫ��֤һֱ�ڷ���CST
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
* Description   : ���׶�
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
	//�����³�ʼֵ
	memset(&BMS_BCL_ContextBak, 0, sizeof(_BMS_BCL_CONTEXT));
    //GB/T 27930-2015��29ҳ�е�"���ϴ��յ�������TimeoutΪ1s",
    //�Ƿ�Ӧ���Ƿ���CRO����(0xAA)��TimeoutΪ1s��??
    //��ʱ����������ȥд
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BCL_STEP, SYS_DELAY_1s);
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BCS_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_B,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s);
    while(1)
    {
        result = WaitConnectBMSmessageB(SYS_DELAY_10ms);
		//����Ҫ����CCS������	����������һ�κ�Ͳ���ʹ����
        if (((BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCL_SUCCESS)&&(BMS_CONNECT_Control[GUN_B].laststate == BMS_BCS_SUCCESS))
		 ||((BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCS_SUCCESS)&&(BMS_CONNECT_Control[GUN_B].laststate == BMS_BCL_SUCCESS)))//�յ���BCL��BCS��
        {
        	SetCEMState(GUN_B,CEM_BCL, ERR_TYPE_NORMAL);
			SetCEMState(GUN_B,CEM_BCS, ERR_TYPE_NORMAL);

            //����CCS���ģ��˴����ܻᵼ�²������ڵķ���
            if (startcharge == FALSE)
            {
                startcharge = TRUE;
                printf("AAABCPBCP,%x\r\n",BMS_BCP_Context[GUN_B].MaxVoltage);
                BMS_BCL_ContextBak.DemandVol = BMS_BCL_Context[GUN_B].DemandVol;
				BMS_BCL_ContextBak.DemandCur = BMS_BCL_Context[GUN_B].DemandCur;
				//�����������BCP����߳���ѹ������
				//����BCP����߳���ѹ�������
				if (BMS_BCL_Context[GUN_B].DemandVol > BMS_BCP_Context[GUN_B].MaxVoltage)
				{
					BMS_BCL_Context[GUN_B].DemandVol = BMS_BCP_Context[GUN_B].MaxVoltage;
				}
				if (BMS_BCL_Context[GUN_B].DemandCur < BMS_BCP_Context[GUN_B].MaxCurrent) 
				{
					BMS_BCL_Context[GUN_B].DemandCur = BMS_BCP_Context[GUN_B].MaxCurrent;
				}
				
                //���Ϳ�ʼ���������Ϣ����������
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                send_message11.DivNum = APP_POWER_OUTPUT;
				send_message11.GunNum = GUN_B;
                OSQPost(Control_PeventB, &send_message11);
                OSTimeDly(SYS_DELAY_2ms);               
				//��ʼ����CCS����
                BMS_CONNECT_StepSet(GUN_B,BMS_CCS_SEND);
            }
        }
		//���濪ʼ��ȫ�����жϸ��Ե�
        //��س��������BCL������ʱʱ��Ϊ1�룬��ʱ��Ӧ����������磬����Ӧ��Ϊ50ms
        if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCL_SUCCESS)//���յ�BCL
        {
        	SetCEMState(GUN_B,CEM_BCL, ERR_TYPE_NORMAL);
			if (GetChargePauseState(GUN_B) == FALSE) //�������
			{
				//����BCL������ʵʱ�������,��ѹ���������б仯
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
					{//��ʼ����CCS���ģ���Щ�ɹ��공��һ����BCL��BCS���ܿ췢����������涨
						//�¹����BCL��BCS���յ��󣬲��ܷ���CCS
                        BMS_CONNECT_StepSet(GUN_B,BMS_CCS_SEND);
					}
	            }
				else
				{
					if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCL_DEMAND_UPDATA, SYS_DELAY_2s) )
					{//������û�б仯ʱ����ʱ5s����һ�ε�ǰ����
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
			//�ڹ涨��ʱ��1s���յ���BCL���������ó�ʱʱ��
            BMS_TIMEOUT_ENTER(GUN_B,BMS_BCL_STEP, SYS_DELAY_1s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;//��һ��֮���ùܵ�ǰ״̬��
			}
        }
        else if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCL_STEP, SYS_DELAY_1s) )
        {
            //BCL��ʱ
            //���ͳ�����ֹ��籨��(CST)��BMS
            //���ý���BCL��ʱ״̬
            SetCEMState(GUN_B,CEM_BCL, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E21",3);
			printf("GUN_B 12\r\n");
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCLTIMTOUT);
            return FALSE;
        }

        if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BCS_SUCCESS)//���յ�BCS
        {
        	SetCEMState(GUN_B,CEM_BCS, ERR_TYPE_NORMAL);
            //�ڹ涨��ʱ��5s���յ���BCS���������ó�ʱʱ��
            BMS_TIMEOUT_ENTER(GUN_B,BMS_BCS_STEP, SYS_DELAY_5s);
			if(startcharge)
			{
				BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;//��һ��֮���ùܵ�ǰ״̬��
			}
        }
        else if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BCS_STEP, SYS_DELAY_5s))//BCS 5s��ʱ
        {//��س����״̬����BCS��ʱʱ��Ϊ5�룬��ʱ��Ӧ����������磬����Ӧ��Ϊ250ms
            //��ʱ����
            //���ͳ�����ֹ��籨��(CST)��BMS
            //���ý���BCS��ʱ״̬
            SetCEMState(GUN_B,CEM_BCS, ERR_TYPE_TIMEOUT);
            //���볬ʱ����
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E20",3);
			printf("GUN_B 13\r\n");
//			SysState.TotalErrState	|= ERR_CHARGERXTIMEOUT_TYPE;
//			APP_Set_ERR_Branch(STOP_BCSTIMTOUT);
            return FALSE;
        }

		if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BSM_SUCCESS)
		{//BSM��ͣ���
			if (BMS_BSM_Context[GUN_B].State1.OneByte.ChargeAllowBits == BMS_CHARGE_PAUSE)
			{//δ��ͣ�������Ҫ��ͣ���
				if (GetChargePauseState(GUN_B) == FALSE)
				{//��ʱ��磬ֻ�ǹر�ģ�����
					send_message13.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
	                send_message13.DivNum = APP_CHARGE_PAUSE;
					send_message13.GunNum = GUN_B;
	                OSQPost(Control_PeventB, &send_message13);
                    //��ͣ��翪ʼ���㳬ʱ10����
                    BMS_TIMEOUT_ENTER(GUN_B,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			else if (BMS_BSM_Context[GUN_B].State1.OneByte.ChargeAllowBits == BSM_CHARGE_ENABLE)
			{//�ָ����
				if (GetChargePauseState(GUN_B) == TRUE)
				{
					SetChargePauseState(GUN_B,FALSE);
					BMS_BCL_ContextBak.DemandVol = 0;
					BMS_BCL_ContextBak.DemandCur = 0;
                    //�����ͣ��糬ʱ10����
                    BMS_TIMEOUT_ENTER(GUN_B,BMS_PAUSE_STEP, SYS_DELAY_10M);
				}
			}
			if ( (BMS_BSM_Context[GUN_B].State0.OneByte.UnitVolBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_B].State0.OneByte.WholeSOCBits != STATE_NORMAL)\
				|| (BMS_BSM_Context[GUN_B].State0.OneByte.ChargeCurBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_B].State0.OneByte.BatteryTempBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_B].State1.OneByte.BatteryInsulationBits == STATE_TROUBLE)\
				|| (BMS_BSM_Context[GUN_B].State1.OneByte.BatteryConnectBits == STATE_TROUBLE) )
			{//BSM����״̬������
				
				SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //����ֹͣ
				SysState[GUN_B].TotalErrState	|= ERR_BMSSTOP_TYPE;
				
				if(BMS_BSM_Context[GUN_B].State0.OneByte.UnitVolBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMBATVOLTHIGH); //BMS��ѽ
				}
				else if(BMS_BSM_Context[GUN_B].State0.OneByte.WholeSOCBits != STATE_NORMAL)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMBATVOLTHIGH); //BSM�����ѹ����
				}
				else if(BMS_BSM_Context[GUN_B].State0.OneByte.ChargeCurBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMCURRUP);  //BMS����
				}
				else if(BMS_BSM_Context[GUN_B].State0.OneByte.BatteryTempBits == STATE_TROUBLE)
				{
					APP_Set_ERR_Branch(GUN_B,STOP_BSMTEMPUP); //BMS���¶�
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
        {//�յ�BMS����ֹ���(BST)����,����10msһ�����ڣ�
            //������ֹԭ��ΪBMS����ֹͣ
            SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.BMSStopBits = STATE_TROUBLE;
			
			//BST��ֹͣԭ����
			//CheckBSTErr(GUN_B,(INT8U*)&stoptype);   //����Ĺ���ԭ����д
			APP_Set_ERR_Branch(GUN_B,STOP_BSMNORMAL);		//�������ԭ�򣬾���Ϊ��BMS����ֹͣ����Щ��������ֹͣҲ��������ģ�   20211015
			printf("11126\r\n");
            break;
        }
        if (result == APP_END_CHARGING)
        {//�յ�׮�屾��Ľ����������
            SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.UserStopBits = STATE_TROUBLE;
            stoptype = STOP_USER; //�û�����ֹͣ������ԭ��ǰ���Ѿ�ϸ��
			SysState[GUN_B].TotalErrState |= ERR_USER_STOP_TYPE;
			printf("User Stop1\r\n");
            break;
        }
        if (GetChargePauseState(GUN_B) == TRUE)
        {//��ͣ���״̬��
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_PAUSE_STEP, SYS_DELAY_10M) )
            {//CST��д������ֹ
                SysState[GUN_B].StopReason.ChargeStop.State0.OneByte.ErrStopBits = STATE_TROUBLE;
                stoptype = STOP_ERR; //����ֹͣ
				SysState[GUN_B].TotalErrState	|= ERR_CHARGESTOP_TYPE;
				APP_Set_ERR_Branch(GUN_B,STOP_OTHERERR);
				printf("11111\r\n");
                break;
            }
        }

        if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BEM_SUCCESS)
        {//�յ�BMS���͵Ĵ�����BEM
            //���볬ʱ����
            ChargeErrSwitch(GUN_B);
			NB_WriterReason(GUN_B,"E19",3);
			printf("GUN_B 14\r\n");
            return FALSE;
        }
		
		if(GetBMSStartCharge(GUN_B) == FALSE)    //�����������  20211117
		{
			printf("GUN_B startcharge State Err\r\n");
			break;
		}
    }

	//���˴�ֻ���յ�BST�����յ����·��Ľ������֡
    //���BST��ʱ�ˣ���Ҫ����CEM���ģ���Ҫȥ�ȴ�BSD�ˣ�������缴��
    //���BSTδ��ʱ����Ҫȥ�ȴ�BSD
	SetBMSConnectStep(GUN_B,BMS_CONNECT_END);
    result = APP_DEFAULT;
    //���ͳ�����ֹ��籨��(CST)��BMS
    BMS_CONNECT_StepSet(GUN_B,BMS_CST_SEND);
    BMS_TIMEOUT_ENTER(GUN_B,BMS_BST_STEP, SYS_DELAY_5s);
	BMS_TIMEOUT_ENTER(GUN_B,BMS_BSD_STEP, SYS_DELAY_10s); //BSD�ĳ�ʱҲ�Ǵӷ���CST��ʼ����
	//�������յ�BST֡
    if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BST_SUCCESS)//BST�Ѿ�����
    {
    	SetCEMState(GUN_B,CEM_BST, ERR_TYPE_NORMAL);
        send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
        send_message11.DivNum = APP_CHARGE_END;  //ֹͣ�������
		NB_WriterReason(GUN_B,"E2",2);
        send_message11.DataLen = 1;
		send_message11.GunNum = GUN_B;
        send_message11.pData = (INT8U *)&stoptype;
        OSQPost(Control_PeventB, &send_message11);
        OSTimeDly(SYS_DELAY_10ms);
		printf("Get BST\r\n");
    }
    else//���CST�ȷ��͹�ȥ
    {
	    //��Ҫ�����µ�ǰ״̬������CST����ҪBMS�ظ�һ��BST
	    BMS_CONNECT_Control[GUN_B].currentstate = BMS_CONNECT_DEFAULT;
        while (1)
        {
            result = WaitConnectBMSmessageB(SYS_DELAY_20ms);
            if(result == APP_RXDEAL_SUCCESS)//���յ�����
            {
                if(BMS_CONNECT_Control[GUN_B].currentstate == BMS_BST_SUCCESS)//BST�Ѿ�����
                {
                	SetCEMState(GUN_B,CEM_BST, ERR_TYPE_NORMAL);
                    send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message11.DivNum = APP_CHARGE_END;  //ֹͣ�������
//					NB_WriterReason(GUN_B,"E1",2);
                    send_message11.DataLen = 1;
					send_message11.GunNum = GUN_B;
                    send_message11.pData = (INT8U *)&stoptype;
                    OSQPost(Control_PeventB, &send_message11);
                    OSTimeDly(SYS_DELAY_10ms);
					printf("First CST,Get BST\r\n");
                    break;//������ǰѭ��
                }             
            }
            if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BST_STEP, SYS_DELAY_5s) )
            {
                //BST��ʱ
                //���ý���BST��ʱ״̬
                //�˴�BST����Ҫ���жϳ�ʱ�ģ�����������3��������
                send_message11.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
//				NB_WriterReason(GUN_B,"E0",2);
                send_message11.DivNum = APP_CHARGE_END;  //ֹͣ�������
                send_message11.DataLen = 1;
				send_message11.GunNum = GUN_B;
                send_message11.pData = (INT8U *)&stoptype;
                OSQPost(Control_PeventB, &send_message11);
                SetCEMState(GUN_B,CEM_BST, ERR_TYPE_TIMEOUT);
//                BMS_CONNECT_StepSet(BMS_CEM_SEND);
                ChargeErrDeal(GUN_B,ERR_L3_CHAGING_OVER_TIMEOUT);//ֱ�ӽ������
				printf("BST Timeout\r\n");
                return FALSE;
            }
			//�˴��ж���CST֡�Ƿ��޸��ˣ��˴�Ҫ��֤һֱ�ڷ���CST
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
* Description   : �������׶�
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
	//BSD�ĳ�ʱ�Ǵӷ���CST��ʼ�����10��
	//����˴���˵���Ѿ��յ���BST
    if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BSD_SUCCESS)
    {
    	//�յ�BSD���ٷ���CSD
    	BMS_CONNECT_StepSet(GUN_A,BMS_CSD_SEND);
    	SetCEMState(GUN_A,CEM_BSD, ERR_TYPE_NORMAL);
        //������,�յ�BSD����
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
            //���յ�BMSͳ������(BSD)����
            if(result == APP_RXDEAL_SUCCESS)
            {
                if (BMS_CONNECT_Control[GUN_A].currentstate == BMS_BSD_SUCCESS)
                {
                	//�յ�BSD���ٷ���CSD
    				BMS_CONNECT_StepSet(GUN_A,BMS_CSD_SEND);
                	SetCEMState(GUN_A,CEM_BSD, ERR_TYPE_NORMAL);
                    //������,�յ�BSD����
                    send_message30.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message30.DivNum = APP_CHARGE_BSD;
                    send_message30.DataLen = sizeof(_BMS_BSD_CONTEXT);
					send_message30.GunNum =GUN_A;
                    send_message30.pData = (INT8U *)&BMS_BSD_Context[GUN_A].EndChargeSOC;
                    OSQPost(Control_PeventA, &send_message30);
                    break;//������ǰѭ��
                }
            }
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_A,BMS_BSD_STEP, SYS_DELAY_10s))//BSD 10s��ʱ
            {
                //��ʱ����
                //���ý���BSD��ʱ״̬
                SetCEMState(GUN_A,CEM_BSD, ERR_TYPE_TIMEOUT);
                ChargeErrDeal(GUN_A,ERR_L3_CHAGING_OVER_TIMEOUT);//ֱ�ӽ������,���Ǵ���ʽB
                return FALSE;
            }
			//�˴��ж���CST֡�Ƿ��޸��ˣ��˴�Ҫ��֤һֱ�ڷ���CST
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
* Description   : �������׶�
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
	//BSD�ĳ�ʱ�Ǵӷ���CST��ʼ�����10��
	//����˴���˵���Ѿ��յ���BST
    if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BSD_SUCCESS)
    {
    	//�յ�BSD���ٷ���CSD
    	BMS_CONNECT_StepSet(GUN_B,BMS_CSD_SEND);
    	SetCEMState(GUN_B,CEM_BSD, ERR_TYPE_NORMAL);
        //������,�յ�BSD����
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
            //���յ�BMSͳ������(BSD)����
            if(result == APP_RXDEAL_SUCCESS)
            {
                if (BMS_CONNECT_Control[GUN_B].currentstate == BMS_BSD_SUCCESS)
                {
                	//�յ�BSD���ٷ���CSD
    				BMS_CONNECT_StepSet(GUN_B,BMS_CSD_SEND);
                	SetCEMState(GUN_B,CEM_BSD, ERR_TYPE_NORMAL);
                    //������,�յ�BSD����
                    send_message30.MsgID = (_BSP_MSGID)BSP_MSGID_BMS;
                    send_message30.DivNum = APP_CHARGE_BSD;
                    send_message30.DataLen = sizeof(_BMS_BSD_CONTEXT);
					send_message30.GunNum =GUN_B;
                    send_message30.pData = (INT8U *)&BMS_BSD_Context[GUN_B].EndChargeSOC;
                    OSQPost(Control_PeventB, &send_message30);
                    break;//������ǰѭ��
                }
            }
            if(BMS_TIME_OUT == BMS_TIMEOUT_CHECK(GUN_B,BMS_BSD_STEP, SYS_DELAY_10s))//BSD 10s��ʱ
            {
                //��ʱ����
                //���ý���BSD��ʱ״̬
                SetCEMState(GUN_B,CEM_BSD, ERR_TYPE_TIMEOUT);
                ChargeErrDeal(GUN_B,ERR_L3_CHAGING_OVER_TIMEOUT);//ֱ�ӽ������,���Ǵ���ʽB
                return FALSE;
            }
			//�˴��ж���CST֡�Ƿ��޸��ˣ��˴�Ҫ��֤һֱ�ڷ���CST
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
* Description   : ʵ�ָ�BMS�����Է��͵�����
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
    _BSP_MESSAGE *pMSG;   // ������Ϣָ��

    TaskSendFrame_preventA = OSQCreate(txBMSCANOSQA, BMSCANOSQ_NUM); // �����¼�(��Ϣ����)

    OSTimeDlyHMSM(0, 0, 0, 500); //��ʱ500MS��������������
    while(1)
    {
        nowSysTime = OSTimeGet();
		
        pMSG = OSQPend(TaskSendFrame_preventA, SYS_DELAY_10ms, &err);
        if(err == OS_ERR_NONE)
        {
            //������BMSͨѶ��ǰ״̬
            switch((_SEND_BMS_STEP)pMSG->MsgID)
            {
                case BMS_CHM_SEND: //����������ֽ׶α���
                case BMS_CRM_SEND: //������ʶ����
                    CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //��һ�η��ͺ���CHM����CRM,
                    break;
                case BMS_PARA_SEND: //��������ʱ��ͬ����Ϣ���ļ���������������
//                    CAN_Send_Table[(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //��һ�η��ͺ���,������Ҫ����ô��
                    CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //��һ�η��ͺ���,
                    break;
				case BMS_CTS_SEND:
					CAN_Send_Table[GUN_A][2].CANSendFrametoBMS();//ʹ�ü����ۺ��ľɹ��공�����û��CTS���ģ��ͻᱨ��������ʱ����һ�� 
					break;
                case BMS_CRO_SEND:
                case BMS_CRO_UNREADY_SEND:
                case BMS_CCS_SEND:
                case BMS_CST_SEND:
                case BMS_CSD_SEND:
                case BMS_CEM_SEND:    
                    CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //��һ�η��ͺ���,
                    break;
				case BMS_CEM_CST_SEND:
					CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //����CEM
					CAN_Send_Table[GUN_A][(INT8U)(pMSG->MsgID) + 1].CANSendFrametoBMS();  //����CST
                    break;
				default:
					break;
            }
            oldSysTime = nowSysTime;//��һ���Ѿ����ͣ��ȱ��ݷ��ͺ��ʱ�䡣
        }
        else
        {
            for(count = 0; count < CANSENDTALELEN; count++)
            {
                if(BMS_CONNECT_Control[GUN_A].step  == CAN_Send_Table[GUN_A][count].step)//����ʲô�׶�����
                {
                    //��������ֵ,OSTimeGetֵһ���Ѿ��ܴ��ˣ����49��ʱ����,�߼��ϻ���Ҫ�����¡�
                    if((nowSysTime >= oldSysTime) ? ((nowSysTime - oldSysTime) >= CAN_Send_Table[GUN_A][count].cycletime ) : \
                       ((nowSysTime + (INT32U_MAX_NUM - oldSysTime)) >= CAN_Send_Table[GUN_A][count].cycletime) )
                    {
//                        if(!CanFrameControl.state)
                        CAN_Send_Table[GUN_A][count].CANSendFrametoBMS();//���ͺ���
                        oldSysTime = nowSysTime;
                    }
                }
            }
        }
		for(i=0;i<3;i++)
		{//���⴦��BMV\BMT\BSP
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
* Description   : ʵ�ָ�BMS�����Է��͵�����
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
    _BSP_MESSAGE *pMSG;   // ������Ϣָ��

    TaskSendFrame_preventB = OSQCreate(txBMSCANOSQB, BMSCANOSQ_NUM); // �����¼�(��Ϣ����)

//while(1)
//{
//	OSTimeDlyHMSM(0, 0, 0, 500); //��ʱ500MS��������������
//	CHM_SendShakeHandFrameB();
//}
	OSTimeDlyHMSM(0, 0, 0, 500); //��ʱ500MS��������������
    while(1)
    {
        nowSysTime = OSTimeGet();
		
        pMSG = OSQPend(TaskSendFrame_preventB, SYS_DELAY_10ms, &err);
        if(err == OS_ERR_NONE)
        {
            //������BMSͨѶ��ǰ״̬
            switch((_SEND_BMS_STEP)pMSG->MsgID)
            {
                case BMS_CHM_SEND: //����������ֽ׶α���
                case BMS_CRM_SEND: //������ʶ����
                    CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //��һ�η��ͺ���CHM����CRM,
                    break;
                case BMS_PARA_SEND: //��������ʱ��ͬ����Ϣ���ļ���������������
//                    CAN_Send_Table[(INT8U)(pMSG->MsgID) - 1].CANSendFrametoBMS(); //��һ�η��ͺ���,������Ҫ����ô��
                    CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //��һ�η��ͺ���,
                    break;
				case BMS_CTS_SEND:
					CAN_Send_Table[GUN_B][2].CANSendFrametoBMS();//ʹ�ü����ۺ��ľɹ��공�����û��CTS���ģ��ͻᱨ��������ʱ����һ�� 
					break;
                case BMS_CRO_SEND:
                case BMS_CRO_UNREADY_SEND:
                case BMS_CCS_SEND:
                case BMS_CST_SEND:
                case BMS_CSD_SEND:
                case BMS_CEM_SEND:    
                    CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //��һ�η��ͺ���,
                    break;
				case BMS_CEM_CST_SEND:
					CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID)].CANSendFrametoBMS();  //����CEM
					CAN_Send_Table[GUN_B][(INT8U)(pMSG->MsgID) + 1].CANSendFrametoBMS();  //����CST
                    break;
				default:
					break;
            }
            oldSysTime = nowSysTime;//��һ���Ѿ����ͣ��ȱ��ݷ��ͺ��ʱ�䡣
        }
        else
        {
            for(count = 0; count < CANSENDTALELEN; count++)
            {
                if(BMS_CONNECT_Control[GUN_B].step  == CAN_Send_Table[GUN_B][count].step)//����ʲô�׶�����
                {
                    //��������ֵ,OSTimeGetֵһ���Ѿ��ܴ��ˣ����49��ʱ����,�߼��ϻ���Ҫ�����¡�
                    if((nowSysTime >= oldSysTime) ? ((nowSysTime - oldSysTime) >= CAN_Send_Table[GUN_B][count].cycletime ) : \
                       ((nowSysTime + (INT32U_MAX_NUM - oldSysTime)) >= CAN_Send_Table[GUN_B][count].cycletime) )
                    {
//                        if(!CanFrameControl.state)
                        CAN_Send_Table[GUN_B][count].CANSendFrametoBMS();//���ͺ���
                        oldSysTime = nowSysTime;
                    }
                }
            }
        }
		for(i=0;i<3;i++)
		{//���⴦��BMV\BMT\BSP
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
    InitConnectBMSACAN();     //��ʼ������BMS����ϵͳ��can
    ClearCanFrameInformation(GUN_A);//��ʼ�����һ�½ṹ��
    OSTimeDlyHMSM(0, 0, 0, SYS_DELAY_500ms); //��ʱ500MS��������������
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
        if ((GetChargeMode(GUN_A) != MODE_AUTO) && (GetChargeMode(GUN_A) != MODE_VIN))   //���Զ�ģʽ
        {
            if (BMS_CONNECT_Control[GUN_A].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
            }
            continue;
        }

        if (GetBMSStartCharge(GUN_A) == FALSE) 
        {//�յ����ָ��
            if (BMS_CONNECT_Control[GUN_A].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
            }
            continue;
        }
		
		if (ChargeBMSSelfCheck(GUN_A) == FALSE)  //ǹ��������������
        {
            continue;
        }
#endif

			OSQFlush(TaskConnectBMS_peventA);
			APP_SetGBType(GUN_A,BMS_GB_2015);         //ÿ��Ĭ�����¹���
			if (ChargeBMSHandshakeA() == FALSE)//����
			{//���ֳ�ʱ
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			if (ChargeParameterConfigStepA() == FALSE)//��������
			{//�������ó�ʱ
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
		
			if (ChargeRunningStepA() == FALSE) //���׶�
			{//�����̳�ʱ
				BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			ChargeEndStepA();            //�������׶�
			SetBMSStartCharge(GUN_A,FALSE);
			printf("GUN_A 466656589\r\n");
        //ֹͣ�����κ�֡
        BMS_CONNECT_StepSet(GUN_A,BMS_SEND_DEFAULT);
        BMS_CONNECT_ControlInit(GUN_A);  //���³�ʼ��
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
    InitConnectBMSBCAN();     //��ʼ������BMS����ϵͳ��can
    ClearCanFrameInformation(GUN_B);//��ʼ�����һ�½ṹ��
    OSTimeDlyHMSM(0, 0, 0, SYS_DELAY_500ms); //��ʱ500MS��������������
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
        {//���Զ�ģʽ
            if (BMS_CONNECT_Control[GUN_B].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
            }
            continue;
        }
		
        if (GetBMSStartCharge(GUN_B) == FALSE) 
        {//�յ����ָ��
            if (BMS_CONNECT_Control[GUN_B].step != BMS_SEND_DEFAULT)
            {
                BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
            }
            continue;
        }
		printf("BMS GUN B Start/r/n");
		if (ChargeBMSSelfCheck(GUN_B) == FALSE)
        {//ǹ��������������
            continue;
        }
#endif
		OSQFlush(TaskConnectBMS_peventB);
			APP_SetGBType(GUN_B,BMS_GB_2015);         //ÿ��Ĭ�����¹���
			if (ChargeBMSHandshakeB() == FALSE)//����
			{//���ֳ�ʱ
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			if (ChargeParameterConfigStepB() == FALSE)//��������
			{//�������ó�ʱ
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			if (ChargeRunningStepB() == FALSE) //���׶�
			{//�����̳�ʱ
				BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
				  OSTimeDly(SYS_DELAY_500ms);
				continue;
			}
			ChargeEndStepB();            //�������׶�
			printf("BMS GUN B STOP/r/n");
			SetBMSStartCharge(GUN_B,FALSE);
			        //ֹͣ�����κ�֡
			BMS_CONNECT_StepSet(GUN_B,BMS_SEND_DEFAULT);
			BMS_CONNECT_ControlInit(GUN_B);  //���³�ʼ��
    }
}

/************************(C)COPYRIGHT 2010 �����Ƽ�*****END OF FILE****************************/
