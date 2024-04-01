/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: connectbms.h
* Author			: Ҷϲ��
* Date First Issued	: 10/29/2013
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
//BMS���궨��
#define BMS_OLD_GB2011        (1u)            //2011���Ϲ���
#define BMS_NEW_GB2015        (2u)            //2015���¹���
//��ǰBMS����ѡ��ֻ�ܶ�ѡһ
//#define BMS_CUR_GB            BMS_OLD_GB2011 //�ɹ���
#define BMS_CUR_GB            BMS_NEW_GB2015 //�¹���

#define BMSCANOSQ_NUM         (32u)      //����BMS��CAN��Ϣ����
#define BMSCANRX_BUFF_LEN     (16u)      //���ջ��峤��

#define BMS_RX_LEN            (256u)     //���յ�BMS���ݵ�buffer���棬��ҳ����ȫ�����������ﴦ��

//#define BMS_ERR_HANDLE_LEN    (17u)      //BMS���ϴ������
#define BMS_ERR_HANDLE_LEN    (4u)      //BMS���ϴ������

#define BMS_MSG_TABLE_LEN     (1u)       //���տ��������͵���Ϣ����

#define BMS_ERR_RETRY_NUM     (3u)       //BMS�������Դ���
/* Private typedef----------------------------------------------------------------------------*/
typedef enum
{
    BMS_GB_2011,    //2011
    BMS_GB_2015,    //2015
}_BMS_CUR_GB;
//���ϴ������Դ���
typedef enum
{
    ERR_RETRY_DEC,      //���ϴ��������1
    ERR_RETRY_INC,      //���ϴ��������1      
    ERR_RETRY_CLEAR,    //���ϴ��������0
    ERR_RETRY_SET,      //���ϴ������ΪBMS_ERR_RETRY_NUM
}_BMS_ERR_RETRY;


typedef enum
{
    ERR_TYPE_NORMAL = 0, //00:����
    ERR_TYPE_TIMEOUT,    //01:��ʱ
    ERR_TYPE_UNTRUST,    //02:������
}_BMS_ERR_STATE_TYPE;

typedef enum
{
	BMS_CHARGE_PAUSE = 0, //00:��ͣ���
	BSM_CHARGE_ENABLE,	  //01:�������
}_BMS_CHARGE_PAUSE_STATE;

//CEM��������
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

//���ϵȼ�1 2 3
typedef enum
{
    ERR_LEVEL_DEFAULT = 0,  //���ϵȼ���
    ERR_LEVEL_1 = 1,        //���ϵȼ�1
    ERR_LEVEL_2,
    ERR_LEVEL_3,  
}_BMS_ERR_LEVEL;
//�����Ϸ���
typedef enum
{
    //���ϼ���Ϊ1
    ERR_L1_INSULATION = 0x01,      //��Ե����
    ERR_L1_LEAKAGE,                //©�����
    ERR_L1_EMERGENCY,              //��ͣ����

    //���ϼ���Ϊ2
    ERR_L2_CONNECT = 0x10,         //����������
    ERR_L2_CONN_TEMP_OVER,         //BMSԪ�����������������
    ERR_L2_BGROUP_TEMP_OVER,       //������¶ȹ���
    ERR_L2_BAT_VOL,                //�����ص�ѹ���͡�����
    ERR_L2_BMS_VOL_CUR,            //BMS��⵽��������������ѹ�쳣
    ERR_L2_CHARGE_VOL_CUR,         //������⵽��������ƥ������ѹ�쳣
    ERR_L2_CHARGE_TEMP_OVER,       //�����ڲ�����
    ERR_L2_CHARGE_POWER_UNARRIVAL, //�����������ܴ���
    ERR_L2_CAR_CONN_ADHESION,      //�����Ӵ���ճ��

    //���ϼ���Ϊ3
    ERR_L3_CHAGING_TIMEOUT = 0x20, //������ֽ׶Ρ����ý׶Ρ������̳�ʱ
    ERR_L3_CHAGING_OVER_TIMEOUT,   //��������ʱ

    ERR_LF_METHOD_A = 0x30,        //ֱ�Ӳ��ô���ʽ��Ϊ����
    ERR_LF_METHOD_B,
    ERR_LF_METHOD_C,
}_BMS_ERR_TYPE;

//���ϴ���ʽ
typedef enum
{
    ERR_METHOD_UNDEF = 0,          //δ����ķ�ʽ 
    ERR_METHOD_A = 1,              //��ʽA:��������ͣ��ͣ��
    ERR_METHOD_B,                  //��ʽB:ֹͣ���γ�磬���ɽ��׼�¼(�����²��ǹ���ܽ�����һ�γ��)
    ERR_METHOD_C,                  //��ʽC:��ֹ��磬�����������ų�������ͨ�����ֿ�ʼ���
    ERR_METHOD_END,                //ֱ�ӽ���
    ERR_METHOD_BMS,                //BMS����BEM����
}_BMS_ERR_MOTHOD;

//����״̬
typedef enum
{
    ERR_STATE_NORMAL = 0,          //�޴���
    ERR_STATE_METHOD_A,            //����ʽA
    ERR_STATE_METHOD_B,            //����ʽB
    ERR_STATE_METHOD_C,            //����ʽC
	ERR_STATE_METHOD_END,          //ֱ�ӽ���
    ERR_STATE_BMS,
}_BMS_ERR_STATE;

typedef enum
{
    BMS_CONNECT_DEFAULT =0,
    BMS_RTS_SUCCESS = 1,      //01�յ�BMS���ص�RTS��֡���ݱ���
    BMS_BHM_SUCCESS,          //02�յ�BMS���ص�BHM�������ֱ���   
    BMS_BRM_SUCCESS,          //03�յ�BMS���ص�BRM������ʶ����    
    BMS_BCP_SUCCESS,          //04�յ�BMS���صĶ������س���������    
    BMS_BRO_SUCCESS,          //05�յ�BMS���صĵ�س��׼������״̬����
    BMS_BCL_SUCCESS,          //06�յ�BMS���صĵ�س��������
    BMS_BCS_SUCCESS,          //07�յ�BMS���صĵ�س����״̬����
    BMS_BSM_SUCCESS,          //08�յ�BMS���صĶ�������״̬��Ϣ����
    BMS_BMV_SUCCESS,          //09�յ�BMS���صĵ��嶯�����ص�ѹ����
    BMS_BMT_SUCCESS,          //10�յ�BMS���صĶ��������¶ȱ���
    BMS_BSP_SUCCESS,          //11�յ�BMS���صĶ�������Ԥ������
    BMS_BST_SUCCESS,          //12�յ�BMS���ص���ֹ��籨��
    BMS_BSD_SUCCESS,          //13�յ�BMS���ص�ͳ�����ݱ���
    BMS_BEM_SUCCESS,          //14�յ�BMS���صĴ�����
    //���׶εļ������������
    BMS_BCL_BCS_SUCCESS,
    //BMS_BCL_BCS_BSM_SUCCESS,
    RX_BMS_TIMEOUT,    //BMS��ʱ
}_CONNECT_BMS_STATE;   


//ר�ſ���һ�������������������̽ڵ��Ƿ�����أ�
typedef struct
{
    _SEND_BMS_STEP		step;
    _CONNECT_BMS_STATE  laststate;     //��һ״̬
    _CONNECT_BMS_STATE  currentstate;  //��ǰ״̬
    //_CONNECT_BMS_STATE  needreconnect; //��������   �� RX_BMS_TIMEOUT��������        
}_BMS_CONNECT_CONTROL;

extern _BMS_CONNECT_CONTROL BMS_CONNECT_Control[GUN_MAX];

typedef struct
{
    INT8U state;         //����֡��״̬���ڴ�����������Ϣ֡�����,1����ʾ��Ҫ��֡����
    INT8U nextpagenum;   //�����ж���ҳ��������ҳ��
    INT8U currentpagenum;//��ǰҳ��
    INT8U pagenum;       //ʵ�ʻ�ȡ��ҳ��
    INT8U IDtablepos;    //id�ڱ���е�λ�� 
    INT32U FrameID;      //֡��ID    
    INT16U datalength;   //���ݳ���  
}_CANFRAMECONTROL;

extern _CANFRAMECONTROL CanFrameControl[GUN_MAX];

//�յ������������Ϣ��ṹ
typedef struct
{
    INT8U DataID;
    INT8U (*function)(void *pdata);
}_RECEIVE_Q_TABLE;

//�����ϴ���ṹ
typedef struct
{
    _BMS_ERR_TYPE	type;                  	//����������
    _BMS_ERR_MOTHOD	method;                 //����ʽ
    INT8U (*fun)(_BMS_ERR_MOTHOD method);   //���ϴ�����
}_BMS_ERR_HANDLE;  

extern const _BMS_ERR_HANDLE BMSErrHandleTable[GUN_MAX][BMS_ERR_HANDLE_LEN];

typedef struct
{
	_BMS_ERR_LEVEL ErrLevle;    //���ϵȼ�
    _BMS_ERR_STATE ErrState;    //����״̬
    INT8U MethodCRetry;         //���Դ������ڹ���C��������
    INT8U SelfCheck;            //�Լ� TRUE---��Ҫ���²��ǹ�Լ�
    INT8U GunInput;             //ǹ���ӳ�
    INT8U ChargeRunning;        //����ģ���Ѿ����
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
	INT8U	Get_Flag;		//����PGN����յ���־
	INT8U	Count_Delay;	//�ӳټ���
}_SPECIAL_PGN;				//����PGN
typedef struct
{
	INT8U VinState;			//1��ʾ��Ȩ�ɹ� 0��ʾδ��Ȩ�ɹ�
}_VIN_CONTROL;
extern	_SPECIAL_PGN	Special_PGN[GUN_MAX][3];

extern OS_EVENT  	*TaskConnectBMS_peventA;    			//CAN��Ϣ����ָ��
extern BSPCANTxMsg   BMSCANTxbuffA;      				//���ͻ���
extern BSPCANRxMsg   BMSCANRxbuffA[BMSCANRX_BUFF_LEN];	//���ջ���
extern BSP_CAN_Set	 bxBMSCANSetA;

extern OS_EVENT  	*TaskConnectBMS_peventB;    			//CAN��Ϣ����ָ��
extern BSPCANTxMsg   BMSCANTxbuffB;      				//���ͻ���
extern BSPCANRxMsg   BMSCANRxbuffB[BMSCANRX_BUFF_LEN];	//���ջ���
extern BSP_CAN_Set	 bxBMSCANSetB;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BMS_CONNECT_ControlInit BMS_CONNECT_StateSet
* Description	: ������̿��ƽṹ�塣�����ú���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
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
* Description   : ��ȡ��������
* Input         :
* Output        :
* Note(s)       :
* Contributor   : wb 20160922
***********************************************************************************************/
_BMS_CUR_GB APP_GetGBType(INT8U gun);
/***********************************************************************************************
* Function      : APP_SetGBType
* Description   : ����
* Input         :
* Output        :
* Note(s)       :
* Contributor   : wb 20160922
***********************************************************************************************/
void APP_SetGBType(INT8U gun, _BMS_CUR_GB dat);
/*****************************************************************************
* Function      : APP_GetBMSBHM
* Description   : ��ȡBHM֡
* Input         : _BMS_BHM_CONTEXT *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016��7��19��  Ҷϲ��
*****************************************************************************/
INT8U APP_GetBMSBHM(INT8U gun,_BMS_BHM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BSD
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BSD(INT8U gun,_BMS_BSD_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BCP
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BCP(INT8U gun,_BMS_BCP_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_CML
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_CML(INT8U gun,_BMS_CML_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BRM
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BRM(INT8U gun,_BMS_BRM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BCL
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BCL(INT8U gun,_BMS_BCL_CONTEXT *pdata);

/***********************************************************************************************
* Function		: APP_GetBMS_BSM
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BSM(INT8U gun,_BMS_BSM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BST
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BST(INT8U gun,_BMS_BST_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BEM
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 
***********************************************************************************************/
void APP_GetBMS_BEM(INT8U gun,_BMS_BEM_CONTEXT *pdata);
/***********************************************************************************************
* Function		: APP_GetBMS_BEM
* Description	: ��ȡBMS����
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2016-07-11 \
***********************************************************************************************/
void APP_GetBMS_CEM(INT8U gun,_BMS_CEM_CONTEXT *pdata);

/***********************************************************************************************
* Function      : Set_VIN_Success
* Description   : VIN�Ƿ��Ȩ�ɹ�  1��ʾ�ɹ� �������ĵ�
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-07-11 Yxy
***********************************************************************************************/
void Set_VIN_Success(_GUN_NUM gun,_VIN_STATE state);

/***********************************************************************************************
* Function      : Set_VIN_Send
* Description   : VIN ����
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-07-11 Yxy
***********************************************************************************************/
void Set_VIN_Send(INT8U state);

/***********************************************************************************************
* Function      : Set_VIN_Send
* Description   : VIN ����
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-07-11 Yxy
***********************************************************************************************/
INT8U Get_VIN_Send(void);

#endif	//__CONNECT_BMS_H_
/************************(C)COPYRIGHT 2016 �����Ƽ�*****END OF FILE****************************/


