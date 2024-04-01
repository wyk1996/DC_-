﻿/*****************************************Copyright(C)******************************************
******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_exti_irq.h
* Author			: 
* Date First Issued	: 
* Version			: V
* Description		:  
*----------------------------------------历史版本信息-------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_EXTI_IRQ_H_
#define	__BSP_EXTI_IRQ_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#endif	//__BSP_LED_H_
/* Private define-----------------------------------------------------------------------------*/
typedef enum
{
		ENABLE_KEY1 = 0,
		ENABLE_KEY2,
		ENABLE_KEY3,
		ENABLE_ALL,						//所有外部按键使能
}_ENABLE_KEY;
/* Private macro------------------------------------------------------------------------------*/
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
* Contributor	: Yxy
***********************************************************************************************/
INT8U BSP_KEYExtiIrqInit(_ENABLE_KEY num);
/************************(C)COPYRIGHT 2018 杭州快电****END OF FILE****************************/
