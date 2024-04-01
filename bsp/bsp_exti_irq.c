/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_exti_irq.c
**硬          件: STM32
**创    建    人:  
**创  建  日  期:  
**最  新  版  本: V0.1
**描          述: 外部中断控制程序
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include	"sysconfig.h"
#include	"bsp_exti_irq.h"
/* Private define-----------------------------------------------------------------------------*/
//引脚定义
/*******************************************************/
//按键1 	PH3
#define KEY1_PIN                  GPIO_Pin_3                 
#define KEY1_GPIO_PORT            GPIOH                      
#define KEY1_GPIO_CLK             RCC_AHB1Periph_GPIOH
#define	KEY1_EXTI_GPIOX       		EXTI_PortSourceGPIOH
#define KEY1_EXTI_PIN							EXTI_PinSource3

//按键2   PH2
#define KEY2_PIN                  GPIO_Pin_2                 
#define KEY2_GPIO_PORT            GPIOH                      
#define KEY2_GPIO_CLK             RCC_AHB1Periph_GPIOH
#define	KEY2_EXTI_GPIOX       		EXTI_PortSourceGPIOH
#define KEY2_EXTI_PIN							EXTI_PinSource2


//按键3   PC13
#define KEY3_PIN                  GPIO_Pin_13                 
#define KEY3_GPIO_PORT            GPIOC                     
#define KEY3_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define	KEY3_EXTI_GPIOX          	EXTI_PortSourceGPIOC
#define KEY3_EXTI_PIN							EXTI_PinSource13

/* Private macro------------------------------------------------------------------------------*/
// 按键硬件配置结构
typedef struct
{
	GPIO_TypeDef* GPIOx;							// 引脚
	INT16U Port_Pin;									// 具体脚
	uint32_t Port_Clk;								//端口时钟
	uint8_t EXTI_PortSourceGPIOx;			//gpio
	uint8_t EXTI_PinSourcex;					//中断线
}_KEY_HD;

static const _KEY_HD KEY_HD_Table[ENABLE_ALL] =
{
	{KEY1_GPIO_PORT, KEY1_PIN, KEY1_GPIO_CLK ,KEY1_EXTI_GPIOX , KEY1_EXTI_PIN},
	
	{KEY2_GPIO_PORT, KEY2_PIN, KEY2_GPIO_CLK ,KEY2_EXTI_GPIOX , KEY2_EXTI_PIN},
	
	{KEY3_GPIO_PORT, KEY3_PIN, KEY3_GPIO_CLK ,KEY3_EXTI_GPIOX , KEY3_EXTI_PIN},
};
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_KEYExtiIrqInit
* Description	: 初始化外部中断
* Input				: num:按键号
							ENABLE_KEY1			按键1初始化
							ENABLE_KEY2			按键1初始化
							ENABLE_KEY3			按键1初始化
							ENABLE_ALL			初始化所有按键
* Output		: 
* Note(s)		: 
* Contributor	:2018年5月25日
***********************************************************************************************/
INT8U BSP_KEYExtiIrqInit(_ENABLE_KEY num)  
{  
		GPIO_InitTypeDef 	GPIO_InitStructure;
		EXTI_InitTypeDef EXTI_InitStructure;  
		INT8U count = 0;
		if(num > ENABLE_ALL)
		{
				return FALSE;
		}

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  									//中断模式
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;         			//下降沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
		if(num == ENABLE_ALL)   //开启全部中断
		{
				for(count = 0; count < ENABLE_ALL; count++)
				{
						RCC_AHB1PeriphClockCmd(KEY_HD_Table[count].Port_Clk, ENABLE);            
						GPIO_InitStructure.GPIO_Pin = KEY_HD_Table[count].Port_Pin;    
						GPIO_Init(KEY_HD_Table[count].GPIOx, &GPIO_InitStructure);    
						//gpio与外部中断对应
						SYSCFG_EXTILineConfig(KEY_HD_Table[count].EXTI_PortSourceGPIOx,KEY_HD_Table[count].EXTI_PinSourcex);  
						EXTI_ClearITPendingBit(KEY_HD_Table[count].Port_Pin);  							//EXTI_Line 与 Port_Pin对应				
						EXTI_InitStructure.EXTI_Line = KEY_HD_Table[count].Port_Pin;  					//EXTI_Line 与 Port_Pin对应		 
						EXTI_Init(&EXTI_InitStructure);	
				}
		}
		else				  //只需要初始化当前输入按键
		{
				RCC_AHB1PeriphClockCmd(KEY_HD_Table[num].Port_Clk, ENABLE);            
				GPIO_InitStructure.GPIO_Pin = KEY_HD_Table[num].Port_Pin;    
				GPIO_Init(KEY_HD_Table[num].GPIOx, &GPIO_InitStructure);    
				//gpio与外部中断对应
				SYSCFG_EXTILineConfig(KEY_HD_Table[num].EXTI_PortSourceGPIOx,KEY_HD_Table[num].EXTI_PinSourcex);  
				EXTI_ClearITPendingBit(KEY_HD_Table[num].Port_Pin);  									//EXTI_Line 与 Port_Pin对应				
				EXTI_InitStructure.EXTI_Line = KEY_HD_Table[num].Port_Pin;  					//EXTI_Line 与 Port_Pin对应		 
				EXTI_Init(&EXTI_InitStructure);	
		}
	return TRUE;
} 
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
