/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp.h
* Author			：
* Version			: V
* Description		        : bsp的头文件,主要是统一包涵所有的驱动程序,本来想整个模块使能的宏定义的,但是
现在编译器比较聪明就不需要这样了,全包涵就行了
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_H_
#define	__BSP_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
//一下为系统驱动接口文件，根据system的需要，在bsp_config.h中使能相关的驱动
#include "bsp_conf.h"
#include "bsp_MPulse.h"
#include "bsp_io.h"



#if ( (BSP_FLASHENABLE > 0u) || (BSP_FMENABLE > 0u) )
#include "bsp_spi.h"
#if (BSP_FLASHENABLE > 0u)
#include "bsp_mx25.h"
#endif  //BSP_FLASHENABLE

#if (BSP_FMENABLE > 0u)
#include "bsp_fm.h"
#endif  //BSP_FMENABLE
#endif  //#if ( (BSP_FLASHENABLE > 0u) || (BSP_FMENABLE > 0u) )

#if (BSP_UARTENABLE > 0u)
#include "bsp_uart.h"
#endif  //BSP_UARTNABLE

#if (BSP_CANENABLE > 0u)
#include "bsp_can.h"
#endif  //BSP_CANENABLE

#if (BSP_ICCARDENABLE > 0u) 
//#include "bsp_iccard.h"
#include "bsp_fm1702sl.h"
#endif //BSP_ICCARDENABLE




#if (BSP_RLCENABLE > 0u)
#include "bsp_io.h"
#endif //BSP_RLCEENABLE

#if (BSP_RTCENABLE > 0u)
#include "bsp_rtc.h"
#endif //BSP_RTCENABLE

#if (BSP_WDGENABLE > 0u)
#include "bsp_wdg.h"
#endif //BSP_WDGENABLE

#if (BSP_ADENABLE > 0u)  
#include "bsp_adc.h"
#endif //BSP_ADENABLE

// #ifdef BSP_USBHOSTENABLE
// #include "bsp_usb.h"
// #endif  //BSP_USBHOSTENABLE





//#if (BSP_USBHOSTENABLE > 0u)
//#include "usbh_usr.h" 
//#include "bsp_usb.h"
//#endif
 
#if (BSP_SDENABLE > 0u)
#include "bsp_sdio_sd.h"
#endif // BSP_SDENABLE

//#if (BSP_LWIPENABLE > 0u)
//#include "bsp_lan8720.h"
//#endif //BSP_LWIPENABLE

//#if (BSP_GPRSENABLE > 0u)
//#include "bsp_gprs.h"
//#endif //#if (BSP_GPRSENABLE > 0u)

//#if (BSP_DEBUGENABLE > 0u)
//#include "bsp_debug.h"
//#endif //BSP_DEBUGENABLE

#if (BSP_MYMALLOCENABLE > 0u)
#include "MyMalloc.h"
#endif

/* Private define-----------------------------------------------------------------------------*/

/***********************************************************************************************
* Function		: BSP_ArmReset
* Description	: ARM软件复位
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月25日
***********************************************************************************************/
void BSP_ArmReset(void);

/***********************************************************************************************
* Function		: BSP_TIME3Init
* Description	: 定时器3初始化
* Input			: 
* Output		: 
* Note(s)		: 普通定时器,中断,用于键盘扫描, 遥信检测
* Contributor	: 2018年7月25日
***********************************************************************************************/
void BSP_TIME3Init(void);

///*****************************************************************************
//* Function     : BSP_GetChipUniqueID
//* Description  : 获取芯片唯一设备ID
//* Input        : UID_TypeDef* Uid  
//* Output       : None
//* Return       : 
//* Note(s)      : 
//* Contributor  : 2018年7月25日
//*****************************************************************************/
//INT8U BSP_GetChipUniqueID(UID_TypeDef* Uid);

/*****************************************************************************
* Function     : BSP_GetEthernetMac
* Description  : 获取以太网MAC地址
* Input        : INT8U *pMac  
                 INT8U Len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U BSP_GetEthernetMac(INT8U *pMac, INT8U Len);

/***********************************************************************************************
* Function		: TargetInit
* Description	: 初始化硬件
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	:2018年7月25日
***********************************************************************************************/
void TargetInit(void);

/***********************************************************************************************
* Function		: BSPInit
* Description	: 驱动初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月25日
***********************************************************************************************/
void BSP_Init(INT8U *pdata, INT16U len);


#endif	//__BSP_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/


