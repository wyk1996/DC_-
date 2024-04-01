/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : RTC_task.c
* Author            : 
* Date First Issued :
* Version           : V0.1
* Description       : 给个人任务发送时钟消息
*----------------------------------------历史版本信息-------------------------------------------
* History       :
* Description   : 
*----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include <time.h>
#include "RTC_task.h"
#include "bsp_rtc.h"
#include "DataChangeInterface.h"


#if (BSP_RTCENABLE > 0u)
/* Private define-----------------------------------------------------------------------------*/
#define EIGHT_HOURS_SECONDS  (28800ul)   //8小时转成秒数
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
 _BSPRTC_TIME gRtcTime;            //当前RTC时间
static time_t UnixTimestamp;             //unix时间戳
//RTC初始化值 2017-07-01 10:00 星期六
const _BSPRTC_TIME gRTCInit = {0x00, 0x12, 0x09, 0x02, 0x09, 0x20, 0x06};
//RTC消息支持的最大个数
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/                                    
/*****************************************************************************
* Function     : GetCurTime
* Description  : 获取当前RTC时间
* Input        : _BSPRTC_TIME* pTime  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月14日        
*****************************************************************************/
INT8U GetCurTime(_BSPRTC_TIME* pTime)
{
    if (pTime == NULL)
    {
        return FALSE;
    }
    memcpy(pTime, &gRtcTime, sizeof(_BSPRTC_TIME) );
    return TRUE;
}

/*****************************************************************************
* Function     : GetCurUnixTimestamp
* Description  : 获取当前Uinx当前时间戳
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月14日       
*****************************************************************************/
INT32U GetCurUnixTimestamp(void)
{
    return UnixTimestamp;
}

/*****************************************************************************
* Function     : Change2Unixstamp
* Description  : unix时间戳转换为北京时间，注意北京时间为GMT+8时区，输入的参数都为16进制数
* Input        : unsigned int year  
                 unsigned int mon   
                 unsigned int day   
                 unsigned int hour  
                 unsigned int min   
                 unsigned int sec   
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年6月11日        
*****************************************************************************/
time_t Change2Unixstamp(INT32U year, INT32U mon, INT32U day, INT32U hour,
                        INT32U min, INT32U sec)
{
    if (0 >= (int) (mon -= 2)){    /**//* 1..12 -> 11,12,1..10 */
         mon += 12;      /**//* Puts Feb last since it has leap day */
         year -= 1;
    }
 
    return ( ( ( ( (time_t) (year/4 - year/100 + year/400 + 367*mon/12 + day) + year*365 - 719499
               )*24 + hour /**//* now have hours */
             )*60 + min /**//* now have minutes */
           )*60 + sec) - EIGHT_HOURS_SECONDS; /**//* finally seconds */
}
/*****************************************************************************
+* Function     : TaskRTC
* Description  : RTC任务
* Input        : void *pdata  
* Output       : None
* Return       : 
* Note(s)      : 读硬件时钟，提供准确的时间
* Contributor  : 2018年6月14日      
*****************************************************************************/
void TaskRTC(void *pdata)
{
    OSTimeDly(SYS_DELAY_100ms);
    bsp_InitRTC();
    OSTimeDly(SYS_DELAY_100ms);
	
    //获取RTC时间
	#if (USER_SYSTTC == 1)
    if (BSP_RTCGetTime(&gRtcTime) == FALSE)
    {
        printf("Get RTC error,set RTC default");
        //获取RTC时间错误，设置RTC
        if (BSP_RTCSetTime((_BSPRTC_TIME *)&gRTCInit) == FALSE)
        {
            //设置RTC失败，挂起RTC任务
            printf("Set RTC error, TaskRTC suspend!!!!!!!!!!");
            OSTaskSuspend(OS_PRIO_SELF);            //挂起当前任务   
        }
        //设置RTC时间成功，获取RTC时间
        OSTimeDly(SYS_DELAY_1s);
        if (BSP_RTCGetTime(&gRtcTime) == FALSE)
        {
            //再次获取RTC失败，挂起RTC任务
            printf("Get RTC error, TaskRTC suspend!!!!!!!!!!");
            OSTaskSuspend(OS_PRIO_SELF);            //挂起当前任务   
        }
    }
	#endif
    OSTimeDly(SYS_DELAY_1s);
    
	while(1)
	{
		OSTimeDly(SYS_DELAY_500ms);
		#if (USER_SYSTTC == 1)
		if (BSP_RTCGetTime(&gRtcTime) == FALSE)		 //获取时间
		#else
		if (APP_DWRTCGetTime(&gRtcTime) == FALSE)		 //获取时间
		#endif
		{
            continue;
		}
        //转化为unix时间戳
        UnixTimestamp = Change2Unixstamp(ByteBcdToHex(gRtcTime.Year) + 2000, ByteBcdToHex(gRtcTime.Month),
                                         ByteBcdToHex(gRtcTime.Day), ByteBcdToHex(gRtcTime.Hour), ByteBcdToHex(gRtcTime.Minute), ByteBcdToHex(gRtcTime.Second));
	}
}

#endif //#ifdef BSP_RTCENABLE 
/************************(C)COPYRIGHT 2018  杭州快电*****END OF FILE****************************/
