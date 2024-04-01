/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                    MICRIUM BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                         STM3240G-EVAL
*                                        Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : FF
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define   BSP_MODULE
#include  "bsp.h"
#include  "stm32f4xx_rcc.h"
#include  "StmFlash.h"

/*****************************************************************************
* Function     : SysTickConfiguartion
* Description  : 滴答定时器配置，作为ucos系统时钟,周期为1000/OS_TICKS_PER_SEC(ms)
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2017年5月11日        Xieyb
*****************************************************************************/
static void SysTickConfiguartion(void)
{
	RCC_ClocksTypeDef RCC_Clocks;
	
	RCC_GetClocksFreq(&RCC_Clocks);									// 得到系统时钟
	
	SysTick_Config((RCC_Clocks.HCLK_Frequency/OS_TICKS_PER_SEC));	// 设置系统tick工作在10ms

    DelayInit(RCC_Clocks.HCLK_Frequency);                           // 初始化延时因子

    STMFLASH_ReadKey();
}

/*******************************************************************************
* Function Name  : GPIO_AIM_Init
* Description    : Default Configure all unused GPIO port pins in Analog Input 
                   mode(floating input trigger OFF), this will reduce the power 
				   consumption and increase the device immunity against EMI/EMC
* Input          : FGPort
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_AIM_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

    //全部设置为模拟输入
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
  	GPIO_Init(GPIOC, &GPIO_InitStructure);
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
  	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/*****************************************************************************
* Function     : NVICConfiguration
* Description  : 中断分组
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2017年5月11日        Xieyb
*****************************************************************************/
static void NVICConfiguration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
	// 设置中断向量表空间
	// 选择使用优先级分组
	// 说明:CortexM3使用抢占式优先级加响应优先级的概念,两个优先级共用8位(抢占式在高位)
	// 所谓抢占式优先级,就是高优先级的中断可以打断低优先级的中断(中断嵌套),级数别太多
	// 响应优先级,相同抢占式优先级的2个中断之间是不能相互打断的,只是如果2个中断同时来时,先响应高的那个
	// 可以有8种分组方式(缺8位都用于抢占式优先级的分组)
	// STM32只定义了4位空间,所以有5种分组方式
	// 第0组：所有4位用于指定响应优先级
	// 第1组：最高1位用于指定抢占式优先级，最低3位用于指定响应优先级
	// 第2组：最高2位用于指定抢占式优先级，最低2位用于指定响应优先级
	// 第3组：最高3位用于指定抢占式优先级，最低1位用于指定响应优先级
	// 第4组：所有4位用于指定抢占式优先级
	// 注:确定好分组方式后,保证下面的优先级设置参数不超过分组的最大值,否则将产生意想不到的结果
	// 数值越小级别越高
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);	// 分组3,可以有3层嵌套
	
    /* Enable the WWDG Interrupt，lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the UART4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the UART6 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  /* 485接口有延时，降低优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the TIM3 Interrupt*/ 
	// 定时器3中断,用于按键扫描
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	/* Enable the EXTI0 Interrupt: BATT_GPIO_TEST_LINE (NPFO) 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); */

	/* Enable the EXTI9_5 Interrupt:NIRQ_CY_PIN
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); */
	/* Enable CAN TX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable CAN RX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable CAN TX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable CAN RX0 interrupt IRQ channel */
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable the EXTI1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;//EXTI1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);   

    //配置SDIO的中断
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);
    //配置SDIO的DMA中断
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_Init (&NVIC_InitStructure);


    //以太网中断配置
    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;  //以太网中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    //配置串口1的DMA发送中断
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
    NVIC_Init (&NVIC_InitStructure);
}


/*****************************************************************************
* Function     : TargetInit
* Description  : 硬件初始化
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2017年5月11日        Xieyb
*****************************************************************************/
void TargetInit(void)
{
	SysTickConfiguartion();						// ucos系统定时器初始化	
    GPIO_AIM_Init();
	NVICConfiguration();						// 可屏蔽中断初始化
}

/*****************************************************************************
* Function     : BSP_GetChipUniqueID
* Description  : 获取芯片唯一设备ID
* Input        : UID_TypeDef* Uid  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2017年7月27日        Xieyb
*****************************************************************************/
INT8U BSP_GetChipUniqueID(UID_TypeDef* Uid)
{
    if (Uid == NULL)         //stm32f4的唯一ID有三个寄存器，共96位
    {
        return FALSE;
    }
    *Uid = *UIDMCU;
    return TRUE;
}

/*****************************************************************************
* Function     : BSP_GetEthernetMac
* Description  : 获取以太网MAC地址
* Input        : INT8U *pMac  
                 INT8U Len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2017年7月27日        Xieyb
*****************************************************************************/
INT8U BSP_GetEthernetMac(INT8U *pMac, INT8U Len)
{
    if ( (pMac == NULL) || (Len < 6) )
    {
        return FALSE;
    }
    
    UID_TypeDef ChipUid;
    if (BSP_GetChipUniqueID(&ChipUid) == FALSE)
    {
        return FALSE;
    }
    pMac[0] = 2;                               //mac地址的钱3个字节固定为2，0，0
    pMac[1] = 0;
    pMac[2] = 0;
    pMac[3] = (ChipUid.UID_LOW >> 16) & 0xFF;  //stm32唯一ID的低24位
    pMac[4] = (ChipUid.UID_LOW >> 8) & 0xFF;
    pMac[5] = (ChipUid.UID_LOW >> 0) & 0xFF;
    return TRUE;
}

/*****************************************************************************
* Function     : BSP_Init
* Description  : 外设初始化
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2017年5月11日        Xieyb
*****************************************************************************/
void BSP_Init(INT8U *pdata, INT16U len)
{             
#if (BSP_FMENABLE > 0u)				
    BSP_InitFm();							   //铁电初始化 
#endif //  BSP_FMENABLE

#if (BSP_UARTENABLE > 0u)
    BSP_UARTInit();                            //串口初始化
#endif // BSP_UARTENABLE

#if (BSP_FLASHENABLE > 0u)
    BSP_MX25Init();                            //MX25L128 flash初始化
#endif //BSP_FLASHENABLE
	
#if (BSP_BEEPENABLE > 0u)
	BSP_BeepInit();                            //蜂鸣器初始化
#endif  // BSP_BEEPENABLE

#if (BSP_ICCARDENABLE > 0u)    
	BSP_FM1702SLSPI_Init();					   //IC卡读头初始化
#endif //BSP_ICCARDENABLE

#if (BSP_RTCENABLE > 0u)
	BSP_RTCInit();								//RTC初始化
#endif //BSP_RTCENABLE

#if (BSP_RLCENABLE > 0u)
	BSP_RLCInit();								//RLC初始化
#endif //BSP_RLCENABLE

#if (BSP_LEDENABLE > 0u)
	BSP_LEDInit();								//LED初始化
#endif //BSP_RLCENABLE

#if (BSP_REMOTESIGENABLE > 0u)
	BSP_RemoteSignalInit();                     //遥信初始化
#endif //BSP_RETEMOSIGENABLE

#if (BSP_WDGENABLE > 0u)
	BSP_WDGInit();								//看门狗
#endif //BSP_WDGENABLE
   
#if (BSP_ADENABLE > 0u)  
	BSP_ADCInit();                              //ADC初始化
#endif //BSP_ADENABLE

#if (BSP_LOCKENABLE > 0u)                       //电磁锁
    BSP_MagneticInit();
#endif //BSP_LOCKENABLE

#if (BSP_LWIPENABLE > 0u)   
    if (ETH_BSP_Config() == FALSE)              //LWIP使能
    {
        SetLan8720InitFlag(FALSE);              //需要重新初始化
    }
    else
    {
        SetLan8720InitFlag(TRUE);               //不需要重新初始化
    }
#endif  //BSP_USBHOSTENABLE

#if (BSP_GPRSENABLE > 0u)
    BSP_GPRSInit();                             //GPRS使能
#endif// #if (BSP_GPRSENABLE > 0u)

#if (BSP_DEBUGENABLE > 0)                       //使能串口作为debug
    BSP_DebugUartInit(115200, pdata, len);
#endif //BSP_DEBUGENABLE

#if (BSP_MYMALLOCENABLE > 0u)
    MyMemoryInit(SRAM_INTERNAL);                //初始化内部内存
    MyMemoryInit(SRAM_CCM);                     //初始化内部CCM
#endif


}


/***********************************************************************************************
* Function		: BSP_OSPost
* Description	: 封装发送消息的函数
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 090925	wangyao
***********************************************************************************************/
// 发送事件消息
#if	BSP_OS
void BSP_OSPost(_OS_EVENT *pEvent, void *pMsg)
{
#if	BSP_UCOSII
	if(pEvent == NULL)
		return;
#if OS_Q_EN
	if(pEvent->OSEventType == OS_EVENT_TYPE_Q)
		OSQPost(pEvent, pMsg);
#endif	//OS_Q_EN
#if OS_MBOX_EN
	else if(pEvent->OSEventType == OS_EVENT_TYPE_MBOX)
		OSMboxPost(pEvent, pMsg);
#endif	//OS_MBOX_EN
#if OS_SEM_EN
	else if(pEvent->OSEventType == OS_EVENT_TYPE_SEM)
		OSSemPost(pEvent);
#endif	//OS_SEM_EN
	else
		NOP();
#endif	//BSP_UCOSII
}
#endif	//BSP_OS

