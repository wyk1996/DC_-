/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.c
* Author			:
* Date First Issued	:
* Version			: V
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "main.h"
#include "stm32f4xx_conf.h"
#include "netconf.h"
#include "bsp_adc.h"
#include "bsp_conf.h"
#include "bsp_uart.h"
#include "bsp_rtc.h"
#include "bsp_io.h"
#include "spi_flash.h"
#include "bsp_SST25VF016.h"
#include "ff.h"
#include	"bsp_MPulse.h"
#include "w25qxx.h"
#include "bsp_debug_usart.h"
#include "version.h"
/* Private define-----------------------------------------------------------------------------*/



#define OS_INIT_TASK_STACK_SIZE		0x200		 //初始化任务堆栈大小
OS_STK	InitTaskStk[OS_INIT_TASK_STACK_SIZE];	//初始化任务堆栈


//新任务
#define TASK_STK_SIZE_ConnectBMSA  0x800
OS_STK TaskConnectBMSAStk[TASK_STK_SIZE_ConnectBMSA];    // 跟汽车对接任务堆栈

#define TASK_STK_SIZE_ConnectBMSB  0x800
OS_STK TaskConnectBMSBStk[TASK_STK_SIZE_ConnectBMSB];   // 跟汽车对接任务堆栈


#define	TASK_STK_SIZE_DISP		0x800
OS_STK TaskDisplayStk[TASK_STK_SIZE_DISP];				//显示任务任务堆栈


#define	TASK_STK_SIZE_SAM		0x800
OS_STK TaskSAMStk[TASK_STK_SIZE_SAM];					//采样任务

//周期性任务
#define TASK_STK_SIZE_PERIOD       0x800
OS_STK TaskPeriodStk[TASK_STK_SIZE_PERIOD];				//周期性检测任务

#define	TASK_STK_SIZE_JUST		0x800
OS_STK TaskJustStk[TASK_STK_SIZE_SAM];					//AD校准任务



OS_STK TaskUsbMainStk[0x800];					//模块发送任务
OS_STK TaskUsbStk[0x800];					//模块发送任务




#define TASK_STK_SIZE_GPRSMIAN  0x800
OS_STK TaskStkGPRSMainStk[TASK_STK_SIZE_GPRSMIAN];		            // 任务栈大小检查任务堆栈

#define TASK_STK_SIZE_GPRSSEND  0x800
OS_STK TaskStkGPRSSendStk[TASK_STK_SIZE_GPRSSEND];		            // 任务栈大小检查任务堆栈

#define TASK_STK_SIZE_GPRSRECV  0x800
OS_STK TaskStkGPRSRecvStk[TASK_STK_SIZE_GPRSRECV];		            // 任务栈大小检查任务堆栈

#define TASK_STK_SIZE_RTC       0x800
OS_STK TaskRTCStk[TASK_STK_SIZE_RTC];		            	// RTC任务堆栈

#define TASK_STK_SIZE_METER       0x100
OS_STK TaskStkMeter[TASK_STK_SIZE_METER];					//抄表任务堆栈

#define TASK_STK_SIZE_BILL       0x800
OS_STK TaskStkBILL[TASK_STK_SIZE_BILL];					//计费任务

#define TASK_STK_SIZE_CPUCARD       0x800
OS_STK TaskCPUCardStk[TASK_STK_SIZE_CPUCARD];				//CPU卡任务堆栈

#define TASK_STK_SIZE_CONTROL       0x800
OS_STK TaskControlStkA[TASK_STK_SIZE_CONTROL];				//CPU卡任务堆栈
OS_STK TaskControlStkB[TASK_STK_SIZE_CONTROL];				//CPU卡任务堆栈


#define TASK_STK_SIZE_TIST  0x800
OS_STK TaskConnectTESTStk[TASK_STK_SIZE_TIST];
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/


///*****************************************************************************
//* Function     : TaskStkChk
//* Description  : 个人任务栈使用率检查
//* Input        : void
//* Output       : None
//* Return       :
//* Note(s)      :
//* Contributor  : 2018年5月15日
//*****************************************************************************/
//void TaskStkChk(void *pdata)
//{
////	OS_STK_DATA StackBytes;
////	while(1)
////	{
////		OSTaskStkChk(OS_TASK_INIT_PRIO,&StackBytes);
////		printf("OS_TASK_INIT_PRIO OSFree = %d,OSUsed = %d",StackBytes.OSFree,StackBytes.OSUsed);
////		OSTimeDly(SYS_DELAY_1s);
////
////		OSTaskStkChk(PRI_TEST,&StackBytes);
////		printf("PRI_TEST OSFree = %d,OSUsed = %d",StackBytes.OSFree,StackBytes.OSUsed);
////		OSTimeDly(SYS_DELAY_1s);
////
////		OSTaskStkChk(PRI_LED,&StackBytes);
////		printf("PRI_LED OSFree = %d,OSUsed = %d",StackBytes.OSFree,StackBytes.OSUsed);
////		OSTimeDly(SYS_DELAY_1s);
////
////		OSTaskStkChk(PRI_UART,&StackBytes);
////		printf("PRI_UART OSFree = %d,OSUsed = %d",StackBytes.OSFree,StackBytes.OSUsed);
////		OSTimeDly(SYS_DELAY_1s);
////
////		OSTaskStkChk(PRI_CNTRECV,&StackBytes);
////		printf("PRI_CNTRECV OSFree = %d,OSUsed = %d",StackBytes.OSFree,StackBytes.OSUsed);
////		OSTimeDly(SYS_DELAY_1s);
////
////		OSTaskStkChk(PRI_USB,&StackBytes);
////		printf("PRI_USB OSFree = %x,OSUsed = %x",StackBytes.OSFree,StackBytes.OSUsed);
////		OSTimeDly(SYS_DELAY_1s);
////
////		OSTaskStkChk(PRI_USBTEST,&StackBytes);
////		printf("PRI_USBTEST OSFree = %d,OSUsed = %d",StackBytes.OSFree,StackBytes.OSUsed);
////		OSTimeDly(SYS_DELAY_1s);
////
////		OSTaskStkChk(OS_TASK_INIT_PRIO,&StackBytes);
////		printf("OS_TASK_INIT_PRIO OSFree = %d,OSUsed = %d",StackBytes.OSFree,StackBytes.OSUsed);
//		OSTimeDly(SYS_DELAY_1s);
////	}
//}

void flash_test(void)
{

	//	W25QXX_Write(writebuf,FLASH_SIZE,sizeof(writebuf));
//	W25QXX_Read(readbuf,FLASH_SIZE,sizeof(readbuf));
	INT8U temp[10] = {0,1,2,3,4,5,6,7,8,9};
	INT8U buf[10] = {0};
	W25QXX_Write(temp,100,10);
	W25QXX_Read(buf,100,10);
//	SST26_WriteBuffer(temp,100,10);
//	SST26_ReadBuffer(buf,100,10);
	NOP();
}


/*
 * 设置 IWDG 的超时时间
 * Tout(ms) = prv / 40 * rlv
 * prv可以是[4,8,16,32,64,128,256]
 * prv:预分频器值，取值如下：
 * 	@arg IWDG_Prescaler_4: IWDG prescaler set to 4
 *  @arg IWDG_Prescaler_8: IWDG prescaler set to 8
 *  @arg IWDG_Prescaler_16: IWDG prescaler set to 16
 *  @arg IWDG_Prescaler_32: IWDG prescaler set to 32
 *  @arg IWDG_Prescaler_64: IWDG prescaler set to 64
 *  @arg IWDG_Prescaler_128: IWDG prescaler set to 128
 *  @arg IWDG_Prescaler_256: IWDG prescaler set to 256
 *
 * rlv:预分频器值，取值范围为：0-0XFFF
 * 函数调用举例：
 * IWDG_Config(IWDG_Prescaler_64 ,625);  // IWDG 1s 超时溢出
 */
void IWDG_Config(uint8_t prv, uint16_t rlv)
{
	// 使能 预分频寄存器PR和重装载寄存器RLR可写
	IWDG_WriteAccessCmd( IWDG_WriteAccess_Enable );

	// 设置预分频器值
	IWDG_SetPrescaler( prv );

	// 设置重装载寄存器值
	IWDG_SetReload( rlv );

	// 把重装载寄存器的值放到计数器中
	IWDG_ReloadCounter();

	// 使能 IWDG
	IWDG_Enable();
}


// 喂狗
void IWDG_Feed(void)
{
	// 把重装载寄存器的值放到计数器中，喂狗，防止IWDG复位
	// 当计数器的值减到0的时候会产生系统复位
	IWDG_ReloadCounter();
}

/* Private functions--------------------------------------------------------------------------*/



void jump_to_app(uint32_t app_address)
{
	typedef void (*_func)(void);

	__disable_irq();

	/* MCU peripherals re-initial. */
	{
		GPIO_InitTypeDef GPIO_InitStruct;

		GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
		GPIO_InitStruct.GPIO_Pin &= ~(GPIO_Pin_13 | GPIO_Pin_14); /* SWDIO/SWCLK */
		GPIO_Init(GPIOA, &GPIO_InitStruct);

		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
		GPIO_Init(GPIOB, &GPIO_InitStruct);
		GPIO_Init(GPIOC, &GPIO_InitStruct);
		GPIO_Init(GPIOD, &GPIO_InitStruct);
		GPIO_Init(GPIOE, &GPIO_InitStruct);
		GPIO_Init(GPIOF, &GPIO_InitStruct);
		GPIO_Init(GPIOG, &GPIO_InitStruct);
		GPIO_Init(GPIOH, &GPIO_InitStruct);
		// GPIO_Init(GPIOI, &GPIO_InitStruct);

		/* reset systick */
		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

		/* disable all peripherals clock. */
		RCC->AHB1ENR = (1<<20); /* 20: F4 CCMDAT ARAMEN. */
		RCC->AHB2ENR = 0;
		RCC->AHB3ENR = 0;
		RCC->APB1ENR = 0;
		RCC->APB2ENR = 0;

		/* Switch to default cpu clock. */
		RCC->CFGR = 0;
	} /* MCU peripherals re-initial. */

	/* Disable MPU */
	MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

	/* disable and clean up all interrupts. */
	{
		int i;

		for(i = 0; i < 8; i++)
		{
			/* disable interrupts. */
			NVIC->ICER[i] = 0xFFFFFFFF;

			/* clean up interrupts flags. */
			NVIC->ICPR[i] = 0xFFFFFFFF;
		}
	}

	/* Set new vector table pointer */
	SCB->VTOR = app_address;

	/* reset register values */
	__set_BASEPRI(0);
	__set_FAULTMASK(0);

	/* set up MSP and switch to it */
	__set_MSP(*(uint32_t*)app_address);
	__set_PSP(*(uint32_t*)app_address);
	__set_CONTROL(0);

	/* ensure what we have done could take effect */
	__ISB();

	__disable_irq();

	/* never return */
	((_func)(*(uint32_t*)(app_address + 4)))();
}


INT8U SYS_APP_VER[3] = {24,3,21};  //屏幕显示软件版本号

/***********************************************************************************************
* Function		: init_task_core
* Description	: 创建应用程序任务处理子程序
* Input			:
* Output		:
* Note(s)		:
* Contributor	: yxy
***********************************************************************************************/
static void init_task_core(void *pdata)
{
	char buf[30] = {0};
	pdata = pdata;			//防止编译器警
	Debug_USART_Config();
	OSTimeDly(SYS_DELAY_500ms);
	BSP_ADCInit();
#if (BSP_FLASH25ENABLE > 0)
	FlishW25QXXInit();
#else
	SST26_Init();
	//SST25V_Init();
#endif
	BSP_UARTInit();								//串口初始化
	BSP_IOInit();
	BSP_MPLSInit();
	
	
//	OSStatInit();  //统计任务启动，主要用于查看CPU使用率 （OSCPUsage）
//	flash_test();

//BootLoadWrite(testbuf,0,1);
//JumpToProgramCode();



//网络任务，以下二选一
//
//		OSTaskCreateExt(TaskW5500Connect,				//w5500主任务
//				(void *)0,
//				&TaskW5500Stk[TASK_STK_SIZE_W5500-1],
//				PRI_W5500,
//				PRI_W5500,
//				&TaskW5500Stk[0],
//				TASK_STK_SIZE_W5500,
//				(void *)0,
//				OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

//
//	OSTaskCreateExt(TaskW5500Priod,								//各个任务栈大小查看，以便做出适当调整，一般利用率在50%-80%
//				(void *)0,
//				&TaskStkChkStk[TASK_STK_SIZE_StkChk-1],
//				PRI_STKCHK,
//				PRI_STKCHK,
//				&TaskStkChkStk[0],
//				TASK_STK_SIZE_StkChk,
//				(void *)0,
//				OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
//

//while(1)
//{
//	BSP_OpenBMSPower(GUN_A);
//	OSTimeDly(SYS_DELAY_500ms);
//	BSP_CloseBMSPower(GUN_A);
//	OSTimeDly(SYS_DELAY_500ms);
//}
	OSTaskCreateExt(TaskControlGunA,
	                (void *)0,
	                &TaskControlStkA[TASK_STK_SIZE_CONTROL-1],
	                PRI_CONTROLA,
	                PRI_CONTROLA,
	                &TaskControlStkA[0],
	                TASK_STK_SIZE_CONTROL,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

#if(USER_GUN != USER_SINGLE_GUN)
	OSTaskCreateExt(TaskControlGunB,
	                (void *)0,
	                &TaskControlStkB[TASK_STK_SIZE_CONTROL-1],
	                PRI_CONTROLB,
	                PRI_CONTROLB,
	                &TaskControlStkB[0],
	                TASK_STK_SIZE_CONTROL,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif

	OSTaskCreateExt(TaskCardStk,
	                (void *)0,
	                &TaskCPUCardStk[TASK_STK_SIZE_CPUCARD-1],
	                PRI_CPUCARD,
	                PRI_CPUCARD,
	                &TaskCPUCardStk[0],
	                TASK_STK_SIZE_CPUCARD,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

	OSTaskCreateExt(TaskMeterStk,
	                (void *)0,
	                &TaskStkMeter[TASK_STK_SIZE_METER-1],
	                PRI_METER,
	                PRI_METER,
	                &TaskStkMeter[0],
	                TASK_STK_SIZE_METER,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

	OSTaskCreateExt(TaskBillStk,
	                (void *)0,
	                &TaskStkBILL[TASK_STK_SIZE_BILL-1],
	                PRI_BILL,
	                PRI_BILL,
	                &TaskStkBILL[0],
	                TASK_STK_SIZE_BILL,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);



	OSTaskCreateExt(TaskRTC,						//RTC任务
	                (void *)0,
	                &TaskRTCStk[TASK_STK_SIZE_RTC-1],
	                PRI_RTC,
	                PRI_RTC,
	                &TaskRTCStk[0],
	                TASK_STK_SIZE_RTC,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);


	OSTaskCreateExt(Task4GMain,								//GPRS主任务
	                (void *)0,
	                &TaskStkGPRSMainStk[TASK_STK_SIZE_GPRSMIAN-1],
	                PRI_GPRSMAIN,
	                PRI_GPRSMAIN,
	                &TaskStkGPRSMainStk[0],
	                TASK_STK_SIZE_GPRSMIAN,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

	OSTaskCreateExt(Task4GSend,								//GPRS发送任务
	                (void *)0,
	                &TaskStkGPRSSendStk[TASK_STK_SIZE_GPRSSEND-1],
	                PRI_GPRSSEND,
	                PRI_GPRSSEND,
	                &TaskStkGPRSSendStk[0],
	                TASK_STK_SIZE_GPRSSEND,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

	OSTaskCreateExt(Task4GRecv,								//GPRS接收任务
	                (void *)0,
	                &TaskStkGPRSRecvStk[TASK_STK_SIZE_GPRSRECV-1],
	                PRI_GPRSRECV,
	                PRI_GPRSRECV,
	                &TaskStkGPRSRecvStk[0],
	                TASK_STK_SIZE_GPRSRECV ,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

//				 jump_to_app(LoadCodeAddress); // 跳转到BOOT
//			    //周期性检测任务
	OSTaskCreateExt(TaskPeriod,
	                (void *)0,
	                &TaskPeriodStk[TASK_STK_SIZE_PERIOD - 1],
	                PRI_PERIOD,
	                PRI_PERIOD,
	                &TaskPeriodStk[0],
	                TASK_STK_SIZE_PERIOD,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	
	OSTaskCreateExt(TaskConnectBMSAMain,				//A枪BMS----连接ccu主任务
	                (void *)0,
	                &TaskConnectBMSAStk[TASK_STK_SIZE_ConnectBMSA-1],
	                PRI_CONNECT_BMSA,
	                PRI_CONNECT_BMSA,
	                &TaskConnectBMSAStk[0],
	                TASK_STK_SIZE_ConnectBMSA,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
	
#if(USER_GUN != USER_SINGLE_GUN)
	OSTaskCreateExt(TaskConnectBMSBMain,				//B枪BMS----连接ccu主任务
	                (void *)0,
	                &TaskConnectBMSBStk[TASK_STK_SIZE_ConnectBMSB-1],
	                PRI_CONNECT_BMSB,
	                PRI_CONNECT_BMSB,
	                &TaskConnectBMSBStk[0],
	                TASK_STK_SIZE_ConnectBMSB,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);
#endif
									
//模块

	OSTaskCreateExt(TaskDisplay,
	                (void *)0,
	                &TaskDisplayStk[TASK_STK_SIZE_DISP-1],
	                PRI_DISP,
	                PRI_DISP,
	                &TaskDisplayStk[0],
	                TASK_STK_SIZE_DISP,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);


//创建任务,TaskSample
	OSTaskCreateExt(TaskSample,
	                (void *)0,
	                &TaskSAMStk[TASK_STK_SIZE_SAM - 1],
	                PRI_SAM,
	                PRI_SAM,
	                &TaskSAMStk[0],
	                TASK_STK_SIZE_SAM,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK + OS_TASK_OPT_STK_CLR);


//创建任务,AD采样
	OSTaskCreateExt(TaskADJust,
	                (void *)0,
	                &TaskJustStk[TASK_STK_SIZE_SAM - 1],
	                PRI_JUST,
	                PRI_JUST,
	                &TaskJustStk[0],
	                TASK_STK_SIZE_SAM,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK + OS_TASK_OPT_STK_CLR);


//	jsonmain();
//						//创建任务,USB
//    OSTaskCreateExt(TaskUsbMain,
//                    (void *)0,
//                    &TaskUsbMainStk[TASK_STK_SIZE_MODULET - 1],
//                    PRI_USBMAIN,
//                    PRI_USBMAIN,
//                    &TaskUsbMainStk[0],
//                    TASK_STK_SIZE_MODULET,
//                    (void *)0,
//                    OS_TASK_OPT_STK_CHK + OS_TASK_OPT_STK_CLR);

	OSTaskCreateExt(TestMain,
	                (void *)0,
	                &TaskConnectTESTStk[TASK_STK_SIZE_TIST-1],
	                PRI_TEST_MAIN,
	                PRI_TEST_MAIN,
	                &TaskConnectTESTStk[0],
	                TASK_STK_SIZE_TIST,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK+OS_TASK_OPT_STK_CLR);

//创建任务,USB任务
	OSTaskCreateExt(TaskUsbTask,
	                (void *)0,
	                &TaskUsbStk[0x800 - 1],
	                PRI_USBTASK,
	                PRI_USBTASK,
	                &TaskUsbStk[0],
	                0x800,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK + OS_TASK_OPT_STK_CLR);

#if(USER_dog==1)
	IWDG_Config(IWDG_Prescaler_64 ,625*12);	  //6s---看门狗
#endif

	while(1)
	{
		BSP_IOTurnover(IO_LED_RUN);
		OSTimeDly(SYS_DELAY_500ms);
		IWDG_ReloadCounter();			//喂狗
		
		
////测试继电器开关了
//		static int  abcd=0;
//		if(abcd == 1)
//		{
//			BSP_IOOpen(IO_RLY_FAN);
//		}

//		if(abcd == 2)
//		{
//			BSP_IOOpen(IO_RLY_K1K2A);
//		}
//		if(abcd == 3)
//		{
//			BSP_IOOpen(IO_RLY_LOCKA);
//		}
//		if(abcd == 4)
//		{
//			BSP_CloseLOCK(0);
//			BSP_IOOpen(IO_RLY_BMSPOWERA);
//		}
//		if(abcd == 5)
//		{
//			BSP_IOOpen(IO_RLY_K3);
//		}
////测试继电器开关了
		
		
	}
}

/***********************************************************************************************
* Function		: main
* Description	:
* Input			:
* Output		:
* Note(s)		:
* Contributor	: yxy
***********************************************************************************************/
int main(void)
{
	OSTick_Init();       /*系统时钟初始化设置            	                         */
	NVIC_Configuration();   //等待外设硬件芯片正常工作
	OSInit();			 //初始化OS
	OSTaskCreateExt(init_task_core,
	                (void *)0,
	                (OS_STK *)&InitTaskStk[OS_INIT_TASK_STACK_SIZE - 1],
	                OS_TASK_INIT_PRIO,
	                OS_TASK_INIT_PRIO,
	                (OS_STK *)&InitTaskStk[0],
	                OS_INIT_TASK_STACK_SIZE,
	                (void *)0,
	                OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	OSStart();		//启动多任务环境
	return(0);
}

/***********************************************************************************************
* Function		:  assert_failed
* Description	:
* Input			:
* Output		:
* Note(s)		:
* Contributor	: yxy
***********************************************************************************************/
#ifdef  DEBUG_PARAM
void assert_failed(u8* file, u32 line)
{
	//User can add his own implementation to report the file name and line number,
	//ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line)

	//Infinite loop
	if (1)
	{
		NOP();
	}
}
#endif
/************************(C)COPYRIGHT 2018 汇誉科技***END OF FILE****************************/
