/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: connectbmsframe.c
* Author			: Ҷϲ��
* Date First Issued	: 23/05/2016
* Version			: V0.1
* Description		: ����BMS����֡�ȴ����ļ������з���֡����֯�����ڴ��ļ��� 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "gbstandard.h"
#include "connectbms.h"
#include "controlmain.h"
/* Private define-----------------------------------------------------------------------------*/
#define CTS_CONTROL_BYTE       17     //CTS�Ŀ�����
#define EMA_CONTROL_BYTE       19     //EMA�Ŀ����� End of Msg ACK;
#define CRM_DEVICE_CODENUM      8     //������Ź̶�Ϊ8
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
extern INT8U CRMReadyflag[GUN_MAX];
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: Reply_BMSFrame_CTS
* Description	: �ظ�BMS��RTS�Ļظ�֡CTS
* Input			: 
* Return		: 
* Note(s)		: ��CTS���������������ý׶ε�CTS����CTSΪJ1939Э��Ķ�֡������Ļظ�RTS��CTS
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
void Reply_BMSFrame_CTSA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //����֡ID 1C EC F4 56
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;
    
    BMSCANTxbuffA.Data[0] = CTS_CONTROL_BYTE; //�������͸�BMS�Ŀ����֣�CTS�̶�17(0x11)
    BMSCANTxbuffA.Data[1] = BMS_RTS_FrameInf[GUN_A].messagepagenum; //�ɷ��͵����ݰ���
    BMSCANTxbuffA.Data[2] = 0x01;//�̶�Ϊ��һ��Ҫ���͵����ݰ���� 
    BMSCANTxbuffA.Data[3] = 0xFF;
    BMSCANTxbuffA.Data[4] = 0xFF;//SAEԤ���������ֽ�
    memcpy(&BMSCANTxbuffA.Data[5], BMS_RTS_FrameInf[GUN_A].paranumber, 3);//���յ���RTS�Ĵ����Ϣ�Ĳ�������
    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: Reply_BMSFrame_CTS
* Description	: �ظ�BMS��RTS�Ļظ�֡CTS
* Input			: 
* Return		: 
* Note(s)		: ��CTS���������������ý׶ε�CTS����CTSΪJ1939Э��Ķ�֡������Ļظ�RTS��CTS
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
void Reply_BMSFrame_CTSB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //����֡ID 1C EC F4 56
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;
    
    BMSCANTxbuffB.Data[0] = CTS_CONTROL_BYTE; //�������͸�BMS�Ŀ����֣�CTS�̶�17(0x11)
    BMSCANTxbuffB.Data[1] = BMS_RTS_FrameInf[GUN_B].messagepagenum; //�ɷ��͵����ݰ���
    BMSCANTxbuffB.Data[2] = 0x01;//�̶�Ϊ��һ��Ҫ���͵����ݰ���� 
    BMSCANTxbuffB.Data[3] = 0xFF;
    BMSCANTxbuffB.Data[4] = 0xFF;//SAEԤ���������ֽ�
    memcpy(&BMSCANTxbuffB.Data[5], BMS_RTS_FrameInf[GUN_B].paranumber, 3);//���յ���RTS�Ĵ����Ϣ�Ĳ�������
    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: Reply_TPCM_EndofMsgAckFrame
* Description	: ���Ͷ�֡��Ϣ����Ӧ�𣬱�ʾ������Ϣ�Ѿ������ղ���ȷ����
* Input			: 
* Output		: 
* Note(s)		: CRM���ģ�PGN=236 P=7
* Contributor	: 20101210          
***********************************************************************************************/
void Reply_TPCM_EndofMsgAckFrameA(void)
{
    INT16U messagebytenum = 0;
    
    OSSchedLock();
    messagebytenum = BMS_RTS_FrameInf[GUN_A].messagebytenum;
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //����֡ID
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;
    
    BMSCANTxbuffA.Data[0] = EMA_CONTROL_BYTE; //������,�̶�Ϊ19(0x13)
    BMSCANTxbuffA.Data[1] = messagebytenum&0xFF; //���������Ч�ֽ���
    BMSCANTxbuffA.Data[2] = (messagebytenum>>8)&0xFF; //
    BMSCANTxbuffA.Data[3] = BMS_RTS_FrameInf[GUN_A].messagepagenum; //ȫ�����ݰ�����
    BMSCANTxbuffA.Data[4] = 0xFF;//SAEԤ���������ֽ�
    memcpy( &BMSCANTxbuffA.Data[5],BMS_RTS_FrameInf[GUN_A].paranumber,3);//���յ���RTS�Ĵ����Ϣ�Ĳ�������
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
    OSSchedUnlock();   
}

/***********************************************************************************************
* Function		: Reply_TPCM_EndofMsgAckFrame
* Description	: ���Ͷ�֡��Ϣ����Ӧ�𣬱�ʾ������Ϣ�Ѿ������ղ���ȷ����
* Input			: 
* Output		: 
* Note(s)		: CRM���ģ�PGN=236 P=7
* Contributor	: 20101210          
***********************************************************************************************/
void Reply_TPCM_EndofMsgAckFrameB(void)
{
    INT16U messagebytenum = 0;
    
    OSSchedLock();
    messagebytenum = BMS_RTS_FrameInf[GUN_B].messagebytenum;
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //����֡ID
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;
    
    BMSCANTxbuffB.Data[0] = EMA_CONTROL_BYTE; //������,�̶�Ϊ19(0x13)
    BMSCANTxbuffB.Data[1] = messagebytenum&0xFF; //���������Ч�ֽ���
    BMSCANTxbuffB.Data[2] = (messagebytenum>>8)&0xFF; //
    BMSCANTxbuffB.Data[3] = BMS_RTS_FrameInf[GUN_B].messagepagenum; //ȫ�����ݰ�����
    BMSCANTxbuffB.Data[4] = 0xFF;//SAEԤ���������ֽ�
    memcpy( &BMSCANTxbuffB.Data[5],BMS_RTS_FrameInf[GUN_B].paranumber,3);//���յ���RTS�Ĵ����Ϣ�Ĳ�������
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
    OSSchedUnlock();   
}
/***********************************************************************************************
* Function		: CRM_SendShakeHandIdentifyFrame
* Description	: ���ͳ�����ʶ����
* Input			: 
* Output		: 
* Note(s)		: CRM���ģ�PGN=256 P=6
* Contributor	: 20101210          
***********************************************************************************************/
void CRM_SendShakeHandIdentifyFrameA(void)
{
    OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CRM_PGN_256);  //����֡ID
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;
#if 0
    if(BMS_CONNECT_Control.currentstate == BMS_BRM_SUCCESS)//BRM�Ѿ�ȫ������
        BMSCANTxbuff.Data[0] = 0xAA; //��ʶ���
    else
        BMSCANTxbuff.Data[0] = 0x00; //��ʶ���
#endif
    if (CRMReadyflag[GUN_A])
	{
		BMSCANTxbuffA.Data[0] = 0xAA; //��ʶ���
	}
	else 
	{
		BMSCANTxbuffA.Data[0] = 0x00; //��ʶ���
	}
    BMSCANTxbuffA.Data[1] = CRM_DEVICE_CODENUM; //�������
    BMSCANTxbuffA.Data[2] = 0x00; 
    BMSCANTxbuffA.Data[3] = 0x00;
    BMSCANTxbuffA.Data[4] = 0x00;
    BMSCANTxbuffA.Data[5] = 0xFF;
    BMSCANTxbuffA.Data[6] = 0xFF;
    BMSCANTxbuffA.Data[7] = 0xFF;
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
    OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRM_SendShakeHandIdentifyFrame
* Description	: ���ͳ�����ʶ����
* Input			: 
* Output		: 
* Note(s)		: CRM���ģ�PGN=256 P=6
* Contributor	: 20101210          
***********************************************************************************************/
void CRM_SendShakeHandIdentifyFrameB(void)
{
    OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CRM_PGN_256);  //����֡ID
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;
#if 0
    if(BMS_CONNECT_Control.currentstate == BMS_BRM_SUCCESS)//BRM�Ѿ�ȫ������
        BMSCANTxbuff.Data[0] = 0xAA; //��ʶ���
    else
        BMSCANTxbuff.Data[0] = 0x00; //��ʶ���
#endif
    if (CRMReadyflag[GUN_B])
	{
		BMSCANTxbuffB.Data[0] = 0xAA; //��ʶ���
	}
	else 
	{
		BMSCANTxbuffB.Data[0] = 0x00; //��ʶ���
	}
    BMSCANTxbuffB.Data[1] = CRM_DEVICE_CODENUM; //�������
    BMSCANTxbuffB.Data[2] = 0x00; 
    BMSCANTxbuffB.Data[3] = 0x00;
    BMSCANTxbuffB.Data[4] = 0x00;
    BMSCANTxbuffB.Data[5] = 0xFF;
    BMSCANTxbuffB.Data[6] = 0xFF;
    BMSCANTxbuffB.Data[7] = 0xFF;
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
    OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CHM_SendShakeHandFrame
* Description	: ���ͳ�����ʶ����
* Input			: 
* Output		: 
* Note(s)		: CHM���ģ�PGN=9278 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CHM_SendShakeHandFrameA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CHM_PGN_9728);  //����֡
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 3;
    
    BMSCANTxbuffA.Data[0] = 0x01; //�汾V1.1
    BMSCANTxbuffA.Data[1] = 0x01;
    BMSCANTxbuffA.Data[2] = 0x00; 
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}


/***********************************************************************************************
* Function		: CHM_SendShakeHandFrame
* Description	: ���ͳ�����ʶ����
* Input			: 
* Output		: 
* Note(s)		: CHM���ģ�PGN=9278 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CHM_SendShakeHandFrameB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CHM_PGN_9728);  //����֡
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 3;
    
    BMSCANTxbuffB.Data[0] = 0x01; //�汾V1.1
    BMSCANTxbuffB.Data[1] = 0x01;
    BMSCANTxbuffB.Data[2] = 0x00; 
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CTS_SendSynTimeMessage
* Description	: ����ʱ��ͬ����ϢCTS
* Input			: 
* Output		: 
* Note(s)		: CTS���ģ�PGN=1792 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CTS_SendSynTimeMessageA(void)
{
    _BSPRTC_TIME rtcTime;
	
	OSSchedLock();
    if (BSP_RTCGetTime(&rtcTime) == FALSE)
    {
        rtcTime.Year   = 0x16;
        rtcTime.Month  = 0x07;
        rtcTime.Day    = 0x13;
        rtcTime.Hour   = 0x08;
        rtcTime.Minute = 0x00;
        rtcTime.Second = 0x00;
        rtcTime.Week   = 0x00;
    }
	rtcTime.Week = 0x20;  //�����Week�ֽ�Ĭ��Ϊ��ĸ�λ��

    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CTS_PGN_1792);  //����֡
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 7;//���ݳ���7
    //CTS���ķ��͵�ʱ�䶼��BCD��
    * ( (_BSPRTC_TIME *)BMSCANTxbuffA.Data) = rtcTime;
    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CTS_SendSynTimeMessage
* Description	: ����ʱ��ͬ����ϢCTS
* Input			: 
* Output		: 
* Note(s)		: CTS���ģ�PGN=1792 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CTS_SendSynTimeMessageB(void)
{
    _BSPRTC_TIME rtcTime;
	
	OSSchedLock();
    if (BSP_RTCGetTime(&rtcTime) == FALSE)
    {
        rtcTime.Year   = 0x16;
        rtcTime.Month  = 0x07;
        rtcTime.Day    = 0x13;
        rtcTime.Hour   = 0x08;
        rtcTime.Minute = 0x00;
        rtcTime.Second = 0x00;
        rtcTime.Week   = 0x00;
    }
	rtcTime.Week = 0x20;  //�����Week�ֽ�Ĭ��Ϊ��ĸ�λ��

    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CTS_PGN_1792);  //����֡
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 7;//���ݳ���7
    //CTS���ķ��͵�ʱ�䶼��BCD��
    * ( (_BSPRTC_TIME *)BMSCANTxbuffB.Data) = rtcTime;
    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CML_SendMaxVolandCur
* Description	: �����������������ģ���������ʣ����ʱ��
* Input			: 
* Output		: 
* Note(s)		: CML���ģ�PGN=2048 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CML_SendMaxVolandCurA(void)
{
    //���������������������CML
	
    APP_GetCMLContext(&BMS_CML_Context[GUN_A]);
    
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CML_PGN_2048);  //����֡
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    if (APP_GetGBType(GUN_A) == BMS_GB_2015)  //�¹��� 
        BMSCANTxbuffA.DLC = 8;//���ݳ���8
    else
        BMSCANTxbuffA.DLC = 6;//���ݳ���6

    APP_GetCMLContext(&BMS_CML_Context[GUN_A]);
    
    *( (_BMS_CML_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CML_Context[GUN_A];
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CML_SendMaxVolandCur
* Description	: �����������������ģ���������ʣ����ʱ��
* Input			: 
* Output		: 
* Note(s)		: CML���ģ�PGN=2048 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CML_SendMaxVolandCurB(void)
{
    //���������������������CML
    APP_GetCMLContext(&BMS_CML_Context[GUN_B]);
    
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CML_PGN_2048);  //����֡
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    if (APP_GetGBType(GUN_B) == BMS_GB_2015)  //�¹��� 
        BMSCANTxbuffB.DLC = 8;//���ݳ���8
    else
        BMSCANTxbuffB.DLC = 6;//���ݳ���6

    APP_GetCMLContext(&BMS_CML_Context[GUN_B]);
    
    *( (_BMS_CML_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CML_Context[GUN_B];
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevUnstandby
* Description	: ����CRO(0x00)����
* Input			: 
* Output		: 
* Note(s)		: Cro���ģ�PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevUnstandbyA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //����֡
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 1;//���ݳ���1
    
    //����δ׼����
    BMSCANTxbuffA.Data[0] = GB_CHARGESTATE_NOTREADY; 
    
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevUnstandby
* Description	: ����CRO(0x00)����
* Input			: 
* Output		: 
* Note(s)		: Cro���ģ�PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevUnstandbyB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //����֡
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 1;//���ݳ���1
    
    //����δ׼����
    BMSCANTxbuffB.Data[0] = GB_CHARGESTATE_NOTREADY; 
    
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevStandby
* Description	: ����CRO(0xAA)����
* Input			: 
* Output		: 
* Note(s)		: Cro���ģ�PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevStandbyA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId=0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //����֡
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 1;//���ݳ���1
    
    //����׼����
    BMSCANTxbuffA.Data[0] = GB_CHARGESTATE_STANDBY; 
    
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevStandby
* Description	: ����CRO(0xAA)����
* Input			: 
* Output		: 
* Note(s)		: Cro���ģ�PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevStandbyB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId=0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //����֡
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 1;//���ݳ���1
    
    //����׼����
    BMSCANTxbuffB.Data[0] = GB_CHARGESTATE_STANDBY; 
    
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CCS_DeviceChargeState
* Description	: ����ʱ��ͬ����Ϣ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CCS_DeviceChargeStateA(void)
{
	
    //��ȡCCS����
    APP_GetCSSContext(GUN_A,&BMS_CCS_Context[GUN_A]);
    
	OSSchedLock();
    BMSCANTxbuffA.StdId=0x00;
    BMSCANTxbuffA.ExtId=GB_CONFIGSEND_ID(GB_PRIO_6,GB_CCS_PGN_4608);  //����֡
    BMSCANTxbuffA.RTR=CAN_RTR_DATA;
    BMSCANTxbuffA.IDE=CAN_ID_EXT;
    if (APP_GetGBType(GUN_A) == BMS_GB_2015)  //�¹��� 
        BMSCANTxbuffA.DLC = 7;//���ݳ���8
    else
        BMSCANTxbuffA.DLC = 6;//���ݳ���6

    
    //��������
    * ( (_BMS_CCS_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CCS_Context[GUN_A];
    
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CCS_DeviceChargeState
* Description	: ����ʱ��ͬ����Ϣ
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CCS_DeviceChargeStateB(void)
{

    //��ȡCCS����
    APP_GetCSSContext(GUN_B,&BMS_CCS_Context[GUN_B]);
    
	OSSchedLock();
    BMSCANTxbuffB.StdId=0x00;
    BMSCANTxbuffB.ExtId=GB_CONFIGSEND_ID(GB_PRIO_6,GB_CCS_PGN_4608);  //����֡
    BMSCANTxbuffB.RTR=CAN_RTR_DATA;
    BMSCANTxbuffB.IDE=CAN_ID_EXT;
    if (APP_GetGBType(GUN_B) == BMS_GB_2015)  //�¹��� 
        BMSCANTxbuffB.DLC = 7;//���ݳ���8
    else
        BMSCANTxbuffB.DLC = 6;//���ݳ���6

    
    //��������
    * ( (_BMS_CCS_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CCS_Context[GUN_B];
    
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CST_DeviceEndCharge
* Description	: ��ֹ���
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CST_DeviceEndChargeA(void)
{
    //��ȡCST���ĵ�����
    APP_GetCSTContext(GUN_A,&BMS_CST_Context[GUN_A]);
    
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CST_PGN_6656);  //����֡
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 4;//���ݳ���4
    
    if (APP_GetGBType(GUN_A) == BMS_GB_2011)  //�ɹ��� 
        BMS_CST_Context[GUN_A].State0.EndChargeReason |= 0xc0;

    BMS_CST_Context[GUN_A].State1.EndChargeTroubleReason |= 0xf000;
    BMS_CST_Context[GUN_A].State2.EndChargeErroReason |= 0xf0;
    *( (_BMS_CST_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CST_Context[GUN_A]; 
    
    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
	
}

/***********************************************************************************************
* Function		: CST_DeviceEndCharge
* Description	: ��ֹ���
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CST_DeviceEndChargeB(void)
{
    //��ȡCST���ĵ�����
    APP_GetCSTContext(GUN_B,&BMS_CST_Context[GUN_B]);
    
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CST_PGN_6656);  //����֡
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 4;//���ݳ���4
    
    if (APP_GetGBType(GUN_B) == BMS_GB_2011)  //�ɹ��� 
        BMS_CST_Context[GUN_B].State0.EndChargeReason |= 0xc0;

    BMS_CST_Context[GUN_B].State1.EndChargeTroubleReason |= 0xf000;
    BMS_CST_Context[GUN_B].State2.EndChargeErroReason |= 0xf0;
    *( (_BMS_CST_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CST_Context[GUN_B]; 
    
    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
	
}

/***********************************************************************************************
* Function		: CSD_DeviceChargeEndStatisticsE
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CSD_DeviceChargeEndStatisticsA(void)
{
    //��ȡCSD����
    APP_GetCSDContext(GUN_A,&BMS_CSD_Context[GUN_A]);
    OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CSD_PGN_7424);  //����֡
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;//���ݳ���8
    
    *( (_BMS_CSD_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CSD_Context[GUN_A]; 

    BMSCANTxbuffA.Data[4] = CRM_DEVICE_CODENUM; //�������
    BMSCANTxbuffA.Data[5] = 0x00; 
    BMSCANTxbuffA.Data[6] = 0x00;
    BMSCANTxbuffA.Data[7] = 0x00;

    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CSD_DeviceChargeEndStatisticsE
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CSD_DeviceChargeEndStatisticsB(void)
{
    //��ȡCSD����
    APP_GetCSDContext(GUN_B,&BMS_CSD_Context[GUN_B]);
    OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CSD_PGN_7424);  //����֡
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;//���ݳ���8
    
    *( (_BMS_CSD_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CSD_Context[GUN_B]; 

    BMSCANTxbuffB.Data[4] = CRM_DEVICE_CODENUM; //�������
    BMSCANTxbuffB.Data[5] = 0x00; 
    BMSCANTxbuffB.Data[6] = 0x00;
    BMSCANTxbuffB.Data[7] = 0x00;

    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
}
/*****************************************************************************
* Function      : CEM_DeviceChargeErr
* Description   : CEM���ķ���
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016��6��6��  Ҷϲ��
*****************************************************************************/
void CEM_DeviceChargeErrA(void)
{   
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    //����֡
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_2, GB_CEM_PGN_7936);  
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 4;//���ݳ���4
    
    //CEM���Ľṹ�������Ѿ�������BMS�������̵ĳ�ʱ����ʱ��ֵ������ֻ�ܷ��ͼ���
    *( (_BMS_CEM_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CEM_Context[GUN_A]; 

    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();

}

/*****************************************************************************
* Function      : CEM_DeviceChargeErr
* Description   : CEM���ķ���
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016��6��6��  Ҷϲ��
*****************************************************************************/
void CEM_DeviceChargeErrB(void)
{   
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    //����֡
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_2, GB_CEM_PGN_7936);  
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 4;//���ݳ���4
    
    //CEM���Ľṹ�������Ѿ�������BMS�������̵ĳ�ʱ����ʱ��ֵ������ֻ�ܷ��ͼ���
    *( (_BMS_CEM_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CEM_Context[GUN_B]; 

    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();

}
/************************(C)COPYRIGHT 2016 �����Ƽ�*****END OF FILE****************************/
