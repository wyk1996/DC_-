/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_conf.h
* Author			: 
* Date First Issued	: 
* Version			: V
* Description		: bsp的全局配置文件,主要定义了一些共用的结构类型
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_CONF_H_
#define	__BSP_CONF_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
/* Private define-----------------------------------------------------------------------------*/
#define BSP_DEBUGENABLE          (1u)   //1: 调试口使??
#define BSP_UARTENABLE           (1u)   //1: UART使能
#define BSP_EXTIIRQENABLE    	 (1u)	//1:外部中断使能
#define BSP_SPITISTENABLE	     (1u)   //1:SPI测试使能
#define BSP_CANTESTENABLE        (1u)   //1:can测试使能
#define BSP_ADCNABLE             (1u)   //1：ADC使能
#define BSP_USBHOSTENABLE        (1u)   //1: USB使能
#define BSP_STKCHKENABLE  		 (1u)	//1:各个任务栈使用率检??
#define BSP_GPRSENABLE           (1u)   //1: GPRS使能
#define BSP_RTCENABLE            (1u)   //1: RTC驱动模块使能
#define BSP_CPUCARDENABLE        (1u)   //1: CPU卡模块使??
#define BSP_FMENABLE             (1u)   //1: 铁电存储使能
//flash????

#define BSP_FLASH25ENABLE          (1u)   //1: MX25使能
#define BSP_FLASH26ENABLE          (0u)   //1: MX25使能

#define BSP_GPSENABLE			 (1u)	//1:GPS使能
#define BSP_WT588DENABLE		 (1u)	//1:扬声器使??

#define BSP_BEEPENABLE           (1u)   //1: 蜂鸣器使??
#define BSP_CANENABLE            (1u)   //1: can驱动使能
#define BSP_LEDENABLE            (1u)   //1: LED模块使能

#define BSP_REMOTESIGENABLE      (1u)   //1: 遥信信号模块使能
#define BSP_RLCENABLE            (1u)   //1: 继电器模块使??
#define BSP_WDGENABLE            (1u)   //1: 看门狗使??

#define BSP_ADENABLE             (1u)   //1: AD采样
#define BSP_SDENABLE             (1u)   //1: SD使能
#define BSP_LOCKENABLE           (1u)   //1: 锁驱动使??
#define BSP_PWMENABLE            (1u)   //1: PWM输出使能
#define BSP_LWIPENABLE           (1u)   //1: LWIP使能
#define BSP_WEBSERVERENABLE      (0u)   //1: Web Server使能



/* Private typedef----------------------------------------------------------------------------*/
// 消息ID定义,对不同的消息源进行定??不要??)
typedef enum
{
	BSP_MSGID_OUTTIME = 0x00,		//超时	

	BSP_MSGID_UART_RXOVER = 0x10,	// 串口接收完成
	BSP_MSGID_UART_TXOVER = 0x11,	// 串口发送完??
	
	BSP_MSGID_CYRF_RXOVER=0x12,		// CYRF接收完成
	BSP_MSGID_CYRF_TXOVER,			// CYRF发送完??
	BSP_MSGID_CYRF_RX_OVERTIME,		// CYRF接收超时
	BSP_MSGID_CYRF_TX_OVERTIME,		// CYRF发送超??
	BSP_MSGID_CAN_RXOVER,           // can接收完成
	BSP_MSGID_CAN_TXOVER,           // can发送完??
	BSP_MSGID_TX_DMA,               // DMA发送传输完??

	BSP_MSGID_KEY = 0x20,			// 按键

	BSP_CLOCK_ONE_MIN_MSG = 0x35,	//1分钟计时消息

    BSP_MSGID_USB,                  //来自USB任务
	BSP_GPRS_AT,					//来自GPRS模块发过来的任务
	BSP_GPRS_DATA,					//有数据接??
	BSP_MSGID_CARD,					//刷卡任务消息
	BSP_MSGID_DISP,					//显示任务消息
	BSP_MSGID_TCUCONTEXT,			//TCU??CCU???????
	BSP_MSGID_METER,			    //抄表任务
	BSP_4G_MAIN,
	BSP_4G_RECV,
	
	BSP_MSGID_BMS = 0xB3,           //????BMS????
    BSP_MSGID_BCU,                  //????BCU????
    BSP_MSGID_PCU,                  //????PCU????
    BSP_MSGID_INSULATION,           //?????????????
    BSP_MSGID_CONTROL,              //???????????
    BSP_MSGID_PERIOD,               //??????????????
    BSP_MSGID_GUIYUE,               //??????????
	BSP_PCU_CAN,
}_BSP_MSGID;


/*******************************************************************************
* 事件消息结构,所有线??系统)间消息传递时的数据结构??
*
* 线程(系统)间消息传递Convention (如C语言调用汇编函数那样的Convention)
* 1)由OSMboxPend(或OSQPend)线程，管理归零。归零函数OSMboxPost(或OSQPost)线程
*   提供??
* 
* 2)OSMboxPost(或OSQPost)线程为免“用同一个BSP_MESSAGE变量Msg1存储向某个事件发送的
*   不同消息，当前一个消息未处理时，发送后一个消息就会改变前一个消息的值。接收线
*   程处理该事件接收的消息时，会认为连续收到两个相同的消息。”的情况，OSMboxPost
*   (或OSQPost)线程可以设立Msg1、Msg2，BSP_MESSAGE变量。归零时Msgx.DataLen统一??
*   ??xFFFF，发送消息时可以通过Msgx.DataLen判断是否已使用。所有Msgx已使用则不发
*   送消息，等待归零??
* 
*  以上Convention可以保证线程(系统)间可靠、安全、简便的传递消息??
********************************************************************************/
typedef struct
{
	_BSP_MSGID MsgID;		// 消息ID,具体看_BSP_MSGID定义
	INT8U DivNum;			// 设备??
	INT16U DataLen;			// 数据长度, 操作注意见以上说??
	INT8U *pData;			// 数据指针, 操作注意见以上说??
	INT8U GunNum;			//???
}_BSP_MESSAGE;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
#if	BSP_OS
void BSP_OSPost(_OS_EVENT *pEvent, void *pMsg);
#endif	//BSP_OS
#define	SYSPost(e,m)			BSP_OSPost(e,m)	    // 发送事件函数重定义
#define	DIS_INT					OS_ENTER_CRITICAL	// 禁止中断
#define	EN_INT					OS_EXIT_CRITICAL	// 使能中断
#define	DIS_SCHED				OSSchedLock		    // 禁止调度
#define	EN_SCHED				OSSchedUnlock	    // 使能调度
#define	SYSTimeDly(t)			OSTimeDly(t)	    // 系统延时(单位:系统tick)

#endif	//__BSP_CONF_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
