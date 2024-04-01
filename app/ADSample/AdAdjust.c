/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: adadjust.c
**Ӳ          ��: STM32
**��    ��    ��: wubin
**��  ��  ��  ��: 2015-09-15
**��  ��  ��  ��: V0.1
**��          ��: У������
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��:
**��          ��:
**��          ��:
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "ucos_ii.h"
#include "sysconfig.h"
#include "bsp_conf.h"
#include "bsp.h"
#include "adadjust.h"
#include "controlmain.h"
#include "ADSamplemain.h"
#include "FlashDataDeal.h"

/* Private define-----------------------------------------------------------------------------*/

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/


OS_EVENT  		*JustOSQ_pevent; //
_BSP_MESSAGE	*GuiyueOSQ[8];//ָ������GuiyueOSQ[]����ָ�������Ϣ��ָ��


extern _ADC_JUST_INFO ADCJustInfo[GUN_MAX];   //ADУ׼��Ϣ
/***********************************************************************************************
* Function      : Analyse_Protocol_Frame
* Description   : ����Э��֡
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 17/06/2010    Ҷϲ��
***********************************************************************************************/
INT8U Analyse_Protocol_Frame(INT8U gun,INT8U cmd, INT8U *p, INT16U Len)
{
	static INT32U data;  //��Ե����ѹ�ǽӴ����󼶵�ѹ��һ��
	 _FLASH_OPERATION  FlashOper;
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}

	if(p == NULL || (Len != 4))
	{
		return FALSE;
	}
	data = p[0] | (p[1]<<8) | (p[2]<<16) |(p[3]<<24);
	
	FlashOper.DataID = PARA_JUST_ID;
	FlashOper.Len = PARA_JUST_FLLEN;
	FlashOper.ptr = (INT8U*)ADCJustInfo;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	switch(cmd)
	{
		
//			AdjustKB_NumStruct BatVoltKB;
//	AdjustKB_NumStruct HeadOutKB;
//	AdjustKB_NumStruct JYVolKB;
		
//		  BSPADC_GUN_A,                	//Aǹ��ѹ
//	BSPADC_GUN_B,					//Bǹ��ѹ
//	BSPADC_BAT_A,					//Aǹ��ص�ѹ
//	BSPADC_BAT_B,					//Bǹ��ص�ѹ
//	BSPADC_JY_A,					//Aǹ��Ե���
//	BSPADC_JY_B,					//Bǹ��Ե���
//	BSPADC_T1_A,					//Aǹǹͷ�¶�1
//	BSPADC_T2_A,					//Aǹǹͷ�¶�2
//	BSPADC_T1_B,					//Bǹǹͷ�¶�1
//	BSPADC_T2_B,					//Bǹǹͷ�¶�2
		case JUST_SW_AFTER1:   //��У׼����ǰ��Ҳһ��У׼��

			ADCJustInfo[gun].BatVoltKB.line_Y1 = data;
			//��Ե���У׼���ǽӴ������ڵ�ѹ��һ��
			data = data / 2;
			ADCJustInfo[gun].JYVolKB.line_Y1 = data;
		
			if(gun == GUN_A)
			{
				data = BSP_DataFilter(BSPADC_BAT_A);
				ADCJustInfo[gun].BatVoltKB.line_X1 = data;
			
				data = BSP_DataFilter(BSPADC_JY_A);
				ADCJustInfo[gun].JYVolKB.line_X1 = data;
			}
			else
			{
				data = BSP_DataFilter(BSPADC_BAT_B);
				ADCJustInfo[gun].BatVoltKB.line_X1 = data;
			
				data = BSP_DataFilter(BSPADC_JY_B);
				ADCJustInfo[gun].JYVolKB.line_X1 = data;
			}
			
		
			ADCJustInfo[gun].BatVoltKB.line_K =   (FP32)(ADCJustInfo[gun].BatVoltKB.line_Y2-ADCJustInfo[gun].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[gun].BatVoltKB.line_X2-ADCJustInfo[gun].BatVoltKB.line_X1);
			ADCJustInfo[gun].BatVoltKB.line_B =   (FP32)ADCJustInfo[gun].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[gun].BatVoltKB.line_K* (FP32)ADCJustInfo[gun].BatVoltKB.line_X2;
		
		
			ADCJustInfo[gun].JYVolKB.line_K =   (FP32)(ADCJustInfo[gun].JYVolKB.line_Y2-ADCJustInfo[gun].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[gun].JYVolKB.line_X2-ADCJustInfo[gun].JYVolKB.line_X1);
			ADCJustInfo[gun].JYVolKB.line_B =   (FP32)ADCJustInfo[gun].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[gun].JYVolKB.line_K* (FP32)ADCJustInfo[gun].JYVolKB.line_X2;
			APP_FlashOperation(&FlashOper);
			break;
		case JUST_SW_AFTER2:	//��У׼����ǰ��Ҳһ��У׼��
				ADCJustInfo[gun].BatVoltKB.line_Y2 = data;
			//��Ե���У׼���ǽӴ������ڵ�ѹ��һ��
			data = data / 2;
			ADCJustInfo[gun].JYVolKB.line_Y2 = data;
		
			if(gun == GUN_A)
			{
				data = BSP_DataFilter(BSPADC_BAT_A);
				ADCJustInfo[gun].BatVoltKB.line_X2 = data;
			
				data = BSP_DataFilter(BSPADC_JY_A);
				ADCJustInfo[gun].JYVolKB.line_X2 = data;
			}
			else
			{
				data = BSP_DataFilter(BSPADC_BAT_B);
				ADCJustInfo[gun].BatVoltKB.line_X2 = data;
			
				data = BSP_DataFilter(BSPADC_JY_B);
				ADCJustInfo[gun].JYVolKB.line_X2 = data;
			}
			
		
			ADCJustInfo[gun].BatVoltKB.line_K =   (FP32)(ADCJustInfo[gun].BatVoltKB.line_Y2-ADCJustInfo[gun].BatVoltKB.line_Y1)/ (FP32)(ADCJustInfo[gun].BatVoltKB.line_X2-ADCJustInfo[gun].BatVoltKB.line_X1);
			ADCJustInfo[gun].BatVoltKB.line_B =   (FP32)ADCJustInfo[gun].BatVoltKB.line_Y2 -  (FP32)ADCJustInfo[gun].BatVoltKB.line_K* (FP32)ADCJustInfo[gun].BatVoltKB.line_X2;
		
		
			ADCJustInfo[gun].JYVolKB.line_K =   (FP32)(ADCJustInfo[gun].JYVolKB.line_Y2-ADCJustInfo[gun].JYVolKB.line_Y1)/ (FP32)(ADCJustInfo[gun].JYVolKB.line_X2-ADCJustInfo[gun].JYVolKB.line_X1);
			ADCJustInfo[gun].JYVolKB.line_B =   (FP32)ADCJustInfo[gun].JYVolKB.line_Y2 -  (FP32)ADCJustInfo[gun].JYVolKB.line_K* (FP32)ADCJustInfo[gun].JYVolKB.line_X2;
			APP_FlashOperation(&FlashOper);
			break;

		case JUST_GUN1:
			ADCJustInfo[gun].HeadOutKB.line_Y1 = data;
			if(gun == GUN_A)
			{
				data = BSP_DataFilter(BSPADC_GUN_A);
				ADCJustInfo[gun].HeadOutKB.line_X1 = data;
			}
			else
			{
				data = BSP_DataFilter(BSPADC_GUN_B);
				ADCJustInfo[gun].HeadOutKB.line_X1 = data;
			}
		
			ADCJustInfo[gun].HeadOutKB.line_K =   (FP32)(ADCJustInfo[gun].HeadOutKB.line_Y2-ADCJustInfo[gun].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[gun].HeadOutKB.line_X2-ADCJustInfo[gun].HeadOutKB.line_X1);
			ADCJustInfo[gun].HeadOutKB.line_B =   (FP32)ADCJustInfo[gun].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[gun].HeadOutKB.line_K* (FP32)ADCJustInfo[gun].HeadOutKB.line_X2;
			APP_FlashOperation(&FlashOper);
			break;
		case JUST_GUN2:
			ADCJustInfo[gun].HeadOutKB.line_Y2 = data;
			if(gun == GUN_A)
			{
				data = BSP_DataFilter(BSPADC_GUN_A);
				ADCJustInfo[gun].HeadOutKB.line_X2 = data;
			}
			else
			{
				data = BSP_DataFilter(BSPADC_GUN_B);
				ADCJustInfo[gun].HeadOutKB.line_X2 = data;
			}
		
			ADCJustInfo[gun].HeadOutKB.line_K =   (FP32)(ADCJustInfo[gun].HeadOutKB.line_Y2-ADCJustInfo[gun].HeadOutKB.line_Y1)/ (FP32)(ADCJustInfo[gun].HeadOutKB.line_X2-ADCJustInfo[gun].HeadOutKB.line_X1);
			ADCJustInfo[gun].HeadOutKB.line_B =   (FP32)ADCJustInfo[gun].HeadOutKB.line_Y2 -  (FP32)ADCJustInfo[gun].HeadOutKB.line_K* (FP32)ADCJustInfo[gun].HeadOutKB.line_X2;
			APP_FlashOperation(&FlashOper);
			break;
		case 	JUST_T1_0_GUNA:
			ADCJustInfo[GUN_A].T1_0 = BSP_GetTemp1(GUN_A);
			APP_FlashOperation(&FlashOper);
			break;
		case	JUST_T1_100_GUNA:
			ADCJustInfo[GUN_A].T1_120 = BSP_GetTemp1(GUN_A);
			APP_FlashOperation(&FlashOper);
			break;
		case JUST_T2_0_GUNA:
			ADCJustInfo[GUN_A].T2_0 = BSP_GetTemp2(GUN_A);
			APP_FlashOperation(&FlashOper);
			break;
		case JUST_T2_100_GUNA:
			ADCJustInfo[GUN_A].T2_120 = BSP_GetTemp2(GUN_A);
			APP_FlashOperation(&FlashOper);
			break;
		#if(USER_GUN != USER_SINGLE_GUN)
		case 	JUST_T1_0_GUNB:
			ADCJustInfo[GUN_B].T1_0 = BSP_GetTemp1(GUN_B);
			APP_FlashOperation(&FlashOper);
			break;
		case	JUST_T1_100_GUNB:
			ADCJustInfo[GUN_B].T1_120 = BSP_GetTemp1(GUN_B);
			APP_FlashOperation(&FlashOper);
			break;
		case JUST_T2_0_GUNB:
			ADCJustInfo[GUN_B].T2_0 = BSP_GetTemp2(GUN_B);
			APP_FlashOperation(&FlashOper);
			break;
		case JUST_T2_100_GUNB:
			ADCJustInfo[GUN_B].T2_120 = BSP_GetTemp2(GUN_B);
			APP_FlashOperation(&FlashOper);
			break;	
			#endif
		default:
			break;
	}
	return TRUE;
}

/******************************************************************************
* Function Name     ��TaskADJust
* Description       ����Լ������
* Input             ��none
* Output            ��none
* Contributor       ��
* Date First Issued ��2020-07-15
******************************************************************************/
void TaskADJust(void *pdata)
{
    INT8U err;
    _BSP_MESSAGE *pMsg;
    pdata = pdata;
	JustOSQ_pevent  =  OSQCreate((void **)GuiyueOSQ, 8); // �����¼�(��Ϣ����)
	if(JustOSQ_pevent == NULL)
	{
		return;
	}
    while(1)
    {
        pMsg = OSQPend(JustOSQ_pevent, SYS_DELAY_500ms, &err); //ȡ��Ϣ,Ptrָ����Ϣ�Ĵ�ŵ�ַ
        if(OS_ERR_NONE == err)
        {
            switch(pMsg->MsgID)
            {
                case BSP_MSGID_DISP:     // ������ʾ����
                    Analyse_Protocol_Frame(pMsg->GunNum,pMsg->DivNum, pMsg->pData, pMsg->DataLen);
                    break;
                default:
                    break;

            }
        }
    }
}
/************************(C)COPYRIGHT 2015 �����Ƽ�*****END OF FILE****************************/
