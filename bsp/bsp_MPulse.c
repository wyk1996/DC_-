/*****************************************Copyright(C)******************************************
*******************************************æ­å·å¿«ç”µ*********************************************
*------------------------------------------æ–‡ä»¶ä¿¡æ¯---------------------------------------------
* FileName			: bsp_MPulse.h
* Author			: 
* Date First Issued	:    
* Version			: 
* Description		: 
*----------------------------------------å†å²ç‰ˆæœ¬ä¿¡æ¯-------------------------------------------
* History			: 
* Description		: 
*-----------------------------------------------------------------------------------------------*/
#include	"sysconfig.h"
#include	"bsp_MPulse.h"
#include "app_conf.h"
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
// Âö³å¼ÆÊı·´¶¶¶¯ÑÓÊ±
#define	MPLS_HIGH_DELAY			2				// >=2
#define	MPLS_LOW_DELAY			2				// >=2

typedef enum
{
	MPLS_STATE_HIGH=0u,							// ×´Ì¬¸ß
	MPLS_STATE_LOW,								// ×´Ì¬µÍ
	MPLS_STATE_UNDEF							// Î´¶¨Òå
}_MPLS_STATE;									// Âö³å¼ÆÊı¹Ü½Å×´Ì¬
// MPLSÓ²¼şÅäÖÃ½á¹¹
typedef struct
{
	GPIO_TypeDef *GPIOx;						// Òı½Å
	INT16U Port_Pin;							// Âö³åÊäÈë½Å
}_MPLS_HD;
// MPLSÓ²¼şÅäÖÃ±í


#if(USER_GUN != USER_SINGLE_GUN)
const _MPLS_HD MPLS_HD_Table[BSP_DI_MAX]=
{
	{GPIOE,BIT(13)}, 
	{GPIOE,BIT(14)}, 
	{GPIOE,BIT(15)}, 
	{GPIOB,BIT(10)}, 
	{GPIOD,BIT(10)}, 
	{GPIOD,BIT(11)},
	{GPIOD,BIT(12)}, 
	{GPIOD,BIT(13)},
	{GPIOD,BIT(14)}, 
	{GPIOD,BIT(15)}, 
};
#else
const _MPLS_HD MPLS_HD_Table[BSP_DI_MAX]=
{
	{GPIOE,BIT(9)},  	//¼±Í£
	{GPIOE,BIT(10)}, //·ÀÀ×
	{GPIOE,BIT(11)}, //ÃÅ½û
	{GPIOE,BIT(12)}, 	//½»Á÷¿Õ¿ª
	{GPIOE,BIT(13)}, //Ö±Á÷½Ó´¥Æ÷
	{GPIOE,BIT(14)},//±¸ÓÃ
	{GPIOE,BIT(15)}, //Ç¹ËøA
	{GPIOE,BIT(13)},
	{GPIOB,BIT(10)},   //¸¨ÖúµçÔ´ 
	{GPIOD,BIT(15)}, 
};
#endif
/* Private typedef----------------------------------------------------------------------------*/
// Âö³å¼ÆÊı¿ØÖÆ½á¹¹
typedef struct
{
	_MPLS_STATE Meter_Stat;						// ±£´æ×´Ì¬(Õâ¸ö×´Ì¬ÊÇÓÃÀ´ÅĞ¶ÏÂö³åµÄ)
	INT32U AllPulseInput;						// ±£´æ×ÜÂö³å¼ÆÊıÖµ,²»ÄÜÓÃclrº¯ÊıÇåÁã
	INT16U PulseInput;							// ±£´æÂö³å¼ÆÊıÖµ
	INT8U PinState;								// Òı½Å×´Ì¬(ÓÃÓÚÒ£ĞÅ·µ»Ø)
	INT8U LowStatCount;							// Òı½ÅµÍ×´Ì¬¼ÆÊ±
	INT8U HighStatCount;						// Òı½Å¸ß×´Ì¬¼ÆÊ±
}_MPLS_CONTROL;									// Âö³å¼ÆÊı¿ØÖÆ½á¹¹
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
_MPLS_CONTROL MPLS_Control[BSP_DI_MAX];	// Âö³å¼ÆÊı¿ØÖÆ±äÁ¿
/* Private function prototypes----------------------------------------------------------------*/
// ¶ÁÒı½Å×´Ì¬
#define	MPLSReadPulseState(num)	GPIO_ReadInputDataBit(MPLS_HD_Table[num].GPIOx,MPLS_HD_Table[num].Port_Pin)
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_MPLSScan
* Description	: Âö³å¿ØÖÆ³ÌĞò,Õâ¸ö³ÌĞòÓÉ¶¨Ê±Æ÷ÖĞ¶Ïµ÷ÓÃ,¶ÔÂö³åÒı½Å½øĞĞÅĞ¶Ï,¼ì²âÂö³åÊı.
* Input			: 
* Output		: 
* Note(s)		: ÓÃ»§ÎŞĞèÊ¹ÓÃ´Ëº¯Êı
* Contributor	: 2018å¹´7æœˆ25æ—¥
***********************************************************************************************/
void BSP_MPLSScan(void)
{
	INT8U i;
	for(i=0;i<BSP_DI_MAX;i++)
	{
		if(MPLSReadPulseState(i) == 0x00)		// Òı½ÅµÍ,¼ÆÊı
		{
			MPLS_Control[i].PinState = 0;
			MPLS_Control[i].HighStatCount = 0;
			MPLS_Control[i].LowStatCount++;
		}
		else									// Òı½Å¸ß,¼ÆÊı
		{
			MPLS_Control[i].PinState = 1;
			MPLS_Control[i].HighStatCount++;
			MPLS_Control[i].LowStatCount = 0;
		}
		// Âö³åµçÆ½¸ß
		if(MPLS_Control[i].HighStatCount >= MPLS_HIGH_DELAY)
		{
			MPLS_Control[i].Meter_Stat = MPLS_STATE_HIGH;
			MPLS_Control[i].HighStatCount = 0;
			MPLS_Control[i].LowStatCount = 0;
		}
		// Âö³åµçÆ½µÍ
		if(MPLS_Control[i].LowStatCount >= MPLS_LOW_DELAY)
		{
			if(MPLS_Control[i].Meter_Stat == MPLS_STATE_HIGH)
			{	//¸ßµ½µÍÌø±ä
				//¼ÆÊı ++
				MPLS_Control[i].AllPulseInput++;
				MPLS_Control[i].PulseInput++;
//				NOP();
			}
			MPLS_Control[i].Meter_Stat = MPLS_STATE_LOW;
			MPLS_Control[i].HighStatCount = 0;
			MPLS_Control[i].LowStatCount = 0;
		}
	}
}
/***********************************************************************************************
* Function		: BSP_MPLSReset
* Description	: ¸´Î»Ä³¸öÂö³å
* Input			: num:Âö³åÉè±¸ºÅ(0 ~ BSP_YX_MAX-1)
* Output		: 
* Note(s)		: 
* Contributor	:2018å¹´7æœˆ25æ—¥
***********************************************************************************************/
void BSP_MPLSReset(INT8U num)
{
	INT8U i,*p;
	
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
	
	OS_ENTER_CRITICAL();  // ½ûÖ¹ÖĞ¶Ï£¬·ÀÖ¹Çå³ıµ½Ò»°ëµÄÊ±ºò£¬È¥É¨Ãè

	/* Çå²ÎÊı */
	p = (INT8U *)(&MPLS_Control[num]);
	for(i=0;i<sizeof(_MPLS_CONTROL);i++)
		p[i] = 0x00;
	MPLS_Control[num].Meter_Stat = MPLS_STATE_HIGH;

	OS_EXIT_CRITICAL();
}
/***********************************************************************************************
* Function		: BSP_MPLSClear
* Description	: ÇåÁãÂö³å¼ÆÊıÆ÷µÄÖµ
* Input			: num:Âö³åÉè±¸ºÅ(0 ~ BSP_YX_MAX-1)
* Output		: 
* Note(s)		: 
* Contributor	:2018å¹´7æœˆ25æ—¥
***********************************************************************************************/
void BSP_MPLSClear(INT8U num)
{
	MPLS_Control[num].PulseInput=0;
}
/***********************************************************************************************
* Function		: BSP_MPLSRead
* Description	: ¶ÁÈ¡Âö³åÖµ,ÓÃ»§Í¨¹ı´Ë³ÌĞòµÃµ½Ä³¸öÂö³åÉè±¸µÄÂö³å¼ÆÊıÖµ
* Input			: num:Âö³åÉè±¸ºÅ(0 ~ BSP_YX_MAX-1)
* Output		: Âö³åÖµ
* Note(s)		: 
* Contributor	: 2018å¹´7æœˆ25æ—¥
***********************************************************************************************/
INT16U BSP_MPLSRead(INT8U num)
{
    /* ¹Ü½ÅÅäÖÃË¢ĞÂ£¬Ìá¸ßEMC */
	GPIO_InitTypeDef 	GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = MPLS_HD_Table[num].Port_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(MPLS_HD_Table[num].GPIOx, &GPIO_InitStructure);
	
	return MPLS_Control[num].PulseInput;
}

INT8U BSP_MPLSState(INT8U num)
{
	return MPLS_Control[num].PinState;
}
/***********************************************************************************************
* Function		: BSP_MPLSInit
* Description	: ³õÊ¼»¯,³õÊ¼»¯²ÎÊı,ÅäÖÃÒı½Å
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018å¹´7æœˆ25æ—¥
***********************************************************************************************/
void BSP_MPLSInit(void)
{
	INT8U i;
	GPIO_InitTypeDef 	GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE |RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOD, ENABLE);
	
	for(i=0; i<BSP_DI_MAX; i++)
	{
		// ÅäÖÃÒı½Å
		GPIO_InitStructure.GPIO_Pin = MPLS_HD_Table[i].Port_Pin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Init(MPLS_HD_Table[i].GPIOx, &GPIO_InitStructure);
		// Çå²ÎÊı
		BSP_MPLSReset(i);
	}
}


/***********************************************************************************************
* Function		: BSP_GetJTState
* Description	: »ñÈ¡µç×ÓËø×´Ì¬
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	:20210719
***********************************************************************************************/
INT8U BSP_GetLockState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return 0;
	}
	#if ENBLE_JG
	__NOP;
	#else
	if(gun == GUN_A)
	{
		BSP_MPLSState(BSP_DI_LOCKA);
	}
	else
	{
		BSP_MPLSState(BSP_DI_LOCKB);
	}
	#endif
	return 0;
}

/************************(C)COPYRIGHT 2018 æ­å·å¿«ç”µ*****END OF FILE****************************/
