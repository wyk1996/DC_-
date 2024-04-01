/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: WT588D.C
**硬          件: STM32
**创    建    人:  
**创  建  日  期:  
**最  新  版  本: V0.1
**描          述: LED控制程序
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#include	"sysconfig.h"
#include	"WT588D.h"
#include "delay.h"
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
//引脚定义
/*******************************************************/
#define WT588D_DATA_PIN         	GPIO_Pin_9          		//PF9       
#define WT588D_DATA_PORT            GPIOF                      
#define WT588D_DATA_CLK             RCC_AHB1Periph_GPIOF

#define WT588D_CS_PIN              	GPIO_Pin_6                 
#define WT588D_CS_PORT            	GPIOF                      
#define WT588D_CS_CLK             	RCC_AHB1Periph_GPIOF

#define WT588D_CLK_PIN              GPIO_Pin_7                 
#define WT588D_CLK_PORT            	GPIOF                      
#define WT588D_CLK_CLK             	RCC_AHB1Periph_GPIOF

#define WT588D_RESET_PIN         	GPIO_Pin_4                 
#define WT588D_RESET_PORT           GPIOF                      
#define WT588D_RESET_CLK            RCC_AHB1Periph_GPIOF

#define WT588D_BUSY_PIN             GPIO_Pin_5                 
#define WT588D_BUSY_PORT            GPIOF                      
#define WT588D_BUSY_CLK             RCC_AHB1Periph_GPIOF

#define SPEAKER_OFF_PIN             GPIO_Pin_3                 
#define SPEAKER_OFF_PORT            GPIOF                      
#define SPEAKER_OFF_CLK             RCC_AHB1Periph_GPIOF

#define CS_ENABLE				    GPIO_WriteBit(WT588D_CLK_PORT,WT588D_CS_PIN,Bit_RESET)  //使能为低电平
#define CS_DISENABLE				GPIO_WriteBit(WT588D_CLK_PORT,WT588D_CS_PIN,Bit_SET)

#define DATA_HIGE				    GPIO_WriteBit(WT588D_DATA_PORT,WT588D_DATA_PIN,Bit_SET)
#define DATA_LOW					GPIO_WriteBit(WT588D_DATA_PORT,WT588D_DATA_PIN,Bit_RESET)

#define CLK_HIGE				    GPIO_WriteBit(WT588D_CLK_PORT,WT588D_CLK_PIN,Bit_SET)
#define CLK_LOW						GPIO_WriteBit(WT588D_CLK_PORT,WT588D_CLK_PIN,Bit_RESET)

#define WT588D_MAX_PIN              6       //MT588D需要初始化的引脚有5个        
/* Private typedef----------------------------------------------------------------------------*/
// LED硬件配置结构
typedef struct
{
	GPIO_TypeDef *GPIOx;						// 引脚
	INT16U Port_Pin;							// 具体脚
}_WT588D_HD;
// LED硬件配置表
const _WT588D_HD WT588D_HD_Table[WT588D_MAX_PIN]=
{
	{WT588D_DATA_PORT,	WT588D_DATA_PIN	},
	
	{WT588D_CS_PORT,	WT588D_CS_PIN	},
	
	{WT588D_CLK_PORT,	WT588D_CLK_PIN	},
	
	{WT588D_RESET_PORT,	WT588D_RESET_PIN},
	
	{WT588D_BUSY_PORT,	WT588D_BUSY_PIN	},
	
	{SPEAKER_OFF_PORT,  SPEAKER_OFF_PIN},
};
// LED计数控制结构
typedef struct
{
	INT8U LEDState;								// 引脚状态
}_LED_CONTROL;									// LED控制结构
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_LEDInit
* Description	: 驱动初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月9日
***********************************************************************************************/
void MT588D_Flash(INT8U addr)
{
	INT8U  i;
//	GPIO_WriteBit(WT588D_RESET_PORT,WT588D_RESET_PIN,Bit_RESET);	
//		OSTimeDly(SYS_DELAY_5ms); 
//	GPIO_WriteBit(WT588D_RESET_PORT,WT588D_RESET_PIN,Bit_SET);	
//		OSTimeDly(SYS_DELAY_20ms); 
	CS_ENABLE;
	OSTimeDly(SYS_DELAY_5ms); 
	for(i=0; i< 8; i++)
	{
		CLK_LOW;
		if(addr&i)
		{
			DATA_HIGE;
		}
		else
		{
			DATA_LOW;
		}
		addr >>= 1;
		DelayUs(300);
		CLK_HIGE;
		DelayUs(300);
	}
	CS_DISENABLE;	
}
/***********************************************************************************************
* Function		: BSP_LEDInit
* Description	: 驱动初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月9日
***********************************************************************************************/
void WT588D_20SSInit(void)
{
	INT8U i;
	GPIO_InitTypeDef 	GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF , ENABLE);  //都是RCC_AHB1Periph_GPIOF
	// 配置LED引脚
	for(i=0;i<WT588D_MAX_PIN;i++)
	{

		/*选择要控制的GPIO引脚*/															   
		GPIO_InitStructure.GPIO_Pin = WT588D_HD_Table[i].Port_Pin;	

		/*设置引脚模式为输出模式*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    
    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
    /*设置引脚为上拉模式*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		/*设置引脚速率为2MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(WT588D_HD_Table[i].GPIOx, &GPIO_InitStructure);
	GPIO_WriteBit(WT588D_HD_Table[i].GPIOx,WT588D_HD_Table[i].Port_Pin,Bit_RESET);   //初始化都为低电平		
	}	
	GPIO_WriteBit(WT588D_RESET_PORT,WT588D_RESET_PIN,Bit_SET);
	CS_DISENABLE;	
	GPIO_WriteBit(SPEAKER_OFF_PORT,SPEAKER_OFF_PIN,Bit_SET);	
}
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
