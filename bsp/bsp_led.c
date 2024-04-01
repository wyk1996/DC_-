/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_LED.c
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
#include	"bsp_LED.h"
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
//引脚定义
/*******************************************************/

#define IO_GPIO_CLK    (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOA)
/* Private typedef----------------------------------------------------------------------------*/
// LED硬件配置结构
typedef struct
{
	GPIO_TypeDef *GPIOx;						// 引脚
	INT16U Port_Pin;							// 具体脚
}_LED_HD;
// LED硬件配置表
const _LED_HD LED_HD_Table[BSPIO_MAX]=
{
	{GPIOC,GPIO_Pin_10},
	{GPIOC,GPIO_Pin_11},
	{GPIOA,GPIO_Pin_15},
	{GPIOA,GPIO_Pin_3 },

};
// LED计数控制结构
typedef struct
{
	INT8U LEDState;								// 引脚状态
}_LED_CONTROL;									// LED控制结构
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static _LED_CONTROL LED_Control;				// LED控制变量
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_LEDWrite
* Description	: LED写,改变LED状态
* Input			: num:LED号
					BSPLED_NUM_RUN	运行灯
					BSPLED_NUM_CHONGDIAN 充电灯
					BSPLED_NUM_GUZHANG  故障灯
					BSPLED_NUM_ALL	所有灯
				  state:LED状态,_BSPLED_STATE
				  	BSPLED_STATE_CLOSE		关
					BSPLED_STATE_OPEN		开
					BSPLED_STATE_OVERTURN	LED翻转
				  
* Output		: 
* Note(s)		: 
* Contributor	: WB
***********************************************************************************************/
void BSP_LEDWrite(_SBPIO_NUM num,_BSPLED_STATE state)
{
	INT8U i;

	switch(state)
	{
		case BSPLED_STATE_OPEN: 
			LED_Control.LEDState |= BIT(num);
			GPIO_WriteBit(LED_HD_Table[num].GPIOx,LED_HD_Table[num].Port_Pin,Bit_SET);  
			break;
		case BSPLED_STATE_CLOSE:
			LED_Control.LEDState &= (~BIT(num));
			GPIO_WriteBit(LED_HD_Table[num].GPIOx,LED_HD_Table[num].Port_Pin,Bit_RESET);  
			break;
		case BSPLED_STATE_OVERTURN: 
			LED_Control.LEDState ^= BIT(num);
			if(LED_Control.LEDState & BIT(num))
				GPIO_WriteBit(LED_HD_Table[num].GPIOx,LED_HD_Table[num].Port_Pin,Bit_SET);
			else
				GPIO_WriteBit(LED_HD_Table[num].GPIOx,LED_HD_Table[num].Port_Pin,Bit_RESET);
			break;
		default:
			break;
	}
}

/***********************************************************************************************
* Function		: BSP_LEDInit
* Description	: 驱动初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: WB
***********************************************************************************************/
void BSP_LEDInit(void)
{
	INT8U i;
	GPIO_InitTypeDef 	GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(IO_GPIO_CLK , ENABLE);
	LED_Control.LEDState = 0x00;
	// 配置LED引脚
	for(i=0;i<BSPIO_MAX;i++)
	{

		/*选择要控制的GPIO引脚*/															   
		GPIO_InitStructure.GPIO_Pin = LED_HD_Table[i].Port_Pin;	

		/*设置引脚模式为输出模式*/
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    
    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    
    /*设置引脚为上拉模式*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

		/*设置引脚速率为2MHz */   
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 

		
		GPIO_Init(LED_HD_Table[i].GPIOx, &GPIO_InitStructure);
	}
	
}
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
