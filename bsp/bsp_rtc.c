
#include "bsp_rtc.h" 
#include "DwinProtocol.h"
/* ѡ��RTC��ʱ��Դ */
#define RTC_CLOCK_SOURCE_LSE       /* LSE */
//#define RTC_CLOCK_SOURCE_LSI     /* LSI */ 


__IO uint32_t uwAsynchPrediv = 0x7F;
__IO uint32_t uwSynchPrediv = 0xFF;

RTC_TimeTypeDef RTC_Time;
RTC_TimeTypeDef Alarm_Time,Alarm_TimeTemp;
RTC_DateTypeDef RTC_Date;
/*
*********************************************************************************************************
*	�� �� ��: RTC_Config
*	����˵��: 1. ѡ��ͬ��RTCʱ��ԴLSI����LSE��
*             2. ����RTCʱ�ӡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RTC_Config(void)
{
	RTC_InitTypeDef  RTC_InitStructure;
	
	/* ʹ��PWRʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* �������RTC */
	PWR_BackupAccessCmd(ENABLE);

	/* ѡ��LSI��Ϊʱ��Դ */
#if defined (RTC_CLOCK_SOURCE_LSI)  
	
	/* Enable the LSI OSC */ 
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	/* ѡ��RTCʱ��Դ */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	
	/* ck_spre(1Hz) = RTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
	uwSynchPrediv = 0xFF;
	uwAsynchPrediv = 0x7F;
	
	/* ѡ��LSE��ΪRTCʱ�� */
#elif defined (RTC_CLOCK_SOURCE_LSE)
	/* ʹ��LSE����  */
	RCC_LSEConfig(RCC_LSE_ON);

	/* �ȴ����� */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* ѡ��RTCʱ��Դ */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	uwSynchPrediv = 0xFF;
	uwAsynchPrediv = 0x7F;
#endif 

	/* ʹ��RTCʱ�� */
	RCC_RTCCLKCmd(ENABLE);

	/* �ȴ�RTC APB�Ĵ���ͬ�� */
	RTC_WaitForSynchro();

	/* ����RTC���ݼĴ����ͷ�Ƶ��  */
    /*32.768K/((127+1)*(255+1))=1Hz*/
	RTC_InitStructure.RTC_AsynchPrediv = uwAsynchPrediv;
	RTC_InitStructure.RTC_SynchPrediv = uwSynchPrediv;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;   /*24h*/
	RTC_Init(&RTC_InitStructure);
}
/* 
******************************************************************************** 
*  �� �� ��: RTC_TimeAndDate_Set 
*  ����˵��: ����ʱ�������
*  ��    �Σ�buf����ָ��
*  �� �� ֵ: ��   
******************************************************************************** 
*/ 
void RTC_TimeAndDate_Set(uint8_t *buf)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	
	/* ���������պ����� */
	RTC_DateStructure.RTC_Year = buf[0];
	RTC_DateStructure.RTC_Month = buf[1];
	RTC_DateStructure.RTC_Date = buf[2];
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Thursday;
	RTC_SetDate(RTC_Format_BINorBCD, &RTC_DateStructure);

	/* ����ʱ���룬�Լ���ʾ��ʽ */
	RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours   = buf[3];
	RTC_TimeStructure.RTC_Minutes = buf[4];
	RTC_TimeStructure.RTC_Seconds = buf[5]; 
	RTC_SetTime(RTC_Format_BINorBCD, &RTC_TimeStructure);   

	/* ���ñ��ݼĴ�������ʾ�Ѿ����ù�RTC */
	RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
}
/* 
******************************************************************************** 
*  �� �� ��: RTC_AlarmSet 
*  ����˵��: ��������
*  �� �Σ���  
*  �� �� ֵ: ��   
******************************************************************************** 
*/ 
void RTC_AlarmSet(void)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;
    RTC_AlarmTypeDef  RTC_AlarmStructure;

    /*=============================�ڢٲ�=============================*/
    /* RTC �����ж����� */
    /* RTC�����ж����� */ 
	EXTI_ClearITPendingBit(EXTI_Line17); 
	EXTI_InitStructure.EXTI_Line = EXTI_Line17; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure); 
	/* ʹ��RTC�����ж� */ 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 

    /*=============================�ڢ۲�=============================*/
    /* ʧ������ ������������ʱ���ʱ�������ʧ������*/
    RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	/* �������� BCD��ʽ��05Сʱ��20���ӣ�30�� */
	RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AM;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = 13;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = 30;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 0;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0x31;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;

	/* ���� RTC Alarm A �Ĵ��� */
	RTC_SetAlarm(RTC_Format_BINorBCD, RTC_Alarm_A, &RTC_AlarmStructure);

	/* ʹ�� RTC Alarm A �ж� */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);

	/* ʹ������ */
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	/* ���RTC���ӱ�־ */
	RTC_ClearFlag(RTC_FLAG_ALRAF);
    /* ��� EXTI Line 17 ����λ (�ڲ����ӵ�RTC Alarm) */
    EXTI_ClearITPendingBit(EXTI_Line17);
}

/* 
******************************************************************************** 
*  �� �� ��: bsp_InitRTC 
*  ����˵��: ��ʼ��RTC 
*  �� �Σ���  *  �� �� ֵ: ��   
******************************************************************************** 
*/ 
void bsp_InitRTC(void) 
{ 
	/* ���ڼ���Ƿ��Ѿ����ù�RTC��������ù��Ļ����������ý���ʱ 
	����RTC���ݼĴ���Ϊ0x32F2��������RTC���ݼĴ�������0x32F2   ��ô��ʾû�����ù�����Ҫ����RTC.   */ 
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2) 
	{ 
		/* RTC ���� */ 
		RTC_Config(); 
		
	//	RTC_TimeAndDate_Set(&HMI_Operate_Ctrl.SysTime[0]);

      //  RTC_AlarmSet();
 
		/* ����ϵ縴λ��־�Ƿ����� */ 
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) 
		{ 

		} 
	} 
	else 
	{ 
		/* ����ϵ縴λ��־�Ƿ����� */ 
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) 
		{ 

		} 
		/* ������Ÿ�λ��־�Ƿ����� */ 
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) 
		{ 
 
		} 
		
		/* ʹ��PWRʱ�� */ 
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); 
		/* �������RTC */ 
		PWR_BackupAccessCmd(ENABLE); 
		/* �ȴ� RTC APB �Ĵ���ͬ�� */ 
		RTC_WaitForSynchro(); 
		/* ���RTC���ӱ�־ */ 
		RTC_ClearFlag(RTC_FLAG_ALRAF); 
		/* ���RTC�����жϹ����־ */ 
		EXTI_ClearITPendingBit(EXTI_Line17); 
	} 
	
	//RTC_AlarmSet();
}	

/* 
******************************************************************************** 
*  �� �� ��: Get_RTC_TimeAndDate 
*  ����˵��: ��ȡʱ�������
*  �� �Σ���  
*  �� �� ֵ: ��   
******************************************************************************** 
*/ 
void Get_RTC_TimeAndDate(void) 
{ 
    // ��ȡ����
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
