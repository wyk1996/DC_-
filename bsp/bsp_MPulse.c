/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_MPulse.h
* Author			: 
* Date First Issued	:    
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			: 
* Description		: 
*-----------------------------------------------------------------------------------------------*/
#include	"sysconfig.h"
#include	"bsp_MPulse.h"
#include "app_conf.h"
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/
// ���������������ʱ
#define	MPLS_HIGH_DELAY			2				// >=2
#define	MPLS_LOW_DELAY			2				// >=2

typedef enum
{
	MPLS_STATE_HIGH=0u,							// ״̬��
	MPLS_STATE_LOW,								// ״̬��
	MPLS_STATE_UNDEF							// δ����
}_MPLS_STATE;									// ��������ܽ�״̬
// MPLSӲ�����ýṹ
typedef struct
{
	GPIO_TypeDef *GPIOx;						// ����
	INT16U Port_Pin;							// ���������
}_MPLS_HD;
// MPLSӲ�����ñ�


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
	{GPIOE,BIT(9)},  	//��ͣ
	{GPIOE,BIT(10)}, //����
	{GPIOE,BIT(11)}, //�Ž�
	{GPIOE,BIT(12)}, 	//�����տ�
	{GPIOE,BIT(13)}, //ֱ���Ӵ���
	{GPIOE,BIT(14)},//����
	{GPIOE,BIT(15)}, //ǹ��A
	{GPIOE,BIT(13)},
	{GPIOB,BIT(10)},   //������Դ 
	{GPIOD,BIT(15)}, 
};
#endif
/* Private typedef----------------------------------------------------------------------------*/
// ����������ƽṹ
typedef struct
{
	_MPLS_STATE Meter_Stat;						// ����״̬(���״̬�������ж������)
	INT32U AllPulseInput;						// �������������ֵ,������clr��������
	INT16U PulseInput;							// �����������ֵ
	INT8U PinState;								// ����״̬(����ң�ŷ���)
	INT8U LowStatCount;							// ���ŵ�״̬��ʱ
	INT8U HighStatCount;						// ���Ÿ�״̬��ʱ
}_MPLS_CONTROL;									// ����������ƽṹ
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
_MPLS_CONTROL MPLS_Control[BSP_DI_MAX];	// ����������Ʊ���
/* Private function prototypes----------------------------------------------------------------*/
// ������״̬
#define	MPLSReadPulseState(num)	GPIO_ReadInputDataBit(MPLS_HD_Table[num].GPIOx,MPLS_HD_Table[num].Port_Pin)
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_MPLSScan
* Description	: ������Ƴ���,��������ɶ�ʱ���жϵ���,���������Ž����ж�,���������.
* Input			: 
* Output		: 
* Note(s)		: �û�����ʹ�ô˺���
* Contributor	: 2018年7月25日
***********************************************************************************************/
void BSP_MPLSScan(void)
{
	INT8U i;
	for(i=0;i<BSP_DI_MAX;i++)
	{
		if(MPLSReadPulseState(i) == 0x00)		// ���ŵ�,����
		{
			MPLS_Control[i].PinState = 0;
			MPLS_Control[i].HighStatCount = 0;
			MPLS_Control[i].LowStatCount++;
		}
		else									// ���Ÿ�,����
		{
			MPLS_Control[i].PinState = 1;
			MPLS_Control[i].HighStatCount++;
			MPLS_Control[i].LowStatCount = 0;
		}
		// �����ƽ��
		if(MPLS_Control[i].HighStatCount >= MPLS_HIGH_DELAY)
		{
			MPLS_Control[i].Meter_Stat = MPLS_STATE_HIGH;
			MPLS_Control[i].HighStatCount = 0;
			MPLS_Control[i].LowStatCount = 0;
		}
		// �����ƽ��
		if(MPLS_Control[i].LowStatCount >= MPLS_LOW_DELAY)
		{
			if(MPLS_Control[i].Meter_Stat == MPLS_STATE_HIGH)
			{	//�ߵ�������
				//���� ++
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
* Description	: ��λĳ������
* Input			: num:�����豸��(0 ~ BSP_YX_MAX-1)
* Output		: 
* Note(s)		: 
* Contributor	:2018年7月25日
***********************************************************************************************/
void BSP_MPLSReset(INT8U num)
{
	INT8U i,*p;
	
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif
	
	OS_ENTER_CRITICAL();  // ��ֹ�жϣ���ֹ�����һ���ʱ��ȥɨ��

	/* ����� */
	p = (INT8U *)(&MPLS_Control[num]);
	for(i=0;i<sizeof(_MPLS_CONTROL);i++)
		p[i] = 0x00;
	MPLS_Control[num].Meter_Stat = MPLS_STATE_HIGH;

	OS_EXIT_CRITICAL();
}
/***********************************************************************************************
* Function		: BSP_MPLSClear
* Description	: ���������������ֵ
* Input			: num:�����豸��(0 ~ BSP_YX_MAX-1)
* Output		: 
* Note(s)		: 
* Contributor	:2018年7月25日
***********************************************************************************************/
void BSP_MPLSClear(INT8U num)
{
	MPLS_Control[num].PulseInput=0;
}
/***********************************************************************************************
* Function		: BSP_MPLSRead
* Description	: ��ȡ����ֵ,�û�ͨ���˳���õ�ĳ�������豸���������ֵ
* Input			: num:�����豸��(0 ~ BSP_YX_MAX-1)
* Output		: ����ֵ
* Note(s)		: 
* Contributor	: 2018年7月25日
***********************************************************************************************/
INT16U BSP_MPLSRead(INT8U num)
{
    /* �ܽ�����ˢ�£����EMC */
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
* Description	: ��ʼ��,��ʼ������,��������
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月25日
***********************************************************************************************/
void BSP_MPLSInit(void)
{
	INT8U i;
	GPIO_InitTypeDef 	GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE |RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOD, ENABLE);
	
	for(i=0; i<BSP_DI_MAX; i++)
	{
		// ��������
		GPIO_InitStructure.GPIO_Pin = MPLS_HD_Table[i].Port_Pin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Init(MPLS_HD_Table[i].GPIOx, &GPIO_InitStructure);
		// �����
		BSP_MPLSReset(i);
	}
}


/***********************************************************************************************
* Function		: BSP_GetJTState
* Description	: ��ȡ������״̬
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

/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
