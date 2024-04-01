/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    06-March-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "sysconfig.h"
#include "bsp_uart.h"
#include "usb_hcd_int.h"
#include "usbh_usr.h"
#include "usb_bsp.h"
#include "bsp_can.h"
/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//extern USB_OTG_CORE_HANDLE  USB_OTG_dev;

///* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
//	printf("hardfault!!!\n");
  /* Go to infinite loop when Hard Fault exception occurs */
   while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */	
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}


/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void OS_CPU_SysTickHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif
    
	OS_ENTER_CRITICAL(); /* Tell uC/OS-II that we are starting an ISR  */
    OSIntEnter();
   	OS_EXIT_CRITICAL();
    OSTimeTick();  
	
    OSIntExit();
}
/**
  * @brief  This function handles LTDC global interrupt request.
  * @param  None
  * @retval None
  */
void LTDC_IRQHandler(void)
{
//  CPU_SR_ALLOC();

//	CPU_CRITICAL_ENTER();  
//	OSIntNestingCtr++;
//	CPU_CRITICAL_EXIT();
//  
//  LTDC_ISR_Handler();
//  
//  OSIntExit();
}

/**
  * @brief  This function handles DMA2D global interrupt request.
  * @param  None
  * @retval None
  */
void DMA2D_IRQHandler(void)
{
//  CPU_SR_ALLOC();

//	CPU_CRITICAL_ENTER();  
//	OSIntNestingCtr++;
//	CPU_CRITICAL_EXIT();
//  
//  DMA2D_ISR_Handler();
//  
//  OSIntExit();
}

void EXTI_IRQHandler(void)
{
//	CPU_SR_ALLOC();

//	CPU_CRITICAL_ENTER();  
//	OSIntNestingCtr++;
//	CPU_CRITICAL_EXIT();
//  
//	if(EXTI_GetITStatus(EXTI_INT_EXTI_LINE) != RESET) //确保是否产生了EXTI Line中断
//	{
//		/* Handle new gyro*/
//		gyro_data_ready_cb();
//	
//		EXTI_ClearITPendingBit(EXTI_INT_EXTI_LINE);     //清除中断标志位
//	}  
//  
//  OSIntExit();
}

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_HS  
void OTG_HS_IRQHandler(void)
{
	;
}
#endif

#ifdef USE_USB_OTG_FS  
void OTG_FS_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	  OSIntEnter();		 //进入中断
	  OS_EXIT_CRITICAL();  //开中断
   	USBH_OTG_ISR_Handler(&USB_OTG_Core);
    OSIntExit();		 //退出中断并调度
}

#endif
#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED 
/**
  * @brief  This function handles EP1_IN Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
  CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();  
	OSIntNestingCtr++;
	CPU_CRITICAL_EXIT();
  
  USBD_OTG_EP1IN_ISR_Handler(&USB_OTG_dev);
  
  OSIntExit();
}

/**
  * @brief  This function handles EP1_OUT Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
  CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();  
	OSIntNestingCtr++;
	CPU_CRITICAL_EXIT();
  
  USBD_OTG_EP1OUT_ISR_Handler(&USB_OTG_dev);
  
  OSIntExit();
}
#endif

/**
  * @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
  *         requests.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{


}

/*****************************************************************************
* Function     : USART1_IRQHandler
* Description  : 串口1中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void USART1_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(USART1);            
    OSIntExit();		 //退出中断并调度
}

/*****************************************************************************
* Function     : USART2_IRQHandler
* Description  : 串口2中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void USART2_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(USART2);            
    OSIntExit();		 //退出中断并调度
}

/*****************************************************************************
* Function     : USART3_IRQHandler
* Description  : 串口3中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void USART3_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(USART3);            
    OSIntExit();		 //退出中断并调度

}


/*****************************************************************************
* Function     : UART4_IRQHandler
* Description  : 串口4中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void UART4_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(UART4);            
    OSIntExit();		 //退出中断并调度

}

/*****************************************************************************
* Function     : UART5_IRQHandler
* Description  : 串口5中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月25日
*****************************************************************************/
void UART5_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(UART5);            
    OSIntExit();		 //退出中断并调度
}

/*****************************************************************************
* Function     : USART6_IRQHandler
* Description  : 串口6中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月25日
*****************************************************************************/
void USART6_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(USART6);            
    OSIntExit();		 //退出中断并调度
}


/*****************************************************************************
* Function     : UART5_IRQHandler
* Description  : 串口5中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月25日
*****************************************************************************/
void UART7_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(UART7);            
    OSIntExit();		 //退出中断并调度
}

/*****************************************************************************
* Function     : USART8_IRQHandler
* Description  : 串口8中断处理函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void UART8_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0u;
#endif

    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	OS_EXIT_CRITICAL();  //开中断

    UART_ISR(UART8);            
    OSIntExit();		 //退出中断并调度

}
/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_TX_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
#endif	
    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	BSP_CANTx_IRQHandler(0);
	OS_EXIT_CRITICAL();  //开中断

	//BSP_CANTx_IRQHandler(0);

	OSIntExit();		 //退出中断并调度       

}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN1_RX0_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
#endif	
    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	BSP_CANRx_IRQHandler(0);
	OS_EXIT_CRITICAL();  //开中断

	//BSP_CANRx_IRQHandler(0);

	OSIntExit();		 //退出中断并调度       	
}

//void CAN1_SCE_IRQHandler(void)
// {
//   CAN_ClearITPendingBit(CAN1,CAN_IT_LEC);  
// }
/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN2_TX_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
#endif	
    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	BSP_CANTx_IRQHandler(1);
	OS_EXIT_CRITICAL();  //开中断

//	BSP_CANTx_IRQHandler(1);

	OSIntExit();		 //退出中断并调度       

}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN2_RX0_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
#endif	
    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	BSP_CANRx_IRQHandler(1);
	OS_EXIT_CRITICAL();  //开中断

//	BSP_CANRx_IRQHandler(1);

	OSIntExit();		 //退出中断并调度       	
}


/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN3_TX_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
#endif	
    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	BSP_CANTx_IRQHandler(2);
	OS_EXIT_CRITICAL();  //开中断

//	BSP_CANTx_IRQHandler(1);

	OSIntExit();		 //退出中断并调度       

}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN3_RX0_IRQHandler(void)
{
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
		OS_CPU_SR  cpu_sr = 0;
#endif	
    OS_ENTER_CRITICAL(); //关中断
	OSIntEnter();		 //进入中断
	BSP_CANRx_IRQHandler(2);
	OS_EXIT_CRITICAL();  //开中断

//	BSP_CANRx_IRQHandler(1);

	OSIntExit();		 //退出中断并调度       	
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
