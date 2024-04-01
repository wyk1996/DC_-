
#include "bsp_rtc.h" 
#include "DwinProtocol.h"
/* 选择RTC的时钟源 */
#define RTC_CLOCK_SOURCE_LSE       /* LSE */
//#define RTC_CLOCK_SOURCE_LSI     /* LSI */ 


__IO uint32_t uwAsynchPrediv = 0x7F;
__IO uint32_t uwSynchPrediv = 0xFF;

RTC_TimeTypeDef RTC_Time;
RTC_TimeTypeDef Alarm_Time,Alarm_TimeTemp;
RTC_DateTypeDef RTC_Date;
/*
*********************************************************************************************************
*	函 数 名: RTC_Config
*	功能说明: 1. 选择不同的RTC时钟源LSI或者LSE。
*             2. 配置RTC时钟。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_Config(void)
{
	RTC_InitTypeDef  RTC_InitStructure;
	
	/* 使能PWR时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* 允许访问RTC */
	PWR_BackupAccessCmd(ENABLE);

	/* 选择LSI作为时钟源 */
#if defined (RTC_CLOCK_SOURCE_LSI)  
	
	/* Enable the LSI OSC */ 
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	/* 选择RTC时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
	uwSynchPrediv = 0xFF;
	uwAsynchPrediv = 0x7F;
	
	/* 选择LSE作为RTC时钟 */
#elif defined (RTC_CLOCK_SOURCE_LSE)
	/* 使能LSE振荡器  */
	RCC_LSEConfig(RCC_LSE_ON);

	/* 等待就绪 */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* 选择RTC时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	uwSynchPrediv = 0xFF;
	uwAsynchPrediv = 0x7F;
#endif 

	/* 使能RTC时钟 */
	RCC_RTCCLKCmd(ENABLE);

	/* 等待RTC APB寄存器同步 */
	RTC_WaitForSynchro();

	/* 配置RTC数据寄存器和分频器  */
    /*32.768K/((127+1)*(255+1))=1Hz*/
	RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
	RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;   /*24h*/
	RTC_Init(&RTC_InitStructure);
}
/* 
******************************************************************************** 
*  函 数 名: RTC_TimeAndDate_Set 
*  功能说明: 设置时间和日期
*  形    参：buf数组指针
*  返 回 值: 无   
******************************************************************************** 
*/ 
void RTC_TimeAndDate_Set(uint8_t *buf)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	
	/* 设置年月日和星期 */
	RTC_DateStructure.RTC_Year = buf[0];
	RTC_DateStructure.RTC_Month = buf[1];
	RTC_DateStructure.RTC_Date = buf[2];
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Thursday;
	RTC_SetDate(RTC_Format_BINorBCD, &RTC_DateStructure);

	/* 设置时分秒，以及显示格式 */
	RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours   = buf[3];
	RTC_TimeStructure.RTC_Minutes = buf[4];
	RTC_TimeStructure.RTC_Seconds = buf[5]; 
	RTC_SetTime(RTC_Format_BINorBCD, &RTC_TimeStructure);   

	/* 配置备份寄存器，表示已经设置过RTC */
	RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
}
/* 
******************************************************************************** 
*  函 数 名: RTC_AlarmSet 
*  功能说明: 设置闹钟
*  形 参：无  
*  返 回 值: 无   
******************************************************************************** 
*/ 
void RTC_AlarmSet(void)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;
    RTC_AlarmTypeDef  RTC_AlarmStructure;

    /*=============================第①步=============================*/
    /* RTC 闹钟中断配置 */
    /* RTC闹钟中断配置 */ 
	EXTI_ClearITPendingBit(EXTI_Line17); 
	EXTI_InitStructure.EXTI_Line = EXTI_Line17; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure); 
	/* 使能RTC闹钟中断 */ 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 

    /*=============================第③步=============================*/
    /* 失能闹钟 ，在设置闹钟时间的时候必须先失能闹钟*/
    RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	/* 设置闹钟 BCD格式，05小时，20分钟，30秒 */
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 13;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 30;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;

	/* 配置 RTC Alarm A 寄存器 */
	RTC_SetAlarm(RTC_Format_BINorBCD, RTC_Alarm_A, &RTC_AlarmStructure);

	/* 使能 RTC Alarm A 中断 */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);

	/* 使能闹钟 */
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	/* 清除RTC闹钟标志 */
	RTC_ClearFlag(RTC_FLAG_ALRAF);
    /* 清除 EXTI Line 17 悬起位 (内部连接到RTC Alarm) */
    EXTI_ClearITPendingBit(EXTI_Line17);
}

/* 
******************************************************************************** 
*  函 数 名: bsp_InitRTC 
*  功能说明: 初始化RTC 
*  形 参：无  *  返 回 值: 无   
******************************************************************************** 
*/ 
void bsp_InitRTC(void) 
{ 
	/* 用于检测是否已经配置过RTC，如果配置过的话，会在配置结束时 
	设置RTC备份寄存器为0x32F2。如果检测RTC备份寄存器不是0x32F2   那么表示没有配置过，需要配置RTC.   */ 
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2) 
	{ 
		/* RTC 配置 */ 
		RTC_Config(); 
		
	//	RTC_TimeAndDate_Set(&HMI_Operate_Ctrl.SysTime[0]);

      //  RTC_AlarmSet();
 
		/* 检测上电复位标志是否设置 */ 
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) 
		{ 

		} 
	} 
	else 
	{ 
		/* 检测上电复位标志是否设置 */ 
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) 
		{ 

		} 
		/* 检测引脚复位标志是否设置 */ 
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) 
		{ 
 
		} 
		
		/* 使能PWR时钟 */ 
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); 
		/* 允许访问RTC */ 
		PWR_BackupAccessCmd(ENABLE); 
		/* 等待 RTC APB 寄存器同步 */ 
		RTC_WaitForSynchro(); 
		/* 清除RTC闹钟标志 */ 
		RTC_ClearFlag(RTC_FLAG_ALRAF); 
		/* 清除RTC闹钟中断挂起标志 */ 
		EXTI_ClearITPendingBit(EXTI_Line17); 
	} 
	
	//RTC_AlarmSet();
}	

/* 
******************************************************************************** 
*  函 数 名: Get_RTC_TimeAndDate 
*  功能说明: 获取时间和日期
*  形 参：无  
*  返 回 值: 无   
******************************************************************************** 
*/ 
void Get_RTC_TimeAndDate(void) 
{ 
    // 获取日历
    RTC_GetTime(RTC_Format_BIN, &RTC_Time);
    RTC_GetDate(RTC_Format_BIN, &RTC_Date);

}


INT8U BSP_RTCGetTime(_BSPRTC_TIME *pTime)
{
	 RTC_GetTime(RTC_Format_BIN, &RTC_Time);
    RTC_GetDate(RTC_Format_BIN, &RTC_Date);
	pTime->Day = HEXtoBCD(RTC_Date.RTC_Date);
	pTime->Month = HEXtoBCD(RTC_Date.RTC_Month);
	pTime->Year = HEXtoBCD(RTC_Date.RTC_Year);
	pTime->Week = HEXtoBCD(RTC_Date.RTC_WeekDay);
	pTime->Second = HEXtoBCD(RTC_Time.RTC_Seconds);
	pTime->Minute = HEXtoBCD(RTC_Time.RTC_Minutes);
	pTime->Hour = HEXtoBCD(RTC_Time.RTC_Hours);
	return TRUE;
	
}
INT8U BSP_RTCSetTime(_BSPRTC_TIME *pTime)
{
	INT8U buf[6] = {0};
	buf[0] = BCDtoHEX(pTime->Year);
	buf[1] = BCDtoHEX(pTime->Month);
	buf[2] = BCDtoHEX(pTime->Day);
	buf[3] = BCDtoHEX(pTime->Hour);
	buf[4] = BCDtoHEX(pTime->Minute);
	buf[5] = BCDtoHEX(pTime->Second);
	
	RTC_TimeAndDate_Set(buf);
	return TRUE;
}



/*********************************************END OF FILE**********************/
