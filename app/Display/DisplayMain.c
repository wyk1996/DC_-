/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: MenuDisp.c
* Author			:
* Date First Issued	:
* Version			:
* Description		: �ṩÿ������Ĳ˵��ṹ�壬��DispKey.c����
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2013	        : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "DisplayMain.h"
#include "bsp_conf.h"
#include "DispKeyFunction.h"
#include "MenuDisp.h"
#include "bsp_uart.h"
#include "DataChangeInterface.h"
#include "DispShowStatus.h"
#include "FlashDataDeal.h"
#include "bsp_rtc.h"
#include "card_Interface.h"
#include "charging_Interface.h"
#include "RTC_task.h"
#include "4GMain.h"
#include "bsp_MPulse.h"
#include "controlmain.h"
#include "AdAdjust.h"
#include "PCUMain.h"
#include "connectbms.h"
#include "ADSamplemain.h"
#include "stm32f4xx_flash.h"
#include "version.h"
#include "app_conf.h"

/* Private define-----------------------------------------------------------------------------*/

#define DISP_SEND_BUF_LEN          (120u)
#define DISP_RECV_BUF_LEN          (120u)
#define DISPLAY_Q_LEN               80
#define DISP_TEMP_BUF_LEN          (120u)

#define MAX_PERIOD_SUM_TIME         (1440u)                   //��ʱ��������
/* Private typedef----------------------------------------------------------------------------*/
_DISP_UART_STATUS DispUartStatus = DISP_UART_IDLE;			//����ʾ����������Լ��������Ҫд��ʾ���ڣ��ʼ�æ״̬

INT16U	ChargeRecodeCurNum;
typedef struct
{
	INT32U RecodeCurNum;			//���׼�¼����
	INT32U CurReadRecodeNun;		//��ǰ�����Ľ��׼�¼��ƫ����������ѯʹ�ã�
	INT32U NextReadRecodeNun;		//��һ�������Ľ��׼�¼��ƫ����������ѯʹ�ã�
	INT32U UpReadRecodeNun;			//��һ�������Ľ��׼�¼��ƫ����������ѯʹ�ã�
	INT16U CurNun;					//��ǰ��¼ҳ��
	_CHARGE_RECODE CurRecode;	//��ŵ�ǰ��¼
	_CHARGE_RECODE NextRecode;	//�����һ����¼
	_CHARGE_RECODE UpRecode;	//�����һ����¼
	INT32U CardNum;				//��¼��ѯ���Ŀ���
} _RECODE_CONTROL;

typedef struct
{
	INT32U RecodeCurNum;			//����/�澯��¼����
	INT16U CurNun;					//��ǰ��¼ҳ��
	_ERRWARN_RECODEINFO CurErrWarnRecod; //��ǰ����\�澯�洢
} _ERRWARN_RECODE_CONTROL;




extern OS_EVENT  		*JustOSQ_pevent; //
extern _ADC_JUST_INFO ADCJustInfo[GUN_MAX];   //ADУ׼��Ϣ
extern _VIN_CONTROL VinControl[GUN_MAX];

INT8U Billbuf[GUN_MAX][500] = {0};			//����
INT8U FlashCardVinWLBuf[5000] = {0};		//��Vin������chuli

OS_EVENT *DispMutex;                 //��������ͬһʱ��ֻ����һ����������ٽ�����
/***************************************������Ļ************************************/
#define DIVINFO_SINGLE_SIZE 0x10
//�����52����ͳһ
__packed typedef struct
{
	INT8U DivNum[16];		//׮���
	INT8U ProjectNum[16];	//��Ŀ���
	INT16U res;
	INT16U UseModule;    	//ʹ��ģ������	BIN	1 Byte	�ֱ��ʣ�1����0ƫ����
	INT16U MacMaxVol;		//������ߵ�ѹ   �ֱ��ʣ�1V��0ƫ����
	INT16U MacMinVol;		//������͵�ѹ	 �ֱ��ʣ�1V��0ƫ����
	INT16U MaxOutCur;		//����������	 �ֱ���0.1A��0ƫ����
	INT16U  UseGun;			//ǹʹ�ø��� 1 Byte 	�ֱ��ʣ�1����0ƫ����
	INT16U OutDcVol;		//�����ѹ������ֵ	BIN	2 Byte	�ֱ��ʣ�0.1V��0ƫ����
	INT16U OutDcCur;		//�������������ֵ	BIN	2 Byte	�ֱ��ʣ�0.1A��0ƫ����
	INT16U SOC;				//SOC��ֵ
} _SYS_MENU52_SET1;



//�����42 43 44��Ӧ ʵʱ�豸��Ϣ��ʾ���ӱ��������Ͽ�ʼ
__packed typedef struct
{
	_SHOW_NUM WattMetereSignalErr;				//����ͨ��״̬
	_SHOW_NUM CCUSignalErr;						//��CCU��״̬��
	_SHOW_NUM ChargGun_State;					//ǹ��λ״̬
	_SHOW_NUM CGunUpTemperature_Err;			//���ǹ����
	_SHOW_NUM DZLock_Err;						//������
	_SHOW_NUM BatReverse_Err;					//��ط���
	_SHOW_NUM BMSCommunication_Err;				//��BMSͨ��״̬
	INT8U cc1volt;							//CC1��Ҫ������ʾ
	_SHOW_NUM JyCheck_Err;						//��Ե���
	_SHOW_NUM DCOVP_Warn;            			//ֱ�������ѹ�澯
	_SHOW_NUM DCUVP_Warn;            			//ֱ�����Ƿѹ�澯
	_SHOW_NUM UPCUR_Warn;           			//ֱ����������澯
	_SHOW_NUM CarGuide_Err;						//����г�������
	_SHOW_NUM DCContactor_Err;					//ֱ��ĸ������Ӵ���
} _GUN_STATUS;
__packed typedef struct
{
	_SHOW_NUM SurgeArrester_Err; 			//������
	_SHOW_NUM AC_OVP;						//���������ѹ
	_SHOW_NUM AC_UVP;						//��������Ƿѹ
	_SHOW_NUM SysUptemp_Warn;				//�������¸澯
	_SHOW_NUM Fan_Warn;						//���ȹ���
	_SHOW_NUM ACCircuitBreaker_Err;			//������·������
	_SHOW_NUM Door_Err;						//�Ž�
	_SHOW_NUM Stop_Pluse;					//��ͣ
	_SHOW_NUM ModuleACOVP_Warn;				//���ģ�齻�������ѹ�澯
	_SHOW_NUM ModuleACUVP_Warn;				//���ģ�齹¯����Ƿѹ�澯
	_SHOW_NUM ModuleACPhase_Warn;			//���ģ������ȱ��澯
	_SHOW_NUM ModuleDCShortCircuit_Err;		//���ģ�������·����
	_SHOW_NUM ModuleDCUPCUR_Warn;			//���ģ��ֱ����������澯
	_SHOW_NUM ModuleDCOVP_Warn;   			//���ģ��ֱ�������ѹ�澯
	_SHOW_NUM ModuleDCUVP_Warn;				//���ģ��ֱ�����Ƿѹ�澯
	_GUN_STATUS GunStatus[GUN_MAX];
	INT16U GunVol[GUN_MAX];
} _DIV_STATUS;
_DIV_STATUS  DivStatue;


//�����8/9�˻���Ϣ��Ӧ
__packed typedef struct
{
	INT32U BeforeCardBalance;			//�ۿ������� 0.01
	INT8U BillingStatus[10];			//����״̬
	INT8U TransNum[16];      			//������ˮ��
	INT32U TotPower;         			//�ܵ��� 	 0.01Kwh
	INT8U StopChargeReason[30];			//ֹͣԭ��
	INT32U CardNum;        				//����
	INT8U ChargeType[30];        		//��緽ʽ
	INT8U Gunnum[10];						//ǹ��
	INT32U TotMoney;         			//�ܽ��   0.01Ԫ
	INT8U CarVin[18];         			//�綯����Ψһ��ʶ����Vin�뱾��17λ���������С��λ��2���ֽ�����д18��
	INT16U StartYear;					// ��
	INT16U StartMonth;					// ��
	INT16U StartDay;					// ��
	INT16U StartHour;					// ʱ
	INT16U StartMinute;					// ��
	INT16U StartSecond;					// ��
	INT16U EndYear;						// ��
	INT16U EndMonth;					// ��
	INT16U EndDay;						// ��
	INT16U EndHour;						// ʱ
	INT16U EndMinute;					// ��
	INT16U EndSecond;					// ��
	INT8U StopReason[20];				//�ڲ���Ա�Լ�ʹ��
} _RECODE_MENU8_9_INFO;











/* Private macro------------------------------------------------------------------------------*/
static void *DisplayOSQ[DISPLAY_Q_LEN];
static OS_EVENT *DispTaskEvent;
static INT8U DispRecvSMem[DISPLAY_Q_LEN * sizeof(_BSP_MESSAGE)];
static INT8U DispSendBuf[DISP_SEND_BUF_LEN];
INT8U DispRecvBuf[DISP_RECV_BUF_LEN];
static INT8U DispTempBuf[DISP_TEMP_BUF_LEN];

_DISP_CONTROL DispControl;       				//��ʾ������ƽṹ��
_RECODE_CONTROL RecodeControl;					//���׿���
static _RECODE_MENU8_9_INFO RecodeMenu8_9Info;		//�����8/9�˻���Ϣ��Ӧ
//�������ʾ����һ��
_CHARGE_INFO  ChargeInfo[GUN_MAX] = {(_CHARGE_TYPEDEF)0};		 		//�����Ϣ����緽ʽѡ��
_CHARGEING_INFO ChargeingInfo[GUN_MAX] = {0};			//�������ʾ������
_CARD_INFO CardInfo[GUN_MAX] = {0};					//����г�����Ϣ
_END_CHARGE_INFO  EndChargeInfo[GUN_MAX] = {0};		//���������Ϣ




_SYS_SET SYSSet;								//ϵͳ������Ϣ       		61���ֽ�
_PRICE_SET PriceSet;							//�������
//�ƿ��  ��������޸ķ��ʣ�Aǹ�ķ��ʿ��ܺ�Bǹ�ķ��ʲ�һ��
_PRICE_SET A_PriceSet;							//�������
_PRICE_SET B_PriceSet;							//�������

HandSet_Info HandSedInfo[GUN_MAX] = {0};		//�ֶ�ģʽ��Ϣ
_CHARGE_RECODE ChargeRecodeInfo[GUN_MAX];


_BSPRTC_TIME ReadDwRtc; //������Ļ��ȡ��RTC

//_DIV_STATUS  DivStatue;									//����65/66/67/68�豸��Ϣ �ӱ��������Ͽ�ʼ��A\B��絼����ѹ������ʾ
/* Private variables--------------------------------------------------------------------------*/


/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


/*****************************************************************************
* Function     : NB_WriterReason
* Description  : �ڲ���Ա��ʾ����ԭ�򣬷������ʹ��
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2022��8��24��
*****************************************************************************/
INT8U NB_WriterReason(INT8U gun,const char * preason,INT8U len)
{
	INT16U i;
	if((preason == NULL) || (!len) || (gun >= GUN_MAX))
	{
		return FALSE;
	}
	for(i = 0; i < sizeof(ChargeRecodeInfo[gun].StopReason); i++)
	{
		if(ChargeRecodeInfo[gun].StopReason[i] == 0)
		{
			memcpy(&ChargeRecodeInfo[gun].StopReason[i],preason,MIN((sizeof(ChargeRecodeInfo[gun].StopReason) - i),len));     //���ܽ�����ֹͣԭ�򣬶�������ʾ����
			return TRUE;
		}
	}
	return TRUE;
}
/*****************************************************************************
* Function     : APP_GetCARDWL
* Description  : ��ȡ��������
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
static INT8U* APP_GetCARDWL(void)
{
	_FLASH_OPERATION  FlashOper;

	//��ȡ����
	FlashOper.DataID = PARA_CARDWHITEL_ID;
	FlashOper.Len = PARA_CARDWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read cardwl  err");
		return NULL;
	}
	return FlashCardVinWLBuf;
}


/*****************************************************************************
* Function     : APP_GetVinWL
* Description  : ��ȡVIN������
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U* APP_GetVinWL(void)
{
	_FLASH_OPERATION  FlashOper;

	//��ȡ����
	FlashOper.DataID = PARA_VINWHITEL_ID;
	FlashOper.Len = PARA_VINWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read cardwl  err");
		return NULL;
	}
	return FlashCardVinWLBuf;
}

/*****************************************************************************
* Function     : APP_GetGunNum
* Description  : ��ȡǹ����
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U APP_GetGunNum(void)
{
	if(SYSSet.SysSetNum.UseGun <=  GUN_MAX)
	{
		return SYSSet.SysSetNum.UseGun;
	}
	return GUN_MAX;
}

/*****************************************************************************
* Function     : APP_GetStopChargeReason
* Description  : ���λΪ1��ʾ����ʧ��ֹͣ�����λΪ0��ʾ�����ɹ�ֹͣ
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U APP_GetStopChargeReason(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	return ChargeRecodeInfo[gun].StopChargeReason;
}

/*****************************************************************************
* Function     : APP_GetDispUartStatus
* Description  : ��ȡ��ʾ����״̬
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
_DISP_UART_STATUS APP_GetDispUartStatus(void)
{
	return  DispUartStatus;
}

/*****************************************************************************
* Function     : APP_SetDispUartStatus
* Description  : ������ʾ����״̬
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U  APP_SetDispUartStatus(_DISP_UART_STATUS status)
{
	if(status >= DISP_UART_MAX)
	{
		return FALSE;
	}
	OSSchedLock();
	DispUartStatus = status;
	OSSchedUnlock();
	return TRUE;
}


/*****************************************************************************
* Function     : APP_GetRecodeCurNum
* Description  : ��ȡ��ǰ���׼�¼
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT32U  APP_GetRecodeCurNum(void)
{
	return RecodeControl.RecodeCurNum;
}

/*****************************************************************************
* Function     : APP_GetRecodeCurNum
* Description  : ��ȡ��ǰ���׼�¼
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
void  APP_SetRecodeCurNum(INT32U num)
{
	RecodeControl.RecodeCurNum = num;
}

/*****************************************************************************
* Function     : APP_RWChargeRecode
* Description  : ��д���׼�¼
* Input        :
				count  ��д�ڵڼ��� 1 - 1000��
                 RWChoose  ��д����
                 precode ��������ַ
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U APP_RWChargeRecode(INT16U count,_FLASH_ORDER RWChoose,_CHARGE_RECODE * precode)
{
	_FLASH_OPERATION  FlashOper;

	if((count > 1000) || (precode == NULL) )
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_RECODE_ID;
	FlashOper.ptr = (INT8U*)precode;
	FlashOper.RWChoose = RWChoose;
	APP_ChargeRecodeFlashOperation(count,&FlashOper);
	return TRUE;
}


/*****************************************************************************
* Function     : APP_RWOFFLineRe���߽��׼�¼
* Description  : ��д�������߽��׼�¼
* Input        :
				count  ��д�ڵڼ��� 1 - 100��
                 RWChoose  ��д����
                 precode ��������ַ
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U APP_RWNetOFFLineRecode(INT16U count,_FLASH_ORDER RWChoose,INT8U  * pdata)
{
	_FLASH_OPERATION  FlashOper;

	if((count > 100) || (pdata == NULL) )
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_OFFLINEBILL_ID;
	FlashOper.ptr = (INT8U*)pdata;
	FlashOper.RWChoose = RWChoose;
	APP_ChargeOffLineFlashOperation(count,&FlashOper);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_RWNetFSOFFLineRecode
* Description  : ��д�������߽��׼�¼
* Input        :
				count  ��д�ڵڼ��� 1 - 100��
                 RWChoose  ��д����
                 precode ��������ַ
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U APP_RWNetFSOFFLineRecode(INT16U count,_FLASH_ORDER RWChoose,INT8U  * pdata)
{
	_FLASH_OPERATION  FlashOper;

	if((count > 100) || (pdata == NULL) )
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_OFFLINEBILLFS_ID;
	FlashOper.ptr = (INT8U*)pdata;
	FlashOper.RWChoose = RWChoose;
	APP_ChargeOffLineFlashOperation(count,&FlashOper);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetNetOFFLineRecodeNum
* Description  : ��д���߽��׼�¼
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U APP_GetNetOFFLineRecodeNum(void)
{
	INT8U num = 0;
	_FLASH_OPERATION  FlashOper;


	FlashOper.DataID = PARA_OFFLINEBILLNUM_ID;
	FlashOper.Len = PARA_OFFLINEBILLNUM_FLLEN;
	FlashOper.ptr = (INT8U *)&num;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	//���100��
	if(num > 100)
	{
		num = 0;
		FlashOper.DataID = PARA_OFFLINEBILLNUM_ID;
		FlashOper.Len = PARA_OFFLINEBILLNUM_FLLEN;
		FlashOper.ptr = (INT8U *)&num;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
		return 0;
	}
	return num;
}


/*****************************************************************************
* Function     : APP_GetNetOFFLineRecodeNum
* Description  : ��д���߽��׼�¼
* Input        :
* Output       : None
* Return       : static
* Note(s)      :
* Contributor  : 2018��8��24��
*****************************************************************************/
INT8U APP_SetNetOFFLineRecodeNum(INT8U num)
{
	_FLASH_OPERATION  FlashOper;

	//���100��
	if(num > 100)
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_OFFLINEBILLNUM_ID;
	FlashOper.Len = PARA_OFFLINEBILLNUM_FLLEN;
	FlashOper.ptr = (INT8U *)&num;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);

	return TRUE;;
}



/*****************************************************************************
* Function     : SendPricMsg
* Description  : ���ͼƷ���Ϣ
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U SendPricMsg(_GUN_NUM gun ,_PRIC_CMD cmd)
{
	INT8U err;
	static	_BSP_MESSAGE  msg[GUN_MAX];
	OS_EVENT* pevent = APP_GetPricEvent();
	static INT8U gunnum[GUN_MAX];
	if ((gun >= GUN_MAX) ||( cmd >=  PRIC_MAX ) || (pevent == NULL) )
	{
		return FALSE;
	}
	gunnum[gun] = gun;
	msg[gun].MsgID = BSP_MSGID_DISP;
	msg[gun].DivNum = cmd;
	msg[gun].pData = &gunnum[gun];
	msg[gun].DataLen = sizeof(INT8U);
	if((err = OSQPost(pevent, &msg[gun])) != OS_ERR_NONE)
	{
		printf("Send to price message error %d",err);
		return FALSE;
	}
	return TRUE;
}




/*****************************************************************************
* Function     : SendStartChargeMsg
* Description  : ���͸�����ccu�����Ϳ�ʼ���(��ʼ���ֻ��һ��)
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U SendStartChargeMsg(_GUN_NUM gun,_CHARGE_MODE_TYPE charge_mode)
{
	static _START_CHARGE_CMD StartChargeCmd[GUN_MAX];
	static	_BSP_MESSAGE	send_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	StartChargeCmd[gun].ChargeMode = charge_mode;
	if(StartChargeCmd[gun].ChargeMode == MODE_MAN)
	{
		StartChargeCmd[gun].ChargeCur = HandSedInfo[gun].outcurr; //����һ��С��
		StartChargeCmd[gun].ChargeVol = HandSedInfo[gun].outvolt;	//����һ��С��
	}
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BCU;
	send_message[gun].DivNum = APP_CHARGE_START;
	send_message[gun].DataLen = sizeof(_START_CHARGE_CMD);
	send_message[gun].pData = (INT8U*)(&StartChargeCmd[gun]);
	send_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &send_message[gun]);	//��������������Ϣ
	}
	else
	{
		OSQPost(Control_PeventB, &send_message[gun]);	//��������������Ϣ
	}
	return TRUE;
}



INT8U STOPCNU = 0;
/*****************************************************************************
* Function     : SendStopChargeMsg
* Description  : ���͸�����ccu������ֹͣ��磨ֹͣ��緢��Σ�
* Input        : gun
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U SendStopChargeMsg(_GUN_NUM gun)
{
	static _START_CHARGE_CMD StartChargeCmd[GUN_MAX];
	static	_BSP_MESSAGE	send_message[GUN_MAX];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_BCU;
	send_message[gun].DivNum = APP_CHARGE_END;
	send_message[gun].DataLen = sizeof(_START_CHARGE_CMD);
	send_message[gun].pData = (INT8U*)(&StartChargeCmd[gun]);
	send_message[gun].GunNum = gun;
	if(gun == GUN_A)
	{
		OSQPost(Control_PeventA, &send_message[gun]);	//��������������Ϣ
	}
	else
	{
		OSQPost(Control_PeventB, &send_message[gun]);	//��������������Ϣ
	}
	return TRUE;

}

/*****************************************************************************
* Function     : SendCardMsg
* Description  : ���͸�����Ϣ
* Input        : divnum ��Ϣ�豸��        pdata���ݣ���ʼˢ������Ҫ�������룩 len ���볤��
				������紫�����ѽ��
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U SendCardMsg(_SEND_TO_CARD_CMD divnum, void * pdata,INT8U len)
{
#define CARD_MSG_NUM    10
	static	_BSP_MESSAGE  msg[CARD_MSG_NUM];
	static INT8U i = 0;
	static INT8U databuf[CARD_MSG_NUM][8];
	INT8U err;
	OS_EVENT* pevent = APP_GetCardEvent();



	if((divnum >= CARDCMD_MAX) || (pevent == NULL) )
	{
		return FALSE;
	}
	memcpy(&databuf[i][0],pdata,len);
	msg[i].MsgID = BSP_MSGID_DISP;
	msg[i].DivNum = divnum;
	msg[i].pData = &databuf[i][0];
	msg[i].DataLen = len;

	if((err = OSQPost(pevent, &msg[i])) != OS_ERR_NONE)
	{
		printf("dis Send to Card  message error %d",err);
		return FALSE;
	}
	i++;
	if(i == CARD_MSG_NUM)
	{
		i = 0;
	}
	OSTimeDly(SYS_DELAY_10ms);
	return TRUE;

}

/*****************************************************************************
* Function     : APP_GetMeterAddr
* Description  : ��ȡ���ַ
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
INT8U  APP_GetMeterAddr(_GUN_NUM gun,INT8U *pdata)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	//SYSSet.MeterAdd ΪASICC
	if(SYSSet.MeterAdd[gun][0] != 'a')
	{
		pdata[5] = ByteHexToBcd((SYSSet.MeterAdd[gun][0] - 0x30)*10 + SYSSet.MeterAdd[gun][1] - 0x30);
		pdata[4] = ByteHexToBcd((SYSSet.MeterAdd[gun][2] - 0x30)*10 + SYSSet.MeterAdd[gun][3] - 0x30);
		pdata[3] = ByteHexToBcd((SYSSet.MeterAdd[gun][4] - 0x30)*10 + SYSSet.MeterAdd[gun][5] - 0x30);
		pdata[2] = ByteHexToBcd((SYSSet.MeterAdd[gun][6] - 0x30)*10 + SYSSet.MeterAdd[gun][7] - 0x30);
		pdata[1] = ByteHexToBcd((SYSSet.MeterAdd[gun][8] - 0x30)*10 + SYSSet.MeterAdd[gun][9] - 0x30);
		pdata[0] = ByteHexToBcd((SYSSet.MeterAdd[gun][10] - 0x30)*10 + SYSSet.MeterAdd[gun][11] - 0x30);
	}
	else
	{
		//�㲥��ַ
		pdata[0] = 0xaa;
		pdata[1]= 0xaa;
		pdata[2]= 0xaa;
		pdata[3]= 0xaa;
		pdata[4]= 0xaa;
		pdata[5]= 0xaa;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : ��ȡ�����Ϣ
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_PRICE_SET* APP_GetPriceInfo(void)
{
	return &PriceSet;
}

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : ��ȡ�����Ϣ
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_PRICE_SET* APP_GetAPriceInfo(void)
{
	return &A_PriceSet;
}

/*****************************************************************************
* Function     : APP_GetPriceInfo
* Description  : ��ȡ�����Ϣ
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_PRICE_SET* APP_GetBPriceInfo(void)
{
	return &B_PriceSet;
}

/*****************************************************************************
* Function     : APP_GetWorkState
* Description  :��ȡ����״̬
* Input        :
* Output       :
				00H������
				01H������
				02H��������
				03H�������ͣ
* Return       :
* Note(s)      :
* Contributor  : 2018-8-13
*****************************************************************************/
_WORK_STAT  APP_GetWorkState(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return WORK_MAX;
	}
	return (_WORK_STAT)BCURemoteSignal[gun].state1.State.System_State;
}

/*****************************************************************************
* Function     : APP_GetErrState
* Description  :��ȡ����״̬
* Input        :
* Output       :
				00H������
				01H������
* Return       :
* Note(s)      :
* Contributor  : 2018-8-13
*****************************************************************************/
INT8U  APP_GetErrState(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return WORK_MAX;
	}
	return (_WORK_STAT)BCURemoteSignal[gun].state1.State.System_State;
}


/*****************************************************************************
* Function     : APP_GetChargeingInfo
* Description  :�ṩ���������ʾ������ʾ
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CHARGEING_INFO * APP_GetChargeingInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &ChargeingInfo[gun];
}

/*****************************************************************************
* Function     : APP_GetCarInfo
* Description  :�ṩ��������Ϣ������ʾ������ʾ
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CARD_INFO * APP_GetCarInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &CardInfo[gun];
}

/*****************************************************************************
* Function     : APP_GetChargeInfo
* Description  :
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CHARGE_INFO* APP_GetChargeInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &ChargeInfo[gun];
}


/*****************************************************************************
* Function     : APP_SetCardPriceInfo
* Description  : ��ȡ��ǰ��¼
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_CHARGE_RECODE* APP_GetChargeRecodeInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &ChargeRecodeInfo[gun];
}

/*****************************************************************************
* Function     : APP_SetCountDownTime
* Description  : ���õ���ʱʱ��   ���л�ҳ���ʱ�����õ���ʱʱ�䣬
				�������е���ʱÿ1s -1��ֱ������1Ϊֹ
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
void  APP_SetCountDownTime(INT16U time)
{
	OSSchedLock();
	DispControl.CountDown = time;
	OSSchedUnlock();
}

/*****************************************************************************
* Function     : APP_GetDispControl
* Description  : ��ȡ��ʾ������ƽṹ��
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
_DISP_CONTROL* APP_GetDispControl(void)
{
	return &DispControl;
}

/*****************************************************************************
* Function     : APP_GetDispEvent
* Description  :��ȡ��ʾ�����¼�
* Input        :
* Output       :
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
OS_EVENT* APP_GetDispEvent(void)
{
	return DispTaskEvent;
}

/*****************************************************************************
* Function     : Display_HardwareInit
* Description  : ��ʾ�����ʼ��
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
INT8U Display_HardwareInit(void)
{

	_BSP_UART_SET UartGPRSSet;
	DispTaskEvent = OSQCreate(DisplayOSQ, DISPLAY_Q_LEN);
	if (DispTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //����ǰ����
		return FALSE;
	}
	memset(DispSendBuf, 0 ,sizeof(DispSendBuf));
	memset(DispRecvBuf, 0 ,sizeof(DispRecvBuf));

	UartGPRSSet.BaudRate = 115200;
	UartGPRSSet.DataBits = BSPUART_WORDLENGTH_8;
	UartGPRSSet.Parity   = BSPUART_PARITY_NO;
	UartGPRSSet.StopBits = BSPUART_STOPBITS_1;
	UartGPRSSet.RxBuf = DispRecvBuf;
	UartGPRSSet.RxBufLen = sizeof(DispRecvBuf);
	UartGPRSSet.TxBuf = DispSendBuf;
	UartGPRSSet.TxBufLen = sizeof(DispSendBuf);
	UartGPRSSet.Mode = UART_DEFAULT_MODE | UART_MSG_MODE;
	UartGPRSSet.RxQueue = DispTaskEvent;
	UartGPRSSet.RxMsgMemBuf = DispRecvSMem;
	UartGPRSSet.RxMsgMemLen = sizeof(DispRecvSMem);
	if (BSP_UARTConfigInit(DISPLAY_UART, &UartGPRSSet) == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: CountPeriodSumTime
* Description	: ����������ʱ��
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��8��9��
***********************************************************************************************/
static INT16U CountPeriodSumTime(INT16U  *minute)
{
	INT16U time[TIME_PERIOD_MAX];
	INT16U SumTime;
	time[0] = ((minute[1]>=minute[0])?(minute[1] - minute[0]):((minute[1] + MAX_PERIOD_SUM_TIME) - minute[0]));

	time[1] = ((minute[2]>=minute[1])?(minute[2] - minute[1]):((minute[2] + MAX_PERIOD_SUM_TIME) - minute[1]));

	time[2] = ((minute[3]>=minute[2])?(minute[3] - minute[2]):((minute[3] + MAX_PERIOD_SUM_TIME) - minute[2]));

	time[3] = ((minute[0]>=minute[3])?(minute[0] - minute[3]):((minute[0] + MAX_PERIOD_SUM_TIME) - minute[3]));

	SumTime = time[0] + time[1] + time[2] + time[3];

	if(SumTime == 0)
	{
		SumTime = MAX_PERIOD_SUM_TIME;
	}

	return SumTime;
}



/***********************************************************************************************
* Function		: MENU11_TIMEDispose
* Description	: ����3 16�·��Ŀ����봦��
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu3_16_CodeDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT32U code;

	if((pvalue == NULL) || (len != 4) || (DispControl.CurUserGun >= GUN_MAX) )
	{
		return FALSE;
	}
	code = (pvalue[0] << 24) | (pvalue[1] << 16) | (pvalue[2] << 8) | pvalue[3];

	//������ת��δassic
	DispControl.CardCode[0] = code % 1000000 / 100000 + '0';
	DispControl.CardCode[1] = code % 100000 / 10000 + '0';
	DispControl.CardCode[2] = code % 10000 / 1000 + '0';
	DispControl.CardCode[3] = code % 1000 / 100 + '0';
	DispControl.CardCode[4] = code % 100 / 10 + '0';
	DispControl.CardCode[5] = code % 10 / 1 + '0';

	DisplayCommonMenu(&HYMenu22,NULL);			//����ˢ������
	//��ȡ���Ƿ�Ϊ����
	if(GetCardWorkStatus() != _Card_IDLE)
	{
		return FALSE;
	}
	if(HYMenu21.Menu_PrePage == &HYMenu3)			//��һҳ�ǳ�緽ʽѡ�����
	{
		if(DispControl.CurUserGun == GUN_A)
		{
			if(SendCardMsg(CARDCMD_STARTA, DispControl.CardCode,sizeof(DispControl.CardCode)) == FALSE)
			{
				return FALSE;
			}
		}
		else
		{
			if(SendCardMsg(CARDCMD_STARTB, DispControl.CardCode,sizeof(DispControl.CardCode)) == FALSE)
			{
				return FALSE;
			}
		}
	}
	else
	{
		if(SendCardMsg(CARDCMD_QUERY, DispControl.CardCode,sizeof(DispControl.CardCode)) == FALSE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu73_HANDADispose
* Description	: ����73Aǹ�ֶ�ģʽ����
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu50_HANDADispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	if((pvalue == NULL) || (len != 4) )
	{
		return FALSE;
	}
	HandSedInfo[GUN_A].outvolt =  (pvalue[0] << 8 | pvalue[1])*10; //����һ��С��
	HandSedInfo[GUN_A].outcurr =  (pvalue[2] << 8 | pvalue[3])*10; //����һ��С��
	if(APP_GetWorkState(GUN_A) != WORK_CHARGE)   //���ڳ���У�������������
	{
		SendStartChargeMsg(GUN_A,MODE_MAN);
	}
	else									//�ڳ���У����͵������
	{
		NB_WriterReason(GUN_A,"E63",3);
		SendStopChargeMsg(GUN_A);
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu74_HANDBDispose
* Description	: ����74Bǹ�ֶ�ģʽ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu51_HANDBDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	if((pvalue == NULL) || (len != 4) )
	{
		return FALSE;
	}
	HandSedInfo[GUN_B].outvolt =  (pvalue[0] << 8 | pvalue[1])*10; //����һ��С��
	HandSedInfo[GUN_B].outcurr =  (pvalue[2] << 8 | pvalue[3])*10; //����һ��С��
	if(APP_GetWorkState(GUN_B) != WORK_CHARGE)	 //���ڳ���У�������������
	{
		SendStartChargeMsg(GUN_B,MODE_MAN);
	}
	else									//�ڳ���У����͵������
	{
		NB_WriterReason(GUN_B,"E62",3);
		SendStopChargeMsg(GUN_B);
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_DevnumDispose
* Description	: 52ϵͳ����׮��Ŵ���
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_DevnumDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U dev_num[16];
	INT8U i;
	_FLASH_OPERATION  FlashOper;

	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <16; i++)
	{
		dev_num[i]  = 0x30;
	}
	if(len > 16)
	{
		len = 16;		//ASICC����ʱ�����ܴ���len����16
	}
	for(i = 0; i < 16; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			dev_num[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.SysSetNum.DivNum,dev_num,sizeof(dev_num));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_ProNumDispose
* Description	: 52ϵͳ������Ŀ��Ŵ���
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_ProNumDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U pro_num[16];
	INT8U i;
	_FLASH_OPERATION  FlashOper;


	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <16; i++)
	{
		pro_num[i]  = 0x30;
	}

	//������16���ֽڣ���Ϊ4���ֽ�
	if(len > 4)
	{
		len = 4;		//ASICC����ʱ�����ܴ���len����4
	}
	for(i = 0; i < 4; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			pro_num[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.SysSetNum.ProjectNum,pro_num,sizeof(pro_num));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_ModuleNumDispose
* Description	: 52ϵͳ����ģ��ʹ�ø�������
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_ModuleNumDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U modulenum = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	modulenum =  pvalue[1];
	if(modulenum > 16)
	{
		return FALSE;
	}

	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);


	if(modulenum == PresetValue.PowerModelNumber)  //λ�ı�
	{
		return TRUE;
	}
	PresetValue.PowerModelNumber = modulenum;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;

}

/***********************************************************************************************
* Function		: Munu52_MacMaxVolDispose
* Description	: ������ߵ�ѹ   �ֱ��ʣ�1V��0ƫ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_MacMaxVolDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U MaxVol = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}

	MaxVol =  (pvalue[0]) << 8 | pvalue[1];
	if(MaxVol > 1000)
	{
		return FALSE;
	}
	MaxVol *= 10; //�Ŵ�10��
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);

	if(PresetValue.VolMaxOutput == MaxVol)
	{
		return TRUE;
	}
	PresetValue.VolMaxOutput = MaxVol;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_MacMinVolDispose
* Description	: ������͵�ѹ	 �ֱ��ʣ�1V��0ƫ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_MacMinVolDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U MinVol = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	MinVol =  (pvalue[0]) << 8 | pvalue[1];
	MinVol  *= 10;	//�Ŵ�10��
	if((MinVol <100) || (MinVol > 200))
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.VolMinOutput == MinVol)
	{
		return TRUE;
	}

	PresetValue.VolMinOutput = MinVol;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_MaxOutCurDispose
* Description	: ����������	 �ֱ���0.1A��0ƫ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_MaxOutCurDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U MaxOutCur = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}

	MaxOutCur =  (pvalue[0]) << 8 | pvalue[1];
	if(MaxOutCur >2500)
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.CurMaxOutput == MaxOutCur)
	{
		return TRUE;
	}
	PresetValue.CurMaxOutput = MaxOutCur;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_UseGunDispose
* Description	: ǹʹ�ø��� 1 Byte 	�ֱ��ʣ�1����0ƫ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_UseGunDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U UseGun = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	UseGun =  pvalue[1];
	if(UseGun >4)
	{
		return FALSE;
	}
	if(SYSSet.SysSetNum.UseGun == UseGun)
	{
		return TRUE;
	}
	SYSSet.SysSetNum.UseGun = UseGun;
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_OutDcVolDispose
* Description	: �����ѹ������ֵ	BIN	2 Byte	�ֱ��ʣ�0.1V��0ƫ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_OutDcVolDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U OutDcVol = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}

	OutDcVol =  (pvalue[0]) << 8 | pvalue[1];
	if(OutDcVol >10500)
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.Therold_Value.OverOutVolTherold == OutDcVol)
	{
		return TRUE;
	}
	PresetValue.Therold_Value.OverOutVolTherold = OutDcVol;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_OutDcCurDispose
* Description	:�������������ֵ	BIN	2 Byte	�ֱ��ʣ�0.1A��0ƫ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_OutDcCurDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U OutDcCur = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	OutDcCur =  (pvalue[0]) << 8 | pvalue[1];
	if(OutDcCur >2600)
	{
		return FALSE;
	}

	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.Therold_Value.OverOutCurTherold == OutDcCur)
	{
		return TRUE;
	}
	PresetValue.Therold_Value.OverOutCurTherold = OutDcCur;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu52_SOCDispose
* Description	:soc������ֵ	BIN	2 Byte	�ֱ��ʣ�1%��0ƫ����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu52_SOCDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U SOC = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	SOC =   pvalue[1];
	if(SOC >100)
	{
		return FALSE;
	}
	//SOC��ֵ
	SYSSet.SOCthreshold = SOC;  //
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_acinputupvalueDispose
* Description	:���������ѹ 0.1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu53_acinputupvalueDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U acinputupvalue = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	acinputupvalue =  (pvalue[0]) << 8 | pvalue[1];
	if( (acinputupvalue < 3000) ||  (acinputupvalue >5000))
	{
		return FALSE;
	}

	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if(PresetValue.Therold_Value.OverACVolTherold == acinputupvalue)
	{
		return FALSE;
	}
	PresetValue.Therold_Value.OverACVolTherold = acinputupvalue;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_acinputdownvalueDispose
* Description	:���������ѹ 0.1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu53_acinputdownvalueDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U acinputdownvalue = 0;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	acinputdownvalue =  (pvalue[0]) << 8 | pvalue[1];
	if( (acinputdownvalue < 3000) ||  (acinputdownvalue >5000))
	{
		return FALSE;
	}
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);

	if(PresetValue.Therold_Value.UnderACVolTherold == acinputdownvalue)
	{
		return TRUE;
	}
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	PresetValue.Therold_Value.UnderACVolTherold = acinputdownvalue;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Write ProjectNum err");
		return FALSE;
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_MeterAddADispose
* Description	:aǹ��Ŵ���
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu53_MeterAddADispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U MeterAddA[12];
	INT8U i;
	_FLASH_OPERATION  FlashOper;


	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <12; i++)
	{
		MeterAddA[i]  = 0x30;
	}
	if(len > 12)
	{
		len = 12;		//ASICC����ʱ�����ܴ���len����16
	}
	for(i = 0; i < 12; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			MeterAddA[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.MeterAdd[GUN_A],MeterAddA,sizeof(MeterAddA));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu53_MeterAddBDispose
* Description	:Bǹ��Ŵ���
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu53_MeterAddBDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT8U MeterAddB[12];
	INT8U i;
	_FLASH_OPERATION  FlashOper;


	if(pvalue == NULL)
	{
		return FALSE;
	}
	for (i = 0; i <12; i++)
	{
		MeterAddB[i]  = 0x30;
	}
	if(len > 12)
	{
		len = 12;		//ASICC����ʱ�����ܴ���len����16
	}
	for(i = 0; i < 12; i++)
	{
		if(pvalue[i] >= 0x30 && pvalue[i] <= 0x39)
		{
			MeterAddB[i] = pvalue[i];
		}
		else
		{
			break;
		}
	}

	memcpy(SYSSet.MeterAdd[GUN_B],MeterAddB,sizeof(MeterAddB));
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu54_SetTimeDispose
* Description	:����ʱ��
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu54_SetTimeDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	_BSPRTC_TIME SetTime;                       //�趨ʱ��
	INT8U times = 3;							//�������ʧ�ܷ�����������
	if((pvalue == NULL) || (len != 12))
	{
		return FALSE;
	}
	SetTime.Year   = HEXtoBCD((INT8U)((pvalue[0]<<8 | pvalue[1]) - 2000));//�������ʮ������ת����BCD
	SetTime.Month  = HEXtoBCD(pvalue[3]);
	SetTime.Day    = HEXtoBCD(pvalue[5]);
	SetTime.Hour   = HEXtoBCD(pvalue[7]);
	SetTime.Minute = HEXtoBCD(pvalue[9]);
	SetTime.Second = HEXtoBCD(pvalue[11]);

	while(times--)
	{
		if(BSP_RTCSetTime(&SetTime) == TRUE)    //����RTC
		{
			return TRUE;
		}
	}
	return FALSE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:�Ӵ���ǰ����ѹ��1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu57_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_SW_FRONT1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:�Ӵ���ǰ����ѹ��2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu57_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_SW_FRONT2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:�Ӵ����󼶵�ѹ��1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu58_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.DivNum = JUST_SW_AFTER1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;

}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:�Ӵ����󼶵�ѹ��2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu58_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_SW_AFTER2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}


/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:������1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu59_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_CUR1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:������2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu59_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_CUR2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.MsgID = BSP_MSGID_DISP;
	send_message.GunNum = DispControl.CurUserGun;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:���ǹ��1
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu60_Just1Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_GUN2;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.GunNum = DispControl.CurUserGun;

	send_message.MsgID = BSP_MSGID_DISP;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu57_Just1Dispose
* Description	:���ǹ��2
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu60_Just2Dispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	static INT32U vol = 0;
	static _BSP_MESSAGE send_message;
	if((pvalue == NULL) || (len != 2))
	{
		return FALSE;
	}
	vol =  (pvalue[0]) << 8 | pvalue[1];

	send_message.DivNum = JUST_GUN1;
	send_message.DataLen = sizeof(vol);
	send_message.pData = (INT8U *)&vol;
	send_message.GunNum = DispControl.CurUserGun;
	send_message.MsgID = BSP_MSGID_DISP;
	OSQPost(JustOSQ_pevent ,&send_message);
	return TRUE;
}

/***********************************************************************************************
* Function		: Munu48_PrcDispose
* Description	:�������
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu48_PrcDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	//���·���ʱ���ת��Ϊ���ӣ���Ϊʱ���Ҫ�γɱջ����ʵ�һ��ʱ��εĽ������ڵڶ���ʱ��εĿ�ʼ����ֻ��Ҫ����4��ʱ��μ���
	INT16U minute[TIME_PERIOD_MAX];
	_FLASH_OPERATION  FlashOper;

	if((pvalue == NULL) || (len != 32) )
	{
		return FALSE;
	}
	minute[0] = (pvalue[0] << 8 | pvalue[1]) * 60 +  (pvalue[2] << 8 | pvalue[3]);
	minute[1] = (pvalue[4] << 8 | pvalue[5]) * 60 +	(pvalue[6] << 8 | pvalue[7]);
	minute[2] = (pvalue[8] << 8 | pvalue[9]) * 60 +	(pvalue[10] << 8 | pvalue[11]);
	minute[3] = (pvalue[12] << 8 | pvalue[13]) * 60 +	(pvalue[14] << 8 | pvalue[15]);
	//��������
	if((minute[0] > MAX_PERIOD_SUM_TIME) || (minute[1] > MAX_PERIOD_SUM_TIME) || (minute[2] > MAX_PERIOD_SUM_TIME)\
	        || (minute[3] > MAX_PERIOD_SUM_TIME) )
	{
		Display_PricSet(0);
		Dis_ShowStatus(ADDR_MENU81_SHOW,SHOW_CFG_FAIL,RED);    //����ʧ��
		//��ʾ����
		return FALSE;
	}
	//�������ʱ����Ƿ����
	if(CountPeriodSumTime(minute) != MAX_PERIOD_SUM_TIME)
	{
		Display_PricSet(0);
		Dis_ShowStatus(ADDR_MENU81_SHOW,SHOW_CFG_FAIL,RED);    //����ʧ��
		//��ʾ����
		return FALSE;
	}
	PriceSet.StartTime[0] = minute[0];
	PriceSet.EndTime[0] = minute[1];
	PriceSet.StartTime[1] = minute[1];
	PriceSet.EndTime[1] = minute[2];
	PriceSet.StartTime[2] = minute[2];
	PriceSet.EndTime[2] = minute[3];
	PriceSet.StartTime[3] = minute[3];
	PriceSet.EndTime[3] = minute[0];
	PriceSet.Price[0] = (pvalue[16] << 8 | pvalue[17])*1000;
	PriceSet.Price[1] = (pvalue[18] << 8 | pvalue[19])*1000;
	PriceSet.Price[2] = (pvalue[20] << 8 | pvalue[21])*1000;
	PriceSet.Price[3] = (pvalue[22] << 8 | pvalue[23])*1000;
	PriceSet.ServeFee[0] = (pvalue[24] << 8 | pvalue[25])*1000;
	PriceSet.ServeFee[1] = (pvalue[26] << 8 | pvalue[27])*1000;
	PriceSet.ServeFee[2] = (pvalue[28] << 8 | pvalue[29])*1000;
	PriceSet.ServeFee[3] = (pvalue[30] << 8 | pvalue[31])*1000;
//	PriceSet.ViolaFee = pvalue[26] << 8 | pvalue[27];
	PriceSet.TimeQuantumNum = 4;

	//��ȡ����������Ϣ
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = PARA_PRICALL_FLLEN;
	FlashOper.ptr = (INT8U *)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		Display_PricSet(0);
		Dis_ShowStatus(DIS_ADD(48,0x10),SHOW_CFG_FAIL,RED);    //����ʧ��
		printf("Writer printf set err");
	}
	Dis_ShowStatus(DIS_ADD(48,0x10),SHOW_CFG_SUCCESS,RED);    //����ʧ��
	return TRUE;
}


/***********************************************************************************************
* Function		: Munu10_PortSert
* Description	:�˿�����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu10_PortSert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT16U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp =  (pvalue[0] << 8) | (pvalue[1]);

	//memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
	SYSSet.Port = temp;
	NetConfigInfo[SYSSet.NetYXSelct].port = temp;
	//memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
	//snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu10_IP1Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[0] = temp;
	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu10_IP2Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[1] = temp;
	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu10_IP3Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[2] = temp;
	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu10_IP4Sert(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];

	SYSSet.IP[3] = temp;
	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: Munu10_IP1Sert
* Description	:IP����
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu10_NetNumSet(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT8U temp;
	_FLASH_OPERATION  FlashOper;
	if((pvalue == NULL) || (len != 2) )
	{
		return FALSE;
	}
	temp = pvalue[1];
	if(temp > 2)
	{
		return FALSE;
	}

	SYSSet.NetNum = temp;
	NetConfigInfo[SYSSet.NetYXSelct].NetNum = SYSSet.NetNum;
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read net set err");
	}
	Display_ShowNet();
}

/***********************************************************************************************
* Function		: MENU11_TIMEDispose
* Description	: ����Ա�����·�
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
static INT8U Munu3_CodeDispose(INT16U addr,INT8U *pvalue,INT8U len)
{
	INT32U code;
	INT32U admin_code = 888888;

	if((pvalue == NULL) || (len != 4) || (DispControl.CurUserGun >= GUN_MAX) )
	{
		return FALSE;
	}
	code = (pvalue[0] << 24) | (pvalue[1] << 16) | (pvalue[2] << 8) | pvalue[3];
	if(SYSSet.NetState == DISP_NET)
	{
		if(code == admin_code)   //������ȷ����ת������Ա����
		{
			if(HYMenu23.Menu_PrePage == &HYMenu3)
			{
				DisplayCommonMenu(&HYMenu47,&HYMenu73);  //��ת��ϵͳ���ý���
			}
			else
			{
				DisplayCommonMenu(&HYMenu46,&HYMenu73);  //��ת���û�����
			}
		}
		else
		{
			DisplayCommonMenu(&HYMenu24,&HYMenu73);  //�������
		}
	}

	else
	{
		if(code == admin_code)   //������ȷ����ת������Ա����
		{
			if(HYMenu23.Menu_PrePage == &HYMenu3)
			{
				DisplayCommonMenu(&HYMenu47,NULL);  //��ת��ϵͳ���ý���
			}
			else
			{
				DisplayCommonMenu(&HYMenu46,NULL);  //��ת���û�����
			}
		}
		else
		{
			DisplayCommonMenu(&HYMenu24,&HYMenu3);  //�������
		}

	}


	return TRUE;
}

const _DISP_ADDR_FRAME Disp_RecvFrameTable[] =
{
	/********************************������Ļ*************************/
	{INPUT_MENU21_CODE          	,       Munu3_16_CodeDispose },  // ����21���뿨�����ַ
	{INPUT_MENU23_CODE          	,       Munu3_CodeDispose 	},   // ����23�������Ա�����ַ
	{INPUT_MENU50_HANDA				,		Munu50_HANDADispose  },	//Aǹ�ֶ�ģʽ
	{INPUT_MENU51_HANDB				,		Munu51_HANDBDispose  },	//Bǹ�ֶ�ģʽ
	{DIS_ADD(52,0)					,		Munu52_DevnumDispose }, //׮��Ŵ���
	{DIS_ADD(52,8)					,		Munu52_ProNumDispose }, //��Ŀ��Ŵ���
	{DIS_ADD(52,0x11)					,		Munu52_ModuleNumDispose }, 	//ģ��ʹ�ø���
	{DIS_ADD(52,0x12)					,		Munu52_MacMaxVolDispose },//�豸��ߵ�ѹ
	{DIS_ADD(52,0x13)					,		Munu52_MacMinVolDispose },//�豸��͵�ѹ
	{DIS_ADD(52,0x14)					,		Munu52_MaxOutCurDispose },//����������
	{DIS_ADD(52,0x15)					,		Munu52_UseGunDispose },//����ǹ��
	{DIS_ADD(52,0x16)					,		Munu52_OutDcVolDispose },//��ѹ��ֵ
	{DIS_ADD(52,0x17)					,		Munu52_OutDcCurDispose },//������ֵ
	{DIS_ADD(52,0x18)					,		Munu52_SOCDispose 		},//SOC��ֵ

	{DIS_ADD(53,0)					,		Munu53_acinputupvalueDispose },// ���������ѹ
	{DIS_ADD(53,1)					,		Munu53_acinputdownvalueDispose },//��������Ƿѹ
	{DIS_ADD(53,2)					,		Munu53_MeterAddADispose },//Aǹ���
	{DIS_ADD(53,8)					,		Munu53_MeterAddBDispose },//Bǹ���
	{INPUT_MENU54_CFG				,		Munu54_SetTimeDispose	},//ʱ������

	{DIS_ADD(57,0)					,		Munu57_Just1Dispose		},//�Ӵ���ǰ����ѹУ׼1
	{DIS_ADD(57,1)					,		Munu57_Just2Dispose		},//�Ӵ���ǰ����ѹУ׼2
	{DIS_ADD(58,0)					,		Munu58_Just1Dispose		},//�Ӵ����󼶵�ѹУ׼1
	{DIS_ADD(58,1)					,		Munu58_Just2Dispose		},//�Ӵ����󼶵�ѹУ׼2
	{DIS_ADD(59,0)					,		Munu59_Just1Dispose		},//����У׼1
	{DIS_ADD(59,1)					,		Munu59_Just2Dispose		},//����У׼1
	{DIS_ADD(60,0)					,		Munu60_Just1Dispose		},//δ��ǹ��ѹ
	{DIS_ADD(60,1)					,		Munu60_Just2Dispose		},//��ǹ��ѹ
	{DIS_ADD(48,0)					,		Munu48_PrcDispose		},//��������
	{DIS_ADD(10,0x11)				,		Munu10_PortSert		},			//�˿�����
	{DIS_ADD(10,0x12)				,		Munu10_IP1Sert		},			//ip����
	{DIS_ADD(10,0x13)				,		Munu10_IP2Sert		},			//ip����
	{DIS_ADD(10,0x14)				,		Munu10_IP3Sert		},			//ip����
	{DIS_ADD(10,0x15)				,		Munu10_IP4Sert		},			//ip����
	{DIS_ADD(10,0x16)				,		Munu10_NetNumSet		},		//�����������

};

/***********************************************************************************************
* Function		: DealWithVari
* Description	: ����������
* Input			: INT16U addr       ������ַ
                  INT8U *pvalue     ����ֵָ��
                  INT8U len         ��������
* Output		:
* Note(s)		:
* Contributor	: 2018��7��31��
***********************************************************************************************/
static INT8U DealWithVari(INT16U addr, INT8U *pvalue,INT8U len)
{
	if (addr ==NULL ||pvalue ==NULL || !len)
	{
		return FALSE;
	}
	for (INT8U i = 0; i < sizeof(Disp_RecvFrameTable)/sizeof(_DISP_ADDR_FRAME); i++)
	{
		if (Disp_RecvFrameTable[i].variaddr == addr)                    //���ҵ�ַ
		{
			if (Disp_RecvFrameTable[i].Fun)                             //�ҵ���ͬ������ַ
			{
				return Disp_RecvFrameTable[i].Fun(addr, pvalue, len);   //��������
			}
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_DisplayRecvDataAnalyze
* Description  : ���ݽ��ս���
* Input        : INT8U *pdata
                 INT8U len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
INT8U APP_DisplayRecvDataAnalyze(INT8U *pdata, INT8U len)
{
	ST_Menu* CurMenu = GetCurMenu();
	_LCD_KEYVALUE keyval = (_LCD_KEYVALUE)0;                //ȡ��ֵ
	INT16U KeyID = 0;       								//������ID��ҳ���ID��Ӧ��

	if  ( (pdata == NULL) || (len < 6) || CurMenu == NULL)                       //��������������
	{
		return FALSE;
	}

	INT8U  datalen = pdata[2];                              //���ݳ��� = ֡����-֡ͷ��2byte��-����ռ�ÿռ䣨1byte��
	INT8U  cmd     = pdata[3];                              //����
	INT16U lcdhead = ((pdata[0]<<8) | pdata[1]);

	if (lcdhead == DWIN_LCD_HEAD)            //�ж�֡ͷ֡β
	{
		if ( (datalen + 3 != len) )
		{
			return FALSE;
		}
		if(cmd == VARIABLE_READ)                             //��������ַ��������
		{
			INT16U variaddr  = ((pdata[4]<<8) | pdata[5]);  //��ȡ������ַ
			INT8U  varilen   = pdata[6] * 2;                //��ȡ�������ݳ���(����ת�����ֽ�)
			INT8U *varivalue = &pdata[7];                   //��ȡ����ֵ��ʼ��ַ

			if(variaddr == KEY_VARI_ADDR)                   //���еİ�����ַ����0x0000 ֻ���ü�ֵȥ����
			{
				KeyID = ((pdata[datalen+1]<<8) | pdata[datalen+2]);	   //ȡ����ID
				keyval = (_LCD_KEYVALUE)(pdata[datalen+2] & 0x7f);     //ȡ��ֵ,ȡ��7λ
				if(DIS_ADD(CurMenu->FrameID,keyval) == KeyID)			//ֻ���ڵ�ǰ�����ϵİ�������Ч
				{
					DealWithKey(&keyval);                       		//��������
				}
			}
			else                                            //�������ݷ���
			{
				DealWithVari(variaddr,varivalue,varilen);   //�������ݴ���
			}

		}
#if (USER_SYSTTC == 0)
		if(cmd == REGISTER_READ)      //��ȡ������Ļʱ�䷵��
		{
			if(datalen != 0x0A)
			{
				return FALSE;
			}
			ReadDwRtc.Year = pdata[6];
			ReadDwRtc.Month = pdata[7];
			ReadDwRtc.Day = pdata[8];
			ReadDwRtc.Week = pdata[9];
			ReadDwRtc.Hour = pdata[10];
			ReadDwRtc.Minute = pdata[11];
			ReadDwRtc.Second = pdata[12];
		}
#endif
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_DWRTCGetTime
* Description  : ��ȡDWRTC
* Input        : INT8U *pdata
                 INT8U len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
INT8U APP_DWRTCGetTime(_BSPRTC_TIME *pTime)
{
	memcpy(pTime,&ReadDwRtc,sizeof(_BSPRTC_TIME));
	return TRUE;
}

/*****************************************************************************
* Function     : APP_DisplayRecvDataAnalyze
* Description  : ���ݽ��ս���
* Input        : INT8U *pdata
                 INT8U len
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U ChargeRecode_Dispose(_GUN_NUM gun)
{
	USERINFO* pric_info = NULL;
	_FLASH_OPERATION  FlashOper;
	USERCARDINFO* card_info;
	INT8U* pdata;
	INT32U money;

	//��ȡ������Ϣ
	pric_info =  GetChargingInfo(gun);
	if(pric_info == NULL)
	{
		return FALSE;
	}
	//GetCurTime(&pric_info->EndTime);                         //��¼�����Ʒ�ʱ��
	if(gun == GUN_A)
	{
		card_info = GetGunCardInfo(GUN_A);
	}
	else
	{
		card_info = GetGunCardInfo(GUN_B);
	}
	memcpy((INT8U *)&ChargeRecodeInfo[gun].StartTime,(INT8U *)&pric_info->StartTime,sizeof(pric_info->StartTime) );
	memcpy((INT8U *)&ChargeRecodeInfo[gun].EndTime,(INT8U *)&pric_info->EndTime,sizeof(pric_info->EndTime) );
	ChargeRecodeInfo[gun].TotMoney = pric_info->TotalBill/100;

	ChargeRecodeInfo[gun].TotPower = pric_info->TotalPower4/10;

	ChargeRecodeInfo[gun].ChargeTime = pric_info->ChargeTime;
	if(SYSSet.NetState == DISP_CARD)
	{
		ChargeRecodeInfo[gun].BeforeCardBalance = card_info->balance;
		if(ChargeRecodeInfo[gun].BeforeCardBalance == 0) //����������0
		{
			//if(SHOW_STOP_ERR_NONE == ChargeRecodeInfo[gun].StopChargeReason)
			{

				ChargeRecodeInfo[gun].StopChargeReason = (INT8U)SHOW_STOP_BALANCE - (INT8U)SHOW_STOP_ERR_NONE;			//����
			}
		}
	}
	else
	{
		money = APP_GetNetMoney(gun);
		if(money < ChargeRecodeInfo[gun].TotMoney)
		{
			ChargeRecodeInfo[gun].BeforeCardBalance = 0;
		}
		else
		{
			ChargeRecodeInfo[gun].BeforeCardBalance = APP_GetNetMoney(gun) - ChargeRecodeInfo[gun].TotMoney;
		}
		pdata = APP_GetBatchNum(gun);
		memcpy(ChargeRecodeInfo[gun].TransNum,pdata,sizeof(ChargeRecodeInfo[gun].TransNum));  //������ˮ��
	}
	//SOC�ﵽ
	if(((ChargeingInfo[gun].ChargeSOC & 0xff00) >> 8) >= SYSSet.SOCthreshold)
	{
		//if(SHOW_STOP_ERR_NONE == ChargeRecodeInfo[DispControl.CurUserGun].StopChargeReason)
		{
			ChargeRecodeInfo[gun].StopChargeReason = (INT8U)SHOW_STOP_SOC - (INT8U)SHOW_STOP_ERR_NONE;			//�趨soc�ﵽ
		}
	}




	//д���׼�¼
	//RecodeControl.RecodeCurNum = 250;
	RecodeControl.RecodeCurNum++;
	//д�뽻�׼�¼����
	FlashOper.DataID = PARA_1200_ID;
	FlashOper.Len = PARA_1200_FLLEN;
	FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Writer RecodeCurNum  err");
		return FALSE;
	}
	APP_RWChargeRecode(RECODE_DISPOSE1(RecodeControl.RecodeCurNum%1000),FLASH_ORDER_WRITE,&ChargeRecodeInfo[gun]);


	//����������ٲ���һ��  20220902
	//�ر�ָʾ��
	BSP_CloseRunLed(gun);
	//�ر�K1K2
	BSP_CloseK1K2(gun);
	//����Ϊδ���
	SetChargeRunningState(gun,FALSE);
	return TRUE;
}

INT8U BUG35CNU = 0;
/*****************************************************************************
* Function     : APP_RecvCardDispose
* Description  : ���յ������������
* Input        :_BSP_MESSAGE *pMsg
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U APP_RecvCardDispose(_SEND_TO_CARD_CMD cmd,FRAMERTU rtu)
{
	USERCARDINFO * puser_card_info = NULL;
	INT32U CardID;			//����
	ST_Menu* pcurMenu = NULL;	//��ǰ�˵�

	if(cmd >= CARDCMD_MAX)
	{
		return FALSE;
	}
	if((cmd == CARDCMD_STARTA) || (cmd == CARDCMD_STARTB) )
	{
		switch (rtu)
		{
			case _SuccessRtu:
				//��ת�������н���
				//����������
				//֪ͨ�Ʒ�����ʼ�Ʒ�
				//֪ͨ��ccuͨ������ʼ�������
				//��ȡ����Ϣ
				if(cmd == CARDCMD_STARTA)
				{
					puser_card_info = GetGunCardInfo(GUN_A);
				}
				else
				{
					puser_card_info = GetGunCardInfo(GUN_B);
				}
				if(puser_card_info == NULL)
				{
					return FALSE;
				}
				//������Ϣ�����׼�¼��
				ChargeRecodeInfo[DispControl.CurUserGun].CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
				        (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
				//			ChargeRecodeInfo[DispControl.CurUserGun].AfterCardBalance = puser_card_info->balance;
				ChargeRecodeInfo[DispControl.CurUserGun].Gun = DispControl.CurUserGun;
				ChargeRecodeInfo[DispControl.CurUserGun].BillingStatus = RECODE_UNSETTLEMENT;	  			//��ʼΪδ����
				if(cmd == CARDCMD_STARTA)
				{
					//DisplayCommonMenu(&gsMenu28,NULL);  //��ת��Aǹ��������
					DisplayCommonMenu(&HYMenu30,NULL);  //��ת��Aǹ��������
				}
				else
				{
					//DisplayCommonMenu(&gsMenu34,NULL);  //��ת��bǹ��������
					DisplayCommonMenu(&HYMenu31,NULL);  //��ת��Bǹ��������
				}
				//	SendElecLockMsg(DispControl.CurUserGun,SET_ELECLOCK);	//��������
				SendPricMsg(DispControl.CurUserGun,PRIC_START);			//֪ͨ��ʼ�Ʒ�
				DispControl.SendStartCharge[DispControl.CurUserGun] = TRUE;
				//SendStartChargeMsg(DispControl.CurUserGun,MODE_AUTO);	//֪ͨ�������
				break;
			case _DataOverTime:
			case _FailRtu:   //ʧ�ܲ�����
				//��ʾ�޷�ʶ��˿�
				//DisplayCommonMenu(&gsMenu24,NULL);	//��ʾ�޷�ʶ��˿�
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu28,&HYMenu21);	//��ʾ������ʱ
#else
				DisplayCommonMenu(&HYMenu28,&HYMenu22);	//��ʾ������ʱ
#endif
				break;
			case _BalanceLack:
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu25,&HYMenu21);	//��ʾ�޷�ʶ��˿�
#else
				DisplayCommonMenu(&HYMenu25,&HYMenu22);	//��ʾ�޷�ʶ��˿�
#endif
				break;
			case _FailPIN:   //�������
				//��ת����������������
				//��ȡ����Ϣ
				//��ʾʣ�������������
				//DisplayCommonMenu(&gsMenu21,NULL);  //��ת��������������
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu24,&HYMenu21);	//��ʾ�޷�ʶ��˿�
#else
				DisplayCommonMenu(&HYMenu24,&HYMenu22);	//��ʾ�޷�ʶ��˿�
#endif
				break;
				//PrintNum16uVariable(ADDR_MENU21_SHOW,puser_card_info->RetryCnt); 		//��ʾʣ�������������
			case _GreyLockFull: //�˿�����
			case _LockCard:
				//��ʾ������
				//DisplayCommonMenu(&gsMenu23,NULL);  //��ת��ʾ����������
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu26,&HYMenu21);	//��ʾ�޷�ʶ��˿�
#else
				DisplayCommonMenu(&HYMenu26,&HYMenu22);	//��ʾ�޷�ʶ��˿�
#endif
				break;
			default:
				//��ʾ�޷�ʶ��˿�
				//DisplayCommonMenu(&gsMenu24,NULL);	//��ʾ�޷�ʶ��˿�
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu27,&HYMenu21);	//��ʾ�޷�ʶ��˿�
#else
				DisplayCommonMenu(&HYMenu27,&HYMenu22);	//��ʾ�޷�ʶ��˿�
#endif
				break;
		}
	}
	if((cmd == CARDCMD_STOPA) || (cmd == CARDCMD_STOPB) )
	{
		switch (rtu)
		{
			case _SuccessRtu:
				ChargeRecodeInfo[DispControl.CurUserGun].BillingStatus = RECODE_SETTLEMENT;	  //�Ѿ�����
//				if(cmd == CARDCMD_STOPA)
//				{
//					puser_card_info = GetGunCardInfo(GUN_A);
//				}else{
//					puser_card_info = GetGunCardInfo(GUN_B);
//				}
//							//��ȡ������Ϣ
//				pric_info =  GetChargingInfo(DispControl.CurUserGun);
//				if(pric_info == NULL)
//				{
//					return FALSE;
//				}
//				//GetCurTime(&pric_info->EndTime);                         //��¼�����Ʒ�ʱ��
//				if(CARDCMD_STOPA == cmd){
//					card_info = GetGunCardInfo(GUN_A);
//				}else{
//					card_info = GetGunCardInfo(GUN_B);
//				}
//				memcpy((INT8U *)&ChargeRecodeInfo[DispControl.CurUserGun].StartTime,(INT8U *)&pric_info->StartTime,sizeof(pric_info->StartTime) );
//				memcpy((INT8U *)&ChargeRecodeInfo[DispControl.CurUserGun].EndTime,(INT8U *)&pric_info->EndTime,sizeof(pric_info->EndTime) );
//				ChargeRecodeInfo[DispControl.CurUserGun].TotMoney = pric_info->TotalBill;
//				ChargeRecodeInfo[DispControl.CurUserGun].TotPower = pric_info->TotalPower;
//				ChargeRecodeInfo[DispControl.CurUserGun].ChargeTime = pric_info->ChargeTime;
//				ChargeRecodeInfo[DispControl.CurUserGun].BeforeCardBalance = card_info->balance;
//				//д���׼�¼
//				RecodeControl.RecodeCurNum  = 0;
//				#warning "��������  ������ʱ����"
//				//д�뽻�׼�¼����
//				FlashOper.DataID = PARA_1200_ID;
//				FlashOper.Len = PARA_1200_FLLEN;
//				FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
//				FlashOper.RWChoose = FLASH_ORDER_WRITE;
//				if(APP_FlashOperation(&FlashOper) == FALSE)
//				{
//					printf("Writer RecodeCurNum  err");
//				}
//				APP_RWChargeRecode(RECODE_DISPOSE1(RecodeControl.RecodeCurNum%1000),FLASH_ORDER_WRITE,&ChargeRecodeInfo[DispControl.CurUserGun]);
				if(cmd == CARDCMD_STOPA)
				{
					pcurMenu =  GetCurMenu();
					if(pcurMenu == &HYMenu32)  //��ǰ�˵�Ϊ����ʧ�ܽ������
					{
						ChargeRecode_Dispose(GUN_A);				//���׼�¼����
						DispControl.StartIntTime[GUN_A] = OSTimeGet();
						DisplayCommonMenu(&HYMenu40,&HYMenu32);	//ֱ�ӵ�ת��Aǹ�������
						DispShow_EndChargeDate(GUN_A);			//��ʾ������Ϣ
					}
					else
					{
						DispControl.CardOver[GUN_A] = STATE_OVER;  //����ˢ��ִ���ˡ�
						if(DispControl.StopOver[GUN_A] ==  STATE_UNOVER)   //ֹͣδִ��
						{
							DisplayCommonMenu(&HYMenu35,NULL);	//��ת��ֹͣ�н���
							SendStopChargeMsg(GUN_A);   		//����ֹͣ֡
							NB_WriterReason(GUN_A,"E61",3);
							BUG35CNU++;
						}
						else
						{
							//ˢ��ִ��״̬��ֹֹͣͣ״̬����
							DispControl.CardOver[GUN_A] = STATE_UNOVER;
							DispControl.StopOver[GUN_A] = STATE_UNOVER;
							ChargeRecode_Dispose(GUN_A);				//���׼�¼����
							DispControl.StartIntTime[GUN_A] = OSTimeGet();
							DisplayCommonMenu(&HYMenu40,&HYMenu33);	//ֱ�ӵ�ת��Aǹ�������
							DispShow_EndChargeDate(GUN_A);			//��ʾ������Ϣ
						}
					}
//					SendElecLockMsg(GUN_A,SET_ELECUNLOCK);
				}
				else
				{
					pcurMenu =  GetCurMenu();
					if(pcurMenu == &HYMenu32)  //��ǰ�˵�Ϊ����ʧ�ܽ������
					{
						ChargeRecode_Dispose(GUN_B);				//���׼�¼����
						DispControl.StartIntTime[GUN_B] = OSTimeGet();
						DisplayCommonMenu(&HYMenu41,&HYMenu32);	//ֱ�ӵ�ת��Bǹ�������
						DispShow_EndChargeDate(GUN_B);			//��ʾ������Ϣ
					}
					else
					{
						DispControl.CardOver[GUN_B] = STATE_OVER;  //����ˢ��ִ���ˡ�
						if(DispControl.StopOver[GUN_B] ==  STATE_UNOVER)   //ֹͣδִ��
						{
							DisplayCommonMenu(&HYMenu35,NULL);	//��ת��ֹͣ�н���
							NB_WriterReason(GUN_B,"E60",3);
							SendStopChargeMsg(GUN_B);   		//����ֹͣ֡
							BUG35CNU++;
						}
						else
						{
							//ˢ��ִ��״̬��ֹֹͣͣ״̬����
							DispControl.CardOver[GUN_B] = STATE_UNOVER;
							DispControl.StopOver[GUN_B] = STATE_UNOVER;
							ChargeRecode_Dispose(GUN_B);				//���׼�¼����
							DispControl.StartIntTime[GUN_B] = OSTimeGet();
							DisplayCommonMenu(&HYMenu41,&HYMenu33);	//ֱ�ӵ�ת��Aǹ�������
							DispShow_EndChargeDate(GUN_B);			//��ʾ������Ϣ
						}
					}
				}
				break;
			case _FailRtu:   		//ʧ�ܲ�����
			case _DataOverTime:
				//��ʾ�޷�ʶ��˿�
				pcurMenu =  GetCurMenu();
				//DisplayCommonMenu(&gsMenu24,NULL);	//��ʾ�޷�ʶ��˿�
#if(CARD_USER_CODE == 1)
				DisplayCommonMenu(&HYMenu28,pcurMenu);	//��ʾ������ʱ
#else
				DisplayCommonMenu(&HYMenu28,pcurMenu);	//��ʾ������ʱ
#endif
				break;
			case _NOStartCard:  	//���Ų�һ��
				//DisplayCommonMenu(&gsMenu55,NULL);      //��ת�����Ų�һ�½���
				pcurMenu =  GetCurMenu();
				DisplayCommonMenu(&HYMenu34,pcurMenu);      //��ת�����Ų�һ�½���
				break;
			default:
				break;
		}
	}
	if(cmd == CARDCMD_QUERY)
	{

		DisplayCommonMenu(&HYMenu6,NULL);      //�翨��Ϣ����

		switch (rtu)
		{
			case _SuccessRtu:
				puser_card_info = GetCardInfo();
				if(puser_card_info == NULL)
				{
					return FALSE;
				}
				//��ʾ����Ϣ
				CardID = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) | (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);
				PrintNum32uVariable(DIS_ADD(HYMenu6.FrameID,0),CardID);
				PrintNum32uVariable(DIS_ADD(HYMenu6.FrameID,2),puser_card_info->balance);
				if(puser_card_info->lockstate == 0xff)
				{
					Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_LOCK,RED);   //δ����
				}
//				else if(puser_card_info->lockstate == 0x55){
//				Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_PERLLOCK,RED); //�������
//				}
				else
				{
					Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_UNLOCK,RED); //�ѽ���
				}
				DisplayCommonMenu(&HYMenu6,NULL);      //�翨��Ϣ����
				break;
			case _FailPIN:   //�������
				//��ת����������������
				//��ȡ����Ϣ
				//��ʾʣ�������������
				//DisplayCommonMenu(&gsMenu21,NULL);  //��ת��������������
				DisplayCommonMenu(&HYMenu24,&HYMenu3);  //��ת��������������
				break;
			default:
				//��ʾ�޷�ʶ��˿�
				//DisplayCommonMenu(&gsMenu24,NULL);	//��ʾ�޷�ʶ��˿�
				DisplayCommonMenu(&HYMenu27,&HYMenu3);	//��ʾ�޷�ʶ��˿�
				break;

		}
	}
	SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
	return TRUE;
}

/*****************************************************************************
* Function     : APP_RecvCardAnalyze
* Description  : ��������ˢ���������Ϣ
* Input        :_BSP_MESSAGE *pMsg
* Output       : None
* Return       :
* Note(s)      :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U APP_RecvCardAnalyze(_BSP_MESSAGE *pMsg)
{
	USERCARDINFO* puser_card_info;
	INT32U CardNum;
	INT8U Card[8] = {0};
	INT8U * pdata;
	INT8U num,i;
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	if((pMsg == NULL) || (pMsg->pData == NULL) )
	{
		return FALSE;
	}
	switch(pMsg->DivNum)
	{
		case CARDCMD_QUERY:		      //��ѯ������Ϣ
			APP_RecvCardDispose((_SEND_TO_CARD_CMD)pMsg->DivNum,(FRAMERTU)(*pMsg->pData));
			break;
		case CARDCMD_STARTA:		  //A��ʼ���ˢ��
		case CARDCMD_STOPA:
			if(DispControl.CurUserGun != GUN_A)
			{
				return FALSE;
			}
			APP_RecvCardDispose((_SEND_TO_CARD_CMD)pMsg->DivNum,(FRAMERTU)(*pMsg->pData));
			break;
		case CARDCMD_STARTB:		  //b��ʼ���ˢ��
		case CARDCMD_STOPB:				//bֹͣ���ˢ��
			if(DispControl.CurUserGun != GUN_B)
			{
				return FALSE;
			}
			APP_RecvCardDispose((_SEND_TO_CARD_CMD)pMsg->DivNum,(FRAMERTU)(*pMsg->pData));
			break;
		case NETCARDCMD_STARTA:		  //Aǹ���翪ʼˢ��
			//���Ϳ���Ȩ
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(DispControl.NetGunState[GUN_A] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
					return FALSE;
				}
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_A].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_A].DivNum = APP_CARD_INFO;
					SendMsg[GUN_A].DataLen =GUN_A;
					OSQPost(pevent, &SendMsg[GUN_A]);
				}
				else
				{
					//�ж��Ƿ�Ϊ������
					pdata = APP_GetCARDWL();
					num = pdata[0];	//������������
					if(pdata == NULL)
					{
						return FALSE;
					}
					puser_card_info = GetGunCardInfo(GUN_A); //��ȡ����
					CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
					          (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);

					Card[0] = HEXtoBCD(CardNum / 100000000);
					Card[1] = HEXtoBCD(CardNum % 100000000 /1000000);
					Card[2] = HEXtoBCD(CardNum % 1000000 /10000);
					Card[3] = HEXtoBCD(CardNum % 10000 /100);
					Card[4] = HEXtoBCD(CardNum % 100 /1);
					for(i = 0; i < num; i++)
					{
						if(CmpNBuf(&pdata[1+i*8],Card,8) )
						{
							_4G_SetStartType(GUN_A,_4G_APP_CARD);			//����Ϊ������
							APP_SetStartNetState(GUN_A,NET_STATE_OFFLINE);	//���߳��
							Net_StartCharge(GUN_A,MODE_AUTO);				//��ҲΪ�Զ�������ʽ
							break;
						}
					}
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
			}
			if((SYSSet.NetYXSelct == XY_ZSH)||SYSSet.NetYXSelct == XY_HY||  (SYSSet.NetYXSelct == XY_YL2)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if(DispControl.NetGunState[GUN_A] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
					return FALSE;
				}
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_A].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_A].DivNum = APP_CARD_INFO;
					SendMsg[GUN_A].DataLen =GUN_A;
					OSQPost(pevent, &SendMsg[GUN_A]);
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
			}
			break;
		case NETCARDCMD_STARTB:		  //Bǹ���翪ʼˢ��
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(DispControl.NetGunState[GUN_B] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
					return FALSE;
				}
				//���Ϳ���Ȩ
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_B].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_B].DivNum = APP_CARD_INFO;
					SendMsg[GUN_B].DataLen =GUN_B;
					OSQPost(pevent, &SendMsg[GUN_B]);
				}
				else
				{
					//�ж��Ƿ�Ϊ������
					pdata = APP_GetCARDWL();
					num = pdata[0];	//������������
					if(pdata == NULL)
					{
						return FALSE;
					}
					puser_card_info = GetGunCardInfo(GUN_B); //��ȡ����
					CardNum = (puser_card_info->CardID[0]) | (puser_card_info->CardID[1] << 8) |\
					          (puser_card_info->CardID[2] << 16) | (puser_card_info->CardID[3] << 24);

					Card[0] = HEXtoBCD(CardNum / 100000000);
					Card[1] = HEXtoBCD(CardNum % 100000000 /1000000);
					Card[2] = HEXtoBCD(CardNum % 1000000 /10000);
					Card[3] = HEXtoBCD(CardNum % 10000 /100);
					Card[4] = HEXtoBCD(CardNum % 100 /1);
					for(i = 0; i < num; i++)
					{
						if(CmpNBuf(&pdata[1+i*8],Card,8) )
						{
							_4G_SetStartType(GUN_B,_4G_APP_CARD);			//����Ϊ������
							APP_SetStartNetState(GUN_B,NET_STATE_OFFLINE);	//���߳��
							Net_StartCharge(GUN_B,MODE_AUTO);				//��ҲΪ�Զ�������ʽ
							break;
						}
					}
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
			}

			if((SYSSet.NetYXSelct == XY_ZSH)||SYSSet.NetYXSelct == XY_HY||  (SYSSet.NetYXSelct == XY_YL2)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG) || (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if(DispControl.NetGunState[GUN_B] != GUN_IDLE)
				{
					DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
					SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
					return FALSE;
				}
				//���Ϳ���Ȩ
				if(APP_GetAppRegisterState(0) == STATE_OK)
				{
					SendMsg[GUN_B].MsgID = BSP_MSGID_DISP;
					SendMsg[GUN_B].DivNum = APP_CARD_INFO;
					SendMsg[GUN_B].DataLen =GUN_B;
					OSQPost(pevent, &SendMsg[GUN_B]);
				}
				DisplayCommonMenu(HYMenu22.Menu_PrePage,NULL);
				SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
			}
			break;
		case NETCARDCMD_STOPA:		 //Aǹ����ֹͣ���
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||(SYSSet.NetYXSelct == XY_TT)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if((FRAMERTU)(*pMsg->pData) == _SuccessRtu)
				{
					NB_WriterReason(GUN_A,"E54",3);
					Net_StopCharge(GUN_A);   //ֹͣ���
				}
				SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
			}
			break;
		case NETCARDCMD_STOPB:		 //Bǹ����ֹͣ���
			if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2)|| (SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
			{
				if((FRAMERTU)(*pMsg->pData) == _SuccessRtu)
				{
					NB_WriterReason(GUN_B,"E53",3);
					Net_StopCharge(GUN_B); //ֹͣ���
				}
				SetCardInfoStatus(_Already_Obtain);			//�����ת��ʹ�����ڿ���״̬
				break;
			}
		default:
			break;
	}
	return TRUE;
}

/*****************************************************************************
* Function	   : APP_RecvConectCCUAnalyze
* Description  : ������������ccu�������Ϣ
* Input 	   :_BSP_MESSAGE *pMsg
* Output	   : None
* Return	   :
* Note(s)	   :
* Contributor  :2018-6-14
*****************************************************************************/
static INT8U APP_RecvConectCCUAnalyze(_BSP_MESSAGE *pMsg)
{
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	INT8U gun;
	INT8U state;
	gun = pMsg->GunNum;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	if(SYSSet.NetState == DISP_NET)
	{
		switch(pMsg->DivNum)
		{
			case APP_CHARGE_FAIL:            //����ʧ��
				DispControl.NetSSTState[gun] = NET_STARTFAIL;
				state = GetStartFailType(gun);
				ChargeRecodeInfo[gun].StopChargeReason = state | 0x80;		//��ʾ����ʧ��
				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_PLATFORM;				//��Ӫƽ̨����
				GetCurTime(&GunBillInfo[gun].EndTime);                         //��¼�����Ʒ�ʱ��
				memcpy(&ChargeRecodeInfo[gun].EndTime,&GunBillInfo[gun].EndTime,sizeof(_BSPRTC_TIME) );

				//����ʧ�ܷ��ÿ϶�Ϊ0
				GunBillInfo[gun].TotalBill = 0;
				GunBillInfo[gun].TotalPower4 = 0;
				SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//ֹ֪ͨͣ�Ʒ�
				ChargeRecode_Dispose((_GUN_NUM)gun);			//���׼�¼����
				if(SYSSet.NetYXSelct == XY_66)
				{
					//���Ϳ�ʼ���Ӧ��
					SendMsg[gun].MsgID = BSP_MSGID_DISP;
					SendMsg[gun].DivNum = APP_START_ACK;
					SendMsg[gun].DataLen = gun;
					OSQPost(pevent, &SendMsg[gun]);
				}
				else
				{
					//���Ϳ�ʼ���Ӧ��
					SendMsg[gun].MsgID = BSP_MSGID_DISP;
					SendMsg[gun].DivNum = APP_STOP_BILL;
					SendMsg[gun].DataLen = gun;
					OSQPost(pevent, &SendMsg[gun]);
				}
				if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
				{
					_4G_SetStartType(gun,_4G_APP_START);			//����Ϊapp����
				}
				break;
			case APP_CHARGE_SUCCESS:         //�����ɹ�
				DispControl.NetSSTState[gun] = NET_IDLE;
				DispControl.NetGunState[gun] = GUN_CHARGEING;
				memset(&ChargeingInfo[gun],0,sizeof(_CHARGEING_INFO));
				//ChargeRecodeInfo[gun].StopChargeReason =state | 0x80;
				ChargeRecodeInfo[gun].StopChargeReason  = 0;
				APP_Set_ERR_Branch(gun,STOP_ERR_NONE);
				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_PLATFORM;				//��Ӫƽ̨����
				if(SYSSet.NetState == DISP_NET)
					WriterFmBill((_GUN_NUM)gun,1);
				//���Ϳ�ʼ���Ӧ��
				if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //�����Ϸ�����
				{
					if((SYSSet.NetYXSelct == XY_AP) || (SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_JG)||  (SYSSet.NetYXSelct == XY_YL2) || (SYSSet.NetYXSelct == XY_66))
					{

						SendMsg[gun].MsgID = BSP_MSGID_DISP;
						SendMsg[gun].DivNum = APP_START_ACK;
						SendMsg[gun].DataLen = gun;
						OSQPost(pevent, &SendMsg[gun]);
					}
//					#if(APP_USER_TYPE ==  APP_USER_YKC)
//					SendMsg[gun].MsgID = BSP_MSGID_DISP;
//					SendMsg[gun].DivNum = APP_SJDATA_QUERY;   //����ʵʱ���ݣ�״̬�����˸ı�
//					SendMsg[gun].DataLen = gun;
//					OSQPost(pevent, &SendMsg[gun]);
//					#endif
					if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
					{
						SendMsg[gun].MsgID = BSP_MSGID_DISP;
						SendMsg[gun].DivNum = APP_VIN_INFO_START;
						SendMsg[gun].DataLen = gun;
						OSQPost(pevent, &SendMsg[gun]);
					}
				}
				break;
			case APP_CHARGE_END:
				if(DispControl.StopOverFrame[gun] == STATE_UNOVER) //���ӱ������ܴ���ִ�ж��ֹͣ��ɣ����½�����ת����
				{
					SendMsg[gun].DivNum = APP_STOP_BILL;
					DispControl.NetSSTState[gun] = NET_IDLE;
					DispControl.NetGunState[gun] = GUN_IDLE;
					DispControl.StartIntTime[gun] = OSTimeGet();
					DispControl.StopOverFrame[gun] = STATE_OVER;
					ChargeRecodeInfo[gun].BillingStatus = RECODE_SETTLEMENT;		//�Ѿ�����

					ChargeRecodeInfo[gun].StopChargeReason = APP_Get_ERR_Branch(gun);


					if(ChargeRecodeInfo[gun].StopChargeReason == STOP_HANDERR)
					{
						//��Щ���ӳ��䵽�ٷ�֮90��  ���Զ��Ͽ�ǹ   20210720
						if(BMS_BCS_Context[gun].SOC > 90)
						{
							ChargeRecodeInfo[gun].StopChargeReason = STOP_ERR_NONE;
						}
					}
					SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//ֹ֪ͨͣ�Ʒ�
					GetCurTime(&GunBillInfo[gun].EndTime);                         //��¼�����Ʒ�ʱ��

					memset(&ChargeingInfo[gun],0,sizeof(_CHARGEING_INFO));	//��ֹ�ٴν����������ʾ�ܴ������
					ChargeRecode_Dispose((_GUN_NUM)gun);								//���׼�¼����


					SendMsg[gun].MsgID = BSP_MSGID_DISP;
					SendMsg[gun].DataLen = gun;
					if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
					{
						if((VinControl[gun].VinState ==  VIN_FAIL) && (_4G_GetStartType(gun) == _4G_APP_VIN) )  //������Ȩʧ�ܲ����Ͷ���
						{
							_4G_SetStartType(gun,_4G_APP_START);			//����Ϊapp����
							break;
						}
						_4G_SetStartType(gun,_4G_APP_START);			//����Ϊapp����
					}
					OSQPost(pevent, &SendMsg[gun]);
				}
				break;

			default:
				break;
		}

	}
	else
	{
		ST_Menu* pcur = GetCurMenu();

		if(pMsg == NULL || DispControl.CurUserGun >= GUN_MAX)
		{
			return FALSE;
		}

		if((pcur == &HYMenu50) || (pcur == &HYMenu51) || (pcur == &HYMenu57)  || (pcur == &HYMenu58) || (pcur == &HYMenu59) )  //�ֶ�ģʽ�»�У׼ģʽ�µ�ǰҳ��������ת
		{
			return FALSE;
		}

		switch(pMsg->DivNum)
		{
			case APP_CHARGE_FAIL:            //����ʧ��
				DispControl.StopOverFrame[gun] = STATE_UNOVER;  //ֹͣ���֡δ����
				state = GetStartFailType(gun);
				ChargeRecodeInfo[gun].StopChargeReason = state | 0x80;		//��ʾ����ʧ��
				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_DEVICE;				//Ŀǰ������ʽ���ǳ�������
				GetCurTime(&GunBillInfo[gun].EndTime);                         //��¼�����Ʒ�ʱ��
				memcpy(&ChargeRecodeInfo[gun].EndTime,&GunBillInfo[gun].EndTime,sizeof(_BSPRTC_TIME) );
				//Dis_ShowStartErr(ADDR_MENU70_SHOW,(INT8U)StartOver_Info[DispControl.CurUserGun].startfailreason);//��ʾ����ʧ��ԭ��
				DisplayCommonMenu(&HYMenu32,NULL);		//����ʧ�ܽ���
				if(pMsg->DivNum == APP_CHARGE_FAIL)
				{
					Dis_ShowStatus(DIS_ADD(HYMenu32.FrameID,0),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
				}
				else
				{

					Dis_ShowStatus(DIS_ADD(HYMenu32.FrameID,0),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
				}
				SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//ֹ֪ͨͣ�Ʒ�
				break;
			case APP_CHARGE_SUCCESS:         //�����ɹ�
				APP_Set_ERR_Branch(gun,STOP_ERR_NONE);
				memset(&ChargeingInfo[GUN_A],0,sizeof(_CHARGEING_INFO));
				memset(&ChargeingInfo[GUN_B],0,sizeof(_CHARGEING_INFO));

				memcpy(ChargeRecodeInfo[gun].CarVin,BMS_BRM_Context[gun].VIN,17);
				ChargeRecodeInfo[gun].StartType = TYPE_START_DEVICE;				//Ŀǰ������ʽ���ǳ�������
				//ChargeRecodeInfo[DispControl.CurUserGun].ChargeType = Start_Info[DispControl.CurUserGun].modetype;  //��緽ʽ

				if(gun == GUN_A)
				{
					//DisplayCommonMenu(&gsMenu44,NULL);		//��ת��Aǹ������
					HYDisplayMenu36();						//��ֹ������ʾ����֮ǰ������
					DisplayCommonMenu(&HYMenu36,NULL);		//��ת��Aǹ������
				}
				else
				{
					//DisplayCommonMenu(&gsMenu45,NULL);		//��ת��bǹ������
					HYDisplayMenu37();						//��ֹ������ʾ����֮ǰ������
					DisplayCommonMenu(&HYMenu37,NULL);		//��ת��Aǹ������
				}

				break;
				//ֹͣ�ɹ�������CCU�ȷ��͹��� ����ʹ��DispControl.CurUserGun
			case APP_CHARGE_END:
				if(DispControl.StopOverFrame[gun] == STATE_UNOVER) //���ӱ������ܴ���ִ�ж��ֹͣ��ɣ����½�����ת����
				{
					DispControl.StopOverFrame[gun] = STATE_OVER;
					//ChargeRecodeInfo[DispControl.CurUserGun].SOC = StopOver_Info[DispControl.CurUserGun].EndChargeSOC;
					ChargeRecodeInfo[gun].StopChargeReason = APP_Get_ERR_Branch(gun);
					SendPricMsg((_GUN_NUM)gun,PRIC_STOP);			//ֹ֪ͨͣ�Ʒ�
					GetCurTime(&GunBillInfo[gun].EndTime);                         //��¼�����Ʒ�ʱ��
					//��Ҫ�ж�������ֹͣ���Ǳ���ֹͣ
					if(gun == GUN_A)
					{
						memset(&ChargeingInfo[GUN_A],0,sizeof(_CHARGEING_INFO));	//��ֹ�ٴν����������ʾ�ܴ������
						//�Ʒѵ�Ԫ����ֹͣ

						DispControl.StopOver[GUN_A] = STATE_OVER;  //����ִֹͣ���ˡ�
						if(DispControl.CardOver[GUN_A] ==  STATE_UNOVER)   //ˢ��δִ��
						{
							DispControl.ClickGunNextPage[GUN_A] = &HYMenu33;
							if((pcur == &HYMenu36) || (pcur == &HYMenu30))	//�ڳ���н�������������н���
							{
								DisplayCommonMenu(&HYMenu33,NULL);      //��ת��ˢ���������
							}
						}
						else
						{
							//CCUֹͣ
							//ˢ��ִ��״̬��ֹֹͣͣ״̬����
							DispControl.CardOver[GUN_A] = STATE_UNOVER;
							DispControl.StopOver[GUN_A] = STATE_UNOVER;

							//���ܴ��ڿ۵�Ǯ��ʱ����ʾ��һ����ԭ����ˢ��ֹͣ����������в����˷��ã�
							if((GunBillInfo[GUN_A].TotalBill >= GunBillInfo[GUN_A].RealTotalBill) \
							        && (GunBillInfo[GUN_A].TotalBill <= (GunBillInfo[GUN_A].RealTotalBill + 10000)))
							{
								//��һԪ֮��
								GunBillInfo[GUN_A].TotalBill = GunBillInfo[GUN_A].RealTotalBill;
								GunBillInfo[GUN_A].TotalPower4 = GunBillInfo[GUN_A].RealTotalPower;
							}
							ChargeRecode_Dispose(GUN_A);				//���׼�¼����
							DisplayCommonMenu(&HYMenu40,&HYMenu35);      //��ת���������
							DispShow_EndChargeDate(GUN_A);
							DispControl.StartIntTime[GUN_A] = OSTimeGet();

						}
					}
					else
					{
						memset(&ChargeingInfo[GUN_B],0,sizeof(_CHARGEING_INFO));	//��ֹ�ٴν����������ʾ�ܴ������

						DispControl.StopOver[GUN_B] = STATE_OVER;  //����ִֹͣ���ˡ�
						if(DispControl.CardOver[GUN_B] ==  STATE_UNOVER)   //ˢ��δִ��
						{
							DispControl.ClickGunNextPage[GUN_B] = &HYMenu33;
							if((pcur == &HYMenu37) || (pcur == &HYMenu31))	//�ڳ���н�������������н���
							{
								DisplayCommonMenu(&HYMenu33,NULL);      //��ת��ˢ���������
							}
						}
						else
						{
							//CCUֹͣ
							//ˢ��ִ��״̬��ֹֹͣͣ״̬����
							DispControl.CardOver[GUN_B] = STATE_UNOVER;
							DispControl.StopOver[GUN_B] = STATE_UNOVER;


							//���ܴ��ڿ۵�Ǯ��ʱ����ʾ��һ����ԭ����ˢ��ֹͣ����������в����˷��ã�
							if((GunBillInfo[GUN_B].TotalBill >= GunBillInfo[GUN_B].RealTotalBill) \
							        && (GunBillInfo[GUN_B].TotalBill <= (GunBillInfo[GUN_B].RealTotalBill + 10000)))
							{
								//��һԪ֮��
								GunBillInfo[GUN_B].TotalBill = GunBillInfo[GUN_B].RealTotalBill;
								GunBillInfo[GUN_B].TotalPower4 = GunBillInfo[GUN_B].RealTotalPower;
							}

							ChargeRecode_Dispose(GUN_B);				//���׼�¼����
							DisplayCommonMenu(&HYMenu41,&HYMenu35);      //��ת���������
							DispShow_EndChargeDate(GUN_B);
							DispControl.StartIntTime[GUN_B] = OSTimeGet();
						}
					}
				}

				//Dis_ShowStartErr(ADDR_MENU48_SHOW,0);	//����ʾ��ʾ����ʧ��ԭ��
				break;
			default:
				break;
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Display_ShowSet1
* Description  : ��ʾ�������� 		һ��12�� entrn 0��ʾ��һ�ν���   1��ʾ��һҳ   2��ʾ��һҳ
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_PricSet(INT8U entrn)
{

	ST_Menu * ppage;
	_FLASH_OPERATION  FlashOper;
	static INT8U i;
	INT8U num;
	INT8U starthour[TIME_PERIOD_MAX],startminute[TIME_PERIOD_MAX];
	INT8U stophour[TIME_PERIOD_MAX],stopminute[TIME_PERIOD_MAX];
	//��ȡ����
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = MIN(PARA_PRICALL_FLLEN,sizeof(PriceSet) );
	FlashOper.ptr = (INT8U *)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
		return;
	}

	if(GetCurMenu() !=  &HYMenu48)   //�������ý��洦��
	{
		if(entrn == 0)
		{
			i = 0;
		}
		else if(entrn == 1)   //��һҳ
		{
			if(i == 0)
			{
				if(SYSSet.NetState == DISP_NET)
				{
					ppage =  Disp_NetPageDispos();
					DisplayCommonMenu(ppage,NULL);
				}
				else
				{
					DisplayCommonMenu(&HYMenu3,NULL);
				}
				return;
			}
			else
			{
				i--;
			}
		}
		else
		{
			i++;  //��һҳ
		}
		if((PriceSet.TimeQuantumNum == 0) || (PriceSet.TimeQuantumNum > 48))
		{

			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			return;
		}
		num = (PriceSet.TimeQuantumNum - 1) / 4;
		if(i > num)
		{

			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}
			return;
		}
		else if(i == num)
		{
			DisplayCommonMenu(&HYMenu82,NULL);		//û����һҳ
		}
		else
		{
			DisplayCommonMenu(&HYMenu83,NULL);		//����һҳ
		}
	}

	starthour[0+i*4] = PriceSet.StartTime[0+i*4] / 60;
	startminute[0+i*4] = PriceSet.StartTime[0+i*4] % 60;
	starthour[1+i*4] = PriceSet.StartTime[1+i*4] / 60;
	startminute[1+i*4] = PriceSet.StartTime[1+i*4] % 60;
	starthour[2+i*4] = PriceSet.StartTime[2+i*4] / 60;
	startminute[2+i*4] = PriceSet.StartTime[2+i*4] % 60;
	starthour[3+i*4] = PriceSet.StartTime[3+i*4] / 60;
	startminute[3+i*4] = PriceSet.StartTime[3+i*4] % 60;

	stophour[3+i*4] = PriceSet.EndTime[3+i*4] / 60;
	stopminute[3+i*4] = PriceSet.EndTime[3+i*4] % 60;

	if(GetCurMenu() !=  &HYMenu48)   //�������ý��洦��
	{
		PrintNum16uVariable(DIS_ADD(82,0),starthour[0+i*4]);
		PrintNum16uVariable(DIS_ADD(82,1),startminute[0+i*4]);
		PrintNum16uVariable(DIS_ADD(82,2),starthour[1+i*4]);
		PrintNum16uVariable(DIS_ADD(82,3),startminute[1+i*4]);
		PrintNum16uVariable(DIS_ADD(82,4) ,starthour[2+i*4]);
		PrintNum16uVariable(DIS_ADD(82,5),startminute[2+i*4]);
		PrintNum16uVariable(DIS_ADD(82,6),starthour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(82,7),startminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(82,8),stophour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(82,9),stopminute[3+i*4]);


		PrintNum16uVariable(DIS_ADD(82,10),PriceSet.Price[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,11),PriceSet.Price[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,12),PriceSet.Price[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,13),PriceSet.Price[3+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,14),PriceSet.ServeFee[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,15),PriceSet.ServeFee[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,16),PriceSet.ServeFee[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(82,17),PriceSet.ServeFee[3+i*4]/1000);

		PrintNum16uVariable(DIS_ADD(83,0),starthour[0+i*4]);
		PrintNum16uVariable(DIS_ADD(83,1),startminute[0+i*4]);
		PrintNum16uVariable(DIS_ADD(83,2),starthour[1+i*4]);
		PrintNum16uVariable(DIS_ADD(83,3),startminute[1+i*4]);
		PrintNum16uVariable(DIS_ADD(83,4) ,starthour[2+i*4]);
		PrintNum16uVariable(DIS_ADD(83,5),startminute[2+i*4]);
		PrintNum16uVariable(DIS_ADD(83,6),starthour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(83,7),startminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(83,8),stophour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(83,9),stopminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(83,10),PriceSet.Price[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,11),PriceSet.Price[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,12),PriceSet.Price[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,13),PriceSet.Price[3+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,14),PriceSet.ServeFee[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,15),PriceSet.ServeFee[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,16),PriceSet.ServeFee[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(83,17),PriceSet.ServeFee[3+i*4]/1000);
	}
	else
	{
		//���ý���
		PrintNum16uVariable(DIS_ADD(48,0),starthour[0+i*4]);
		PrintNum16uVariable(DIS_ADD(48,1),startminute[0+i*4]);
		PrintNum16uVariable(DIS_ADD(48,2),starthour[1+i*4]);
		PrintNum16uVariable(DIS_ADD(48,3),startminute[1+i*4]);
		PrintNum16uVariable(DIS_ADD(48,4) ,starthour[2+i*4]);
		PrintNum16uVariable(DIS_ADD(48,5),startminute[2+i*4]);
		PrintNum16uVariable(DIS_ADD(48,6),starthour[3+i*4]);
		PrintNum16uVariable(DIS_ADD(48,7),startminute[3+i*4]);

		PrintNum16uVariable(DIS_ADD(48,8),PriceSet.Price[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,9),PriceSet.Price[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,10),PriceSet.Price[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,11),PriceSet.Price[3+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,12),PriceSet.ServeFee[0+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,13),PriceSet.ServeFee[1+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,14),PriceSet.ServeFee[2+i*4]/1000);
		PrintNum16uVariable(DIS_ADD(48,15),PriceSet.ServeFee[3+i*4]/1000);
	}
	//PrintNum16uVariable(ADDR_MENU81_PRC + 13,PriceSet.ViolaFee);
}

/*****************************************************************************
* Function     : Display_ShowSet1
* Description  : ��ʼ����ʾ����1 		(flashûд����ʱδ������)
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowSet1(void)
{
	_SYS_MENU52_SET1 sys_set1;


	sys_set1.MacMaxVol = (((PresetValue.VolMaxOutput/10) & 0x00ff) << 8) | (((PresetValue.VolMaxOutput/10) & 0xff00) >> 8);
	sys_set1.MacMinVol = (((PresetValue.VolMinOutput/10) & 0x00ff) << 8) | (((PresetValue.VolMinOutput/10) & 0xff00) >> 8);
	sys_set1.MaxOutCur = ((PresetValue.CurMaxOutput & 0x00ff) << 8) | ((PresetValue.CurMaxOutput & 0xff00) >> 8);
	sys_set1.OutDcCur = ((PresetValue.Therold_Value.OverOutCurTherold & 0x00ff) << 8) | ( (PresetValue.Therold_Value.OverOutCurTherold & 0xff00) >> 8);
	sys_set1.OutDcVol = ((PresetValue.Therold_Value.OverOutVolTherold & 0x00ff) << 8) | ((PresetValue.Therold_Value.OverOutVolTherold & 0xff00) >> 8);
	sys_set1.UseModule = (PresetValue.PowerModelNumber << 8);
	sys_set1.UseGun = (SYSSet.SysSetNum.UseGun << 8);
	sys_set1.SOC = (SYSSet.SOCthreshold << 8);
	memcpy(sys_set1.DivNum,SYSSet.SysSetNum.DivNum,MIN(sizeof(sys_set1.DivNum),sizeof(SYSSet.SysSetNum.DivNum)) );
	memcpy(sys_set1.ProjectNum ,SYSSet.SysSetNum.ProjectNum,MIN(sizeof(sys_set1.ProjectNum),sizeof(SYSSet.SysSetNum.ProjectNum)) );
	PrintStr(ADDR_MENU52_CFG,(INT8U *)&sys_set1,sizeof(_SYS_MENU52_SET1));

}

/*****************************************************************************
* Function     : Display_ShowSet2
* Description  : ��ʼ����ʾ����2   		(flashûд����ʱδ������)
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowSet2(void)
{
	PrintNum16uVariable(ADDR_MENU53_CFG,PresetValue.Therold_Value.OverACVolTherold);
	PrintNum16uVariable(ADDR_MENU53_CFG + 1,PresetValue.Therold_Value.UnderACVolTherold);
	PrintStr(ADDR_MENU53_CFG + 2,SYSSet.MeterAdd[GUN_A],12);
	PrintStr(ADDR_MENU53_CFG + 8, SYSSet.MeterAdd[GUN_B],12);
}



/*****************************************************************************
* Function     : Display_ShowSet2
* Description  : ��ʼ����ʾ����3
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowSet3(void)
{
//	SHOW_NET_DJ,
//	SHOW_NET_WL,
//	SHOW_LOCK_CLOSE,
//	SHOW_LOCK_OPEN,
//	SHOW_MODULE_YL_20KWH,
//	SHOW_MODULE_YL_20KW,
//	SHOW_MODULE_SH,
//	SHOW_MODULE_YL_15KWH,
//	SHOW_MODULE_YL_15KW,
	if(SYSSet.NetState == 0)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,0),SHOW_NET_DJ);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,0),SHOW_NET_WL);
	}
	if(SYSSet.LockState == 0)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,10),SHOW_LOCK_CLOSE);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,10),SHOW_LOCK_OPEN);
	}

	if(SYSSet.CurModule == 0)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_20KWH);
	}
	else if(SYSSet.CurModule == 1)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_20KW);
	}
	else if(SYSSet.CurModule == 2)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_SH);
	}
	else if(SYSSet.CurModule == 3)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_15KWH);
	}
	else if (SYSSet.CurModule == 4)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_15KW);
	}
	else if (SYSSet.CurModule == 5)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_TH_30KW);
	}
	else if (SYSSet.CurModule == 6)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_TH_40KW);
	}
	else if (SYSSet.CurModule == 9)
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_20KW_1000V);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu57.FrameID,20),SHOW_MODULE_YL_30KW);
	}
}


/*****************************************************************************
* Function     : Display_ShowNet
* Description  : ��ʾ����Э��ѡ��
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-7-31
*****************************************************************************/
void Display_ShowNet(void)
{
	INT16U port;
	INT16U NetNum;
	INT16U IP[4];
	INT8U i;
	INT8U buf[12];
	if(SYSSet.NetYXSelct == 0)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_HY);
	}
	else if(SYSSet.NetYXSelct == 1)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YKC);
	}
	else if(SYSSet.NetYXSelct == 2)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_AP);
	}
	else if(SYSSet.NetYXSelct == 3)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_XJ);
	}
	else if(SYSSet.NetYXSelct == 4)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL1);
	}
	else if(SYSSet.NetYXSelct == 5)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL2);
	}
	else if(SYSSet.NetYXSelct == 6)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL3);
	}
	else if(SYSSet.NetYXSelct == 7)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_YL4);
	}
	else if(SYSSet.NetYXSelct == 8)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_66);
	}
	else if(SYSSet.NetYXSelct == 9)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_JG);
	}
	else if(SYSSet.NetYXSelct == 10)
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_TT);
	}
	else
	{
		Dis_Show(DIS_ADD(HYMenu10.FrameID,0),SHOW_XY_ZSH);
	}
	port = (SYSSet.Port & 0x00ff) << 8 | (SYSSet.Port & 0xff00) >> 8;
	NetNum = (SYSSet.NetNum & 0x00ff) << 8 | (SYSSet.NetNum & 0xff00) >> 8;
	for(i = 0; i < 4; i++)
	{
		IP[i] = (SYSSet.IP[i] & 0x00ff) << 8 | 0;
	}
	memcpy(buf,&port,sizeof(port));
	memcpy(&buf[2],IP,8);
	memcpy(&buf[10],&NetNum,sizeof(NetNum));
	PrintStr(DIS_ADD(HYMenu10.FrameID,0x11),buf ,sizeof(buf));
}

/*****************************************************************************
* Function     : Display_ParaInit
* Description  : ������ʼ��
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018-6-14
*****************************************************************************/
static void Display_ParaInit(void)
{
	INT8U MeterAdd[12] = {'a','a','a','a','a','a','a','a','a','a','a','a'};
	//INT8U MeterAdd[12] = {'0','0','0','0','0','0','0','0','0','0','0','1'};
	INT8U MeterAddA[12] = {'0','0','0','0','0','0','0','0','0','0','0','1'};
	INT8U MeterAddB[12] = {'0','0','0','0','0','0','0','0','0','0','0','2'};
	INT8U num = 0xFF;
	_FLASH_OPERATION  FlashOper;
	//��ʼ״̬�µ��Aǹ����BǹӦ���ڳ�緽ʽѡ�����
	DispControl.ClickGunNextPage[GUN_A]  = &HYMenu22;
	DispControl.ClickGunNextPage[GUN_B]  = &HYMenu22;
	DispControl.CurUserGun = GUN_A;		//Ĭ��ʹ��Aǹ
	DispControl.GurLockGun = GUN_UNDEFIN;	//��ǰ��������Ϊδ����
	DispControl.CardOver[GUN_A] = STATE_UNOVER;
	DispControl.StopOver[GUN_A] = STATE_UNOVER;
	DispControl.CardOver[GUN_B] = STATE_UNOVER;
	DispControl.StopOver[GUN_B] = STATE_UNOVER;
	DispControl.StopOverFrame[GUN_A] = STATE_UNOVER;
	DispControl.StopOverFrame[GUN_B] = STATE_UNOVER;
	DispControl.SendStartCharge[GUN_A] = FALSE;
	DispControl.SendStartCharge[GUN_B] = FALSE;
	DispControl.CountDown = 0;			//��ʼ������ʱΪ0������Ҫ�л�
	SetCurMenu(&HYMenu3);					//���õ�ǰ����Ϊ�����棨�����濪ʼ�а�ť��
	memset(ChargeInfo,0,(sizeof(_CHARGE_INFO) * GUN_MAX) );
	//��ȡ����������Ϣ
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}

#if(USER_GUN == USER_SINGLE_GUN)
	SYSSet.SysSetNum.UseGun = 1;  //
#else
	SYSSet.SysSetNum.UseGun = 2;  //
#endif

	if((SYSSet.NetYXSelct ==0xFF)||(SYSSet.NetYXSelct >= XY_MAX))
	{
		SYSSet.NetYXSelct = XY_HY;  //
		memcpy(SYSSet.IP,NetConfigInfo[SYSSet.NetYXSelct].IP,sizeof(SYSSet.IP));
		SYSSet.Port = NetConfigInfo[SYSSet.NetYXSelct].port;
		SYSSet.NetNum = 1;
	}

	//������������2��
	if((SYSSet.NetNum ==0xFF)||(SYSSet.NetNum > 2))
	{
		SYSSet.NetNum = 1;
	}

	if((SYSSet.SOCthreshold ==0xFF)||(SYSSet.SOCthreshold  == 0))
	{
		//SOC��ֵ
		SYSSet.SOCthreshold = 98;  //
		FlashOper.DataID = PARA_CFGALL_ID;
		FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
		FlashOper.ptr = (INT8U *)&SYSSet;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("Read SYS set err");
		}
	}

	if(SYSSet.MeterAdd[GUN_A][0] == 0xff)
	{
#if(USER_GUN == USER_SINGLE_GUN)
		memcpy(SYSSet.MeterAdd[GUN_A],MeterAdd,12);
#else
		memcpy(SYSSet.MeterAdd[GUN_A],MeterAddA,12);
		memcpy(SYSSet.MeterAdd[GUN_B],MeterAddB,12);
#endif
	}
	SYSSet.GunTemp = 0;
	if((SYSSet.NetState != DISP_NET) && (SYSSet.NetState != DISP_CARD))
	{
		SYSSet.NetState = DISP_CARD;  //����
	}
	if(SYSSet.LockState == 0xff)
	{
		SYSSet.LockState = 1;  //����
	}
	if(SYSSet.CurModule == 0xff)
	{
		SYSSet.CurModule = 0;  //�����㹦��
	}
	FlashOper.DataID = PARA_CFGALL_ID;
	FlashOper.Len = MIN(PARA_CFGALL_FLLEN,sizeof(_SYS_SET) );
	FlashOper.ptr = (INT8U *)&SYSSet;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}

	//��ȡ����
	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = MIN(PARA_PRICALL_FLLEN,sizeof(PriceSet) );
	FlashOper.ptr = (INT8U *)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}





	memset((INT8U*)&RecodeControl,0,sizeof(_RECODE_CONTROL));
	//��ȡ���׼�¼����
	FlashOper.DataID = PARA_1200_ID;
	FlashOper.Len = PARA_1200_FLLEN;
	FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read SYS set err");
	}



	if(RecodeControl.RecodeCurNum == 0xffffffff)      //20220721  ��Ҫ�Ƿ�ֹ�洢�����ϵ綪ʧ
	{
		OSTimeDly(SYS_DELAY_100ms);
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("Read SYS set err");
		}
	}


	if(RecodeControl.RecodeCurNum == 0xffffffff)
	{
		RecodeControl.RecodeCurNum = 0;
		FlashOper.DataID = PARA_1200_ID;
		FlashOper.Len = PARA_1200_FLLEN;
		FlashOper.ptr = (INT8U *)&RecodeControl.RecodeCurNum;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("Read SYS set err");
		}
	}

	//��ȡ����״̬�����߽��׼�¼����
	num = APP_GetNetOFFLineRecodeNum();
	if(num > 100)
	{
		APP_SetNetOFFLineRecodeNum(0);
	}


	FlashOper.DataID = PARA_CARDWHITEL_ID;
	FlashOper.Len = PARA_CARDWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if((FlashCardVinWLBuf[0] == 0xff) && (FlashCardVinWLBuf[1] == 0xff) && (FlashCardVinWLBuf[2] == 0xff) && (FlashCardVinWLBuf[3] == 0xff) \
	        &&(FlashCardVinWLBuf[4] == 0xff) && (FlashCardVinWLBuf[5] == 0xff) && (FlashCardVinWLBuf[6] == 0xff) && (FlashCardVinWLBuf[7] == 0xff))
	{
		FlashCardVinWLBuf[0] = 0;  //������
		FlashOper.DataID = PARA_CARDWHITEL_ID;
		FlashOper.Len = PARA_CARDWHITEL_FLLEN;
		FlashOper.ptr = FlashCardVinWLBuf;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
	}


	FlashOper.DataID = PARA_VINWHITEL_ID;
	FlashOper.Len = PARA_VINWHITEL_FLLEN;
	FlashOper.ptr = FlashCardVinWLBuf;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if((FlashCardVinWLBuf[0] == 0xff) && (FlashCardVinWLBuf[1] == 0xff) && (FlashCardVinWLBuf[2] == 0xff) && (FlashCardVinWLBuf[3] == 0xff) \
	        &&(FlashCardVinWLBuf[4] == 0xff) && (FlashCardVinWLBuf[5] == 0xff) && (FlashCardVinWLBuf[6] == 0xff) && (FlashCardVinWLBuf[7] == 0xff))
	{
		FlashCardVinWLBuf[0] = 0;   //VIN����
		FlashOper.DataID = PARA_VINWHITEL_ID;
		FlashOper.Len = PARA_VINWHITEL_FLLEN;
		FlashOper.ptr = FlashCardVinWLBuf;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
	}

	FlashOper.DataID = PARA_PRICALL_ID;
	FlashOper.Len = PARA_PRICALL_FLLEN;
	FlashOper.ptr = (INT8U*)&PriceSet;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);
	if((PriceSet.StartTime[0] == 0xff) || (PriceSet.TimeQuantumNum == 0xff))
	{
		memset(&PriceSet,0,sizeof(PriceSet));
		PriceSet.StartTime[0] = 0;
		PriceSet.EndTime[0] = 0;
		PriceSet.Price[0] = 100000;
		PriceSet.CurTimeQuantum[0] = TIME_QUANTUM_J;
		PriceSet.ServeFee[0] = 0;
		PriceSet.TimeQuantumNum = 1;

		FlashOper.DataID = PARA_PRICALL_ID;
		FlashOper.Len = PARA_PRICALL_FLLEN;
		FlashOper.ptr = (INT8U*)&PriceSet;
		FlashOper.RWChoose = FLASH_ORDER_WRITE;
		APP_FlashOperation(&FlashOper);
	}

	if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
	{
		//�ƿ��  ��������޸ķ��ʣ�Aǹ�ķ��ʿ��ܺ�Bǹ�ķ��ʲ�һ��
		memcpy(&A_PriceSet,&PriceSet,sizeof(PriceSet));
		memcpy(&B_PriceSet,&PriceSet,sizeof(PriceSet));
	}



	//��ȡ"CCU"������Ϣ
	FlashOper.DataID = PARA_PRESETVOL_ID;
	FlashOper.Len = sizeof(PresetValue);
	FlashOper.ptr = (INT8U*)&PresetValue;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);

	if((PresetValue.Therold_Value.OverACVolTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.OverACVolTherold==0))
	{
		//���������ѹ��ֵ
		PresetValue.Therold_Value.OverACVolTherold=AC_POWER_OVER_VAL;
	}
	if((PresetValue.Therold_Value.UnderACVolTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.UnderACVolTherold==0))
	{
		//��������Ƿѹ��ֵ
		PresetValue.Therold_Value.UnderACVolTherold=AC_POWER_UNDER_VAL;
	}
	if((PresetValue.Therold_Value.OverOutVolTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.OverOutVolTherold==0))
	{
		//�����ѹ��ֵ
		PresetValue.Therold_Value.OverOutVolTherold=THEROLD_VOL_OUTPUT;
	}
	if((PresetValue.Therold_Value.OverOutCurTherold==0xFFFF)||\
	        (PresetValue.Therold_Value.OverOutCurTherold==0))
	{
		//���������ֵ
		PresetValue.Therold_Value.OverOutCurTherold=THEROLD_CUR_OUTPUT;
	}
	//������ֵ�������Ե�ʱ������ã�ƽʱʹ�þ�����Ϊ���ֵ   20220118 Ҷ
	PresetValue.Therold_Value.OverOutCurTherold=THEROLD_CUR_OUTPUT;

	if((PresetValue.PowerModelNumber==0xFF)||(PresetValue.PowerModelNumber==0))
	{
		//ģ������
		PresetValue.PowerModelNumber=MODULE_MAX_NUM;
	}
	if((PresetValue.VolMaxOutput==0xFFFF)||(PresetValue.VolMaxOutput==0))
	{
		//��������ѹ
		PresetValue.VolMaxOutput=MAX_VOL_OUTPUT;
	}
	if((PresetValue.CurMaxOutput==0xFFFF)||(PresetValue.CurMaxOutput==0))
	{
		//����������
		PresetValue.CurMaxOutput=MAX_CUR_OUTPUT;
	}
	if((PresetValue.VolMinOutput==0xFFFF)||(PresetValue.VolMinOutput > 2000))
	{
		//��С�����ѹΪ200V,����һ��
		PresetValue.VolMinOutput=MIN_VOL_OUTPUT;
	}

	if((PresetValue.ChargeType==0xFF)||(PresetValue.ChargeType==0))
	{
		//��緽ʽ
		PresetValue.ChargeType=0x01;
	}
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);


	//��Ե���У׼ֵ  �������Ƿǽ���оƬ
	FlashOper.DataID = PARA_JUST_ID;
	FlashOper.Len = PARA_JUST_FLLEN;
	FlashOper.ptr = (INT8U*)ADCJustInfo;
	FlashOper.RWChoose = FLASH_ORDER_READ;
	APP_FlashOperation(&FlashOper);


#if(USER_importand == 1)
	//�ǽ���оƬ
	ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1387;
	ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1834;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
	ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
	ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;
	ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1249;
	ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1788;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
	ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
	ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;
	ADCJustInfo[GUN_A].JYVolKB.line_X1 = 670;
	ADCJustInfo[GUN_A].JYVolKB.line_X2 = 1023;
	ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
	ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;

	ADCJustInfo[GUN_B].BatVoltKB.line_X1 = 1387;
	ADCJustInfo[GUN_B].BatVoltKB.line_X2 =1834;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y2 =50000;
	ADCJustInfo[GUN_B].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_Y2-ADCJustInfo[GUN_B].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_X2-ADCJustInfo[GUN_B].BatVoltKB.line_X1);
	ADCJustInfo[GUN_B].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_X2;
	ADCJustInfo[GUN_B].HeadOutKB.line_X1 = 1249;
	ADCJustInfo[GUN_B].HeadOutKB.line_X2 =1788;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y2 =600;
	ADCJustInfo[GUN_B].HeadOutKB.line_K =  (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_Y2-ADCJustInfo[GUN_B].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_X2-ADCJustInfo[GUN_B].HeadOutKB.line_X1);
	ADCJustInfo[GUN_B].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_X2;
	ADCJustInfo[GUN_B].JYVolKB.line_X1 = 670;
	ADCJustInfo[GUN_B].JYVolKB.line_X2 =1023;
	ADCJustInfo[GUN_B].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_B].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_B].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_Y2-ADCJustInfo[GUN_B].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_X2-ADCJustInfo[GUN_B].JYVolKB.line_X1);
	ADCJustInfo[GUN_B].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_B].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_B].JYVolKB.line_X2;
	ADCJustInfo[GUN_A].T1_0 = 1700; //0
	ADCJustInfo[GUN_A].T1_120 = 2215; //120
	ADCJustInfo[GUN_A].T2_0 = 1700;
	ADCJustInfo[GUN_A].T2_120 = 2215;
	ADCJustInfo[GUN_B].T1_0 = 1700;
	ADCJustInfo[GUN_B].T1_120 = 2215;
	ADCJustInfo[GUN_B].T2_0 = 1700;
	ADCJustInfo[GUN_B].T2_120 = 2215;
#else
	//=====����оƬ�ͺ�1200B
	ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1225;  //У׼Aǹ������ѹ
	ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1675;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
	ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
	ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;

	ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1065;  //У׼Aǹ�ߵ�ѹ
	ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1605;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
	ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
	ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;

	ADCJustInfo[GUN_A].JYVolKB.line_X1 = 517;  //У׼��Ե����ѹ
	ADCJustInfo[GUN_A].JYVolKB.line_X2 = 873;
	ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
	ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;

	ADCJustInfo[GUN_B].BatVoltKB.line_X1 = 1225;
	ADCJustInfo[GUN_B].BatVoltKB.line_X2 =1675;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y1 = 30000;
	ADCJustInfo[GUN_B].BatVoltKB.line_Y2 =50000;
	ADCJustInfo[GUN_B].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_Y2-ADCJustInfo[GUN_B].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_X2-ADCJustInfo[GUN_B].BatVoltKB.line_X1);
	ADCJustInfo[GUN_B].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_X2;
	ADCJustInfo[GUN_B].HeadOutKB.line_X1 = 1065;
	ADCJustInfo[GUN_B].HeadOutKB.line_X2 =1605;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y1 = 400;
	ADCJustInfo[GUN_B].HeadOutKB.line_Y2 =600;
	ADCJustInfo[GUN_B].HeadOutKB.line_K =  (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_Y2-ADCJustInfo[GUN_B].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_X2-ADCJustInfo[GUN_B].HeadOutKB.line_X1);
	ADCJustInfo[GUN_B].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_X2;
	ADCJustInfo[GUN_B].JYVolKB.line_X1 = 517;
	ADCJustInfo[GUN_B].JYVolKB.line_X2 =873;
	ADCJustInfo[GUN_B].JYVolKB.line_Y1 = 15000;
	ADCJustInfo[GUN_B].JYVolKB.line_Y2 = 25000;
	ADCJustInfo[GUN_B].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_Y2-ADCJustInfo[GUN_B].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_X2-ADCJustInfo[GUN_B].JYVolKB.line_X1);
	ADCJustInfo[GUN_B].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_B].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_B].JYVolKB.line_X2;

	ADCJustInfo[GUN_A].T1_0 = 1700; //0
	ADCJustInfo[GUN_A].T1_120 = 2215; //120
	ADCJustInfo[GUN_A].T2_0 = 1700;
	ADCJustInfo[GUN_A].T2_120 = 2215;
	ADCJustInfo[GUN_B].T1_0 = 1700;
	ADCJustInfo[GUN_B].T1_120 = 2215;
	ADCJustInfo[GUN_B].T2_0 = 1700;
	ADCJustInfo[GUN_B].T2_120 = 2215;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	APP_FlashOperation(&FlashOper);
#endif



//		#if(USER_GUN == USER_SINGLE_GUN)
//		if(ADCJustInfo[GUN_A].BatVoltKB.line_X1 == 0xFFFFFFFF)      //20220721  ��Ҫ�Ƿ�ֹ�洢�����ϵ綪ʧ
//		{
////			//���ɵ�ǹ����2
//		ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1447;
//		ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1974;
//		ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
//		ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
//		ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
//		ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;
//		ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1289;
//		ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1939;
//		ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
//		ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
//		ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
//		ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;
//		ADCJustInfo[GUN_A].JYVolKB.line_X1 = 597;
//		ADCJustInfo[GUN_A].JYVolKB.line_X2 = 1019;
//		ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
//		ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
//		ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
//		ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;
//		ADCJustInfo[GUN_A].T1_0 = 2015; //0
//		ADCJustInfo[GUN_A].T1_120 = 2630; //120
//		ADCJustInfo[GUN_A].T2_0 = 2015;
//		ADCJustInfo[GUN_A].T2_120 = 2630;
//		ADCJustInfo[GUN_B].T1_0 = 2015;
//		ADCJustInfo[GUN_B].T1_120 = 2630;
//		ADCJustInfo[GUN_B].T2_0 = 2015;
//		ADCJustInfo[GUN_B].T2_120 = 2630;
//		FlashOper.RWChoose = FLASH_ORDER_WRITE;
//		APP_FlashOperation(&FlashOper);
//	}
//////
//#else

//	if(ADCJustInfo[GUN_A].BatVoltKB.line_X1 == 0xFFFFFFFF)      //20220721  ��Ҫ�Ƿ�ֹ�洢�����ϵ綪ʧ
//	{


	//���ڸ���оƬ----˫ǹ----ԭ��
//	ADCJustInfo[GUN_A].BatVoltKB.line_X1 = 1225;  //У׼Aǹ������ѹ
//	ADCJustInfo[GUN_A].BatVoltKB.line_X2 = 1675;
//	ADCJustInfo[GUN_A].BatVoltKB.line_Y1 = 30000;
//	ADCJustInfo[GUN_A].BatVoltKB.line_Y2 = 50000;
//	ADCJustInfo[GUN_A].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_Y2-ADCJustInfo[GUN_A].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].BatVoltKB.line_X2-ADCJustInfo[GUN_A].BatVoltKB.line_X1);
//	ADCJustInfo[GUN_A].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_A].BatVoltKB.line_X2;

//	ADCJustInfo[GUN_A].HeadOutKB.line_X1 = 1065;  //У׼Aǹ�ߵ�ѹ
//	ADCJustInfo[GUN_A].HeadOutKB.line_X2 = 1605;
//	ADCJustInfo[GUN_A].HeadOutKB.line_Y1 = 400;
//	ADCJustInfo[GUN_A].HeadOutKB.line_Y2 = 600;
//	ADCJustInfo[GUN_A].HeadOutKB.line_K = (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_Y2-ADCJustInfo[GUN_A].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].HeadOutKB.line_X2-ADCJustInfo[GUN_A].HeadOutKB.line_X1);
//	ADCJustInfo[GUN_A].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_A].HeadOutKB.line_X2;

//	ADCJustInfo[GUN_A].JYVolKB.line_X1 = 517;  //У׼��Ե����ѹ
//	ADCJustInfo[GUN_A].JYVolKB.line_X2 = 873;
//	ADCJustInfo[GUN_A].JYVolKB.line_Y1 = 15000;
//	ADCJustInfo[GUN_A].JYVolKB.line_Y2 = 25000;
//	ADCJustInfo[GUN_A].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_Y2-ADCJustInfo[GUN_A].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_A].JYVolKB.line_X2-ADCJustInfo[GUN_A].JYVolKB.line_X1);
//	ADCJustInfo[GUN_A].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_A].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_A].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_A].JYVolKB.line_X2;

//	ADCJustInfo[GUN_B].BatVoltKB.line_X1 = 1225;
//	ADCJustInfo[GUN_B].BatVoltKB.line_X2 =1675;
//	ADCJustInfo[GUN_B].BatVoltKB.line_Y1 = 30000;
//	ADCJustInfo[GUN_B].BatVoltKB.line_Y2 =50000;
//	ADCJustInfo[GUN_B].BatVoltKB.line_K = (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_Y2-ADCJustInfo[GUN_B].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].BatVoltKB.line_X2-ADCJustInfo[GUN_B].BatVoltKB.line_X1);
//	ADCJustInfo[GUN_B].BatVoltKB.line_B = (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_K* (FP32)ADCJustInfo[GUN_B].BatVoltKB.line_X2;
//	ADCJustInfo[GUN_B].HeadOutKB.line_X1 = 1065;
//	ADCJustInfo[GUN_B].HeadOutKB.line_X2 =1605;
//	ADCJustInfo[GUN_B].HeadOutKB.line_Y1 = 400;
//	ADCJustInfo[GUN_B].HeadOutKB.line_Y2 =600;
//	ADCJustInfo[GUN_B].HeadOutKB.line_K =  (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_Y2-ADCJustInfo[GUN_B].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].HeadOutKB.line_X2-ADCJustInfo[GUN_B].HeadOutKB.line_X1);
//	ADCJustInfo[GUN_B].HeadOutKB.line_B = (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_K* (FP32)ADCJustInfo[GUN_B].HeadOutKB.line_X2;
//	ADCJustInfo[GUN_B].JYVolKB.line_X1 = 517;
//	ADCJustInfo[GUN_B].JYVolKB.line_X2 =873;
//	ADCJustInfo[GUN_B].JYVolKB.line_Y1 = 15000;
//	ADCJustInfo[GUN_B].JYVolKB.line_Y2 = 25000;
//	ADCJustInfo[GUN_B].JYVolKB.line_K = (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_Y2-ADCJustInfo[GUN_B].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[GUN_B].JYVolKB.line_X2-ADCJustInfo[GUN_B].JYVolKB.line_X1);
//	ADCJustInfo[GUN_B].JYVolKB.line_B = (FP32)ADCJustInfo[GUN_B].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[GUN_B].JYVolKB.line_K* (FP32)ADCJustInfo[GUN_B].JYVolKB.line_X2;

//	ADCJustInfo[GUN_A].T1_0 = 1700; //0
//	ADCJustInfo[GUN_A].T1_120 = 2215; //120
//	ADCJustInfo[GUN_A].T2_0 = 1700;
//	ADCJustInfo[GUN_A].T2_120 = 2215;
//	ADCJustInfo[GUN_B].T1_0 = 1700;
//	ADCJustInfo[GUN_B].T1_120 = 2215;
//	ADCJustInfo[GUN_B].T2_0 = 1700;
//	ADCJustInfo[GUN_B].T2_120 = 2215;
//	FlashOper.RWChoose = FLASH_ORDER_WRITE;
//	APP_FlashOperation(&FlashOper);

//	}



	if(SYSSet.NetYXSelct == XY_66)
	{
		memset(&SYSSet.SysSetNum.DivNum[10],0,6);
		memset(NetConfigInfo[SYSSet.NetYXSelct].pIp,0,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp));
		memcpy(NetConfigInfo[SYSSet.NetYXSelct].pIp,"pile.coulomb-charging.com",strlen("pile.coulomb-charging.com"));
	}




	NetConfigInfo[SYSSet.NetYXSelct].NetNum =  SYSSet.NetNum;
	NetConfigInfo[SYSSet.NetYXSelct].port = SYSSet.Port;
	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",SYSSet.IP[0],SYSSet.IP[1],SYSSet.IP[2],SYSSet.IP[3]);

//

//	SYSSet.NetYXSelct = XY_YKC;
//	SYSSet.NetState = DISP_NET;  //����
//	SYSSet.NetYXSelct = XY_HY;
//	snprintf(NetConfigInfo[SYSSet.NetYXSelct].pIp,sizeof(NetConfigInfo[SYSSet.NetYXSelct].pIp),"%d.%d.%d.%d",116,62,125,35);
//	char * dev_num = "2000000000000000";
//	memcpy(SYSSet.SysSetNum.DivNum,dev_num,sizeof(SYSSet.SysSetNum.DivNum));
}
/*****************************************************************************
* Function     : CountDownDispose
* Description  : ����ʱ����
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static void CountDownDispose(INT32U time)
{
	static INT16U i = 0;

	if(++i >= (SYS_DELAY_1s/time) )    //1s �ӵ���ʱ-1
	{
		if(	DispControl.CountDown > 1) //ֱ������ʱ����1Ϊֹ
		{
			DispControl.CountDown--;
			//��ʾ����ʱ
			PrintNum16uVariable(ADDR_COUNTDOWN_TIME,DispControl.CountDown);
		}
		i = 0;
	}
}

/*****************************************************************************
* Function     : Disp_ShowRTC
* Description  : ��ʾRTC
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U Disp_ShowRTC(void)
{
	_BSPRTC_TIME CurRTC;

	if(GetCurTime(&CurRTC))           			//��ȡϵͳRTC
	{
		Dis_ShowTime(ADDR_TIME,CurRTC);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Disp_ShowRTC
* Description  : ��ʾRTC
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U Disp_ShowGunT(void)
{
	INT16U T1A,T2A,T1B,T2B;

//	#define ADDR_GUNA_T1			(0x1D8A)					//���н���ĵ���ʱ����һ����ַ
//#define ADDR_GUNA_T2			(0x1D8B)					//���н���ĵ���ʱ����һ����ַ
//#define ADDR_GUNB_T1			(0x1D8C)					//���н���ĵ���ʱ����һ����ַ
//#define ADDR_GUNB_T2			(0x1D8D)					//���н���ĵ���ʱ����һ����ַ
	if(SYSSet.GunTemp == 0)
	{
		T1A = GetRM_GunT1Temp(GUN_A);
		T2A = GetRM_GunT2Temp(GUN_A);
		PrintNum16uVariable(ADDR_GUNA_T1,T1A/100);
		PrintNum16uVariable(ADDR_GUNA_T2,T2A/100);
#if(USER_GUN != USER_SINGLE_GUN)
		T1B = GetRM_GunT1Temp(GUN_B);
		T2B = GetRM_GunT2Temp(GUN_B);
		PrintNum16uVariable(ADDR_GUNB_T1,T1B/100);
		PrintNum16uVariable(ADDR_GUNB_T2,T2B/100);
#endif
	}


	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_ChargeingDate
* Description  : ��ң�����ݿ����ͳ����Ϣ����������ʾ���ݵĽṹ���У�
				 ���ڳ���У���ÿ��5s���͸�������ʾ
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U DispShow_ChargeingDate(INT32U time)
{
	INT8U i = 0;
	static INT16U count[GUN_MAX];
	INT32U balance;
	USERINFO* pric_info;
	USERCARDINFO* card_info;

	for(i = 0; i < GUN_MAX; i++)
	{

		card_info = GetGunCardInfo((_GUN_NUM)i);
		pric_info = GetChargingInfo((_GUN_NUM) i);
		if(pric_info == NULL)
		{
			return FALSE;
		}
		ChargeingInfo[i].ChargeTime = (pric_info->ChargeTime & 0x00ff) << 8 | (pric_info->ChargeTime & 0xff00) >> 8;

		if(SYSSet.NetYXSelct == XY_66)
		{
			ChargeingInfo[i].ChargeMoney  = 0;		//66�Ǻ�̨������ʽ��涼��ʾ0
		}
		else
		{
			ChargeingInfo[i].ChargeMoney =( ( ((pric_info->TotalBill/100) & 0x000000ff) << 24) | ( ((pric_info->TotalBill/100) & 0x0000ff00) << 8) |\
			                                (((pric_info->TotalBill/100) & 0x00ff0000) >> 8) | ( ((pric_info->TotalBill/100) & 0xff000000) >> 24));
		}

		ChargeingInfo[i].ChargeEle = ( ((pric_info->TotalPower4/10) & 0x000000ff) << 24) | ( ((pric_info->TotalPower4/10) & 0x0000ff00) << 8) |\
		                             (((pric_info->TotalPower4/10) & 0x00ff0000) >> 8) | ( ((pric_info->TotalPower4/10) & 0xff000000) >> 24) ;
		//���������С10������ʾ���泤�Ȳ�����
		if(SYSSet.NetState == DISP_CARD)
		{
			ChargeingInfo[i].CardMoney = ( ((card_info->balance/10) & 0x000000ff) << 24) | (((card_info->balance/10) & 0x0000ff00) << 8) |\
			                             (((card_info->balance/10) & 0x00ff0000) >> 8) | ( ((card_info->balance/10) & 0xff000000) >> 24) ;
		}
		else
		{
			balance = APP_GetNetMoney(i);
			ChargeingInfo[i].CardMoney = ( ((balance/10) & 0x000000ff) << 24) | (((balance/10) & 0x0000ff00) << 8) |\
			                             (((balance/10) & 0x00ff0000) >> 8) | ( ((balance/10) & 0xff000000) >> 24) ;
		}
		ChargeingInfo[i].ChargeSOC = (BMS_BCS_Context[i].SOC & 0x00ff) << 8 | 0x00;
		ChargeingInfo[i].ChargeVol = (PowerModuleInfo[i].OutputInfo.Vol & 0x00ff) << 8 | (PowerModuleInfo[i].OutputInfo.Vol & 0xff00) >> 8;
		ChargeingInfo[i].ChargeCur = ( PowerModuleInfo[i].OutputInfo.Cur & 0x00ff) << 8 | ( PowerModuleInfo[i].OutputInfo.Cur  & 0xff00) >> 8;
		if(ChargeingInfo[i].ChargeCur == 0XA00F)  //4000
		{
			ChargeingInfo[i].ChargeCur  = 0; //˵��һ��ʼ����û�д�����
		}
		//��ط���һ��8��
		if(BMS_BRM_Context[i].BatteryType < 9)
		{
			ChargeingInfo[i].BatteryType = BMS_BRM_Context[i].BatteryType;		//�������
		}
		else
		{
			ChargeingInfo[i].BatteryType = 9;			//�������
		}

		if(APP_GetWorkState((_GUN_NUM)i) == WORK_CHARGE)							//�ڳ����ÿ��5s����
		{
			if(++count[i] >= (SYS_DELAY_5s/time) )
			{
				//��������
				if(i == GUN_A)
				{
					//��ȥ������ͣ��糵���͵�����ʾ
					PrintStr(DIS_ADD(HYMenu36.FrameID,0),(INT8U*)&ChargeingInfo[GUN_A],sizeof(_CHARGEING_INFO) - 1 );
					//��ʾ�������,�������ʹ�1��ʼ������ʾ����ַ��Ҫ-1
					// Dis_ShowStatus(DIS_ADD(HYMenu36.FrameID,15),(_SHOW_NUM)(ChargeingInfo[GUN_A].BatteryType + (INT8U)SHOW_BATTERY_TYPE1 -1),WHITE);    //��ʾ�������
				}
				else
				{
					PrintStr(DIS_ADD(HYMenu37.FrameID,0),(INT8U*)&ChargeingInfo[GUN_B] ,sizeof(_CHARGEING_INFO) - 1 );
					//��ʾ�������,�������ʹ�1��ʼ������ʾ����ַ��Ҫ-1
					// Dis_ShowStatus(DIS_ADD(HYMenu37.FrameID,15),(_SHOW_NUM)(ChargeingInfo[GUN_B].BatteryType + (INT8U)SHOW_BATTERY_TYPE1 -1),WHITE);    //��ʾ�������
				}
				count[i] = 0;
			}
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_CardDate
* Description  : ��ң�����ݿ����ͳ����Ϣ����������ʾ���ݵĽṹ���У�
				 ���ڳ���У���ÿ��5s���͸�������ʾ
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U DispShow_CardDate(INT32U time)
{
	INT8U i = 0;
	static INT16U count[GUN_MAX];

	for(i = 0; i < GUN_MAX; i++)
	{
		CardInfo[i].bmsneedvolt =  (BMS_BCL_Context[i].DemandVol & 0x00ff) << 8 |  (BMS_BCL_Context[i].DemandVol & 0xff00) >> 8;
		CardInfo[i].bmsneedcurr =  ( (4000-BMS_BCL_Context[i].DemandCur) & 0x00ff) << 8 | ( (4000-BMS_BCL_Context[i].DemandCur) & 0xff00) >> 8;
		CardInfo[i].remaindertime = (BMS_BCS_Context[i].RemainderTime & 0x00ff) << 8 |   (BMS_BCS_Context[i].RemainderTime & 0xff00) >> 8;
		CardInfo[i].bmsMaxVoltage = ((BMS_BCP_Context[i].MaxVoltage) & 0x00ff) << 8 |  ((BMS_BCP_Context[i].MaxVoltage )& 0xff00) >> 8;
		CardInfo[i].bmsMaxcurrent =  ( (4000-BMS_BCP_Context[i].MaxCurrent) & 0x00ff) << 8 | ( (4000-BMS_BCP_Context[i].MaxCurrent) & 0xff00) >> 8;
		CardInfo[i].MaxTemprature =  ( ( BMS_BCP_Context[i].MaxTemprature - 50) & 0x00ff) << 8  | 0x00;
		CardInfo[i].unitbatterymaxvol = (BMS_BCP_Context[i].UnitBatteryMaxVol & 0x00ff) << 8 |  (BMS_BCP_Context[i].UnitBatteryMaxVol & 0xff00) >> 8;
		CardInfo[i].chargevolmeasureval =  ((BMS_BCS_Context[i].ChargeVolMeasureVal/10) & 0x00ff) << 8 |  (BMS_BCS_Context[i].ChargeVolMeasureVal & 0xff00) >> 8;
		if(APP_GetWorkState((_GUN_NUM)i) == WORK_CHARGE)							//�ڳ����ÿ��5s����
		{
			if(++count[i] >= (SYS_DELAY_5s/time) )
			{
				//��������
				if(i == GUN_A)
				{
					PrintStr(DIS_ADD(HYMenu38.FrameID,0),(INT8U*)&CardInfo[GUN_A],sizeof(_CARD_INFO) );

				}
				else
				{
					PrintStr(DIS_ADD(HYMenu39.FrameID,0),(INT8U*)&CardInfo[GUN_B] ,sizeof(_CARD_INFO) );
				}
				count[i] = 0;
			}
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_EndChargeDate
* Description  : ���������ʾ����
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U DispShow_EndChargeDate(_GUN_NUM gun)
{
	INT8U state;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	EndChargeInfo[gun].StartYear = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Year) << 8) | 0;
	EndChargeInfo[gun].StartMouth = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Month) << 8) | 0;
	EndChargeInfo[gun].StartDay = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Day) << 8) | 0;
	EndChargeInfo[gun].StartHour = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Hour) << 8) | 0;
	EndChargeInfo[gun].StartMinute = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Minute) << 8) | 0;
	EndChargeInfo[gun].StartSecond = (BCDtoHEX(ChargeRecodeInfo[gun].StartTime.Second) << 8) | 0;
	EndChargeInfo[gun].StopYear = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Year) << 8) | 0;
	EndChargeInfo[gun].StopMouth = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Month) << 8) | 0;
	EndChargeInfo[gun].StopDay = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Day) << 8) | 0;
	EndChargeInfo[gun].StopHour = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Hour) << 8) | 0;
	EndChargeInfo[gun].StopMinute = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Minute) << 8) | 0;
	EndChargeInfo[gun].StopSecond = (BCDtoHEX(ChargeRecodeInfo[gun].EndTime.Second) << 8) | 0;
	EndChargeInfo[gun].TotPower = ( (ChargeRecodeInfo[gun].TotPower & 0x000000ff) << 24) | ( (ChargeRecodeInfo[gun].TotPower & 0x0000ff00) << 8) |\
	                              ((ChargeRecodeInfo[gun].TotPower & 0x00ff0000) >> 8) | ( (ChargeRecodeInfo[gun].TotPower & 0xff000000) >> 24) ;
	EndChargeInfo[gun].TotMoney = ( (ChargeRecodeInfo[gun].TotMoney & 0x000000ff) << 24) | ( (ChargeRecodeInfo[gun].TotMoney & 0x0000ff00) << 8) |\
	                              ( (ChargeRecodeInfo[gun].TotMoney & 0x00ff0000) >> 8) | ( (ChargeRecodeInfo[gun].TotMoney & 0xff000000) >> 24);
	EndChargeInfo[gun].BeforeCardBalance = ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0x000000ff) << 24) | ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0x0000ff00) << 8) |\
	                                       ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0x00ff0000) >> 8) | ( (ChargeRecodeInfo[gun].BeforeCardBalance & 0xff000000) >> 24);
	EndChargeInfo[gun].EndChargeReason = (INT8U)ChargeRecodeInfo[gun].StopChargeReason;

	state = GetStartFailType(gun);
	if(gun == GUN_A)
	{
		//��Ҫ��ȥֹͣԭ��ֹͣԭ�򵥶���ʾ
		PrintStr(DIS_ADD(HYMenu40.FrameID,0),(INT8U*)&EndChargeInfo[GUN_A],sizeof(_END_CHARGE_INFO) - 1 );
		if(HYMenu40.Menu_PrePage == &HYMenu32)  //����ʧ��ֱ����ת����
		{
			//��ʾ����ʧ��ԭ��
			Dis_ShowStatus(DIS_ADD(HYMenu40.FrameID,0x15),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
		}
		else
		{
			Dis_ShowStatus(DIS_ADD(HYMenu40.FrameID,0x15),(_SHOW_NUM)(EndChargeInfo[GUN_A].EndChargeReason+SHOW_STOP_ERR_NONE),RED);
		}

	}
	if(gun == GUN_B)
	{
		//��Ҫ��ȥֹͣԭ��ֹͣԭ�򵥶���ʾ
		PrintStr(DIS_ADD(HYMenu41.FrameID,0),(INT8U*)&EndChargeInfo[GUN_B],sizeof(_END_CHARGE_INFO) - 1 );
		if(HYMenu41.Menu_PrePage == &HYMenu32)  //����ʧ��ֱ����ת����
		{
			//��ʾ����ʧ��ԭ��
			Dis_ShowStatus(DIS_ADD(HYMenu41.FrameID,0x15),(_SHOW_NUM)(state+SHOW_STARTEND_SUCCESS),RED);
		}
		else
		{
			Dis_ShowStatus(DIS_ADD(HYMenu41.FrameID,0x15),(_SHOW_NUM)(EndChargeInfo[GUN_B].EndChargeReason+SHOW_STOP_ERR_NONE),RED);
		}
	}
	return FALSE;
}

/*****************************************************************************
* Function     : DispShow_NetState
* Description  : ��ʾ����״̬
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U DispShow_NetState(void)
{

	static INT8U state = 0,laststate = 0xff;   //�仯�˲�ִ��
	if(APP_GetNetState(0) == TRUE)
	{
		state = 0;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01E0,1);
		}
	}
	else
	{
		state = 1;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01E0,0);
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : DispShow_CSQState
* Description  : ��ʾ�ź�ǿ��
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U DispShow_CSQState(void)
{

	static INT8U state = 0,laststate = 0xff;   //�仯�˲�ִ��

	//CSQ 0~31 99
	//��5���ȼ�
	if((APP_GetCSQNum() == 0) || (APP_GetCSQNum() == 99) )
	{

		state = 0;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,0);
		}
	}
	else if(APP_GetCSQNum() < 5)
	{
		state = 1;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,1);
		}
	}
	else if(APP_GetCSQNum() < 20)
	{
		state = 2;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,2);
		}
	}
	else if(APP_GetCSQNum() < 25)
	{
		state = 3;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,3);
		}
	}
	else
	{
		state = 4;
		if(state != laststate)
		{
			laststate = state;
			PrintIcon(0x01F0,4);
		}
	}
	return TRUE;
}


extern _MSP_DI MSPIo;
/*****************************************************************************
* Function     : DispShow_State
* Description  : ��������ʾA/B ǹ״̬
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U DispShow_State(INT32U time)
{
	INT8U i = 0,num;
	static INT16U count[GUN_MAX];
	static INT8U cnt[GUN_MAX] = {0,0};
	_SHOW_NUM show_err[GUN_MAX];
	static INT8U chargeicon[GUN_MAX] = {0},lastchargeicon[GUN_MAX] = {1}; //��������ͼ���Ƿ���ʾ 0:����ʾ  1:��ʾ
	static INT32U failcount[GUN_MAX] = {0};
	static INT8U laststate[GUN_MAX] = {GUN_IDLE};
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	num = GUN_MAX;
	if((SYSSet.SysSetNum.UseGun <= GUN_MAX) && (SYSSet.SysSetNum.UseGun != 0) )
	{
		num = SYSSet.SysSetNum.UseGun;
	}
	for(i = 0; i < num; i++)
	{
		show_err[(_GUN_NUM)i] = SHOW_SYS_NULL;	//��ʼ������ʾ
		if((APP_GetErrState((_GUN_NUM)i) == 0x04) || (GetMeterStatus((_GUN_NUM)i) == _COM_FAILED)) 				//�й���
		{
			if(SYSSet.NetState == DISP_NET)
			{
				DispControl.NetGunState[(_GUN_NUM)i] = GUN_FAIL;
			}
			chargeicon[i] = 0;//����ʾ���ͼ��
			if (GetEmergencyState(i) == EMERGENCY_PRESSED)
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_EMERGENCY;  //��ͣ
			}
#if ENBLE_JG
			else if (BSP_MPLSState(BSP_DI_DOOR)) //�Ž�
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_MJ;  //
			}
			else if(BSP_MPLSState(BSP_DI_W) == 0)  //ˮ��
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_SJ;
			}
			else if(BSP_MPLSState(BSP_DI_QX) == 1)  //��б
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_QX;
			}
			else if(MSPIo.SWDC_A)  //Aǹֱ�ӽӴ���
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_ADC;
			}
			else if(MSPIo.SWDC_B)  //Bǹֱ�ӽӴ���
			{

				show_err[(_GUN_NUM)i] = SHOW_SYS_BDC;
			}
			else if(MSPIo.QDC)  //�Ž�
			{
				show_err[(_GUN_NUM)i] = SHOW_SYS_QJ;
			}
#endif
			else
			{
				//CCUд�Ĵ��룬��Щ���ϰ�ǹ����ϻ���Ϣ����Ҫ���Բ��ǹ
				//			SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 4, 0);
				//			SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_2, 5, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_3, 2, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 2, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 5, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 6, 0);
				//        	SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_4, 7, 0);
				//			SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_3, 7, 0);
				//	   		SetBcuRemoteSignalState((INT8U)REMOTE_SIGNALE_3, 5, 0);
				if(((BCURemoteSignal[(_GUN_NUM)i].state2.byte & 0x30) != 0) || ((BCURemoteSignal[(_GUN_NUM)i].state3.byte & 0xA4) != 0) \
				        || ((BCURemoteSignal[(_GUN_NUM)i].state4.byte& 0xE4) != 0) )
				{
					show_err[(_GUN_NUM)i] = SHOW_SYS_AGAINGUN;	//�ٴβ�ǹ
				}
				else
				{
					show_err[(_GUN_NUM)i] = SHOW_SYS_OTHERFAIL;	//ϵͳ����
				}
			}

		}
		else
		{
			if(SYSSet.NetState == DISP_NET)
			{
				if(DispControl.NetGunState[(_GUN_NUM)i] == GUN_FAIL)
				{
					DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
				}
			}
			if(APP_GetErrState((_GUN_NUM)i) == 0x01)  //���ڳ��
			{
				chargeicon[i] = 1;//��ʾ���ͼ��
				show_err[(_GUN_NUM)i] = SHOW_SYS_CHARGE;
				if(SYSSet.NetState == DISP_NET)
				{
					DispControl.NetGunState[(_GUN_NUM)i] = GUN_CHARGEING;
				}

			}
			else
			{
				chargeicon[i] = 0;//����ʾ���ͼ��
				if(SYSSet.NetState == DISP_NET)
				{
					//����ʧ��
					//����ǹ
					if(GetGunState(i) == GUN_DISCONNECTED)
					{
						DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
						show_err[(_GUN_NUM)i] = SHOW_SYS_GUN;   //ǹ���ӳ��ǹ
						DispControl.NetSSTState[(_GUN_NUM)i]  = NET_IDLE;
					}
					else
					{
						//��������
						if(DispControl.NetSSTState[(_GUN_NUM)i] == NET_STARTING)
						{
							failcount[i] = 0;
							DispControl.NetGunState[(_GUN_NUM)i] = GUN_STARTING;
							show_err[(_GUN_NUM)i] = SHOW_SYS_START;   //��������
						}
						else if(DispControl.NetSSTState[(_GUN_NUM)i] == NET_STOPING)
						{
							if(failcount[i]++ >= (SYS_DELAY_10s/time) )   //����10s��ʾֹͣ����Ϊ����
							{
								if(APP_GetErrState((_GUN_NUM)i) != 0x01)  //���ڳ����
								{
									failcount[i] = 0;
									DispControl.NetSSTState[(_GUN_NUM)i] = NET_IDLE;
									DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
									show_err[(_GUN_NUM)i] = SHOW_SYS_NET;  //��ʾɨ����
								}
							}
							else
							{
								DispControl.NetGunState[(_GUN_NUM)i] = GUN_STOPING;
								show_err[(_GUN_NUM)i] = SHOW_SYS_STOP;   //����ֹͣ
							}
						}
						else if(DispControl.NetSSTState[(_GUN_NUM)i] == NET_STARTFAIL)
						{
							if(failcount[i]++ >= (SYS_DELAY_5s/time) )   //����5s��ʾ����ʧ�ܺ���Ϊ����
							{
								failcount[i] = 0;
								DispControl.NetSSTState[(_GUN_NUM)i] = NET_IDLE;
								DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
								show_err[(_GUN_NUM)i] = SHOW_SYS_NET;  //��ʾɨ����
							}
							else
							{
								DispControl.NetGunState[(_GUN_NUM)i] = GUN_STARTFAIL;
								show_err[(_GUN_NUM)i] = SHOW_SYS_FAIL;   //����ʧ��
							}
						}
						else
						{
							failcount[i] = 0;
							DispControl.NetGunState[(_GUN_NUM)i] = GUN_IDLE;
							show_err[(_GUN_NUM)i] = SHOW_SYS_NET;  //��ʾɨ����
						}
					}
				}
				else
				{
					if(i == GUN_A)
					{
						if((DispControl.ClickGunNextPage[GUN_A] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_A] == &HYMenu33))
						{
							show_err[(_GUN_NUM)i] = SHOW_SYS_CARD; //��ˢ������
						}
					}

					if(i == GUN_B)
					{
						if((DispControl.ClickGunNextPage[GUN_B] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_B] == &HYMenu33))
						{
							show_err[(_GUN_NUM)i] = SHOW_SYS_CARD;	//��ˢ������
						}
					}
				}
			}
		}

		if(++count[i] >= (SYS_DELAY_1s/time) )
		{
			count[i] = 0;
			(cnt[i] == 0)?(cnt[i] = 1):(cnt[i] = 0);
#if ENBLE_JG

			if((show_err[(_GUN_NUM)i] == SHOW_SYS_EMERGENCY) || (show_err[(_GUN_NUM)i] == SHOW_SYS_MJ)|| \
			        (show_err[(_GUN_NUM)i] == SHOW_SYS_SJ) || (show_err[(_GUN_NUM)i] == SHOW_SYS_ADC)||\
			        (show_err[(_GUN_NUM)i] == SHOW_SYS_BDC) || (show_err[(_GUN_NUM)i] == SHOW_SYS_QJ) || (show_err[(_GUN_NUM)i] == SHOW_SYS_PCUTIMEOUT)   )
#else
			if((show_err[(_GUN_NUM)i] == SHOW_SYS_EMERGENCY) || (show_err[(_GUN_NUM)i] == SHOW_SYS_PCUTIMEOUT)   )
#endif
			{
				//ϵͳ������ʾ
				if(cnt[GUN_A] == 0)
				{
					Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x20),(_SHOW_NUM)show_err[(_GUN_NUM)i],RED);
				}
				else
				{
					Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x20),(_SHOW_NUM)show_err[(_GUN_NUM)i],BLACK);
				}
				Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x0),SHOW_SYS_NULL,RED);  //����ʾ
				Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x10),SHOW_SYS_NULL,RED);  //����ʾ
			}
			else
			{
				Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x20),SHOW_SYS_NULL,RED);  //����ʾ
				if(i == GUN_A)
				{
					if(cnt[GUN_A] == 0)
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0),(_SHOW_NUM)show_err[(_GUN_NUM)i],RED);
					}
					else
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0),(_SHOW_NUM)show_err[(_GUN_NUM)i],BLACK);
					}
				}
				if(i == GUN_B)
				{
					if(cnt[GUN_B] == 0)
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x10),(_SHOW_NUM)show_err[(_GUN_NUM)i],RED);
					}
					else
					{
						Dis_ShowErr(DIS_ADD(HYMenu3.FrameID,0x10),(_SHOW_NUM)show_err[(_GUN_NUM)i],BLACK);
					}
				}
			}
		}

		if(chargeicon[i] != lastchargeicon[i])
		{
			lastchargeicon[i] = chargeicon[i];
			if(GUN_A == i)
			{
				PrintIcon(0x01C0,chargeicon[i]);
			}
			else
			{
				PrintIcon(0x01D0,chargeicon[i]);
			}
		}
		DispShow_CSQState();
		DispShow_NetState();
	}

	if(((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC) ||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))  && (SYSSet.NetState == DISP_NET))
	{
		for(i = 0; i < GUN_MAX; i++)
		{
			if((laststate[i] != DispControl.NetGunState[(_GUN_NUM)i]) && \
			        ((DispControl.NetGunState[(_GUN_NUM)i] == GUN_IDLE) || (DispControl.NetGunState[(_GUN_NUM)i] == GUN_CHARGEING) \
			         ||(DispControl.NetGunState[(_GUN_NUM)i] == GUN_FAIL)))
			{
				SendMsg[i].MsgID = BSP_MSGID_DISP;
				SendMsg[i].DivNum = APP_SJDATA_QUERY;   //����ʵʱ���ݣ�״̬�����˸ı�
				SendMsg[i].DataLen = i;
				OSQPost(pevent, &SendMsg[i]);
				laststate[i] = DispControl.NetGunState[(_GUN_NUM)i];
			}
		}
	}
	return TRUE;
}

/****************************************************************************
* Function     : ����63������64��ʾ��¼
* Description  :
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static INT8U DispShow_Recode(_CHARGE_RECODE * precode,ST_Menu *pMenu)
{

	if((precode == NULL) || (pMenu == NULL) )
	{
		return FALSE;
	}
	memset(&RecodeMenu8_9Info,0,sizeof(RecodeMenu8_9Info));
	RecodeMenu8_9Info.BeforeCardBalance = ( (precode->BeforeCardBalance & 0x000000ff) << 24) | ( (precode->BeforeCardBalance & 0x0000ff00) << 8) |\
	                                      ((precode->BeforeCardBalance & 0x00ff0000) >> 8) | ( (precode->BeforeCardBalance & 0xff000000) >> 24) ;

	if(precode->BillingStatus == RECODE_SETTLEMENT) //��ʾ�Ѿ�����
	{
		Dis_ShowCopy(RecodeMenu8_9Info.BillingStatus,SHOW_CARD_UNLOCK);
	}
	else if(precode->BillingStatus == RECODE_REPLSETTLEMENT)
	{
		Dis_ShowCopy(RecodeMenu8_9Info.BillingStatus,SHOW_CARD_PERLLOCK);//�������
		//Dis_ShowStatus(DIS_ADD(HYMenu6.FrameID,4),SHOW_CARD_PERLLOCK,RED); //�������
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.BillingStatus,SHOW_CARD_LOCK);
	}
	memcpy(RecodeMenu8_9Info.TransNum,precode->TransNum,sizeof(precode->TransNum) );
	RecodeMenu8_9Info.TotPower =  ( (precode->TotPower & 0x000000ff) << 24) | ( (precode->TotPower & 0x0000ff00) << 8) |\
	                              ((precode->TotPower & 0x00ff0000) >> 8) | ( (precode->TotPower & 0xff000000) >> 24) ;
	if(precode->StopChargeReason >= 0x80)    //���λΪ1��ʾ����ʧ��ֹͣ�����λΪ0��ʾ�����ɹ�ֹͣ
	{
		Dis_ShowCopy(RecodeMenu8_9Info.StopChargeReason,(_SHOW_NUM)(precode->StopChargeReason - 0x80 + SHOW_STARTEND_SUCCESS));
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.StopChargeReason,(_SHOW_NUM)(precode->StopChargeReason + SHOW_STOP_ERR_NONE));
	}
	RecodeMenu8_9Info.CardNum =  ( (precode->CardNum & 0x000000ff) << 24) | ( (precode->CardNum & 0x0000ff00) << 8) |\
	                             ((precode->CardNum & 0x00ff0000) >> 8) | ( (precode->CardNum & 0xff000000) >> 24) ;

	if(precode->StartType == TYPE_START_DEVICE)
	{
		Dis_ShowCopy(RecodeMenu8_9Info.ChargeType,SHOW_START_CARD); //��ʾ��Ϊˢ������
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.ChargeType,SHOW_START_APP); //��ʾΪAPP����
	}
	//�˿ں�
	if(precode->Gun == GUN_A)
	{
		Dis_ShowCopy(RecodeMenu8_9Info.Gunnum,SHOW_GUN_A);
	}
	else
	{
		Dis_ShowCopy(RecodeMenu8_9Info.Gunnum,SHOW_GUN_B);
	}
	RecodeMenu8_9Info.TotMoney =  ( (precode->TotMoney & 0x000000ff) << 24) | ( (precode->TotMoney & 0x0000ff00) << 8) |\
	                              ((precode->TotMoney & 0x00ff0000) >> 8) | ( (precode->TotMoney & 0xff000000) >> 24) ;
	memcpy(RecodeMenu8_9Info.CarVin,precode->CarVin,sizeof(precode->CarVin) );
	memcpy(RecodeMenu8_9Info.TransNum,precode->TransNum,sizeof(RecodeMenu8_9Info.TransNum) );

	RecodeMenu8_9Info.StartYear = (BCDtoHEX(precode->StartTime.Year) << 8) | 0;
	RecodeMenu8_9Info.StartMonth = (BCDtoHEX(precode->StartTime.Month) << 8) | 0;
	RecodeMenu8_9Info.StartDay = (BCDtoHEX(precode->StartTime.Day) << 8) | 0;
	RecodeMenu8_9Info.StartHour = (BCDtoHEX(precode->StartTime.Hour) << 8) | 0;
	RecodeMenu8_9Info.StartMinute = (BCDtoHEX(precode->StartTime.Minute) << 8) | 0;
	RecodeMenu8_9Info.StartSecond = (BCDtoHEX(precode->StartTime.Second) << 8) | 0;
	RecodeMenu8_9Info.EndYear = (BCDtoHEX(precode->EndTime.Year) << 8) | 0;
	RecodeMenu8_9Info.EndMonth = (BCDtoHEX(precode->EndTime.Month) << 8) | 0;
	RecodeMenu8_9Info.EndDay = (BCDtoHEX(precode->EndTime.Day) << 8) | 0;
	RecodeMenu8_9Info.EndHour = (BCDtoHEX(precode->EndTime.Hour) << 8) | 0;
	RecodeMenu8_9Info.EndMinute = (BCDtoHEX(precode->EndTime.Minute) << 8) | 0;
	RecodeMenu8_9Info.EndSecond = (BCDtoHEX(precode->EndTime.Second) << 8) | 0;

	memcpy(RecodeMenu8_9Info.StopReason,precode->StopReason,sizeof(RecodeMenu8_9Info.StopReason));


	//����̫��һ���Է���������
	PrintStr(DIS_ADD(pMenu->FrameID,0),(INT8U*)&RecodeMenu8_9Info,34);
	PrintStr(DIS_ADD(pMenu->FrameID,17),(INT8U*)RecodeMenu8_9Info.StopChargeReason,34 );
	PrintStr(DIS_ADD(pMenu->FrameID,(34)),(INT8U*)RecodeMenu8_9Info.ChargeType,44);
	PrintStr(DIS_ADD(pMenu->FrameID,56),(INT8U*)RecodeMenu8_9Info.CarVin,42);
	PrintStr(DIS_ADD(pMenu->FrameID,77),(INT8U*)RecodeMenu8_9Info.StopReason,20);
	return TRUE;

}

/*****************************************************************************
* Function     : APP_ClearRecodeInfo
* Description  :�����¼��Ϣ
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U APP_ClearRecodeInfo(void)
{
	RecodeControl.UpReadRecodeNun =0;
	RecodeControl.CurReadRecodeNun = 0;
	RecodeControl.NextReadRecodeNun = 0;
	RecodeControl.CurNun = 0;
	return TRUE;
}

/*****************************************************************************
* Function     : APP_SelectCurChargeRecode
* Description  :��ѯ��ǰ���׼�¼������ʾ����һ�ν���
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U APP_SelectCurChargeRecode(void)
{
	INT32U i =0;
	//���׼�¼���1000��

	if(RecodeControl.RecodeCurNum == 0 )
	{
		DisplayCommonMenu(&HYMenu7,NULL);  //δ�ҵ�����¼
		return FALSE;
	}
	for(i = RecodeControl.RecodeCurNum; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum) ; i--)
	{
		//�����µ�һ����ʼ����
		APP_RWChargeRecode( RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.CurRecode);
		//��ѯ�����Ƿ�һ��
		//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.CurRecode.CardNum,6) == TRUE)
		{
			RecodeControl.CurReadRecodeNun = i;
			break;
		}
	}
	if(i ==  RECODE_DISPOSE2(RecodeControl.RecodeCurNum))
	{
		RecodeControl.CurReadRecodeNun = 0;
	}
	else
	{
		RecodeControl.CurNun++;  //ҳ��+1
		//��ѯ�Ƿ�����һ����¼������ҳ����ת��8���� ����9����
		for(i = RecodeControl.CurReadRecodeNun - 1; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum); i--)
		{
			APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.NextRecode);
			//��ѯ�����Ƿ�һ��
			//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.NextRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
			{
				RecodeControl.NextReadRecodeNun = i;
				break;
			}
		}
	}
	if(i ==  RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //��һҳ��
	{
		RecodeControl.NextReadRecodeNun = 0;
	}


	if(RecodeControl.CurReadRecodeNun  == 0)  //�����޼�¼
	{
		DisplayCommonMenu(&HYMenu7,NULL);  //δ���ҵ��˻���Ϣ
	}
	else if(RecodeControl.NextReadRecodeNun == 0) //ֻ��һ����¼
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
		DisplayCommonMenu(&HYMenu9,NULL);
	}
	else	//�ж���
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
		DisplayCommonMenu(&HYMenu8,NULL);
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_SelectNextChargeRecode
* Description  :��ѯ��һ�����׼�¼������ʾ
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U APP_SelectNextChargeRecode(void)
{
	INT32U i;
	if(RecodeControl.NextReadRecodeNun == 0)  //����һ��
	{
		return FALSE;
	}
	//��ǰ��ֵΪ��һ��
	RecodeControl.UpReadRecodeNun = RecodeControl.CurReadRecodeNun;
	memcpy(&RecodeControl.UpRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
	//��һ�����Ƹ���ǰ
	RecodeControl.CurReadRecodeNun = RecodeControl.NextReadRecodeNun;
	memcpy(&RecodeControl.CurRecode,&RecodeControl.NextRecode,sizeof(_CHARGE_RECODE));//��֮ǰ����һ����������ǰ��
	RecodeControl.NextReadRecodeNun = 0;
	//��ѯ�Ƿ�����һ����¼������ҳ����ת��63���� ����64����
	for(i = RecodeControl.CurReadRecodeNun - 1; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum); i--)
	{
		APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.NextRecode);
		//��ѯ�����Ƿ�һ��
		//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.NextRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
		{
			RecodeControl.NextReadRecodeNun = i;
			break;
		}
	}
	RecodeControl.CurNun++;  //ҳ��+1
	if(i == RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //��һҳ��
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
		DisplayCommonMenu(&HYMenu9,NULL);
		RecodeControl.NextReadRecodeNun = 0;
	}
	else	//�ж���
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
		DisplayCommonMenu(&HYMenu8,NULL);
	}
	return TRUE;
}


/*****************************************************************************
* Function     : APP_SelectNextNChargeRecode
* Description  :��ѯ��N�����׼�¼������ʾ
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U APP_SelectNextNChargeRecode(INT16U num)
{
	INT32U i;

	while(num--)
	{
		if(RecodeControl.NextReadRecodeNun == 0)  //����һ��
		{
			DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
			PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
			DisplayCommonMenu(&HYMenu9,NULL);
			RecodeControl.NextReadRecodeNun = 0;
			return TRUE;
		}
		//��ǰ��ֵΪ��һ��
		RecodeControl.UpReadRecodeNun = RecodeControl.CurReadRecodeNun;
		memcpy(&RecodeControl.UpRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
		//��һ�����Ƹ���ǰ
		RecodeControl.CurReadRecodeNun = RecodeControl.NextReadRecodeNun;
		memcpy(&RecodeControl.CurRecode,&RecodeControl.NextRecode,sizeof(_CHARGE_RECODE));//��֮ǰ����һ����������ǰ��
		RecodeControl.NextReadRecodeNun = 0;
		//��ѯ�Ƿ�����һ����¼������ҳ����ת��63���� ����64����
		for(i = RecodeControl.CurReadRecodeNun - 1; i > RECODE_DISPOSE2(RecodeControl.RecodeCurNum); i--)
		{
			APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.NextRecode);
			//��ѯ�����Ƿ�һ��
			//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.NextRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
			{
				RecodeControl.NextReadRecodeNun = i;
				break;
			}
		}
		RecodeControl.CurNun++;  //ҳ��+1
		if(i == RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //��һҳ��
		{
			RecodeControl.NextReadRecodeNun = 0;
		}
	}
	if(i == RECODE_DISPOSE2(RecodeControl.RecodeCurNum))  //��һҳ��
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu9);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
		DisplayCommonMenu(&HYMenu9,NULL);
		RecodeControl.NextReadRecodeNun = 0;
	}
	else	//�ж���
	{
		DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
		PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
		DisplayCommonMenu(&HYMenu8,NULL);
	}
	return TRUE;
}
/*****************************************************************************
* Function     : APP_SelectUpChargeRecode
* Description  :��ѯ��N�����׼�¼������ʾ
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U APP_SelectUpNChargeRecode(INT16U num)
{
	INT32U i;
	ST_Menu * ppage;
	while(num--)
	{
		if(RecodeControl.UpReadRecodeNun == 0)  //����һ��
		{
			APP_ClearRecodeInfo();		//�����¼��Ϣ
			if(SYSSet.NetState == DISP_NET)
			{
				ppage =  Disp_NetPageDispos();
				DisplayCommonMenu(ppage,NULL);
			}
			else
			{
				DisplayCommonMenu(&HYMenu3,NULL);
			}

			return TRUE;
		}
		//��ǰ����ֵ����һ��
		RecodeControl.NextReadRecodeNun = RecodeControl.CurReadRecodeNun;
		memcpy(&RecodeControl.NextRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
		//��һ����ֵ����ǰ��
		RecodeControl.CurReadRecodeNun = RecodeControl.UpReadRecodeNun;
		memcpy(&RecodeControl.CurRecode,&RecodeControl.UpRecode,sizeof(_CHARGE_RECODE) );
		//��ѯ��һ����¼
		for(i = RecodeControl.CurReadRecodeNun + 1; i <= RecodeControl.RecodeCurNum ; i++)
		{
			APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.UpRecode);
			//��ѯ�����Ƿ�һ��
			//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.UpRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
			{
				RecodeControl.UpReadRecodeNun = i;
				break;
			}
		}
		if(i > RecodeControl.RecodeCurNum) 		//��һҳ�޼�¼
		{
			RecodeControl.UpReadRecodeNun = 0;
		}
		RecodeControl.CurNun--;  //ҳ��-1
		if(RecodeControl.CurNun == 1)
		{
			break;
		}
	}
	//�����һҳ���϶�����һҳ�����Ե�ת��63����
	DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
	PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
	DisplayCommonMenu(&HYMenu8,NULL);
	return TRUE;
}


/*****************************************************************************
* Function     : APP_SelectUpChargeRecode
* Description  :��ѯ��һ�����׼�¼������ʾ
* Input        :
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U APP_SelectUpChargeRecode(void)
{
	INT32U i;
	if(RecodeControl.UpReadRecodeNun == 0)  //����һ��
	{
		APP_ClearRecodeInfo();		//�����¼��Ϣ
		DisplayCommonMenu(&HYMenu3,NULL);   //����������
		return TRUE;
	}
	//��ǰ����ֵ����һ��
	RecodeControl.NextReadRecodeNun = RecodeControl.CurReadRecodeNun;
	memcpy(&RecodeControl.NextRecode,&RecodeControl.CurRecode,sizeof(_CHARGE_RECODE) );
	//��һ����ֵ����ǰ��
	RecodeControl.CurReadRecodeNun = RecodeControl.UpReadRecodeNun;
	memcpy(&RecodeControl.CurRecode,&RecodeControl.UpRecode,sizeof(_CHARGE_RECODE) );
	//��ѯ��һ����¼
	for(i = RecodeControl.CurReadRecodeNun + 1; i <= RecodeControl.RecodeCurNum ; i++)
	{
		APP_RWChargeRecode(RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&RecodeControl.UpRecode);
		//��ѯ�����Ƿ�һ��
		//if(APP_ContrastCardNum(RecodeControl.CardNum,RecodeControl.UpRecode.CardNum,sizeof(RecodeControl.CardNum)) == TRUE)
		{
			RecodeControl.UpReadRecodeNun = i;
			break;
		}
	}
	if(i > RecodeControl.RecodeCurNum) 		//��һҳ�޼�¼
	{
		RecodeControl.UpReadRecodeNun = 0;
	}
	RecodeControl.CurNun--;  //ҳ��-1
	//�����һҳ���϶�����һҳ�����Ե�ת��63����
	DispShow_Recode(&RecodeControl.CurRecode,&HYMenu8);
	PrintNum16uVariable(ADDR_RECODE_NUM,RecodeControl.CurNun);  //��ʾҳ��
	DisplayCommonMenu(&HYMenu8,NULL);
	return TRUE;
}

/*****************************************************************************
* Function     : APP_GetBillInfo
* Description  : ��ȡǹ������Ϣ
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
INT8U *APP_GetBillInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return &Billbuf[gun][1];
}


/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  : �ڳ���������Դ����¼���׼�¼   				  billflag:0��ʾ�Ѿ�����			billflag��1��ʾδ����
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
INT8U WriterFmBill(_GUN_NUM gun,INT8U billflag)
{
//��һ���ֽڱ�ʾʱ���Ѿ�����     				  billflag:0��ʾ�Ѿ�����			billflag����ʾδ����
	_FLASH_OPERATION  FlashOper;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	Billbuf[gun][0] = billflag;
	if(billflag != 0)
	{
		Pre4GBill(gun,&Billbuf[gun][1]);
	}

	//��ȡ����������Ϣ
	if(gun == GUN_A )
	{
		FlashOper.DataID = PARA_BILLGUNA_ID;
	}
	else
	{
		FlashOper.DataID = PARA_BILLGUNB_ID;
	}
	if(billflag == 0)
	{
		FlashOper.Len =	1;	//0ֻ�������Ƿ��ͱ�־λ
	}
	else
	{
		FlashOper.Len =	500;
	}
	FlashOper.ptr = &Billbuf[gun][0];
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Writ Bill Error");
	}
	return TRUE;
}



/*****************************************************************************
* Function     : WriterFmOFFlineBill
* Description  : ���氲�������߽��׼�¼
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
INT8U WriterFmOFFlineBill(_GUN_NUM gun)
{
//��һ���ֽڱ�ʾʱ���Ѿ�����     				  billflag:0��ʾ�Ѿ�����			billflag����ʾδ����
	_FLASH_OPERATION  FlashOper;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	//��ȡ����������Ϣ
	if(gun == GUN_A )
	{
		FlashOper.DataID = PARA_BILLGUNA_ID;
	}
	else
	{
		FlashOper.DataID = PARA_BILLGUNB_ID;
	}
//	if(billflag == 0)
//	{
//		FlashOper.Len =	1;	//0ֻ�������Ƿ��ͱ�־λ
//	}
//	else
//	{
//		FlashOper.Len =	200;
//	}
	FlashOper.ptr = &Billbuf[gun][0];
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Writ Bill Error");
	}
	return TRUE;
}

/*****************************************************************************
* Function     : ReadFmBill
* Description  : ��ȡ����
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
INT8U ReadFmBill(_GUN_NUM gun)
{
	_FLASH_OPERATION  FlashOper;

	FlashOper.RWChoose = FLASH_ORDER_READ;
	FlashOper.Len = 500;


	FlashOper.DataID = PARA_BILLGUNA_ID;
	FlashOper.ptr = &Billbuf[GUN_A][0];
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read Bill Error");
	}

	FlashOper.DataID = PARA_BILLGUNB_ID;
	FlashOper.ptr = &Billbuf[GUN_B][0];
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read Bill Error");
	}

	if(gun == GUN_A)
	{
		if(Billbuf[GUN_A][0] == 1)
		{
			APP_SetResendBillState(GUN_A,1);
		}
		else
		{

			APP_SetResendBillState(GUN_A,0);
		}
	}
	if(gun == GUN_B)
	{
		if(Billbuf[GUN_B][0] == 1)
		{
			APP_SetResendBillState(GUN_B,1);
		}
		else
		{

			APP_SetResendBillState(GUN_B,0);
		}
	}
	return TRUE;
}


/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  : �ڳ���������Դ����¼���׼�¼
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
static INT8U Period_WriterFmBill(INT32U time)
{
	static INT16U count[GUN_MAX] = {0,0};
	_BSPRTC_TIME CurRTC;

	BSP_RTCGetTime(&CurRTC);

	if(APP_GetWorkState(GUN_A) == WORK_CHARGE)
	{
		if(++count[GUN_A] >= ((SYS_DELAY_5M*4)/time) )   //֮ǰΪ30s��һ�£�flash�ò��˶�ã�Ŀǰ��ʱ�ĳ�5����һ�� 20210623
			//if(++count[GUN_A] >= ((SYS_DELAY_5s)/time) )
		{

			count[GUN_A] = 0;
			//д��Aǹ��¼
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(APP_GetStartNetState(GUN_A) == NET_STATE_ONLINE)
				{
					WriterFmBill(GUN_A,1);			//���߱���
				}
			}
			else
			{
				WriterFmBill(GUN_A,1);
			}

		}
	}
	else
	{
		count[GUN_A] = 0;
	}
	if(APP_GetWorkState(GUN_B) == WORK_CHARGE)
	{
		if(++count[GUN_B] >= ((SYS_DELAY_5M*4)/time) )   //֮ǰΪ30s��һ�£�flash�ò��˶�ã�Ŀǰ��ʱ�ĳ�5����һ�� 20210623
			//	if(++count[GUN_B] >= ((SYS_DELAY_5s)/time) )
		{

			count[GUN_B] = 0;
			if(SYSSet.NetYXSelct == XY_AP)
			{
				if(APP_GetStartNetState(GUN_B) == NET_STATE_ONLINE)
				{
					WriterFmBill(GUN_B,1);			//���߱���
				}

			}
			else
			{
				WriterFmBill(GUN_B,1);
			}
			//д��Aǹ��¼
		}
	}
	else
	{
		count[GUN_B] = 0;
	}
	return TRUE;
}


/*****************************************************************************
* Function     : DispShow_42DivInfo
* Description  : ״̬��ʾ
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
INT8U DispShow_StatusDivInfo(void)
{
	INT16U i = 0;
	_SHOW_NUM * pshow_num;
	pshow_num = (_SHOW_NUM *)&DivStatue;
	INT16U gunvol[GUN_MAX];
	ST_Menu* pMenu = GetCurMenu();

	gunvol[GUN_A] = DivStatue.GunVol[GUN_A];
	gunvol[GUN_B] = DivStatue.GunVol[GUN_B];
	for(i = 0; i < (sizeof(DivStatue)/(sizeof(_SHOW_NUM))); i++) //һ��ҳ��һ����18��״̬
	{
		//ȫ����ʾ���ڽ��濨��
		if((pMenu == &HYMenu42) && (i < 18))
		{
			if(*(pshow_num + i) == SHOW_DIV_ERR)
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),RED);  //Ŀǰȫ����ʾ��ɫ
			}
			else
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),BLACK);  //Ŀǰȫ����ʾ��ɫ
			}
		}
		if((i >= 18) && (i < 36) && (pMenu == &HYMenu43))
		{
			if(*(pshow_num + i) == SHOW_DIV_ERR)
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),RED);  //Ŀǰȫ����ʾ��ɫ
			}
			else
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),BLACK);  //Ŀǰȫ����ʾ��ɫ
			}
			PrintNum16uVariable(DIS_ADD(43,0),gunvol[GUN_A]);
		}
		if((i >= 36) && (pMenu == &HYMenu44))
		{
			if(*(pshow_num + i) == SHOW_DIV_ERR)
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),RED);  //Ŀǰȫ����ʾ��ɫ
			}
			else
			{
				Dis_ShowDivStatus(ADDR_DEV_STATUS+ (DIVINFO_SINGLE_SIZE * i),*(pshow_num + i),BLACK);  //Ŀǰȫ����ʾ��ɫ
			}
			PrintNum16uVariable(DIS_ADD(44,0),gunvol[GUN_B]);
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Period_WriterFmRecode
* Description  :�����Կ����豸��Ϣ
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
static INT8U Period_GerDivStatus()
{
	INT8U i = 0;
	//����ABǹ���ʻ�ȡAǹ�ļ���
	//������
	BCURemoteSignal[GUN_A].state2.State.SurgeArrester_Err == 0 ? (DivStatue.SurgeArrester_Err = SHOW_DIV_NORMAL) :(DivStatue.SurgeArrester_Err = SHOW_DIV_ERR);
	//���������ѹ
	BCURemoteSignal[GUN_A].state3.State.AC_OVP == 0 ? (DivStatue.AC_OVP = SHOW_DIV_NORMAL) :(DivStatue.AC_OVP = SHOW_DIV_ERR);
	//��������Ƿѹ
	BCURemoteSignal[GUN_A].state3.State.AC_UVP == 0 ? (DivStatue.AC_UVP = SHOW_DIV_NORMAL) :(DivStatue.AC_UVP = SHOW_DIV_ERR);

	//�������¸澯
	DivStatue.SysUptemp_Warn = SHOW_DIV_NORMAL;
	//BCURemoteSignal[GUN_A].state1.State.SysUptemp_Warn == 0 ? (DivStatue.SysUptemp_Warn = SHOW_DIV_NORMAL) :(DivStatue.SysUptemp_Warn = SHOW_DIV_ERR);
	//���ȹ���
	BCURemoteSignal[GUN_A].state2.State.Fan_Warn == 0 ? (DivStatue.Fan_Warn = SHOW_DIV_NORMAL) :(DivStatue.Fan_Warn = SHOW_DIV_ERR);
	//������·������
	BCURemoteSignal[GUN_A].state1.State.ACCircuitBreaker_Err == 0 ?  (DivStatue.ACCircuitBreaker_Err = SHOW_DIV_NORMAL) :(DivStatue.ACCircuitBreaker_Err = SHOW_DIV_ERR);
	//�Ž�
	if(BSP_MPLSState(BSP_DI_DOOR))
	{
		DivStatue.Door_Err = SHOW_DIV_ERR;
	}
	else
	{
		DivStatue.Door_Err = SHOW_DIV_NORMAL;
	}
	//BCURemoteSignal[GUN_A].state1.State.Door_Err == 0 ? (DivStatue.Door_Err = SHOW_DIV_NORMAL) :(DivStatue.Door_Err = SHOW_DIV_ERR);
	//��ͣ
	BCURemoteSignal[GUN_A].state1.State.Stop_Pluse == 0 ? (DivStatue.Stop_Pluse = SHOW_DIV_NORMAL) :(DivStatue.Stop_Pluse = SHOW_DIV_ERR);
	//���ģ�齻�������ѹ�澯
	BCURemoteSignal[GUN_A].state4.State.ModuleACOVP_Warn == 0 ? (DivStatue.ModuleACOVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleACOVP_Warn = SHOW_DIV_ERR);
	//���ģ�齹¯����Ƿѹ�澯
	BCURemoteSignal[GUN_A].state4.State.ModuleACUVP_Warn == 0 ? (DivStatue.ModuleACUVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleACUVP_Warn = SHOW_DIV_ERR);
	//���ģ������ȱ��澯
	BCURemoteSignal[GUN_A].state4.State.ModuleACPhase_Warn == 0? (DivStatue.ModuleACPhase_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleACPhase_Warn = SHOW_DIV_WARNING);
	//���ģ�������·����
	BCURemoteSignal[GUN_A].state4.State.ModuleDCShortCircuit_Err == 0? (DivStatue.ModuleDCShortCircuit_Err = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCShortCircuit_Err = SHOW_DIV_ERR);
	//���ģ��ֱ����������澯
	BCURemoteSignal[GUN_A].state4.State.ModuleDCUPCUR_Warn == 0 ?  (DivStatue.ModuleDCUPCUR_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCUPCUR_Warn = SHOW_DIV_ERR);
	//���ģ��ֱ�������ѹ�澯
	BCURemoteSignal[GUN_A].state4.State.ModuleDCOVP_Warn == 0 ?  (DivStatue.ModuleDCOVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCOVP_Warn = SHOW_DIV_WARNING);
	//���ģ��ֱ�����Ƿѹ�澯
	BCURemoteSignal[GUN_A].state5.State.ModuleDCUVP_Warn == 0 ?  (DivStatue.ModuleDCUVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.ModuleDCUVP_Warn = SHOW_DIV_WARNING);
	//��Ҫ��A/Bǹ

	for(i = 0; i < (INT8U)GUN_MAX; i++)
	{
		if(GetMeterStatus((_GUN_NUM)i) == _COM_NORMAL)
		{
			DivStatue.GunStatus[i].WattMetereSignalErr = SHOW_DIV_NORMAL;
		}
		else
		{
			DivStatue.GunStatus[i].WattMetereSignalErr = SHOW_DIV_ERR;
		}
		DivStatue.GunStatus[i].CCUSignalErr = SHOW_DIV_NORMAL;
		//DivStatue.GunVol[i] = ((YC_Info[i].cc1volt/10) & 0x00ff) << 8 | ((YC_Info[i].cc1volt/10) & 0xff00) >> 8;
		DivStatue.GunVol[i] =  BSP_GetHandVolt((_GUN_NUM)i) / 10; //����һ��С��
		//���ǹ��λ״̬
#if ENBLE_JG
		BSP_MPLSState(BSP_DI_QGW) == 0 ? (DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_HOMING) :(DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_UNHOMING);
#else
		BCURemoteSignal[i].state3.State.ChargGun_State == 0 ? (DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_HOMING) :(DivStatue.GunStatus[i].ChargGun_State = SHOW_GUN_UNHOMING);
#endif
		//���ǹ����״̬
		BCURemoteSignal[i].state3.State.CGunUpTemperature_Err == 0 ? (DivStatue.GunStatus[i].CGunUpTemperature_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].CGunUpTemperature_Err = SHOW_DIV_WARNING);
		//������
		BCURemoteSignal[i].state2.State.DZLock_Err == 0 ? (DivStatue.GunStatus[i].DZLock_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DZLock_Err = SHOW_DIV_ERR);
		//��ط���
		BCURemoteSignal[i].state2.State.BatReverse_Err == 0 ? (DivStatue.GunStatus[i].BatReverse_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].BatReverse_Err = SHOW_DIV_ERR);
		//��BMSͨ��״̬
		BCURemoteSignal[i].state3.State.BMSCommunication_Err == 0 ? (DivStatue.GunStatus[i].BMSCommunication_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].BMSCommunication_Err = SHOW_DIV_ERR);
		//��Ե���
		BCURemoteSignal[i].state2.State.JyCheck_Err == 0 ? (DivStatue.GunStatus[i].JyCheck_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].JyCheck_Err = SHOW_DIV_ERR);
		//���ģ��ֱ����������澯
		BCURemoteSignal[i].state3.State.DCOVP_Warn == 0 ? (DivStatue.GunStatus[i].DCOVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DCOVP_Warn = SHOW_DIV_ERR);
		//���ģ��ֱ�������ѹ�澯
		BCURemoteSignal[i].state3.State.DCUVP_Warn == 0 ? (DivStatue.GunStatus[i].DCUVP_Warn = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DCUVP_Warn = SHOW_DIV_ERR);
		//���ģ��ֱ�����Ƿѹ�澯
		BCURemoteSignal[i].state3.State.UPCUR_Warn == 0 ? (DivStatue.GunStatus[i].UPCUR_Warn = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].UPCUR_Warn = SHOW_DIV_ERR);
		//����г�������
		BCURemoteSignal[i].state2.State.CarGuide_Err == 0 ? (DivStatue.GunStatus[i].CarGuide_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].CarGuide_Err = SHOW_DIV_ERR);
		//ֱ������Ӵ���

		BCURemoteSignal[i].state1.State.DCContactor_Err == 0 ? (DivStatue.GunStatus[i].DCContactor_Err = SHOW_DIV_NORMAL) :(DivStatue.GunStatus[i].DCContactor_Err = SHOW_DIV_ERR);
		//A/Bǹcc1��Ҫ������ʾ����ֵΪ SHOW_MAX����ʾ״̬����ʾ
		DivStatue.GunStatus[i].cc1volt = SHOW_MAX;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : Dispose_SOCThreshold
* Description  :soc��ֵ����
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
static INT8U Dispose_SOCThreshold(void)
{
	INT8U i;
	static INT16U lastsoc[GUN_MAX] = {0};

	for(i = 0; i < GUN_MAX; i++)
	{
		if(APP_GetWorkState((_GUN_NUM)i) == WORK_CHARGE)
		{
			if(ChargeingInfo[i].ChargeSOC != lastsoc[i])
			{
				lastsoc[i] = ChargeingInfo[i].ChargeSOC;
				if(SYSSet.SOCthreshold != 100)
				{
					if(((ChargeingInfo[i].ChargeSOC & 0xff00) >> 8) >= SYSSet.SOCthreshold)  //SOC�ﵽ��ֵ����ֹͣ
					{
						NB_WriterReason(i,"E59",3);
						SendStopChargeMsg((_GUN_NUM)i);
					}
				}
			}
		}
	}
	return FALSE;
}

/***********************************************************************************************
* Function		: APP_GetDevNum
* Description	: ��ȡ�豸��
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 20210111
***********************************************************************************************/
INT8U * APP_GetDevNum( void)
{
	return SYSSet.SysSetNum.DivNum;
}


/***********************************************************************************************
* Function		: DisplayGunQRCode
* Description	: ��ʾ��ά��
* Input 		:
* Output		:
* Note(s)		:
* Contributor	: 2018��7��18��
***********************************************************************************************/
INT8U DisplayGunQRCode(_GUN_NUM gun)
{
	INT8U buf[100]  = {0};
	//char * net = "http://smallprogram.evchong.com/EVCHONG_SP/startCharge/";  //ɭͨ
	char * net = "https://api.huichongchongdian.com/cpile/coffee/";
	//char * net = "https://zj.fengjingit.com/cpile/";				//����
	char * jgnet = "https://www.jgpowerunit.com/cpile/";  //����
	char * acnet = "https://www.chuangyuechongdian.com/cpile/";
	char * xxcnet = "https://qrcode.starcharge.com/#/";
	char * sycnet = "https://er.quicklycharge.com/scancode/connectorid/";

	char * ykcnet = "http://www.ykccn.com/MPAGE/index.html?pNum=";
	char * ykzsh = "https://ne.gdsz.sinopec.com/h5/MPAGE/index.html?pNum=";
	//char * ykcnet = "http://wx.evking.cn/app/download?orgNo=MA59MJR32&code=";  //ε����
	//char * ykcnet = "http://www.coulomb-charging.com/scans/result.html?data=";	 //����
	char * ttcnet = "https://nev.chinatowercom.cn?pNum=";

	char * dkycnet = "https://qrcode.dazzlesky.com/scancode/connectorid/";
	char * xjnet = "https://epower.xiaojukeji.com/epower/static/resources/xcxconf/XIAOJU.101437000.";
	char * annet = "https://zjec.evshine.cn/scan/scan/scanTransfer?gunNo=";
	INT8U code[17];
	INT8U apcode[18];
	INT8U len;


	if(gun > GUN_MAX)
	{
		return FALSE;
	}
	if(SYSSet.NetYXSelct == XY_HY)
	{
		len = strlen(net);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,net,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
		if(gun == GUN_A)
		{
			code[16] = '0';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0800,buf,17+len);
		}
		else
		{
			code[16] = '1';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0880,buf,17+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_JG)
	{
		len = strlen(jgnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,jgnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
		if(gun == GUN_A)
		{
			code[16] = '0';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0800,buf,17+len);
		}
		else
		{
			code[16] = '1';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0880,buf,17+len);
		}
	}

	if (SYSSet.NetYXSelct == XY_YL2)
	{
		len = strlen(acnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,acnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
		if(gun == GUN_A)
		{
			code[16] = '0';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0800,buf,17+len);
		}
		else
		{
			code[16] = '1';
			memcpy(&buf[len],code,17);
			DisplayQRCode(0x0880,buf,17+len);
		}
	}
	if (SYSSet.NetYXSelct == XY_YL1)
	{

		len = strlen(dkycnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,dkycnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_AP)
	{
		len = strlen(annet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,annet,len);
		memcpy(apcode,SYSSet.SysSetNum.DivNum,16);
		if(gun == GUN_A)
		{
			apcode[16] = '0';
			apcode[17] = '1';
			memcpy(&buf[len],apcode,18);
#if(USER_GUN == USER_SINGLE_GUN)
			DisplayQRCode(0x0800,buf,len + 16);
#else
			DisplayQRCode(0x0800,buf,len + 18);
#endif
		}
		else
		{
			apcode[16] = '0';
			apcode[17] = '2';
			memcpy(&buf[len],apcode,18);
			DisplayQRCode(0x0880,buf,len + 18);
		}
	}


	if(SYSSet.NetYXSelct == XY_YKC)
	{
		len = strlen(ykcnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,ykcnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_ZSH)
	{
		len = strlen(ykzsh);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,ykzsh,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}

	if(SYSSet.NetYXSelct == XY_TT)
	{
		len = strlen(ttcnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,ttcnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_YL3)
	{
		len = strlen(xxcnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,xxcnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[8] = '0';
			code[9] = '1';
			memcpy(&buf[len],code,10);
			DisplayQRCode(0x0800,buf,10+len);
		}
		else
		{
			code[8] = '0';
			code[9] = '2';
			memcpy(&buf[len],code,10);
			DisplayQRCode(0x0880,buf,10+len);
		}
	}
	if(SYSSet.NetYXSelct == XY_YL4)
	{
		len = strlen(sycnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,sycnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}

	if(SYSSet.NetYXSelct == XY_XJ)
	{
		len = strlen(xjnet);
		if(len > 80)
		{
			return FALSE;
		}
		memcpy(buf,xjnet,len);
		memcpy(code,SYSSet.SysSetNum.DivNum,14);
		if(gun == GUN_A)
		{
			code[14] = '0';
			code[15] = '1';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0800,buf,16+len);
		}
		else
		{
			code[14] = '0';
			code[15] = '2';
			memcpy(&buf[len],code,16);
			DisplayQRCode(0x0880,buf,16+len);
		}
	}
//		if(gun == GUN_A)
//		{
//			memcpy(&buf,SYSSet.SysSetNum.DivNum,14);
//			buf[14] = '0';
//			buf[15] = '1';
//			DisplayQRCode(0x0800,buf,16);
//		}else{
//			memcpy(&buf,SYSSet.SysSetNum.DivNum,14);
//			buf[14] = '0';
//			buf[15] = '2';
//			DisplayQRCode(0x0880,buf,16);
//		}

	return TRUE;
}

/*****************************************************************************
* Function     : Dispose_ACSW
* Description  :�����Ӵ�������
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
static INT8U Dispose_ACSW(void)
{
	static INT32U lasttime;
	INT32U nowSysTime = OSTimeGet();
#if(USER_GUN != USER_SINGLE_GUN)
	if((GetGunState(GUN_A) == GUN_DISCONNECTED)  && (GetGunState(GUN_B) == GUN_DISCONNECTED))
	{
		if((nowSysTime >= lasttime) ? ((nowSysTime - lasttime) >= SYS_DELAY_1M) : \
		        ((nowSysTime + (0xFFFFFFFF - lasttime)) >= SYS_DELAY_1M))
		{
			lasttime = nowSysTime;
			BSP_IOClose(IO_RLY_K3);
		}
	}
	else
	{
		DispControl.StartIntTime[GUN_A] = OSTimeGet() + SYS_DELAY_5s;
		DispControl.StartIntTime[GUN_B] = OSTimeGet() + SYS_DELAY_5s;
		lasttime = nowSysTime;
		BSP_IOOpen(IO_RLY_K3);
	}
#else
	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
	{
		if((nowSysTime >= lasttime) ? ((nowSysTime - lasttime) >= SYS_DELAY_1M) : \
		        ((nowSysTime + (0xFFFFFFFF - lasttime)) >= SYS_DELAY_1M))
		{
			lasttime = nowSysTime;
			BSP_IOClose(IO_RLY_K3);
		}
	}
	else
	{
		DispControl.StartIntTime[GUN_A] = OSTimeGet() + SYS_DELAY_5s;
		lasttime = nowSysTime;
		BSP_IOOpen(IO_RLY_K3);
	}
#endif
}

/*****************************************************************************
* Function     : Dispose_SendStartCharge
* Description  :����������磬��ҪΪ�˴����ϸ�ֹͣ����һ����ʼ������һ����ʱ��,CCU��Ҫֹͣ״̬����Ŀǰ����δ8s
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
static INT8U Dispose_SendStartCharge(void)
{
	INT8U gun;
	INT32U nowSysTime = OSTimeGet();
	for(gun = 0; gun < GUN_MAX; gun++)
	{
		if(DispControl.SendStartCharge[gun] == TRUE)
		{
			if((nowSysTime >= DispControl.StartIntTime[gun]) ? ((nowSysTime - DispControl.StartIntTime[gun]) >= SYS_DELAY_10s) : \
			        ((nowSysTime + (0xFFFFFFFF - DispControl.StartIntTime[gun])) >= SYS_DELAY_10s))
			{
				DispControl.SendStartCharge[gun] = FALSE;
				DispControl.StartIntTime[gun] = nowSysTime;
				if(DispControl.StartType == MODE_VIN)
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_VIN);
				}
				else
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_AUTO);	//֪ͨ�������
				}
			}
		}
	}
	return TRUE;
}




/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : �����Լ������
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
ST_Menu * Disp_NetPageDispos(void)
{
	INT8U   i;
	INT8U gun_state[GUN_MAX];   //0:�й��ϡ����� 			1���

	for(i = 0; i < GUN_MAX; i++)
	{
		if(DispControl.NetGunState[(_GUN_NUM)i] == GUN_IDLE) 				//����
		{
			gun_state[i] = 0;   //��ά�����
		}
		else
		{
			gun_state[i] = 1;	//�޶�ά�����
		}
	}

	if((gun_state[GUN_A] == 0) && (gun_state[GUN_B] == 0))
	{
		return &HYMenu73;
	}
	else if((gun_state[GUN_A] == 1) && (gun_state[GUN_B] == 0))
	{
		if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
		{
			return &HYMenu77;
		}
		else
		{
			return &HYMenu74;
		}
	}
	else if((gun_state[GUN_A] == 0) && (gun_state[GUN_B] == 1))
	{
		if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
		{
			return &HYMenu78;
		}
		else
		{
			return &HYMenu75;
		}
	}
	else
	{
		//4�ֿ���
		if((DispControl.NetGunState[GUN_A] == GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] == GUN_CHARGEING) )
		{
			return &HYMenu79;
		}
		else if((DispControl.NetGunState[GUN_A] == GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] != GUN_CHARGEING) )
		{
			if(DispControl.NetGunState[GUN_B] == GUN_IDLE)
			{
				return &HYMenu77;
			}
			else
			{
				return &HYMenu81;
			}
		}
		else if((DispControl.NetGunState[GUN_A] != GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] == GUN_CHARGEING) )
		{
			if(DispControl.NetGunState[GUN_A] == GUN_IDLE)
			{
				return &HYMenu78;
			}
			else
			{
				return &HYMenu80;
			}
		}
		else
		{
			return &HYMenu76;
		}
	}
}


/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : �����Լ������
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U  Net_StartCharge(_GUN_NUM gunnum,INT8U type)
{
	if(gunnum >= GUN_MAX)
	{
		return FALSE;
	}
	OSSchedLock();

	ChargeRecodeInfo[gunnum].Gun = gunnum;
	ChargeRecodeInfo[gunnum].BillingStatus = RECODE_UNSETTLEMENT;	  			//��ʼΪδ����
	SendPricMsg(gunnum,PRIC_START);			//֪ͨ��ʼ�Ʒ�
	DispControl.SendStartCharge[gunnum] = TRUE;
	DispControl.NetSSTState[gunnum] = NET_STARTING;
	DispControl.StartType = type;		//������ʽ
	OSSchedUnlock();
	return TRUE;
}

/*****************************************************************************
* Function     : Disp_NetPageDispos
* Description  : �����Լ������
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
INT8U  Net_StopCharge(_GUN_NUM gunnum)
{
	if(gunnum >= GUN_MAX)
	{
		return FALSE;
	}
	OSSchedLock();
	SendStopChargeMsg(gunnum);			//֪ͨAǹ�ػ�
	DispControl.NetSSTState[gunnum] = NET_STOPING;
	OSSchedUnlock();
	return TRUE;
}

/*****************************************************************************
* Function     : NetCardDispose
* Description  : �����ˢ������
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��16��
*****************************************************************************/
static void NetCardDispose(INT32U time)
{
	ST_Menu* CurMenu = GetCurMenu();
	static INT8U gun = 0;
	INT8U buf[2] = {0x55,0x55};
	if(GetCardWorkStatus() != _Card_IDLE)     //�����вŷ���
	{
		return;
	}
	if((SYSSet.NetYXSelct != XY_AP) && (SYSSet.NetYXSelct != XY_HY)&& (SYSSet.NetYXSelct != XY_JG)&&(SYSSet.NetYXSelct != XY_YL2) \
	        && (SYSSet.NetYXSelct != XY_ZSH)	&& (SYSSet.NetYXSelct != XY_YKC)&& (SYSSet.NetYXSelct != XY_TT)&& (SYSSet.NetYXSelct != XY_YL3)&& (SYSSet.NetYXSelct != XY_YL4) && ((SYSSet.NetYXSelct != XY_XJ)) \
	        && ((SYSSet.NetYXSelct != XY_YL1)))
	{
		return;
	}

	if(DispControl.NetGunState[gun] == GUN_CHARGEING)  //���״̬���͹ػ�ָ��
	{
		if((_4G_GetStartType(gun) == _4G_APP_CARD) || (_4G_GetStartType(gun) == _4G_APP_BCCARD)) //ˢ�������ŷ���
		{
			if(gun == GUN_A)
			{
				SendCardMsg(NETCARDCMD_STOPA, (void *)&buf,sizeof(buf));
			}
			else
			{
				SendCardMsg(NETCARDCMD_STOPB, (void *)&buf,sizeof(buf));
			}
		}
	}
	gun ++;
	if(gun >= GUN_MAX)
	{
		gun = GUN_A;
	}

}

/*****************************************************************************
* Function     : bug_dispose
* Description  :��ǹ������ȴ�ڳ���У���Ҫ��λ   20230308
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��8��31��
*****************************************************************************/
static INT8U bug_dispose(void)
{
	INT8U gun;
	INT32U nowSysTime = OSTimeGet();
	for(gun = 0; gun < GUN_MAX; gun++)
	{
		if((APP_GetErrState((_GUN_NUM)gun) == 0x01)  &&  ( (GetRM_HandVolt(gun) == INTERFACE_UNFULCONNECTED) ||
		        (GetGunState(gun) == GUN_DISCONNECTED)) ) //���ڳ��)
		{
			if((nowSysTime >= DispControl.StartIntTime[gun]) ? ((nowSysTime - DispControl.StartIntTime[gun]) >= SYS_DELAY_10s) : \
			        ((nowSysTime + (0xFFFFFFFF - DispControl.StartIntTime[gun])) >= SYS_DELAY_10s))
			{
				DispControl.SendStartCharge[gun] = FALSE;
				DispControl.StartIntTime[gun] = nowSysTime;
				if(DispControl.StartType == MODE_VIN)
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_VIN);
				}
				else
				{
					SendStartChargeMsg((_GUN_NUM)gun,MODE_AUTO);	//֪ͨ�������
				}
			}
		}
	}
	return TRUE;
}

#define DEFPOWER    500		//���������Ĭ��50kw���
INT16U curpowera = 0;
INT16U curpowerb = 0;
/*****************************************************************************
* Function     : Period_PowerRegulate
* Description  :�������ʵ���
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2023��12��8��
*****************************************************************************/
static INT8U Period_PowerRegulate(void)
{
	INT16U kwa = 0,kwb = 0;
	INT16U power = 0;
	INT8U * pdevnum = APP_GetDevNum();


#if (ENBLE_JG == 0)   //�Ǿ��� ���蹦�ʵ���
	curpowera = 2000;
	curpowerb = 2000;
#else
	OSSchedLock();

	if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
	{
		kwa = PowerModuleInfo[GUN_A].OutputInfo.Vol * PowerModuleInfo[GUN_A].OutputInfo.Cur / 10000;
	}
	if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
	{
		kwb = PowerModuleInfo[GUN_B].OutputInfo.Vol * PowerModuleInfo[GUN_B].OutputInfo.Cur / 10000;
	}

	if(APP_GetModuleConnectState(0) == STATE_OK)
	{
		power = APP_GetHYCurPower();  //����ʵʱ����
	}
	else
	{
		power = DEFPOWER;				//δ����Ĭ������
	}
	if(power < 100)  					//�ܹ��ʲ�������100kw ����100kw ��ػ�
	{
		if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
		{
			Net_StopCharge(GUN_A);
		}
		if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
		{
			Net_StopCharge(GUN_B);
		}
	}


	//2��ǹ���ڳ�����Ĳ���ƽ����
	if((DispControl.NetGunState[GUN_A] == GUN_CHARGEING) && (DispControl.NetGunState[GUN_B] == GUN_CHARGEING))
	{
		if((kwb+kwa) >= power)   //�ܹ��ʴ�
		{
			//����������
			curpowerb = (kwb * (power * 100 / (kwb+kwa))) / 100;
			curpowera = (kwa * (power * 100 / (kwb+kwa))) / 100;
		}
		else
		{
			curpowera = (power - kwa - kwb)/2+kwa;
			curpowerb = (power - kwa - kwb)/2+kwb;
		}
	}
	else if(DispControl.NetGunState[GUN_A] == GUN_CHARGEING)
	{
		//Aǹ�ٳ��
		if (GetGunState(GUN_B) == GUN_CONNECTED)
		{
			curpowera = power - 50;			//Ԥ��5kw��B
			curpowerb = 50;
		}
		else
		{
			curpowera = power;
			curpowerb = 0;
		}
	}
	else if(DispControl.NetGunState[GUN_B] == GUN_CHARGEING)
	{
		//Bǹ�ٳ��
		if (GetGunState(GUN_A) == GUN_CONNECTED)
		{
			curpowerb = power - 50;			//Ԥ��5kw��a
			curpowera = 50;
		}
		else
		{
			curpowerb = power;
			curpowera = 0;
		}
	}
	else
	{
		curpowerb = power/2;
		curpowera = power/2;
	}
	OSSchedUnlock();
#endif
	return TRUE;
}

/*****************************************************************************
* Function     : DispPeriod
* Description  : ��ʾ����500msִ��һ��   ���뷢��TaskDisplay ��ʾ�����У��������ݱ�����ͬһ������
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static void DispPeriod(void)
{
	char buf[30] = {0};
	static INT32U delay = SYS_DELAY_500ms;   //�����Լ�����ڣ����ܳ���1s
	//ǹδ���ӣ�����������ǹ�ţ�������ѡ�����


	ST_Menu* CurMenu;
	static ST_Menu * ppage = NULL,*plastpage = NULL;
	CurMenu = GetCurMenu();
	static ST_Menu* lastMenu = NULL;
	static INT8U LastGunState[GUN_MAX] = {GUN_DISCONNECTED};      //ǹ״̬�ı���Ҫ����ʵʱ����
	OS_EVENT* pevent = APP_Get4GMainEvent();
	static _BSP_MESSAGE SendMsg[GUN_MAX];
	//����汾ҳ���л�


	//��ʾ����汾��+׮���
	if(lastMenu != CurMenu)
	{
		lastMenu =CurMenu;
		snprintf((char *)buf, sizeof(buf), "DC%02d%02d%02d",SYS_APP_VER[0],SYS_APP_VER[1],SYS_APP_VER[2]);

		PrintStr(0x0100,(INT8U *)buf,strlen(buf));
		PrintStr(0x0200,(INT8U *)SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum)); //����汾
	}


	if(SYSSet.NetState == DISP_NET)
	{
		if((CurMenu == &HYMenu73) || (CurMenu == &HYMenu74) || (CurMenu == &HYMenu75) || (CurMenu == &HYMenu76) \
		        || (CurMenu == &HYMenu77) || (CurMenu == &HYMenu78) || (CurMenu == &HYMenu79) ||  (CurMenu == &HYMenu80) || (CurMenu == &HYMenu81))
		{
			ppage = Disp_NetPageDispos();
			//����2��һ�������л�����ֹ����,ԼΪ1s��һ��
			if(plastpage != ppage)
			{
				plastpage = ppage;
			}
			else
			{
				if(ppage != CurMenu)
				{
					DisplayCommonMenu(ppage,NULL);
				}
			}
		}

		//ǹ״̬�ı���Ҫ����ʵʱ����
		if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
		{
			if(GetGunState(GUN_A) != LastGunState[GUN_A])
			{
				SendMsg[GUN_A].MsgID = BSP_MSGID_DISP;
				SendMsg[GUN_A].DivNum = APP_SJDATA_QUERY;   //����ʵʱ���ݣ�״̬�����˸ı�
				SendMsg[GUN_A].DataLen = GUN_A;
				OSQPost(pevent, &SendMsg[GUN_A]);

				LastGunState[GUN_A] =GetGunState(GUN_A);
			}
			if(GetGunState(GUN_B) != LastGunState[GUN_B])
			{
				SendMsg[GUN_B].MsgID = BSP_MSGID_DISP;
				SendMsg[GUN_B].DivNum = APP_SJDATA_QUERY;   //����ʵʱ���ݣ�״̬�����˸ı�
				SendMsg[GUN_B].DataLen = GUN_B;
				OSQPost(pevent, &SendMsg[GUN_B]);

				LastGunState[GUN_B] =GetGunState(GUN_B);
			}
		}
	}

	if(SYSSet.NetState == DISP_NET)
	{
		NOP();
	}
	else
	{
		if(GetGunState(GUN_A) == GUN_DISCONNECTED)
		{
			if((DispControl.ClickGunNextPage[GUN_A] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_A] == &HYMenu33) || \
			        (DispControl.ClickGunNextPage[GUN_A] == &HYMenu36) )
			{
				DispControl.CardOver[GUN_A] = STATE_UNOVER;
				DispControl.StopOver[GUN_A] = STATE_UNOVER;
				ChargeRecode_Dispose(GUN_A);				//���׼�¼����
				DispControl.ClickGunNextPage[GUN_A] = &HYMenu22;
				DisplayCommonMenu(&HYMenu3,NULL);
			}
		}
		if(GetGunState(GUN_B) == GUN_DISCONNECTED)
		{
			if((DispControl.ClickGunNextPage[GUN_B] == &HYMenu32) || (DispControl.ClickGunNextPage[GUN_B] == &HYMenu33) || \
			        (DispControl.ClickGunNextPage[GUN_B] == &HYMenu37) )
			{
				DispControl.CardOver[GUN_B] = STATE_UNOVER;
				DispControl.StopOver[GUN_B] = STATE_UNOVER;
				ChargeRecode_Dispose(GUN_B);				//���׼�¼����
				DispControl.ClickGunNextPage[GUN_B] = &HYMenu22;
				DisplayCommonMenu(&HYMenu3,NULL);
			}
		}
	}
	CountDownDispose(delay);				//����ʱ����
	Period_GerDivStatus();
	Disp_ShowRTC();							//������ʾRTC
	Disp_ShowGunT();						//��ʾǹ��
	DispShow_ChargeingDate(delay);			//���������ݿ������Լ����ڳ���У�ÿ��1�����ʾ����������
	DispShow_CardDate(delay);				//������Ϣ����������ݿ������Լ����ڳ���У�ÿ��1�����ʾ����������
	//��ǹ������ȴ�ڳ���У���Ҫ��λ   20230308
	Dispose_ACSW();			//�����Ӵ�������
	DispShow_State(delay);					//��������ʾA Bǹ״̬
	Dispose_SOCThreshold();//SOC�ﵽ��ֵ���
	Dispose_SendStartCharge(); //����������磬��ҪΪ�˴����ϸ�ֹͣ����һ����ʼ������һ����ʱ��,CCU��Ҫֹͣ״̬����Ŀǰ����δ8s
	if(SYSSet.NetState == DISP_NET)
	{
		//����ģʽ�¹��ʵ���
		if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
		{
			Period_PowerRegulate();					//����ģʽ�¹��ʵ���
		}
		Period_WriterFmBill(delay);	         	//�ڳ���������Դ����¼���׼�¼
		NetCardDispose(delay);					//����״̬�°����������Է���ˢ��ָ��
	}
}





/****************************************����flash  ����ر�ι��*********************************/
//#include "stm32f4xx_flash.h"
//#define FLASH_BASE_ADD  0x080F0000


//#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
//#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
//#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
//#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

//uint32_t GetSector(uint32_t Address)
//{
//  uint32_t sector = 0;
//
//  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
//  {
//    sector = FLASH_Sector_0;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
//  {
//    sector = FLASH_Sector_1;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
//  {
//    sector = FLASH_Sector_2;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
//  {
//    sector = FLASH_Sector_3;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
//  {
//    sector = FLASH_Sector_4;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
//  {
//    sector = FLASH_Sector_5;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
//  {
//    sector = FLASH_Sector_6;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
//  {
//    sector = FLASH_Sector_7;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
//  {
//    sector = FLASH_Sector_8;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
//  {
//    sector = FLASH_Sector_9;
//  }
//  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
//  {
//    sector = FLASH_Sector_10;
//  }
//  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
//  {
//    sector = FLASH_Sector_11;
//  }

//  return sector;
//}



//INT8U write_flash(uint32_t add,uint16_t *FlashWriteBuf,uint16_t len)
//{
//	if((add < ADDR_FLASH_SECTOR_0) || (add > 0x080FFFFF) )
//	{
//		return FALSE;
//	}

//	FLASH_Unlock();	//����
//    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
//                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

//	if (FLASH_COMPLETE != FLASH_EraseSector(GetSector(add),VoltageRange_2)) //������������
//    {
//		return FALSE;
//	}
//
//	for (int i = 0; i < len/2; i++)
//	{
//		if (FLASH_COMPLETE != FLASH_ProgramHalfWord(add, FlashWriteBuf[i]))	//д��16λ����
//		{
//			return FALSE;
//		}
//		add += 2;	//16λ����ƫ������λ��
//	}

//	FLASH_Lock();	//����
//
//	return TRUE;
//}

//INT16U read_flash(INT32U add)
//{
//	return *(INT16U*)add;
//}

//INT8U read_flashbuf(INT32U add,INT16U * buf,INT16U len)
//{
//	INT8U num;
//
//	for(num = 0;num < len/2;num++)
//	{
//		buf[num] = read_flash(add);
//		add +=2;
//	}
//}

//INT16U WFlashbuf[5] = {0x1111,0x2222,0x3333,0x4444,0x5555};
//INT16U RFlashbuf[5] = {0};

//INT8U Nflash_test(void)
//{
//	write_flash(ADDR_FLASH_SECTOR_11,WFlashbuf,sizeof(WFlashbuf));
//	read_flashbuf(ADDR_FLASH_SECTOR_11,RFlashbuf,sizeof(RFlashbuf));
//	__NOP();
//}
/*************************************flash  end*************************************************/
/*****************************************************************************
* Function     : DispPeriod
* Description  : ��ʾ����500msִ��һ��   ���뷢��TaskDisplay ��ʾ�����У��������ݱ�����ͬһ������
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��27��
*****************************************************************************/
static void Display66GunQRCode(void)
{
	static INT8U gunshow[GUN_MAX] = {0};
	INT8U len;
	INT8U Devlen;
	INT8U buf[17] = {0};

	memcpy(buf,SYSSet.SysSetNum.DivNum,sizeof(SYSSet.SysSetNum.DivNum));
	if(SYSSet.NetYXSelct == XY_66)
	{
		//SYSSet.SysSetNum.DivNum[15] = 0;
		if(gunshow[GUN_A] == 0)
		{
			if(Qrcode[GUN_A][0] != 0X00)
			{
				gunshow[GUN_A] = 1;
				len = strlen((char*)&Qrcode[GUN_A][0]);
				Devlen = strlen((char*)buf);
				memcpy(&Qrcode[GUN_A][len],SYSSet.SysSetNum.DivNum,Devlen);
				Qrcode[GUN_A][len + Devlen] = '0';
				Qrcode[GUN_A][len + Devlen + 1] = '1';
				//DisplayQRCode(0x0800,&Qrcode[GUN_A][0],len + Devlen + 2);
				DisplayQRCode(0x0800,&Qrcode[GUN_A][0],len);

			}
		}
		if(gunshow[GUN_B] == 0)
		{
			if(Qrcode[GUN_B][0] != 0X00)
			{
				gunshow[GUN_B] = 1;
				len = strlen((char*)&Qrcode[GUN_B][0]);
				Devlen = strlen((char*)buf);
				memcpy(&Qrcode[GUN_B][len],SYSSet.SysSetNum.DivNum,Devlen);
				Qrcode[GUN_B][len + Devlen] = '0';
				Qrcode[GUN_B][len + Devlen + 1] = '2';
				//DisplayQRCode(0x0880,&Qrcode[GUN_B][0],len + Devlen + 2);
				DisplayQRCode(0x0880,&Qrcode[GUN_B][0],len);
			}
		}

	}
}


/*****************************************************************************
* Function     : TaskDisplay
* Description  : ��ʾ����
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��7��16��
*****************************************************************************/
void TaskDisplay(void *pdata)
{

	INT8U err;
	_BSP_MESSAGE *pMsg;
	const ST_Menu* CurMenu;
	_LCD_KEYVALUE key;
	static INT32U curtime,lasttime;
	//ST_Menu * ppage = NULL;

	//DispMutex = OSMutexCreate(PRI_DIS_MUTEX, &err);      //����������

	//�ڲ�flash  ����
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_PrefetchBufferCmd(ENABLE);

	OSTimeDly(SYS_DELAY_500ms);
	Display_HardwareInit();
	OSTimeDly(SYS_DELAY_500ms);
	//ϵͳ��ʼ������
//	DisplayCommonMenu(&gsMenu0,NULL);
	//��ʼ�������ת�������棬Ŀǰֻ����ʱ5s


	Display_ParaInit();				//������ʼ��

	if(SYSSet.NetState == DISP_NET)
	{
		ReadFmBill(GUN_A);					//��ȡ����
		ReadFmBill(GUN_B);					//��ȡ����
		DisplayCommonMenu(&HYMenu73,NULL);
		//��ʾA��Bǹ��ά��
		DisplayGunQRCode(GUN_A);
		DisplayGunQRCode(GUN_B);
	}
	else
	{
		DisplayCommonMenu(&HYMenu3,NULL);
	}
	OSTimeDly(SYS_DELAY_2s);
//	INT8U paddr[16] = {0x33,0x31,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x31,0x30,0x30,0x30,0x32};
//	Munu52_DevnumDispose(0,paddr,16);
	//PrintIcon(0x12b0,0);

	//Nflash_test();

	while(1)
	{
		if((APP_GetSIM7600Mode() == MODE_HTTP)  || (APP_GetSIM7600Mode() == MODE_FTP))   //Զ�����������޹�����֡������Ҫ���ͺʹ���
		{
			OSTimeDly(SYS_DELAY_1000ms);
			continue; //δ�����Ϸ�������ATָ���
		}
		pMsg = OSQPend(DispTaskEvent, SYS_DELAY_500ms, &err);
		curtime = OSTimeGet();
		if (OS_ERR_NONE == err)
		{
			switch (pMsg->MsgID)
			{
				case BSP_MSGID_UART_RXOVER:
					if (UART_MsgDeal(&pMsg, DispTempBuf, sizeof(DispTempBuf)) == TRUE)
					{
						APP_DisplayRecvDataAnalyze(pMsg->pData, pMsg->DataLen); //���ݽ���
					}
					UART_FreeOneMsg(DISPLAY_UART, pMsg);
					break;
				case BSP_MSGID_CARD:
					APP_RecvCardAnalyze(pMsg);										//���տ�������Ϣ����
					break;
				case BSP_MSGID_CONTROL:
					APP_RecvConectCCUAnalyze(pMsg);									//���Կ����������Ϣ
					break;
				case BSP_MSGID_METER:
					if(SYSSet.NetState == DISP_NET) //����汾
					{
						//���������
						if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1))
						{
							APP_Set_ERR_Branch(pMsg->pData[0],STOP_BALANCE);
							NB_WriterReason((_GUN_NUM)pMsg->DataLen,"E52",3);
							Net_StopCharge((_GUN_NUM)pMsg->pData[0]);  //�ƿ��������Ҫ����ֹͣ
						}
					}
					else
					{
						NB_WriterReason((_GUN_NUM)pMsg->pData[0],"E50",3);
						SendStopChargeMsg((_GUN_NUM)pMsg->pData[0]);   							//����ֹͣ֡
					}
					break;
				case BSP_4G_MAIN:		//����APP���͹�������Ϣ
					if(pMsg->DataLen >= GUN_MAX)  //ǹ��
					{
						break;
					}
					if(pMsg->DivNum ==  APP_START_CHARGE) //��ʼ���
					{
						if(DispControl.NetGunState[pMsg->DataLen] == GUN_IDLE)
						{
							if(DispControl.NetSSTState[pMsg->DataLen] != NET_STARTING)
							{
								HYDisplayMenu37();						//��ֹ������ʾ����֮ǰ������
								Net_StartCharge((_GUN_NUM)pMsg->DataLen,MODE_AUTO);
							}
						}
					}
					if(pMsg->DivNum ==  APP_STOP_CHARGE) //�������
					{
						if(DispControl.NetGunState[pMsg->DataLen] == GUN_CHARGEING)
						{
							if(DispControl.NetSSTState[pMsg->DataLen] != NET_STOPING)
							{
								NB_WriterReason((_GUN_NUM)pMsg->DataLen,"E51",3);
								Net_StopCharge((_GUN_NUM)pMsg->DataLen);
							}
						}
					}
					if(pMsg->DivNum ==  APP_START_VIN) //VIN��Ȩ
					{
						if((_4G_GetStartType((_GUN_NUM)pMsg->DataLen) == _4G_APP_VIN) || (_4G_GetStartType((_GUN_NUM)pMsg->DataLen) == _4G_APP_BCVIN))
						{
							if(pMsg->pData[0])
							{
								//��Ȩ�ɹ�
								Set_VIN_Success((_GUN_NUM)pMsg->DataLen,VIN_SECCSEE);
							}
							else
							{
								Set_VIN_Success((_GUN_NUM)pMsg->DataLen,VIN_FAIL);
								//��Ȩʧ��
							}
						}
					}
					if(pMsg->DivNum ==  APP_START_QRCODE) //��ά��
					{
						if(pMsg->DataLen == GUN_A)
						{
							pMsg->pData[140] = 0;
							DisplayQRCode(0x0800,pMsg->pData,strlen((char *)pMsg->pData));
						}
						if(pMsg->DataLen == GUN_B)
						{
							pMsg->pData[140] = 0;
							DisplayQRCode(0x0880,pMsg->pData,strlen((char *)pMsg->pData));
						}
					}

					break;
				default:
					break;
			}
		}

		//��ȡ��ǰ����
		CurMenu = GetCurMenu();                                         //����
		if (CurMenu && CurMenu->function3)
		{
			CurMenu->function3();                                       //������ʾ
		}
		if(	DispControl.CountDown == 1)									//����ʱ����1
		{
			//ҳ���л������쵱ǰҳ��ġ���һҳ���������桷��ťЧ��
			if((CurMenu == &HYMenu8) || (CurMenu == &HYMenu9) || (CurMenu == &HYMenu36) ||(CurMenu == &HYMenu37) \
			        || (CurMenu == &HYMenu42) || (CurMenu == &HYMenu43) || (CurMenu == &HYMenu44) || (CurMenu == &HYMenu32) || (CurMenu == &HYMenu33) )
			{
				//��¼��ѯ���߳���������豸��Ϣ�������ˢ�����泬ʱֱ�ӷ���������
				key = LCD_KEY1;
			}
			else
			{
				key = LCD_KEY2;
			}
			if((CurMenu == &HYMenu36) && (SYSSet.SysSetNum.UseGun == 1) && (SYSSet.NetState == DISP_CARD))
			{
				NOP(); //��ǹ�����ڳ����治��ת
			}
			else
			{
				DealWithKey(&key);
			}
		}

//		#if(SYSSet.NetState == DISP_NET)
//		CurMenu = GetCurMenu();
//		//����汾ҳ���л�
//		if((CurMenu == &HYMenu73) || (CurMenu == &HYMenu74) || (CurMenu == &HYMenu75) || (CurMenu == &HYMenu76) \
//			|| (CurMenu == &HYMenu77) || (CurMenu == &HYMenu78) || (CurMenu == &HYMenu79))
//		{
//			ppage = Disp_NetPageDispos();
//			if(ppage != CurMenu)
//			{
//				DisplayCommonMenu(ppage,NULL);
//			}
//		}
//		#endif
		//500ms ��������������
		if(SYSSet.NetYXSelct == XY_66)
		{
			Display66GunQRCode();
		}
		if((curtime - lasttime) > SYS_DELAY_500ms)
		{
			lasttime = curtime;
			DispPeriod();
		}
	}
}
/************************(C)COPYRIGHT 2020 �����Ƽ�*****END OF FILE****************************/
