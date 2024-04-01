/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : bsp.c
* Author            : 叶喜雨
* Date First Issued : 10/12/2010
* Version           : V0.1
* Description       : 大部分驱动都可以在这里初始化
*----------------------------------------历史版本信息-------------------------------------------
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
INT8U TempJustMode = 0; //0 不校验， 1校验
extern _ADC_JUST_INFO ADCJustInfo[GUN_MAX];   //AD校准信息
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function      : GetRM_RemoteSignal
* Description   : 获取遥测
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
* Description   : 获取温度
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
* Description   : 温度校验模式选择
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
* Description   : 电池是否反接 
* Input         :
* Output        :TRUE 反接  FALSE 未反接  基准电压未0.529V 最大电压未3.3v  阈值 = 0.529/3.3*4096  误差为百分之20，则为590
* Note(s)       :
* Contributor   : 2016-06-06 
***********************************************************************************************/
INT8U GetRS_BatReverse(INT8U gun)
{
	INT32U cur;
	if(gun >= GUN_MAX)
	{
		return TRUE;		//电池反接
		
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
		return TRUE;		//电池反接
	}
	//电池未反接
	return FALSE;
}

/***********************************************************************************************
* Function      : YCRealTime_Check
* Description   : 遥测实时
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
		RM_ImForm[gun].DC_OutVolt = BSP_GetOUTBAT((_GUN_NUM)gun);	 	//输出电压(接触器后级电压)
		RM_ImForm[gun].DC_HandVolt = BSP_GetHandVolt((_GUN_NUM)gun);	//握手电压
		RM_ImForm[gun].DC_GunT1Temp = BSP_GetTemp1((_GUN_NUM)gun);  //温度1 
		RM_ImForm[gun].DC_GunT2Temp = BSP_GetTemp2((_GUN_NUM)gun);	//温度2
		RM_ImForm[gun].JY_Vol = BSP_GetJYVol((_GUN_NUM)gun);  		//绝缘检测
	}
}
/***********************************************************************************************
* Function      : Return_HandSta
* Description   : 握手状态
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
    else if(RM_ImForm[gun].DC_HandVolt >= 300 && RM_ImForm[gun].DC_HandVolt <= 500)//测试PE断线。特殊处理
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
* Description   : 获取继电器后级电压
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  叶喜雨
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
* Description   : 获取绝缘缘检测电压
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  叶喜雨
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
* Description   : 获取枪头握手电压
* Input         :
* Output        :
* Note(s)       :
* Contributor   :2020-7-16 叶喜雨
***********************************************************************************************/
INT8U GetRM_HandVolt(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return  INTERFACE_UNCONNECTED;
	}
    return Return_HandSta(gun);//修改为枚举类型，方便外部使用，不用判断
}
/***********************************************************************************************
* Function      : GetYC_GunTailTemp
* Description   : 获取  小数点2位，大概数值，100度的时候数值如果校准过来，就比较准，主要用于显示用
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  叶喜雨
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
	temp = (1200000 / (ADCJustInfo[gun].T1_120  -  ADCJustInfo[gun].T1_0)) * (RM_ImForm[gun].DC_GunT1Temp - ADCJustInfo[gun].T1_0); //120°
	temp = temp/100; //小数点2位
    return temp;
}
/***********************************************************************************************
* Function      : GetYC_GunHeadTemp
* Description   : 获取枪头温度  大概数值，100度的时候数值如果校准过来，就比较准 主要用于显示用
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020-7-16  叶喜雨
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
	temp = (1200000 / (ADCJustInfo[gun].T2_120  -  ADCJustInfo[gun].T2_0)) * (RM_ImForm[gun].DC_GunT2Temp - ADCJustInfo[gun].T2_0); //100°
	temp = temp/100; //小数点2位
    return temp;
}

/***********************************************************************************************
* Function      : TaskSample
* Description   : 电压电流采样处理任务
* Input           :
* Output          :
* Note(s)         :
* Contributor   :
***********************************************************************************************/
void TaskSample(void *pdata)   //定时执行
{
    OSTimeDly(SYS_DELAY_4s);

    while(1)
    {
        RMRealTime_Check();
		OSTimeDly(SYS_DELAY_50ms);//20161013 Test
    }
}



/************************(C)COPYRIGHT 2020 汇誉科技*****END OF FILE****************************/
