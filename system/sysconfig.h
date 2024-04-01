/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: SYSCONFIG.h
* Author			: yxy
* Date First Issued	: 
* Version			: V
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef  _SYSCONFIG_H_
#define	 _SYSCONFIG_H_
/* Includes-----------------------------------------------------------------------------------*/
/* Includes-----------------------------------------------------------------------------------*/
#include "uCOS_II.H"
#include "stm32f4xx_conf.h"
#include "ucos_ii.h"
#include "main.h"
#include "arm_math.h"
#include "delay.h"
#include "app_conf.h"
#include "app_struct.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
typedef volatile INT8U					VINT8U;		// 可变的无符号8位整型变量
typedef volatile INT8S					VINT8S;		// 可变的有符号8位整型变量
typedef volatile INT8U const			VINT8UC;	// 可变的无符号8位整型变量(只读)
typedef volatile INT8S const			VINT8SC;	// 可变的有符号8位整型变量(只读)
typedef volatile INT16U					VINT16U;	// 可变的无符号16位整型变量
typedef volatile INT16S					VINT16S;	// 可变的有符号16位整型变量
typedef volatile INT16U const			VINT16UC;	// 可变的无符号16位整型变量(只读)
typedef volatile INT16S const			VINT16SC;	// 可变的有符号16位整型变量(只读)
typedef volatile INT32U  				VINT32U;	// 可变的无符号32位整型变量
typedef volatile INT32S 				VINT32S;	// 可变的有符号32位整型变量
typedef volatile INT32U const		    VINT32UC;	// 可变的无符号32位整型变量(只读)
typedef volatile INT32S const			VINT32SC;	// 可变的有符号32位整型变量(只读)

typedef unsigned long long int			INT64U;		// 可变的无符号64位整型变量
typedef signed long long int			INT64S;		// 可变的有符号64位整型变量

typedef volatile unsigned long long int			VINT64U;	// 可变的无符号64位整型变量
typedef volatile signed long long int			VINT64S;	// 可变的有符号64位整型变量
typedef volatile unsigned long long int const	VINT64UC;	// 可变的无符号64位整型变量(只读)
typedef volatile signed long long int const		VINT64SC;	// 可变的有符号64位整型变量(只读)


typedef enum {FALSE = 0, TRUE = !FALSE} bool;

#ifndef	NULL
#define	NULL					0				// 空
#endif
#ifndef NONE_PARITY
#define	NONE_PARITY				0	            // 无校验
#endif
#ifndef ODD_PARITY
#define	ODD_PARITY				1	            // 奇校验
#endif
#ifndef EVEN_PARITY
#define	EVEN_PARITY				2				// 偶校验
#endif
#ifndef BIT
#define BIT(x)					(1uL<<(x))
#endif
#ifndef NOP
#define	NOP()					__NOP()
#endif
#ifndef	CurrentSP
#define	CurrentSP()				__current_sp()
#endif	//CurrentSP
#ifndef	ReturnAddr
#define	ReturnAddr()			__return_address()
#endif	//ReturnAddr
#ifndef	CurrentPC
#define	CurrentPC()				__current_pc()
#endif	//CurrentPC
#ifndef Stringequal
#define Stringequal(s1,s2,count)  (memcmp((s1),(s2),count)==0)
#endif	//Stringequal
#define	OFFSETOF				offsetof
#pragma	anon_unions								// 使能简化的union命名

#define  Delay_10ms(x)											DelayUS(x*10000)

#define  maxoutbool(x,y)                        ( x>y?1:0 )
#define  compare(x,y)                           (x==y?TRUE:FALSE)
#define  maxoutfact(x,y)                        (x>=y?x:y)
#define  minoutfact(x,y)                        (x<=y?x:y)
#define  calcsize(x,y)                          (x>=y?x-y:y-x)
// 定义特殊寄存器
// 软件延时，系统延时时间定义
// 用于OSTimeDly()延时
// 当OS_TICKS_PER_SEC < 100 时，无法延时10ms
/**/
#define SYS_DELAY_1ms			 (OS_TICKS_PER_SEC/1000)
#define SYS_DELAY_2ms        	 (OS_TICKS_PER_SEC/500)
#define SYS_DELAY_5ms			 (OS_TICKS_PER_SEC/200)
#define SYS_DELAY_10ms			 (OS_TICKS_PER_SEC/100)
#define SYS_DELAY_20ms			 (OS_TICKS_PER_SEC/50)
#define SYS_DELAY_25ms			 (OS_TICKS_PER_SEC/40)
#define SYS_DELAY_40ms			 (OS_TICKS_PER_SEC/25)
#define SYS_DELAY_50ms			 (OS_TICKS_PER_SEC/20)
#define SYS_DELAY_100ms			 (OS_TICKS_PER_SEC/10)
#define SYS_DELAY_200ms			 (OS_TICKS_PER_SEC/5)
#define SYS_DELAY_250ms			 (OS_TICKS_PER_SEC/4)
#define SYS_DELAY_500ms			 (OS_TICKS_PER_SEC/2)
#define SYS_DELAY_1000ms		 (OS_TICKS_PER_SEC/1)
#define SYS_DELAY_1s			 (OS_TICKS_PER_SEC/1)
#define SYS_DELAY_2s			 (OS_TICKS_PER_SEC*2)
#define SYS_DELAY_3s			 (OS_TICKS_PER_SEC*3)
#define SYS_DELAY_4s			 (OS_TICKS_PER_SEC*4)
#define SYS_DELAY_5s			 (OS_TICKS_PER_SEC*5)
#define SYS_DELAY_6s			 (OS_TICKS_PER_SEC*6)
#define SYS_DELAY_8s			 (OS_TICKS_PER_SEC*8)
#define SYS_DELAY_10s			 (OS_TICKS_PER_SEC*10)
#define SYS_DELAY_15s			 (OS_TICKS_PER_SEC*15)
#define SYS_DELAY_20s			 (OS_TICKS_PER_SEC*20)
#define SYS_DELAY_30s			 (OS_TICKS_PER_SEC*30)
#define SYS_DELAY_40s			 (OS_TICKS_PER_SEC*40)
#define SYS_DELAY_1M			 (OS_TICKS_PER_SEC*60)
#define SYS_DELAY_2M			 (OS_TICKS_PER_SEC*120)
#define SYS_DELAY_5M			 (OS_TICKS_PER_SEC*300)
#define SYS_DELAY_10M			 (OS_TICKS_PER_SEC*600)

extern   INT32U   OSInterrputSum;                                  /* used by os to store critical-region times */
extern volatile   INT32U   WDG_clr_flag;	                       /* flag for window watch dog       */



void  RCC_Configuration(void);				                       /* Configures the system clocks    */
void  NVIC_Configuration(void);				                       /* Configures Vector Table base    */
void  Tmr_TickInit(void);					                       /* init the timer as os_tick	      */
//void  GPIO_AIM_Init(INT16U FGPort);			                       /* Config GPIO ports as analog-in  */
void  DelayUS(volatile INT32U nCount);                             /* delay us,according to clock     */
void  Delay200NS(void);                                            /* delay 200NS,according to clock  */
//void  GPIO_CFG(void);                                              /* Config GPIO pins 			      */
void  WWdg_Init(void);						                       /* Config Windows Watch dog	      */
void OSTick_Init(void);                                            /* Config  ucosii system dida*/


#endif //_SYSCONFIG_H_
/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/
