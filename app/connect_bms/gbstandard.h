/*****************************************Copyright(H)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: gbstandard.h
* Author			  :      
* Date First Issued	: 10/6/2015
* Version			  : V0.1
* Description		: �󲿷������������������ʼ��
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
#define PRI_SendFrame              6     //�����������ȼ�  

//��ʶ��
#define  GB_STANDARD_DEVICE_ID         0x56    // ����ID
#define  GB_STANDARD_BMS_ID            0xF4    // BMS��ID

//����Ȩ
#define  GB_PRIO_0                     0u
#define  GB_PRIO_1                     1u
#define  GB_PRIO_2                     2u
#define  GB_PRIO_3                     3u
#define  GB_PRIO_4                     4u
#define  GB_PRIO_5                     5u
#define  GB_PRIO_6                     6u
#define  GB_PRIO_7                     7u

//J1393����Э�����ӹ����ҳ����RTS֡PGN
#define  GB_RTS_PGN_235                60160u   //��֡ʵ���������PGN,��DT
#define  GB_RTS_PGN_236                60416u   //��֡�������PGN����RTS��CTS��EndOfMsgACK��Conn_Abort

//PGN��������
//���ֽ׶�
#define  GB_CHM_PGN_9728               9728u
#define  GB_BHM_PGN_9984               9984u

#define  GB_CRM_PGN_256                256u
#define  GB_BRM_PGN_512                512u
//���������ý׶�
#define  GB_BCP_PGN_1536               1536u
#define  GB_CTS_PGN_1792               1792u
#define  GB_CML_PGN_2048               2048u
#define  GB_BRO_PGN_2304               2304u
#define  GB_CRO_PGN_2560               2560u
//���׶�
#define  GB_BCL_PGN_4096               4096u
#define  GB_BCS_PGN_4352               4352u
#define  GB_CCS_PGN_4608               4608u
#define  GB_BSM_PGN_4864               4864u
#define  GB_BMV_PGN_5376               5376u
#define  GB_BMT_PGN_5632               5632u
#define  GB_BSP_PGN_5888               5888u
#define  GB_BST_PGN_6400               6400u
#define  GB_CST_PGN_6656               6656u

//�������׶�
#define  GB_BSD_PGN_7168               7168u
#define  GB_CSD_PGN_7424               7424u
//������
#define  GB_BEM_PGN_7680               7680u
#define  GB_CEM_PGN_7936               7936u


#define  GB_BMSSTATE_NOTREADY          0x00  //BMSδ���
#define  GB_BMSSTATE_STANDBY           0xAA  //BMS��ɳ��׼��
#define  GB_BMSSTATE_INVALID           0xFF  //BMS״̬��Ч

#define  GB_CHARGESTATE_NOTREADY       0x00  //����δ׼����
#define  GB_CHARGESTATE_STANDBY        0xAA  //������ɳ��׼��
#define  GB_CHARGESTATE_INVALID        0xFF  //����״̬��Ч


//����ʽֻ��Թ���Ҫ���PDU1��ʽ�����м�8λ��PSΪĿ���ֵַ�������ֽ�8λλԴ��ַ
//���㱨�ĵ�ID����:����Ȩ | R=0 |DFP =0 |  PF = PGN/256  | Ŀ�ĵ�ַ(BMS) | Դ��ַ
//������ID��ʽ
#define GB_CONFIGSEND_ID(P,PGN)    (INT32U)(((INT32U)(P&0x07) <<26))| ((((INT32U)PGN>>8)&0x00FF)<<16)\
                                   | ((INT32U)GB_STANDARD_BMS_ID << 8) | GB_STANDARD_DEVICE_ID
//������ID��ʽ
#define GB_CONFIGRECEIVE_ID(P,PGN) (INT32U)(((INT32U)(P&0x07) <<26))| ((((INT32U)PGN>>8)&0x00FF)<<16)\
                                   | ((INT32U)GB_STANDARD_DEVICE_ID << 8) | GB_STANDARD_BMS_ID
                                   
                                   
#define CANRECEIVETABLELEN  14u        

#define INT32U_MAX_NUM          (0xFFFFFFFFu)
/* Private typedef----------------------------------------------------------------------------*/
//����ȡֵѡֵ����BMSЭ�鶨��һ��
typedef enum
{
    STATE_NORMAL = 0,   //����
    STATE_TROUBLE,      //����
    STATE_UNTRUST,      //������
    //����ȡֵ�����
    STATE_LOW = 10,     //����
    STATE_HIGH,         //����
}_BMS_ERR_VALUE;

//BMS����״̬
typedef enum
{
    BMS_STEP_DEFAULT =0,
    BMS_RTS_STEP = 1,//01BMS RTS����  
    BMS_BHM_STEP,    //02BMS������ֽ׶α���
    BMS_BRM_STEP,    //03BMS��ʶ����  
    BMS_BCP_STEP,    //04�������س�����  
    BMS_BRO_STEP,    //05��س��׼������״̬
    BMS_BCL_STEP,    //06��س������  
    BMS_BCS_STEP,    //07��س����״̬  
    BMS_BSM_STEP,    //08��������״̬��Ϣ  
    BMS_BMV_STEP,    //09���������¶�  
    BMS_BMT_STEP,    //10�������ص�ѹ  
    BMS_BSP_STEP,    //11��������Ԥ������
    BMS_BST_STEP,    //12BMS��ֹ���
    BMS_BSD_STEP,    //13BMSͳ������
    BMS_BEM_STEP,    //14����ͳ������
    BMS_OTH_STEP,    //15������ʱ
    BMS_OTH1_STEP,   //16����1��ʱ
    BMS_OTH2_STEP,   //17����2��ʱ		
    BMS_BSD_TIMEOUT, //18LCD��ȡBSD��ʱ
    BMS_FAULT_SETP,  //19������ϳ�ʱ
    BMS_PAUSE_STEP,  //20BMS��ͣ��糬ʱ
	BMS_BCL_DEMAND_UPDATA,//21BMS������¼���
    BMS_MAX_STEP,    //22�������
    //��ʱ
    BMS_TIME_OUT,
    //��������
    BMS_PARA_ERR,
}_BMS_STEP;

//׮�����״̬
typedef enum
{
    BMS_SEND_DEFAULT = 0,
    BMS_CHM_SEND    = 1,  //01����������ֽ׶α��� 
    BMS_CRM_SEND,         //02������ʶ����
    //�������ý׶�
    BMS_PARA_SEND,        //03��������ʱ��ͬ����Ϣ����������������
    BMS_CRO_SEND,         //04�������׼������״̬(0xAA)
    BMS_CRO_UNREADY_SEND, //05�������׼������״̬(0x00)
    //���׶�
    BMS_CCS_SEND,         //06�������״̬
    BMS_CST_SEND,         //07������ֹ���
    BMS_CSD_SEND,         //08����ͳ������
    //������Ϣ
    BMS_CEM_SEND,         //09����������
    BMS_CEM_CST_SEND,	  //10CEM��CSTһ����
    BMS_CTS_SEND,         //11����CTSͬ��ʱ��
}_SEND_BMS_STEP; 
/* Private macro------------------------------------------------------------------------------*/
//BHM��������
__packed typedef struct
{
    INT16U  MaxChargeWholeVol;   //����������ܵ�ѹ
}_BMS_BHM_CONTEXT;

extern  _BMS_BHM_CONTEXT  BMS_BHM_Context[GUN_MAX];

//BRM��������
__packed typedef struct
{
    INT8U  BMSVersion[3];        //BMSͨ��Э��汾
    INT8U  BatteryType;          //�������
    INT16U RatedCapacity;        //������������ϵͳ����� /Ah;0.1Ah/λ
    INT16U RatedVol;             //������������ϵͳ���ѹ/V;0.1V/
    INT8U  BatteryFactory[4];    //���������������
    INT8U  BatterySerialNum[4];  //��������
    INT8U  BatteryProduceYear;   //�����������1��/bit  1985--2235
    INT8U  BatteryProduceMonth;  //�����������1��/bit  
    INT8U  BatteryProduceDay;    //�����������1��/bit  
    INT8U  BatteryChargedFreq[3];//����������
    INT8U  BatteryRightFlag;     //������Ȩ��ʾ
    INT8U  defaultbyte;          //Ԥ���ֽ�
    INT8U  VIN[17];              //����ʶ����
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //�¹��� 
    INT8U  BMSSoftwareVer[8];    //BMS����汾�ţ��¹������ݣ�
#endif //#if (BMS_CUR_GB == BMS_NEW_GB2015)

}_BMS_BRM_CONTEXT;

extern  _BMS_BRM_CONTEXT BMS_BRM_Context[GUN_MAX];

//BCP��������
//size=13
__packed typedef struct
{
    INT16U UnitBatteryMaxVol;    //���嶯����������������ѹ
    INT16U MaxCurrent;           //������������
    INT16U BatteryWholeEnergy;   //�������ر��������
    INT16U MaxVoltage;           //����������ܵ�ѹ
    INT8U  MaxTemprature;        //��������¶�
    INT16U BatterySOC;           //�������غɵ�״̬:SOC
    INT16U BatteryPresentVol;    //�����������ص�ǰ��ص�ѹ,0.1V/λ
}_BMS_BCP_CONTEXT;

extern _BMS_BCP_CONTEXT BMS_BCP_Context[GUN_MAX];

//BRO��������
__packed typedef struct
{
    INT8U StandbyFlag;      //BMS�Ƿ��Ѿ�׼����
}_BMS_BRO_CONTEXT;

extern _BMS_BRO_CONTEXT BMS_BRO_Context[GUN_MAX];

//BCL��������
__packed typedef struct
{
    INT16U DemandVol;      //��ѹ����V
    INT16U DemandCur;      //��������A
    INT8U  ChargeMode;     //���ģʽ  01����ѹ���   02���������
}_BMS_BCL_CONTEXT;

extern _BMS_BCL_CONTEXT BMS_BCL_Context[GUN_MAX];

//BCS��������
__packed typedef struct
{
    INT16U ChargeVolMeasureVal;      //����ѹ����ֵ
    INT16U ChargeCurMeasureVal;      //����������ֵ
    INT16U MaxUnitVolandNum;         //��ߵ��嶯�����ص�ѹ�Լ����
    INT8U  SOC;                      //��ǰ�ɵ�״̬%
    INT16U RemainderTime;            //ʣ����ʱ��min
}_BMS_BCS_CONTEXT;

extern _BMS_BCS_CONTEXT BMS_BCS_Context[GUN_MAX];

//BSM��������
__packed typedef struct
{
    INT8U  MaxUnitVolandNum;         //��ߵ��嶯�����ص�ѹ��������
    INT8U  MaxbatteryTemprature;     //��߶��������¶�
    INT8U  MaxTempMeasurepnum;       //����¶ȼ�����
    INT8U  MinbatteryTemprature;     //��Ͷ�����������
    INT8U  MinTempMeasurepnum;       //��Ͷ��������¶ȼ�����
#if 0  
    INT16U UnitVolState;             //���嶯�����ص�ѹ���ߣ� 00���� 01���� 10����
    INT16U WholeSOCState;            //�����������غɵ�״̬��00���� 01���� 10����
    INT16U ChargeCurState;           //�������س������� 00 ���� 01����  10 ������״̬
    INT16U BatteryTempState;         //���������¶ȣ�00����  01����  10������״̬
    INT16U BatteryInsulationState;   //�������ؾ�Ե״̬��00 ���� 01 ������ 10 ������״̬
    INT16U BatteryConnectState;      //���������������������״̬ 00 ����  01������  10 ������״̬
    INT16U ChargeAllow;              //������� 00 ��ֹ 01����    
#endif
    __packed union
    {
        INT8U AllBits;          
        __packed struct
        {
            INT8U UnitVolBits:2;      //���嶯�����ص�ѹ���ߣ� 00���� 01���� 10����
            INT8U WholeSOCBits:2;     //�����������غɵ�״̬��00���� 01���� 10����
            INT8U ChargeCurBits:2;    //�������س������� 00 ���� 01����  10 ������״̬
            INT8U BatteryTempBits:2;  //���������¶ȣ�00����  01����  10������״̬
        }OneByte;
    }State0;
    
    __packed union
    {
        INT8U AllBits;          
        __packed struct
        {
            INT8U BatteryInsulationBits:2; //�������ؾ�Ե״̬��00 ���� 01 ������ 10 ������״̬
            INT8U BatteryConnectBits:2;    //���������������������״̬ 00 ����  01������  10 ������״̬
            INT8U ChargeAllowBits:2;       //������� 00 ��ֹ 01����   
            INT8U ResverBist:2;            //Ԥ��
        }OneByte;
    }State1;

}_BMS_BSM_CONTEXT;

extern _BMS_BSM_CONTEXT BMS_BSM_Context[GUN_MAX];


//BST��������
__packed typedef struct
{
    __packed union
    {
        INT8U EndChargeReason;          //BMS��ֹ���ԭ��
        __packed struct
        {
            INT8U SocTargetBits:2;    //����SOCĿ��ֵ��00--δ�ﵽ  01--�ﵽ  10 --������
            INT8U TolVolValBits:2;    //�ܵ�ѹ�趨ֵ��00--δ�ﵽ   01--�ﵽ  10---������
            INT8U VolSetValBits:2;    //�����ѹ�趨ֵ��00--δ�ﵽ   01--�ﵽ  10---������
            INT8U ChargeStopBits:2;   //����������ֹ��00--����   01--������ֹ(�յ�CST֡)  10---������
        }OneByte;
    }State0;

    __packed union
    {
        INT16U  EndChargeTroubleReason;     //BMS��ֹ������ԭ��
        __packed struct
        {
            INT8U InsulationErrBits:2;     //��Ե���ϣ�00--����  01--����  10 --������
            INT8U ConnectorOverTempBits:2; //������������¹��ϣ�00--����  01--����  10 --������
            INT8U ElementOverTempBits:2;   //BMSԪ�����¹��ϣ�00--����  01--����  10 --������
            INT8U ConnectorErrBits:2;      //������������ϣ�00--����  01--����  10 --������
            INT8U BatOverTempBits:2;       //�������¹��ϣ�00--����  01--����  10 --������
            INT8U HighPresRelayBits:2;     //��ѹ�̵������ϣ�00--����  01--����  10 --������
            INT8U TPTwoVolErrBits:2;       //����2��ѹ�����ϣ�00--����  01--����  10 --������
            INT8U OtherErrBits:2;          //�������ϴ���00--����  01--����  10 --������
        }OneByte;
    }State1;

    __packed union
    {
        INT8U EndChargeErroReason;      //BMS��ֹ������ԭ��
        __packed struct
        {
            INT8U OverCurErrBits:2;     //BMS��������00--��������  01--������������ֵ  10 --������
            INT8U OverVolErrBits:2;     //BMS��ѹ����00--��ѹ����  01--��ѹ�쳣  10 --������
            INT8U ResverBits:4;         //Ԥ��
        }OneByte;
    }State2;
}_BMS_BST_CONTEXT;

extern _BMS_BST_CONTEXT BMS_BST_Context[GUN_MAX];
//�������׶�
//BSD��������
__packed typedef struct
{
    INT8U   EndChargeSOC;          //��ֹ�ɵ�״̬SOC%
    INT16U  UnitBatteryminVol;     //�������ص�����͵�ѹ
    INT16U  UnitBatteryMaxVol;     //�������ص�����ߵ�ѹ
    INT8U   BatteryMinTemp;        //������������¶�
    INT8U   BatteryMaxTemp;        //������������¶�  
}_BMS_BSD_CONTEXT;

extern _BMS_BSD_CONTEXT BMS_BSD_Context[GUN_MAX];

//������
//BEM��������,Ԫ���ñ��ļ����д�����»������֣������е㲻���ϱ�̹淶������ͳһ����ʶ���Ժ�Щ
//size = 4
__packed typedef struct
{
    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CRM00Bits:2;    //���ճ�����ʶ���ĳ�ʱ(0x00) 00--���� 01--��ʱ 10--������
            INT8U CRMAAits:2;     //���ճ�����ʶ���ĳ�ʱ(0xAA)
            INT8U RevBits:4;      //Ԥ��
        }OneByte;
    }State0;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CTS_CMLBits:2; //���ճ�����ʱ��ͬ���ͳ������������������ĳ�ʱ
            INT8U CROBits:2;     //���ճ�����ɳ��׼�����ĳ�ʱ
            INT8U RevBits:4;     //Ԥ��
        }OneByte;
    }State1;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CCSBits:2;    //���ճ������״̬��ʱ
            INT8U CSTBits:2;    //���ճ�����ֹ��籣�ĳ�ʱ
            INT8U RevBits:4;    //Ԥ��
        }OneByte;
    }State2;

    __packed union
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U CSDBits:2;    //���ճ���ͳ�Ʊ��ĳ�ʱ
            INT8U OthBits:6;    //��������
        }OneByte;
    }State3;
}_BMS_BEM_CONTEXT;

extern _BMS_BEM_CONTEXT BMS_BEM_Context[GUN_MAX];


//�����ǳ���Ҫ���͵�����
//CML��������
__packed typedef struct
{
    INT16U MaxOutVol;    //��������ѹ
    INT16U MinOutVol;    //��������ѹ
    INT16U MaxOutCur;    //����������
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //�¹��� 
    INT16U MinOutCur;    //����������
#endif   
}_BMS_CML_CONTEXT;

extern _BMS_CML_CONTEXT BMS_CML_Context[GUN_MAX];


//�������״̬����
__packed typedef struct
{
    INT16U OutputVol;       //��ѹ���ֵ��0.1V/λ
    INT16U OutputCur;       //�������ֵ��0.1A/λ��-400Aƫ����
    INT16U TotalChargeTime; //�ۼƳ��ʱ��(min)��1min/λ
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //�¹��� 
    INT8U ChargePause;      //�������00:��ͣ 01:����ֻ�е�2λ��Ч
#endif
}_BMS_CCS_CONTEXT;
extern _BMS_CCS_CONTEXT BMS_CCS_Context[GUN_MAX];

//������ֹ��籨��
__packed typedef struct
{
    __packed union
    {
        INT8U EndChargeReason;          //������ֹ���ԭ��
        __packed struct
        {
            INT8U ReachConditionBits:2; //�ﵽ�����趨��������ֹ��00--���� 01--�ﵽ���� 10--������
            INT8U UserStopBits:2;       //�û�������ֹ��00--���� 01--�û���ֹ 10--������
            INT8U ErrStopBits:2;        //������ֹ��00--���� 01--������ֹ 10--������
#if (BMS_CUR_GB == BMS_NEW_GB2015)   //�¹��� 
            INT8U BMSStopBits:2;        //BMS������ֹ��00--���� 01--BMS��ֹ(�յ�BST֡) 10--������
#else                                //�ɹ���
            INT8U ResverBits:2;         //Ԥ��
#endif
        }OneByte;
    }State0;

    __packed union
    {
        INT16U  EndChargeTroubleReason;     //������ֹ������ԭ��
        __packed struct
        {
            INT8U ChargeOverTempBits:2;    //�������¹��ϣ�00--����  01--����  10 --������
            INT8U ConnectorOverTempBits:2; //������������¹��ϣ�00--����  01--����  10 --������
            INT8U ChargeOverTempInBits:2;  //�����ڲ����¹��ϣ�00--����  01--����  10 --������
            INT8U PowerUnreach:2;          //������ܲ��ܴ�����ϣ�00--����  01--����  10 --������
            INT8U UrgentStopBits:2;        //������ͣ���ϣ�00--����  01--����  10 --������
            INT8U OtherErrBits:2;          //�������ϣ�00--����  01--����  10 --������
            INT8U ResverBits:4;            //Ԥ��
        }OneByte;
    }State1;

    __packed union
    {
        INT8U EndChargeErroReason;         //������ֹ������ԭ��
        __packed struct
        {
            INT8U OverCurErrBits:2;     //������������00--��������  01--������������ֵ  10 --������
            INT8U OverVolErrBits:2;     //������ѹ����00--��ѹ����  01--��ѹ�쳣  10 --������
            INT8U ResverBits:4;         //Ԥ��
        }OneByte;
    }State2;
    
}_BMS_CST_CONTEXT;

extern _BMS_CST_CONTEXT BMS_CST_Context[GUN_MAX];

__packed typedef struct
{
    INT16U TotalChargeTime; //�ۼƳ��ʱ��(min)
    INT16U PowerOut;        //�������,0.1kW.h/λ
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //�¹��� 
    INT32U ChargeSN;        //�������,1/λ
#endif
}_BMS_CSD_CONTEXT;
extern _BMS_CSD_CONTEXT BMS_CSD_Context[GUN_MAX];

//����������CEM
__packed typedef struct
{
    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BRMBits:2;    //����BMS�ͳ�����ʶ���ʶ��ʱ
            INT8U RevBits:6;    //Ԥ��
        }OneByte;
    }State0;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BCPBits:2;    //���յ�س��������ĳ�ʱ
            INT8U BROBits:2;    //����BMS��ɳ��׼�����ĳ�ʱ
            INT8U RevBits:4;    //Ԥ��
        }OneByte;
    }State1;

    __packed union 
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BCSBits:2;    //���յ�س����״̬���ĳ�ʱ
            INT8U BCLBits:2;    //���յ�س��Ҫ���ĳ�ʱ
            INT8U BSTBits:2;    //����BMS��ֹ��籨�ĳ�ʱ
            INT8U RevBits:2;    //Ԥ��
        }OneByte;
    }State2;

    __packed union
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U BSDBits:2;    //����BMS���ͳ�Ʊ��ĳ�ʱ
#if (BMS_CUR_GB == BMS_NEW_GB2015)  //�¹��� 
            INT8U OthBits:6;    //����
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
	INT32U PGNID;       // ���ݱ�ʶ
    INT32U Timeout;    // ֡���ȣ�����ID���ڵ�֡,���֡�������ع�����֡������Ϣ,NULL��Ϊ���ز��������֡
	INT8U (*DealwithDataIDFunction)(CanInterfaceStruct *controlstrcut);  //id��Ӧ�Ĳ���
	INT8U  (*Updatafunction)(void);//ִ�������õȲ�������Ҫ���µĲ���ִ�к���,һЩС�Ĳ����ĸ��²���������
}_PROTOCOL_CAN_MANAGE_TABLE;

extern const _PROTOCOL_CAN_MANAGE_TABLE  Can_Data_Manage_Table[GUN_MAX][CANRECEIVETABLELEN];


#define CANSENDTALELEN       (12u)     //����֡�����񳤶�  
typedef struct
{
	_SEND_BMS_STEP  step;          // ��������¿���   
    INT32U cycletime;         // ִ�����ڣ���Ӧ�������汨������
	void (*CANSendFrametoBMS)(void);  //id��Ӧ�Ĳ���
}_PROTOCOL_CAN_SEND_TABLE;


#endif //__GBSTANDARD_H_
/************************(H)COPYRIGHT 2010 �����Ƽ�*****END OF FILE****************************/
