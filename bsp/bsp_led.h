/*****************************************Copyright(C)******************************************
******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_led.h
* Author			: WB
* Date First Issued	: 2018/04/01
* Version			: V
* Description		:  
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_LED_H_
#define	__BSP_LED_H_

#include "sysconfig.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"


////B 蓝色灯
//#define LED3_PIN                  GPIO_Pin_12                 
//#define LED3_GPIO_PORT            GPIOH                       
//#define LED3_GPIO_CLK             RCC_AHB1Periph_GPIOH
typedef enum
{
	BSPIO_RUN = 0,		//运行灯
	BSPIO_FAIL,			//故障灯
	BSPIO_WARN,			//告警灯
	BSPIO_BEEOP,		//蜂鸣器
	BSPIO_MAX
}_SBPIO_NUM;	


#define	BSPLED_MAX_NUM			2				// 直接STM32控制LED个数
// 状态类型,一般用于位控制
#define	BSP_STATE_RESET			  0u				// 清0
#define	BSP_STATE_SET			  1u				// 置1
#define	BSP_STATE_OVERTURN		  2u				// 翻转

// LED状态定义
typedef enum
{
	BSPLED_STATE_CLOSE = BSP_STATE_SET,			// LED开
	BSPLED_STATE_OPEN = BSP_STATE_RESET,		// LED关
	BSPLED_STATE_OVERTURN = BSP_STATE_OVERTURN,	// LED翻转
}_BSPLED_STATE;

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
void BSP_LEDWrite(_SBPIO_NUM num,_BSPLED_STATE state);


/***********************************************************************************************
* Function		: BSP_LEDBlink
* Description	: LED闪烁
* Input			: num:LED号,_BSPLED_NUM
					BSPLED_NUM_RUN	运行灯
					BSPLED_NUM_ALL	所有灯
				  TimeMS:   亮-灭跳变的间隔时间,毫秒单位.
				  BlinkNum: 亮-灭跳变的次数.
				  state:    闪烁以后LED状态
				  	BSPLED_STATE_CLOSE		关(灭)
					BSPLED_STATE_OPEN		开(亮)
					BSPLED_STATE_OVERTURN	系统使用, 应用层不得设置
* Output		: 
* Note(s)		: 
* Contributor	: WB
***********************************************************************************************/
void BSP_LEDBlink(_SBPIO_NUM num, INT16U TimeMS, INT16U BlinkNum, _BSPLED_STATE state);

/***********************************************************************************************
* Function		: BSP_LEDInit
* Description	: 驱动初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: WB
***********************************************************************************************/
void BSP_LEDInit(void);
#endif	//__BSP_LED_H_
/************************(C)COPYRIGHT 2018 杭州快电****END OF FILE****************************/
