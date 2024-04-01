	 /*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: sysconfig.c
* Author			: 
* Date First Issued	: 2018��5��25��
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "ucos_ii.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include <string.h>
#include "sysConfig.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration(void)
{   

}
/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
 void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//NVIC_DeInit();

#ifdef  VECT_TAB_RAM  
  	/* Set the Vector Table base location at 0x20000000 */ 
  	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
 	/* Set the Vector Table base location at 0x08044800 */ 
  	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (INT32U)0x00);    
#endif

	/* Configure three bit for preemption priority */
	/* one bit for sub priority                    */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);



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
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 	NVIC_Init(&NVIC_InitStructure);

 	/* Enable the UART4 Interrupt */
 	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 	NVIC_Init(&NVIC_InitStructure);

 	/* Enable the UART5 Interrupt */ 
 	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 	NVIC_Init(&NVIC_InitStructure);
	
	 	/* Enable the UART6 Interrupt */ 
 	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
 	NVIC_Init(&NVIC_InitStructure);
		 	/* Enable the UART7 Interrupt */ 
#if (BSP_CANTESTENABLE > 0)	
	//can1 ʹ��
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);

//			NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn;  
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
//	NVIC_Init(&NVIC_InitStructure);  
	
		NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN3_TX_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN3_RX0_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);
#endif
}
	
/***********************************************************************************************
* Function		: DelayUS 
* Description	: Inserts a delay time.
* Input			: nCount: specifies the delay time length in us.
* Output		: 
* Note(s)		: 
* Contributor	: 2018��5��25��
***********************************************************************************************/
void DelayUS(volatile INT32U nCount)
{
	volatile INT32U Tmpi;

	/* system clock = 168MHZ,1us = 168 instruction circle */
	nCount = nCount * 18;

  	for(; nCount != 0; nCount--)
	{   /* 10 instruction circle delay */
		Tmpi = nCount + 1;	  //do as NOP
//		Tmpi = nCount + 2;	  //do as NOP
//		Tmpi = nCount + 3;	  //do as NOP
//		Tmpi = nCount + 4;	  //do as NOP
//		Tmpi = nCount + 5;	  //do as NOP
		/* another three instruction for "nCount--"+1 "CMP"+1 and "BNE"+3 */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_DelayUS
*	����˵��: �������ʱ����CPU���ڲ�����ʵ�֣�32λ������
*             	OSSchedLock(&err);
*				bsp_DelayUS(5);
*				OSSchedUnlock(&err); ����ʵ����������Ƿ���Ҫ�ӵ�������ѡ����ж�
*	��    �Σ�n : �ӳٳ��ȣ���λ1 us
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_DelayUS(uint32_t _ulDelayTime)
{
//    uint32_t ticks;
//	uint32_t told,tnow,tcnt=0;
//		    	 
//	ticks = _ulDelayTime * (SystemCoreClock / 1000000);	 /* ��Ҫ�Ľ����� */ 		 
//	tcnt = 0;
//	told = (uint32_t)CPU_TS_TmrRd();         /* �ս���ʱ�ļ�����ֵ */

//	while(1)
//	{
//		tnow = (uint32_t)CPU_TS_TmrRd();	
//		if(tnow != told)
//		{	
//		    /* 32λ�������ǵ��������� */    
//			if(tnow > told)
//			{
//				tcnt += tnow - told;	
//			}
//			/* ����װ�� */
//			else 
//			{
//				tcnt += UINT32_MAX - told + tnow;	
//			}	
//			
//			told = tnow;

//			/*ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳� */
//			if(tcnt >= ticks)break;
//		}  
//	}
}
/******************************************************************************
* Function Name: OSTick_Init
* Description: ϵͳ�δ�ʱ������
* Input: 
* Output: 
* Return: 
******************************************************************************/
void OSTick_Init(void)
{
	RCC_ClocksTypeDef RCC_ClocksStructure;
	RCC_GetClocksFreq(&RCC_ClocksStructure);  //��ȡϵͳʱ��Ƶ��
	/* ��ʼ��������SysTick�������ж� */
	SysTick_Config(RCC_ClocksStructure.HCLK_Frequency / OS_TICKS_PER_SEC);
	DelayInit(RCC_ClocksStructure.HCLK_Frequency);                           // ��ʱ������ʼ��
}
/***********************************************************************************************
* Function		: Delay200NS
* Description	: delay time 200 NS.
* Input			: Nothing
* Output		: 
* Note(s)		: 
* Contributor	: 2018��5��25��
***********************************************************************************************/
void Delay200NS(void)
{
	volatile INT32U Tmpi;
	volatile INT32U Tmpj;
	
	/* system clock = 168MHZ, 200NS = 8 instruction circles */
	
	/* "BL W" , 3 instruction circles                      */

	Tmpi = 100;	      //do as NOP

	/* "BX x" , 3 instruction circles                      */	
}
/***********************************************************************************************
* Function		: WWdg_Init
* Description	: Window Watch dog config.
                  1)We must make sure that all the code in Critical region less
				   than ONE WDGTB tick(~6.5ms,as below).

				  2)Other case, We must make sure OSTaskIdleHook may be run within
				   (0x7F - 0x3F) WDGTB ticks(~419ms,as below).
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
void WWdg_Init(void)
{

}
/************************(C)COPYRIGHT 2018 �����Ƽ�****END OF FILE****************************/


