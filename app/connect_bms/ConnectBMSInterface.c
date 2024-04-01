/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: connectbmsinterface.c
* Author			: Ҷϲ��
* Date First Issued	: 23/05/2016
* Version			: V0.1
* Description		: BMS����֡�����Լ��ӿ��ļ�
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
#include "connectbmsframe.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
//���µ�������BMS���͹���������
//BHM��������
_BMS_BHM_CONTEXT BMS_BHM_Context[GUN_MAX];
//BRM��������
_BMS_BRM_CONTEXT BMS_BRM_Context[GUN_MAX];
//BCP��������
_BMS_BCP_CONTEXT BMS_BCP_Context[GUN_MAX];
//BRO��������
_BMS_BRO_CONTEXT BMS_BRO_Context[GUN_MAX];
//BCL��������
_BMS_BCL_CONTEXT BMS_BCL_Context[GUN_MAX];
//BCS��������
_BMS_BCS_CONTEXT BMS_BCS_Context[GUN_MAX];
//BSM��������
_BMS_BSM_CONTEXT BMS_BSM_Context[GUN_MAX];
//BST��������
_BMS_BST_CONTEXT BMS_BST_Context[GUN_MAX];
//�������׶�
//BSD��������
_BMS_BSD_CONTEXT BMS_BSD_Context[GUN_MAX];
//������
//BEM��������,Ԫ���ñ��ļ����д�����»������֣������е㲻���ϱ�̹淶������ͳһ����ʶ���Ժ�Щ
_BMS_BEM_CONTEXT BMS_BEM_Context[GUN_MAX];

//���µ������ǳ�������Ҫ���͵�����
//CML��������
_BMS_CML_CONTEXT BMS_CML_Context[GUN_MAX];
//�������״̬����
_BMS_CCS_CONTEXT BMS_CCS_Context[GUN_MAX];
//������ֹ��籨��
_BMS_CST_CONTEXT BMS_CST_Context[GUN_MAX];
//����ͳ�Ʊ�������
_BMS_CSD_CONTEXT BMS_CSD_Context[GUN_MAX];
//��������ԭ��
_BMS_CEM_CONTEXT BMS_CEM_Context[GUN_MAX];
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: CanProtocolOperationRTS
* Description	: RTS֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationRTSA(CanInterfaceStruct *controlstrcut)
{
	INT16U	PGN_Temp=0;
    _BMS_RTS_FRAME_INF *pStruct;
    
    if((controlstrcut->DataID) == (GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236))) //RTS֡ID
    {
        if(controlstrcut->len > 8)
            controlstrcut->len = 8;//����������
        //��Чֵ����������ṹ��
        pStruct = (_BMS_RTS_FRAME_INF *)&controlstrcut->pdata[0];
		//if (pStruct->messagepagenum > 8) //�������8�������Ӧ����7��
		if (pStruct->messagepagenum > 255) //����EQ PGN15 �����0X18��
		{
			return FALSE;
		}
        BMS_RTS_FrameInf[GUN_A] = *pStruct; //�ѽ��յ���RTS���浽BMS_RTS_FrameInf
        Reply_BMSFrame_CTSA();//����BMS������RTS֡�Ļظ�֡��Ȼ������ҳ���չ���
		
		PGN_Temp=((INT16U)((INT16U)(BMS_RTS_FrameInf[GUN_A].paranumber[1]<<8)) | (BMS_RTS_FrameInf[GUN_A].paranumber[0]));
		
		if((PGN_Temp == GB_BMV_PGN_5376)||(PGN_Temp == GB_BMT_PGN_5632)||(PGN_Temp == GB_BSP_PGN_5888))
		{//���⴦��BMV\BMT\BSP
			Special_PGN[GUN_A][BMS_RTS_FrameInf[GUN_A].paranumber[1]-0x15].Get_Flag = 0x01;
			Special_PGN[GUN_A][BMS_RTS_FrameInf[GUN_A].paranumber[1]-0x15].Count_Delay = 0x00;
		}
//        BMS_CONNECT_StateSet(BMS_RTS_SUCCESS);//����״̬Ϊ����RTS�ɹ�
        return TRUE;
    }
    return FALSE;
}

/***********************************************************************************************
* Function		: CanProtocolOperationRTS
* Description	: RTS֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationRTSB(CanInterfaceStruct *controlstrcut)
{
	INT16U	PGN_Temp=0;
    _BMS_RTS_FRAME_INF *pStruct;
    
    if((controlstrcut->DataID) == (GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236))) //RTS֡ID
    {
        if(controlstrcut->len > 8)
            controlstrcut->len = 8;//����������
        //��Чֵ����������ṹ��
        pStruct = (_BMS_RTS_FRAME_INF *)&controlstrcut->pdata[0];
		//if (pStruct->messagepagenum > 8) //�������8�������Ӧ����7��
		if (pStruct->messagepagenum > 255) //����EQ PGN15 �����0X18��
		{
			return FALSE;
		}
        BMS_RTS_FrameInf[GUN_B] = *pStruct; //�ѽ��յ���RTS���浽BMS_RTS_FrameInf
        Reply_BMSFrame_CTSB();//����BMS������RTS֡�Ļظ�֡��Ȼ������ҳ���չ���
		
		PGN_Temp=((INT16U)((INT16U)(BMS_RTS_FrameInf[GUN_B].paranumber[1]<<8)) | (BMS_RTS_FrameInf[GUN_B].paranumber[0]));
		
		if((PGN_Temp == GB_BMV_PGN_5376)||(PGN_Temp == GB_BMT_PGN_5632)||(PGN_Temp == GB_BSP_PGN_5888))
		{//���⴦��BMV\BMT\BSP
			Special_PGN[GUN_B][BMS_RTS_FrameInf[GUN_B].paranumber[1]-0x15].Get_Flag = 0x01;
			Special_PGN[GUN_B][BMS_RTS_FrameInf[GUN_B].paranumber[1]-0x15].Count_Delay = 0x00;
		}
//        BMS_CONNECT_StateSet(BMS_RTS_SUCCESS);//����״̬Ϊ����RTS�ɹ�
        return TRUE;
    }
    return FALSE;
}

/***********************************************************************************************
* Function		: CanProtocolOperationBHM
* Description	: BHM֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBHMA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BHM_CONTEXT *pStruct;
    
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BHM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BHM_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BHM_SUCCESS);//����״̬Ϊ����BHM�ɹ�
    return TRUE;
 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBHM
* Description	: BHM֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBHMB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BHM_CONTEXT *pStruct;
    
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BHM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BHM_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BHM_SUCCESS);//����״̬Ϊ����BHM�ɹ�
    return TRUE;
 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBRM
* Description	: ֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBRMA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(_BMS_BRM_CONTEXT)) 
        controlstrcut->len = sizeof(_BMS_BRM_CONTEXT);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BRM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRM_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BRM_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE;
}

/***********************************************************************************************
* Function		: CanProtocolOperationBRM
* Description	: ֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBRMB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(_BMS_BRM_CONTEXT)) 
        controlstrcut->len = sizeof(_BMS_BRM_CONTEXT);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BRM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRM_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BRM_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE;
  
}
/***********************************************************************************************
* Function		: CanProtocolOperationBCP
* Description	: ֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBCPA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCP_CONTEXT *pStruct;
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BCP_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCP_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BCP_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCP
* Description	: ֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBCPB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCP_CONTEXT *pStruct;
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BCP_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCP_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BCP_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBRO
* Description	: ֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBROA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRO_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BRO_Context))
        controlstrcut->len = sizeof(BMS_BRO_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BRO_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRO_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BRO_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBRO
* Description	: ֡���մ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBROB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRO_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BRO_Context))
        controlstrcut->len = sizeof(BMS_BRO_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BRO_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRO_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BRO_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCL
* Description	: ��س��������BCL
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBCLA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCL_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BCL_Context))
        controlstrcut->len = sizeof(BMS_BCL_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BCL_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCL_Context[GUN_A] = *pStruct;
	
	
			//20211118
//	if(BMS_BCL_Context[GUN_A].DemandVol != 5500)
//	{
//		printf("GUN_A BCL  ERR,%d",BMS_BCL_Context[GUN_A].DemandVol);
//	}
	
	
    BMS_CONNECT_StateSet(GUN_A,BMS_BCL_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCL
* Description	: ��س��������BCL
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBCLB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCL_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BCL_Context))
        controlstrcut->len = sizeof(BMS_BCL_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BCL_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCL_Context[GUN_B] = *pStruct;
	
	
	
	
		//20211118
//	if(BMS_BCL_Context[GUN_B].DemandVol != 3000)
//	{
//		printf("GUN_B BCL  ERR,%d",BMS_BCL_Context[GUN_B].DemandVol);
//	}
//	
	
	
    BMS_CONNECT_StateSet(GUN_B,BMS_BCL_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBCS
* Description	: ��س����״̬BCS
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBCSA(CanInterfaceStruct *controlstrcut)
{
	_BMS_BCS_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BCS_Context))
		controlstrcut->len = sizeof(BMS_BCS_Context);//����������
	//��Чֵ����������ṹ��
	pStruct = (_BMS_BCS_CONTEXT *)&controlstrcut->pdata[0];
	
	//BMS_BCS_Context[GUN_A] = *pStruct;
	memcpy(&BMS_BCS_Context[GUN_A],controlstrcut->pdata,sizeof(_BMS_BCS_CONTEXT));
	

	//20211118
//	if(BMS_BCS_Context[GUN_A].SOC != 50)
//	{
//		printf("GUN_A BCS  ERR,%d",BMS_BCS_Context[GUN_A].SOC);
//	}
	
	
	
	BMS_CONNECT_StateSet(GUN_A,BMS_BCS_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCS
* Description	: ��س����״̬BCS
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBCSB(CanInterfaceStruct *controlstrcut)
{
	_BMS_BCS_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BCS_Context))
		controlstrcut->len = sizeof(BMS_BCS_Context);//����������
	//��Чֵ����������ṹ��
	pStruct = (_BMS_BCS_CONTEXT *)&controlstrcut->pdata[0];
	
	//BMS_BCS_Context[GUN_B] = *pStruct;
	memcpy(&BMS_BCS_Context[GUN_B],controlstrcut->pdata,sizeof(_BMS_BCS_CONTEXT));
	
	
	//20211118
//	if(BMS_BCS_Context[GUN_B].SOC != 80)
//	{
//		printf("GUN_B BCS  ERR,%d",BMS_BCS_Context[GUN_B].SOC);
//	}
	
	
	
	BMS_CONNECT_StateSet(GUN_B,BMS_BCS_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBSM
* Description	: ��س����״̬BSM
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSMA(CanInterfaceStruct *controlstrcut)
{
	_BMS_BSM_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BSM_Context))
		controlstrcut->len = sizeof(BMS_BSM_Context);//����������
	//��Чֵ����������ṹ��
	pStruct = (_BMS_BSM_CONTEXT *)&controlstrcut->pdata[0];
	BMS_BSM_Context[GUN_A] = *pStruct;
	BMS_CONNECT_StateSet(GUN_A,BMS_BSM_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBSM
* Description	: ��س����״̬BSM
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSMB(CanInterfaceStruct *controlstrcut)
{
	_BMS_BSM_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BSM_Context))
		controlstrcut->len = sizeof(BMS_BSM_Context);//����������
	//��Чֵ����������ṹ��
	pStruct = (_BMS_BSM_CONTEXT *)&controlstrcut->pdata[0];
	BMS_BSM_Context[GUN_B] = *pStruct;
	BMS_CONNECT_StateSet(GUN_B,BMS_BSM_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBMV
* Description	: ���嶯�����ص�ѹ
* Input			: 
* Return		: 
* Note(s)		: ���嶯�����ص�ѹ��ֻ�������߼����ƣ��������������ݣ�ֻ������
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBMVA(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMV_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//����������
//    //��Чֵ����������ṹ��
//    pStruct = (_BMS_BMV_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMV_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BMV_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBMV
* Description	: ���嶯�����ص�ѹ
* Input			: 
* Return		: 
* Note(s)		: ���嶯�����ص�ѹ��ֻ�������߼����ƣ��������������ݣ�ֻ������
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBMVB(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMV_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//����������
//    //��Чֵ����������ṹ��
//    pStruct = (_BMS_BMV_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMV_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BMV_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBMT
* Description	: ���������¶�
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBMTA(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMT_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//����������
//    //��Чֵ����������ṹ��
//    pStruct = (_BMS_BMT_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMT_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BMT_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBMT
* Description	: ���������¶�
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBMTB(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMT_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//����������
//    //��Чֵ����������ṹ��
//    pStruct = (_BMS_BMT_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMT_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BMT_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBSP
* Description	: ��������Ԥ������
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSPA(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BSP_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//����������
//    //��Чֵ����������ṹ��
//    pStruct = (_BMS_BSP_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BSP_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BSP_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBSP
* Description	: ��������Ԥ������
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSPB(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BSP_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//����������
//    //��Чֵ����������ṹ��
//    pStruct = (_BMS_BSP_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BSP_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BSP_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBST
* Description	: BMS��ֹ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSTA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BST_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BST_Context))
        controlstrcut->len = sizeof(BMS_BST_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BST_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BST_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BST_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBST
* Description	: BMS��ֹ���
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSTB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BST_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BST_Context))
        controlstrcut->len = sizeof(BMS_BST_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BST_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BST_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BST_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBSD
* Description	: BMSͳ������
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSDA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BSD_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BSD_Context))
        controlstrcut->len = sizeof(BMS_BSD_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BSD_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BSD_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BSD_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBSD
* Description	: BMSͳ������
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBSDB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BSD_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BSD_Context))
        controlstrcut->len = sizeof(BMS_BSD_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BSD_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BSD_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BSD_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBEM
* Description	: BMS������
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBEMA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BEM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BEM_Context))
        controlstrcut->len = sizeof(BMS_BEM_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BEM_CONTEXT *)(&controlstrcut->pdata[0]);
    BMS_BEM_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BEM_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBEM
* Description	: BMS������
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationBEMB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BEM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BEM_Context))
        controlstrcut->len = sizeof(BMS_BEM_Context);//����������
    //��Чֵ����������ṹ��
    pStruct = (_BMS_BEM_CONTEXT *)(&controlstrcut->pdata[0]);
    BMS_BEM_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BEM_SUCCESS);//����״̬Ϊ�ɹ�
    return TRUE; 
}
/************************(C)COPYRIGHT 2016 �����Ƽ�*****END OF FILE****************************/
