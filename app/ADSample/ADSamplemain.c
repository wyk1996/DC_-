/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName          : bsp.c
* Author            : Ҷϲ��
* Date First Issued : 10/12/2010
* Version           : V0.1
* Description       : �󲿷������������������ʼ��
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History           :
* //2010            : V
* Description       :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp.h"
#include "ADSamplemain.h"
#include "bsp_adc.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//volatile SystemStatus Sys;
//volatile YC_Member YC;
SYS_RemoteMeter		RM_ImForm[GUN_MAX];
SYS_RemoteSignal	RS_ImForm[GUN_MAX];
INT8U TempJustMode = 0; //0 ��У�飬 1У��
extern _ADC_JUST_INFO ADCJustInfo[GUN_MAX];   //ADУ׼��Ϣ
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function      : GetRM_RemoteSignal
* Description   : ��ȡң��
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-06-06 
***********************************************************************************************/
void GetRM_RemoteSignal(SYS_RemoteMeter *data)
{
    memcpy(data , &RM_ImForm, sizeof(SYS_RemoteMeter));
}

/***********************************************************************************************
* Function      : Get_TempAdjust
* Description   : ��ȡ�¶�
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-06-06 
***********************************************************************************************/
INT8U Get_TempAdjust(void)
{
    return TempJustMode;
}
/***********************************************************************************************
* Function      : Set_TempAdjust
* Description   : �¶�У��ģʽѡ��
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2016-06-06 
***********************************************************************************************/
void Set_TempAdjust(INT8U dat)
{
    TempJustMode = dat;
}

/***********************************************************************************************
* Function      : GetRS_BatReverse
* Description   : ����Ƿ񷴽� 
* Input         :
* Output        :TRUE ����  FALSE δ����  ��׼��ѹδ0.529V ����ѹδ3.3v  ��ֵ = 0.529/3.3*4096  ���Ϊ�ٷ�֮20����Ϊ590
* Note(s)       :
* Contributor   : 2016-06-06 
***********************************************************************************************/
INT8U GetRS_BatReverse(INT8U gun)
{
	INT32U cur;
	if(gun >= GUN_MAX)
	{
		return TRUE;		//��ط���
		
	}
	if(gun ==GUN_A)
	{
		cur  = BSP_DataFilter(BSPADC_BAT_A);
	}
	else
	{
		cur  = BSP_DataFilter(BSPADC_BAT_B);
	}
    if(cur < 525)
	{
		return TRUE;		//��ط���
	}
	//���δ����
	return FALSE;
}

/***********************************************************************************************
* Function      : YCRealTime_Check
* Description   : ң��ʵʱ
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-06-06
***********************************************************************************************/
void RMRealTime_Check(void)
{
	INT8U gun;
	
	for(gun = 0;gun < GUN_MAX;gun++)
	{
		RM_ImForm[gun].DC_OutVolt = BSP_GetOUTBAT((_GUN_NUM)gun);	 	//�����ѹ(�Ӵ����󼶵�ѹ)
		RM_ImForm[gun].DC_HandVolt = BSP_GetHandVolt((_GUN_NUM)gun);	//���ֵ�ѹ
		RM_ImForm[gun].DC_GunT1Temp = BSP_GetTemp1((_GUN_NUM)gun);  //�¶�1 
		RM_ImForm[gun].DC_GunT2Temp = BSP_GetTemp2((_GUN_NUM)gun);	//�¶�2
		RM_ImForm[gun].JY_Vol = BSP_GetJYVol((_GUN_NUM)gun);  		//��Ե���
	}
}
/***********************************************************************************************
* Function      : Return_HandSta
* Description   : ����״̬
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16 
***********************************************************************************************/
INT8U Return_HandSta(INT8U gun)
{

	if(gun >= GUN_MAX)
	{
		return INTERFACE_UNCONNECTED;
	}
    if(RM_ImForm[gun].DC_HandVolt >= 1120 && RM_ImForm[gun].DC_HandVolt <= 1280)
    {
        return INTERFACE_UNCONNECTED;
    }
    else if(RM_ImForm[gun].DC_HandVolt >= 520 && RM_ImForm[gun].DC_HandVolt <= 680)
    {
        return INTERFACE_UNFULCONNECTED;
    }
    else if(RM_ImForm[gun].DC_HandVolt >= 300 && RM_ImForm[gun].DC_HandVolt <= 500)//����PE���ߡ����⴦��
    {
        return  INTERFACE_CONNECTED;
    }
    else
    {
        return INTERFACE_UNKNOW;
    }
}
/***********************************************************************************************
* Function      : GetYC_DC_OutVolt
* Description   : ��ȡ�̵����󼶵�ѹ
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  Ҷϲ��
***********************************************************************************************/
INT32U GetRM_DCOutVolt(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return  0;
	}
    return RM_ImForm[gun].DC_OutVolt;
}

/***********************************************************************************************
* Function      : GetJY_DCOutVolt
* Description   : ��ȡ��ԵԵ����ѹ
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  Ҷϲ��
***********************************************************************************************/
INT32U GetJY_DCOutVolt(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return  0;
	}
    return RM_ImForm[gun].JY_Vol;
}

/***********************************************************************************************
* Function      : GetYC_HandVolt
* Description   : ��ȡǹͷ���ֵ�ѹ
* Input         :
* Output        :
* Note(s)       :
* Contributor   :2020-7-16 Ҷϲ��
***********************************************************************************************/
INT8U GetRM_HandVolt(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return  INTERFACE_UNCONNECTED;
	}
    return Return_HandSta(gun);//�޸�Ϊö�����ͣ������ⲿʹ�ã������ж�
}
/***********************************************************************************************
* Function      : GetYC_GunTailTemp
* Description   : ��ȡ  С����2λ�������ֵ��100�ȵ�ʱ����ֵ���У׼�������ͱȽ�׼����Ҫ������ʾ��
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  Ҷϲ��
***********************************************************************************************/
INT32U GetRM_GunT1Temp(_GUN_NUM gun)
{
	INT32U temp;
	if(gun >= GUN_MAX)
	{
		return  0;
	}
	if(ADCJustInfo[gun].T1_0 >= ADCJustInfo[gun].T1_120)
	{
		return 0;
	}
	if(ADCJustInfo[gun].T1_0 >=  RM_ImForm[gun].DC_GunT1Temp)
	{
		return 0;
	}
	temp = (1200000 / (ADCJustInfo[gun].T1_120  -  ADCJustInfo[gun].T1_0)) * (RM_ImForm[gun].DC_GunT1Temp - ADCJustInfo[gun].T1_0); //120��
	temp = temp/100; //С����2λ
    return temp;
}
/***********************************************************************************************
* Function      : GetYC_GunHeadTemp
* Description   : ��ȡǹͷ�¶�  �����ֵ��100�ȵ�ʱ����ֵ���У׼�������ͱȽ�׼ ��Ҫ������ʾ��
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  Ҷϲ��
***********************************************************************************************/
INT32U GetRM_GunT2Temp(_GUN_NUM gun)
{
	
	INT32U temp;
	if(gun >= GUN_MAX)
	{
		return  0;
	}
	if(ADCJustInfo[gun].T2_0 >= ADCJustInfo[gun].T2_120)
	{
		return 0;
	}
	if(ADCJustInfo[gun].T2_0 >=  RM_ImForm[gun].DC_GunT2Temp)
	{
		return 0;
	}
	temp = (1200000 / (ADCJustInfo[gun].T2_120  -  ADCJustInfo[gun].T2_0)) * (RM_ImForm[gun].DC_GunT2Temp - ADCJustInfo[gun].T2_0); //100��
	temp = temp/100; //С����2λ
    return temp;
}

/***********************************************************************************************
* Function      : TaskSample
* Description   : ��ѹ����������������
* Input           :
* Output          :
* Note(s)         :
* Contributor   :
***********************************************************************************************/
void TaskSample(void *pdata)   //��ʱִ��
{
    OSTimeDly(SYS_DELAY_4s);

    while(1)
    {
        RMRealTime_Check();
		OSTimeDly(SYS_DELAY_50ms);//20161013 Test
    }
}



/************************(C)COPYRIGHT 2020 �����Ƽ�*****END OF FILE****************************/
