#ifndef __BSP_RTC_H
#define	__BSP_RTC_H




#include "stm32f4xx.h"
#include  "bsp_conf.h"
__packed typedef struct
{
	INT8U Second;								// 秒
	INT8U Minute;								//分
	INT8U Hour;									// 时
	INT8U Day;									// 日
	INT8U Month;								// 月
	INT8U Year;									// 年
	INT8U Week;									// 周
}_BSPRTC_TIME;
// 时间格式宏定义
#define RTC_Format_BINorBCD  RTC_Format_BIN
/*******************************************************/
void bsp_InitRTC(void);
INT8U BSP_RTCGetTime(_BSPRTC_TIME *pTime);
INT8U BSP_RTCSetTime(_BSPRTC_TIME *pTime);
#endif /* __LED_H */

