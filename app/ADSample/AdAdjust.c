/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: adadjust.c
**硬          件: STM32
**创    建    人: wubin
**创  建  日  期: 2015-09-15
**最  新  版  本: V0.1
**描          述: 校正程序
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人:
**日          期:
**版          本:
**描          述:
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
_BSP_MESSAGE	*GuiyueOSQ[8];//指针数组GuiyueOSQ[]容纳指向各个消息的指针


extern _ADC_JUST_INFO ADCJustInfo[GUN_MAX];   //AD校准信息
/***********************************************************************************************
* Function      : Analyse_Protocol_Frame
* Description   : 解析协议帧
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 17/06/2010    叶喜雨
***********************************************************************************************/
INT8U Analyse_Protocol_Frame(INT8U gun,INT8U cmd, INT8U *p, INT16U Len)
{
	static INT32U data;  //绝缘检测电压是接触器后级电压的一般
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
		
//		  BSPADC_GUN_A,                	//A枪电压
//	BSPADC_GUN_B,					//B枪电压
//	BSPADC_BAT_A,					//A枪电池电压
//	BSPADC_BAT_B,					//B枪电池电压
//	BSPADC_JY_A,					//A枪绝缘检测
//	BSPADC_JY_B,					//B枪绝缘检测
//	BSPADC_T1_A,					//A枪枪头温度1
//	BSPADC_T2_A,					//A枪枪头温度2
//	BSPADC_T1_B,					//B枪枪头温度1
//	BSPADC_T2_B,					//B枪枪头温度2
		case JUST_SW_AFTER1:   //后级校准，把前级也一起校准了

			ADCJustInfo[gun].BatVoltKB.line_Y1 = data;
			//绝缘检测校准，是接触器后期电压的一半
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
		case JUST_SW_AFTER2:	//后级校准，把前级也一起校准了
				ADCJustInfo[gun].BatVoltKB.line_Y2 = data;
			//绝缘检测校准，是接触器后期电压的一半
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
* Function Name     ：TaskADJust
* Description       ：规约主任务
* Input             ：none
* Output            ：none
* Contributor       ：
* Date First Issued ：2020-07-15
******************************************************************************/
void TaskADJust(void *pdata)
{
    INT8U err;
    _BSP_MESSAGE *pMsg;
    pdata = pdata;
	JustOSQ_pevent  =  OSQCreate((void **)GuiyueOSQ, 8); // 建立事件(消息队列)
	if(JustOSQ_pevent == NULL)
	{
		return;
	}
    while(1)
    {
        pMsg = OSQPend(JustOSQ_pevent, SYS_DELAY_500ms, &err); //取消息,Ptr指向消息的存放地址
        if(OS_ERR_NONE == err)
        {
            switch(pMsg->MsgID)
            {
                case BSP_MSGID_DISP:     // 来自显示任务
                    Analyse_Protocol_Frame(pMsg->GunNum,pMsg->DivNum, pMsg->pData, pMsg->DataLen);
                    break;
                default:
                    break;

            }
        }
    }
}
/************************(C)COPYRIGHT 2015 汇誉科技*****END OF FILE****************************/
