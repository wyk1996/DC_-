/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : bsp_adc.h
* Author            : 
* Date First Issued : 
* Version           : 
* Description       : AD采样头文件，建立一个y=kx+b的模型
*----------------------------------------历史版本信息-------------------------------------------
* History           :
* Description       :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __BSP_ADC_H_
#define __BSP_ADC_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp_conf.h"
/* Private define-----------------------------------------------------------------------------*/

typedef enum
{
    BSPADC_GUN_A,                	//A枪电压
	BSPADC_GUN_B,					//B枪电压
	BSPADC_BAT_A,					//A枪电池电压
	BSPADC_BAT_B,					//B枪电池电压
	BSPADC_JY_A,					//A枪绝缘检测
	BSPADC_JY_B,					//B枪绝缘检测
	BSPADC_T1_A,					//A枪枪头温度1
	BSPADC_T2_A,					//A枪枪头温度2
	BSPADC_T1_B,					//B枪枪头温度1
	BSPADC_T2_B,					//B枪枪头温度2
    BSPADC__MAX,                                           		 //1:最大数目
} _BSP_SAMPLE_TYPE;

#define ADCCOUNTPERMS           (20u)                          //每个ADC采样计数
#define ADC_MAX_NUM         	BSPADC__MAX                        	//要采样的ADC个数
#define ADCOUTNBUFFLENGTH       (ADCCOUNTPERMS * ADC_MAX_NUM)  //保持ADC采样值内存的大小
#define ADC1_DR_Address         ((u32)ADC1 + 0x4c)             
/* Private typedef----------------------------------------------------------------------------*/
__packed typedef struct  LINEKB
{
    INT32U line_X1;
    INT32U line_Y1;
    INT32U line_X2;
    INT32U line_Y2;
    FP32 line_K;
    FP32 line_B;
} AdjustKB_NumStruct;

__packed typedef struct
{
	AdjustKB_NumStruct BatVoltKB;
	AdjustKB_NumStruct HeadOutKB;
	AdjustKB_NumStruct JYVolKB;
	INT16U T1_0;		//0°时枪温度采样的原始值
	INT16U T1_120;	//120°时枪温度采样的原始值
	INT16U T2_0;		//0°时枪温度采样的原始值
	INT16U T2_120;	//120°时枪温度采样的原始值
}_ADC_JUST_INFO;

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : BSP_GetADSampleValue
* Description  : 获取AD采样值，中位值数值滤波方法
* Input        : _BSP_SAMPLE_TYPE SampleType  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月23日        Yxy
*****************************************************************************/
INT16U BSP_GetADSampleValue(_BSP_SAMPLE_TYPE SampleType);

/***********************************************************************************************
* Function		: BSP_ADCInit
* Description	: AD采样初始化,DMA配置：在传输过程中Memory的地址是增加的，外设(ADC的采样数据地址)的地址不增加，
                  表示仅传送该外设使用的通道X的采样数据，未使用的通道的采样数据REG不会传输。
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 10/12/2010
***********************************************************************************************/
INT8U BSP_ADCInit(void);

/***********************************************************************************************
* Function		: BSP_GetADCJustInfo
* Description	: 更新KB数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2015-08-11 Yxy
***********************************************************************************************/
_ADC_JUST_INFO * BSP_GetADCJustInfo(_GUN_NUM gun);

/***********************************************************************************************
* Function      : BSP_GetHandshakeSignal
* Description   : 读取握手信号
* Input         :
* Output        :
* Note(s)       : Break   U = 1.7(1±12%)V;               adcvalue = U/2.5*4096 = 2785(1±12%)
              Fully connected  U = 1.22(1±12%)V; adcvalue = U/2.5*4096   实际1.2V
               Not fully connected  U = 0.91(1±12%)V ;    adcvalue = U/2.5*4096

* Note(s)       : Break   U = 1.7(1±12%)V;               adcvalue = U/2.5*4096 = 2785(1±12%)
              Not fully connected  U = 0.91(1±12%)V ;    adcvalue = U/2.5*4096=1490
              Fully connected  U = 0.6(1±3)V; adcvalue = U/2.5*4096 = 938(1±3%)
* Contributor   : 
***********************************************************************************************/
INT32U BSP_GetHandVolt(_GUN_NUM gun);

/***********************************************************************************************
* Function      : BSP_GetOUT1DC
* Description   :电池电压
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetOUTBAT(_GUN_NUM gun);

/***********************************************************************************************
* Function      : BSP_GetPT1000N01
* Description   : 获取温度1
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetTemp1(_GUN_NUM gun);

/***********************************************************************************************
* Function      : BSP_GetPT1000N01
* Description   : 获取温度2
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetTemp2(_GUN_NUM gun);

/***********************************************************************************************
* Function      : BSP_GetJYVol
* Description   : 获取绝缘检测电压
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetJYVol(_GUN_NUM gun);

/*****************************************************************************
* Function     : BSP_GetADSampleValue
* Description  : 获取AD采样值，中位值数值滤波方法
* Input        : _BSP_SAMPLE_TYPE SampleType  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月23日        
*****************************************************************************/
INT32U BSP_DataFilter(_BSP_SAMPLE_TYPE SampleType);
#endif  //__BSP_ADC_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
