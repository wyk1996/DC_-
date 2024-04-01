/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: MAIN.h
* Author			:  
* Date First Issued	: 10/12/2010
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __MAIN_H_
#define __MAIN_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_conf.h"
/* Private define-----------------------------------------------------------------------------*/								 
#define	__DEBUG_TASK_TEST_ENABLE				 // 测试任务使能
//#define LWIP_TASK_START_PRIO	10
//#define LWIP_TASK_END_PRIO		18
/* Private typedef----------------------------------------------------------------------------*/

//特别注意  优先级 10 15 16 17 不能使用


#define PRI_USB_MUTEX           			52                  //USB互斥锁优先级
#define PRI_CONNECT_BMSA          5      //跟汽车对接任务
#define PRI_CONNECT_BMSB          6      //跟汽车对接任务
#define PRI_SEND_BMSA			  7		 //bms发送
#define PRI_SEND_BMSB			  8		 //bms发送

#define PRI_MODULE_SEND_MUTEX     9     //模块发送互斥

#define PRI_DISP				14		//显示任务
#define PRI_SAM					11		//采样任务

#define PRI_TEST_MAIN		12		//模块周期性任务


#define PRI_CONTROLA				18		//控制任务
#define PRI_CONTROLB				19		//控制任务
#define PRI_GPRSMAIN			20
#define PRI_GPRSSEND			21
#define PRI_GPRSRECV 			22
#define PRI_BILL				30
#define PRI_PERIOD        		 32    //周期性任务
#define PRI_JUST				 35		//校准任务
#define PRI_USBMAIN				38
#define PRI_USBTASK				39
#define PRI_RTC					51
#define PRI_METER				52
#define PRI_CPUCARD				53

#define OS_TASK_INIT_PRIO 		56		//系统初始化任务优先级

#define LoadCodeAddress    0x08080000  //512K


#define PRI_4GSEND_MUTEX					13					//4G发送互斥
#define VERSIONS    "V2.0"
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
void TaskStart(void *pdata);
#ifdef __DEBUG_TASK_TEST_ENABLE
void TaskTest(void *pdata);
#endif	//__DEBUG_TASK_TEST_ENABLE
static void init_task_core(void *pdata);
void TaskLED(void *pdata);
void TaskWDG(void *pdata);
#if (BSP_CANTESTENABLE > 0)
/*****************************************************************************
* Function     : TaskCntCCUMain
* Description  :  连接CCU主任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年5月22日        
*****************************************************************************/
void TaskCntCCUMain(void *pdata);
//void TaskCntCCURecv(void *pdata);
void TaskCntCCUSend(void *pdata);
void jump_to_app(uint32_t app_address);

#endif
#if (BSP_USBHOSTENABLE > 0)
/*****************************************************************************
* Function     : TaskCanTest
* Description  : Can测试任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年5月22日        Yxy
*****************************************************************************/
void TaskUsbMain(void *pdata);

void TaskUsbTask(void *pdata);
#endif

void TaskW5500Connect(void *pdata);
void TaskW5500Priod(void *pdata);
void TaskSDMaster(void *pdata);
void TaskPeriod(void *pdata);


void TaskControlGunA(void *pdata);
void TaskControlGunB(void *pdata);
void TaskCardStk(void *pdata);
void TaskMeterStk(void *pdata);
void TaskBillStk(void *pdata);
void TaskRTC(void *pdata);
void Task4GSend(void* pdata);
void Task4GRecv(void* pdata);
void Task4GMain(void* pdata);
void TaskDisplay(void *pdata);
void TaskConnectBMSAMain(void *pdata);
void TaskConnectBMSBMain(void *pdata);
void TaskSample(void *pdata);
void TaskPeriod(void *data);
void TaskADJust(void *pdata);	
void TestMain(void *pdata);
void IWDG_Config(uint8_t prv, uint16_t rlv);
extern INT8U SYS_APP_VER[3];
/* Private functions--------------------------------------------------------------------------*/
#endif
/************************(C)COPYRIGHT 2014 EFC*****END OF FILE****************************/
