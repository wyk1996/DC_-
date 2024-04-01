#ifndef __BSP_RTC_H
#define	__BSP_RTC_H




#include "stm32f4xx.h"
#include  "bsp_conf.h"
__packed typedef struct
{
	INT8U Second;								// ��
	INT8U Minute;								//��
	INT8U Hour;									// ʱ
	INT8U Day;									// ��
	INT8U Month;								// ��
	INT8U Year;									// ��
	INT8U Week;									// ��
}_BSPRTC_TIME;
// ʱ���ʽ�궨��
#define RTC_Format_BINorBCD  RTC_Format_BIN
/*******************************************************/
void bsp_InitRTC(void);
INT8U BSP_RTCGetTime(_BSPRTC_TIME *pTime);
INT8U BSP_RTCSetTime(_BSPRTC_TIME *pTime);
#endif /* __LED_H */

