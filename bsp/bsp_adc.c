/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : bsp_adc.c
* Author            : 
* Date First Issued :
* Version           : 
* Description       : 采样电压，电流驱动程序
*----------------------------------------历史版本信息-------------------------------------------
* History           :
* Description       :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "bsp_adc.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//ADC采样值保存缓存
INT16U ADC_ConvertedValue[ADCCOUNTPERMS][ADC_MAX_NUM];

_ADC_JUST_INFO ADCJustInfo[GUN_MAX];   //AD校准信息
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

/*****************************************************************************
* Function     : BubbleSort
* Description  : 冒泡法排序
* Input        : INT16U * const buf  
                 INT16U len         
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月23日        
*****************************************************************************/
static INT8U BubbleSort(INT16U *buf, INT16U len)
{
    if ( (buf == NULL) || !len)
    {
        return FALSE;
    }
    INT16U i, j;
    INT16U temp;
    
    for (i = 0; i < len; i++)
    {
        for (j = i; j < len; j++)
        {
            if (buf[i] > buf[j])
            {
                temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }
    return TRUE;
}

/*****************************************************************************
* Function     : BSP_GetADSampleValue
* Description  : 获取AD采样值，中位值数值滤波方法
* Input        : _BSP_SAMPLE_TYPE SampleType  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月23日        
*****************************************************************************/
INT32U BSP_DataFilter(_BSP_SAMPLE_TYPE SampleType)
{
    INT8U i;
    INT16U value_buf[ADCCOUNTPERMS] = {0};
    INT32U sum = 0;

    if ( SampleType >= ADC_MAX_NUM)
    {
        return 0;
    }
    for (i = 0; i < ADCCOUNTPERMS; i++)
    {
        value_buf[i] = ADC_ConvertedValue[i][SampleType];
    }
    
    BubbleSort(value_buf, ADCCOUNTPERMS);       //去掉最小值、最大值，中间取平均值
    for(i = 1; i < ADCCOUNTPERMS - 1; i++)
    {
        sum += value_buf[i];
    }
    return (INT32U)(sum / (ADCCOUNTPERMS - 2) );
}


/***********************************************************************************************
* Function		: BSP_ADCInit
* Description	: AD采样初始化,DMA配置：在传输过程中Memory的地址是增加的，外设(ADC的采样数据地址)的地址不增加，
                  表示仅传送该外设使用的通道X的采样数据，未使用的通道的采样数据REG不会传输。
* Input         :
* Output        : FALSE:错误    TRUE:正确
* Note(s)       :
* Contributor   : 2018年5月25日
***********************************************************************************************/
INT8U BSP_ADCInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_InitTypeDef   ADC_InitStructure;
    DMA_InitTypeDef   DMA_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    
    /* Enable GPIOC clock                                                       */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOA, ENABLE);

    /* Enable DMA clock                                                         */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);


    /* Configure PA0-PA7 (ADC Channel0-Channel7) as analog input                */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;														   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                               //模拟输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;                            //不上下拉
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* DMA Channel1 Configuration ----------------------------------------------*/
    DMA_DeInit(DMA2_Stream0);                                                   //将通道1的相关寄存器复位为默认值
    DMA_InitStructure.DMA_Channel            = DMA_Channel_0;                   //通道0
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;                 //外设地址
    DMA_InitStructure.DMA_Memory0BaseAddr    = (u32)ADC_ConvertedValue;         //内存地址,实际上就是一个内部SRAM变量地址
    /*BufferSize=2，因为ADC转换序列有2个通道,如此设置，
    使序列1结果放在ADC_ConvertedValue[0],序列2结果放在ADC_ConvertedValue[1]*/ 
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralToMemory;      //dma传输方向,从外设到内存
    DMA_InitStructure.DMA_BufferSize         = ADCOUTNBUFFLENGTH;               //缓冲区大小，指一次性传输的数据量
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;       //设置DMA的外设递增模式，外设寄存器只有一个，地址不需要递增
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;            //设置DMA的内存递增模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据字长为半字16bits
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;     //内存数据字长为半字      16bits
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular;               //设置DMA的传输模式：连续不断的循环模式,Buffer写满后，自动回到初始地址开始传输
    DMA_InitStructure.DMA_Priority           = DMA_Priority_High;               //设置DMA的优先级别为高，当使用一个DMA通道时，优先级设置不影响
    DMA_InitStructure.DMA_FIFOMode           = DMA_FIFOMode_Disable;            //FIFO大小，FIFO模式禁止时不需要配置 
    DMA_InitStructure.DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);                                 //初始化DMA流，流相当于一个大的管道，管道里有很多小管道
//  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);//打开DMA 中断
    /* Enable DMA Channel1 */
//  DMA_Cmd(DMA1_Channel1, ENABLE);

    /* Enable ADC1 clock                                                        */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    //复位ADC1
    ADC_DeInit();
    //获取默认值
    ADC_StructInit(&ADC_InitStructure);

    //ADC通用寄存器配置
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     //独立模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                  //预分频系数为4分频
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      //禁止DMA直接访问模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //两个采样间隔5个时钟
    ADC_CommonInit(&ADC_CommonInitStructure);

    //ADC寄存器配置
    ADC_InitStructure.ADC_Resolution          = ADC_Resolution_12b;              //12位模式
    ADC_InitStructure.ADC_ScanConvMode        = (ADC_MAX_NUM > 1) ? ENABLE : DISABLE;                         //多通道扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode  = ENABLE;                          //连续转换开启
    ADC_InitStructure.ADC_ExternalTrigConv    = ADC_ExternalTrigConvEdge_None;   //禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign           = ADC_DataAlign_Right;             //数据格式采用右对齐
    ADC_InitStructure.ADC_NbrOfConversion     = ADC_MAX_NUM;                     //1个通道  /设置转换序列长度为1
    ADC_Init(ADC1, &ADC_InitStructure);


#if(USER_GUN != USER_SINGLE_GUN)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 4,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 5,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 6,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 7,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 8,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 9,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 10,  ADC_SampleTime_144Cycles);
#else
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1,  ADC_SampleTime_144Cycles);   //A枪电压
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 3,  ADC_SampleTime_144Cycles);   //A枪电池电压
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 5,  ADC_SampleTime_144Cycles);	//A枪绝缘检测
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6,  ADC_SampleTime_144Cycles);	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 7,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 8,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 9,  ADC_SampleTime_144Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 10,  ADC_SampleTime_144Cycles);
#endif
	
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);                            //循环模式必须打开，否则就只能转一次                          
    ADC_Cmd(ADC1, ENABLE);                                                       //开启ADC转换器
    ADC_DMACmd(ADC1, ENABLE);                                                    //开启ADC DMA通道

    ADC_SoftwareStartConv(ADC1);                                                 //开始ADC转换
    DMA_Cmd(DMA2_Stream0, ENABLE);                                               //校准AD的时候会触发DMA导致通道错位，因此校准AD基准前不要启用DMA。
	return TRUE;
}


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
INT32U BSP_GetHandVolt(_GUN_NUM gun)
{
    INT32U adcvalue = 0;
    INT32S tempvolt;
    FP32 advolt = 0;
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	if(gun == GUN_A)
	{
		adcvalue = BSP_DataFilter(BSPADC_GUN_A);
	}
	else
	{
		adcvalue = BSP_DataFilter(BSPADC_GUN_B);
	}

    advolt = adcvalue * (ADCJustInfo[gun].HeadOutKB.line_Y2 - ADCJustInfo[gun].HeadOutKB.line_Y1) / (ADCJustInfo[gun].HeadOutKB.line_X2 - ADCJustInfo[gun].HeadOutKB.line_X1)
             + ADCJustInfo[gun].HeadOutKB.line_B;

    tempvolt = advolt;
    if(tempvolt < 0)
        return 0;
    else
        return (INT32U)tempvolt;
}

/***********************************************************************************************
* Function      : BSP_GetOUT1DC
* Description   :电池电压
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetOUTBAT(_GUN_NUM gun)
{

    INT32S tempvolt;
	INT32U adcvalue = 0;
	if(gun >= GUN_MAX)
	{
		return  0;
	}
	if(gun == GUN_A)
	{
		adcvalue = BSP_DataFilter(BSPADC_BAT_A);
	}
	else
	{
		adcvalue = BSP_DataFilter(BSPADC_BAT_B);
	}
	
    tempvolt = adcvalue * ADCJustInfo[gun].BatVoltKB.line_K + ADCJustInfo[gun].BatVoltKB.line_B;
    if(tempvolt < 0)
        return 0;
    else
        return (INT32U)tempvolt;

}



/***********************************************************************************************
* Function      : BSP_GetPT1000N01
* Description   : 获取温度1
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetTemp1(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	if(gun == GUN_A)
	{
		return BSP_DataFilter(BSPADC_T1_A);
	}
	else
	{
		return BSP_DataFilter(BSPADC_T1_B);
	}
}

/***********************************************************************************************
* Function      : BSP_GetPT1000N01
* Description   : 获取温度2
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetTemp2(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	if(gun == GUN_A)
	{
		return BSP_DataFilter(BSPADC_T2_A);
	}
	else
	{
		return BSP_DataFilter(BSPADC_T2_B);
	}
}

/***********************************************************************************************
* Function      : BSP_GetJYVol
* Description   : 获取绝缘检测电压
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2015-8-31 
***********************************************************************************************/
INT32U BSP_GetJYVol(_GUN_NUM gun)
{
	 INT32S tempvolt;
	INT32U adcvalue = 0;
	
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	if(gun == GUN_A)
	{
		adcvalue = BSP_DataFilter(BSPADC_JY_A);
	}
	else
	{
		adcvalue = BSP_DataFilter(BSPADC_JY_B);
	}
	
    tempvolt = adcvalue * (ADCJustInfo[gun].JYVolKB.line_Y2 - ADCJustInfo[gun].JYVolKB.line_Y1) / (ADCJustInfo[gun].JYVolKB.line_X2 - ADCJustInfo[gun].JYVolKB.line_X1)
               + ADCJustInfo[gun].JYVolKB.line_B;
    if(tempvolt < 0)
        return 0;
    else
        return (INT32U)tempvolt;
}


/***********************************************************************************************
* Function		: BSP_GetADCJustInfo
* Description	: 更新KB数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2015-08-11 Yxy
***********************************************************************************************/
_ADC_JUST_INFO * BSP_GetADCJustInfo(_GUN_NUM gun)
{
	if(gun >= GUN_MAX)
	{
		return NULL;
	}
	return	&ADCJustInfo[gun];
}
/************************(C)COPYRIGHT 2018 杭州快电***END OF FILE****************************/
