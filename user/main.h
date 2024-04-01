/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: MAIN.h
* Author			:  
* Date First Issued	: 10/12/2010
* Version			: V
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __MAIN_H_
#define __MAIN_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "bsp_conf.h"
/* Private define-----------------------------------------------------------------------------*/								 
#define	__DEBUG_TASK_TEST_ENABLE				 // ��������ʹ��
//#define LWIP_TASK_START_PRIO	10
//#define LWIP_TASK_END_PRIO		18
/* Private typedef----------------------------------------------------------------------------*/

//�ر�ע��  ���ȼ� 10 15 16 17 ����ʹ��


#define PRI_USB_MUTEX           			52                  //USB���������ȼ�
#define PRI_CONNECT_BMSA          5      //�������Խ�����
#define PRI_CONNECT_BMSB          6      //�������Խ�����
#define PRI_SEND_BMSA			  7		 //bms����
#define PRI_SEND_BMSB			  8		 //bms����

#define PRI_MODULE_SEND_MUTEX     9     //ģ�鷢�ͻ���

#define PRI_DISP				14		//��ʾ����
#define PRI_SAM					11		//��������

#define PRI_TEST_MAIN		12		//ģ������������


#define PRI_CONTROLA				18		//��������
#define PRI_CONTROLB				19		//��������
#define PRI_GPRSMAIN			20
#define PRI_GPRSSEND			21
#define PRI_GPRSRECV 			22
#define PRI_BILL				30
#define PRI_PERIOD        		 32    //����������
#define PRI_JUST				 35		//У׼����
#define PRI_USBMAIN				38
#define PRI_USBTASK				39
#define PRI_RTC					51
#define PRI_METER				52
#define PRI_CPUCARD				53

#define OS_TASK_INIT_PRIO 		56		//ϵͳ��ʼ���������ȼ�

#define LoadCodeAddress    0x08080000  //512K


#define PRI_4GSEND_MUTEX					13					//4G���ͻ���
#define VERSIONS    "V2.0"
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
void TaskStart(void *pdata);
#ifdef __DEBUG_TASK_TEST_ENABLE
void TaskTest(void *pdata);
#endif	//__DEBUG_TASK_TEST_ENABLE
static void init_task_core(void *pdata);
void TaskLED(void *pdata);
void TaskWDG(void *pdata);
#if (BSP_CANTESTENABLE > 0)
/*****************************************************************************
* Function     : TaskCntCCUMain
* Description  :  ����CCU������
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��5��22��        
*****************************************************************************/
void TaskCntCCUMain(void *pdata);
//void TaskCntCCURecv(void *pdata);
void TaskCntCCUSend(void *pdata);
void jump_to_app(uint32_t app_address);

#endif
#if (BSP_USBHOSTENABLE > 0)
/*****************************************************************************
* Function     : TaskCanTest
* Description  : Can��������
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��5��22��        Yxy
*****************************************************************************/
void TaskUsbMain(void *pdata);

void TaskUsbTask(void *pdata);
#endif

void TaskW5500Connect(void *pdata);
void TaskW5500Priod(void *pdata);
void TaskSDMaster(void *pdata);
void TaskPeriod(void *pdata);


void TaskControlGunA(void *pdata);
void TaskControlGunB(void *pdata);
void TaskCardStk(void *pdata);
void TaskMeterStk(void *pdata);
void TaskBillStk(void *pdata);
void TaskRTC(void *pdata);
void Task4GSend(void* pdata);
void Task4GRecv(void* pdata);
void Task4GMain(void* pdata);
void TaskDisplay(void *pdata);
void TaskConnectBMSAMain(void *pdata);
void TaskConnectBMSBMain(void *pdata);
void TaskSample(void *pdata);
void TaskPeriod(void *data);
void TaskADJust(void *pdata);	
void TestMain(void *pdata);
void IWDG_Config(uint8_t prv, uint16_t rlv);
extern INT8U SYS_APP_VER[3];
/* Private functions--------------------------------------------------------------------------*/
#endif
/************************(C)COPYRIGHT 2014 EFC*****END OF FILE****************************/
