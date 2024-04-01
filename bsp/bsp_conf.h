/*****************************************Copyright(C)******************************************
*******************************************���ݿ��*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: bsp_conf.h
* Author			: 
* Date First Issued	: 
* Version			: V
* Description		: bsp��ȫ�������ļ�,��Ҫ������һЩ���õĽṹ����
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
#define BSP_DEBUGENABLE          (1u)   //1: ���Կ�ʹ??
#define BSP_UARTENABLE           (1u)   //1: UARTʹ��
#define BSP_EXTIIRQENABLE    	 (1u)	//1:�ⲿ�ж�ʹ��
#define BSP_SPITISTENABLE	     (1u)   //1:SPI����ʹ��
#define BSP_CANTESTENABLE        (1u)   //1:can����ʹ��
#define BSP_ADCNABLE             (1u)   //1��ADCʹ��
#define BSP_USBHOSTENABLE        (1u)   //1: USBʹ��
#define BSP_STKCHKENABLE  		 (1u)	//1:��������ջʹ���ʼ�??
#define BSP_GPRSENABLE           (1u)   //1: GPRSʹ��
#define BSP_RTCENABLE            (1u)   //1: RTC����ģ��ʹ��
#define BSP_CPUCARDENABLE        (1u)   //1: CPU��ģ��ʹ??
#define BSP_FMENABLE             (1u)   //1: ����洢ʹ��
//flash????

#define BSP_FLASH25ENABLE          (1u)   //1: MX25ʹ��
#define BSP_FLASH26ENABLE          (0u)   //1: MX25ʹ��

#define BSP_GPSENABLE			 (1u)	//1:GPSʹ��
#define BSP_WT588DENABLE		 (1u)	//1:������ʹ??

#define BSP_BEEPENABLE           (1u)   //1: ������ʹ??
#define BSP_CANENABLE            (1u)   //1: can����ʹ��
#define BSP_LEDENABLE            (1u)   //1: LEDģ��ʹ��

#define BSP_REMOTESIGENABLE      (1u)   //1: ң���ź�ģ��ʹ��
#define BSP_RLCENABLE            (1u)   //1: �̵���ģ��ʹ??
#define BSP_WDGENABLE            (1u)   //1: ���Ź�ʹ??

#define BSP_ADENABLE             (1u)   //1: AD����
#define BSP_SDENABLE             (1u)   //1: SDʹ��
#define BSP_LOCKENABLE           (1u)   //1: ������ʹ??
#define BSP_PWMENABLE            (1u)   //1: PWM���ʹ��
#define BSP_LWIPENABLE           (1u)   //1: LWIPʹ��
#define BSP_WEBSERVERENABLE      (0u)   //1: Web Serverʹ��



/* Private typedef----------------------------------------------------------------------------*/
// ��ϢID����,�Բ�ͬ����ϢԴ���ж�??��Ҫ??)
typedef enum
{
	BSP_MSGID_OUTTIME = 0x00,		//��ʱ	

	BSP_MSGID_UART_RXOVER = 0x10,	// ���ڽ������
	BSP_MSGID_UART_TXOVER = 0x11,	// ���ڷ�����??
	
	BSP_MSGID_CYRF_RXOVER=0x12,		// CYRF�������
	BSP_MSGID_CYRF_TXOVER,			// CYRF������??
	BSP_MSGID_CYRF_RX_OVERTIME,		// CYRF���ճ�ʱ
	BSP_MSGID_CYRF_TX_OVERTIME,		// CYRF���ͳ�??
	BSP_MSGID_CAN_RXOVER,           // can�������
	BSP_MSGID_CAN_TXOVER,           // can������??
	BSP_MSGID_TX_DMA,               // DMA���ʹ�����??

	BSP_MSGID_KEY = 0x20,			// ����

	BSP_CLOCK_ONE_MIN_MSG = 0x35,	//1���Ӽ�ʱ��Ϣ

    BSP_MSGID_USB,                  //����USB����
	BSP_GPRS_AT,					//����GPRSģ�鷢����������
	BSP_GPRS_DATA,					//�����ݽ�??
	BSP_MSGID_CARD,					//ˢ��������Ϣ
	BSP_MSGID_DISP,					//��ʾ������Ϣ
	BSP_MSGID_TCUCONTEXT,			//TCU??CCU???????
	BSP_MSGID_METER,			    //��������
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
* �¼���Ϣ�ṹ,������??ϵͳ)����Ϣ����ʱ�����ݽṹ??
*
* �߳�(ϵͳ)����Ϣ����Convention (��C���Ե��û�ຯ��������Convention)
* 1)��OSMboxPend(��OSQPend)�̣߳�������㡣���㺯��OSMboxPost(��OSQPost)�߳�
*   �ṩ??
* 
* 2)OSMboxPost(��OSQPost)�߳�Ϊ�⡰��ͬһ��BSP_MESSAGE����Msg1�洢��ĳ���¼����͵�
*   ��ͬ��Ϣ����ǰһ����Ϣδ����ʱ�����ͺ�һ����Ϣ�ͻ�ı�ǰһ����Ϣ��ֵ��������
*   �̴�����¼����յ���Ϣʱ������Ϊ�����յ�������ͬ����Ϣ�����������OSMboxPost
*   (��OSQPost)�߳̿�������Msg1��Msg2��BSP_MESSAGE����������ʱMsgx.DataLenͳһ??
*   ??xFFFF��������Ϣʱ����ͨ��Msgx.DataLen�ж��Ƿ���ʹ�á�����Msgx��ʹ���򲻷�
*   ����Ϣ���ȴ�����??
* 
*  ����Convention���Ա�֤�߳�(ϵͳ)��ɿ�����ȫ�����Ĵ�����Ϣ??
********************************************************************************/
typedef struct
{
	_BSP_MSGID MsgID;		// ��ϢID,���忴_BSP_MSGID����
	INT8U DivNum;			// �豸??
	INT16U DataLen;			// ���ݳ���, ����ע�������˵??
	INT8U *pData;			// ����ָ��, ����ע�������˵??
	INT8U GunNum;			//???
}_BSP_MESSAGE;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
#if	BSP_OS
void BSP_OSPost(_OS_EVENT *pEvent, void *pMsg);
#endif	//BSP_OS
#define	SYSPost(e,m)			BSP_OSPost(e,m)	    // �����¼������ض���
#define	DIS_INT					OS_ENTER_CRITICAL	// ��ֹ�ж�
#define	EN_INT					OS_EXIT_CRITICAL	// ʹ���ж�
#define	DIS_SCHED				OSSchedLock		    // ��ֹ����
#define	EN_SCHED				OSSchedUnlock	    // ʹ�ܵ���
#define	SYSTimeDly(t)			OSTimeDly(t)	    // ϵͳ��ʱ(��λ:ϵͳtick)

#endif	//__BSP_CONF_H_
/************************(C)COPYRIGHT 2018 ���ݿ��*****END OF FILE****************************/
