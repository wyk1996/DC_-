/*****************************************Copyright(C)******************************************
*******************************************���ݿ��*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: GPRSRecv.c
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
//#include "GPSTest.h"
#include <string.h>
#include "sysconfig.h"
#include "bsp_uart.h"
/* Private define-----------------------------------------------------------------------------*/
#define GPSRECV_Q_LEN      20
#define GPS_UART 	   		UART8

#define GPS_SEND_BUF_LEN          (120u)      //��ʾ������buf��С
#define GPS_RECV_BUF_LEN          (120u)      //��ʾ�������buf��С
#define GPS_Q_LEN              	(16u)       //��ʾ������Ϣ���и���
#define GPS_TEMP_BUF_LEN          (120u)      //��ʾ���񱣴�һ֡���ݵ�buf��С

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPSRECVOSQ[GPSRECV_Q_LEN];					// ��Ϣ����
static OS_EVENT *GPSRECVTaskEvent;				            // ʹ�õ��¼�

static INT8U GPSSendBuf[GPS_SEND_BUF_LEN];            // ��ʾ���ͻ���
static INT8U GPSRecvBuf[GPS_RECV_BUF_LEN];            // ��ʾ���ջ���
static INT8U GPSRecvSMem[GPS_Q_LEN * sizeof(_BSP_MESSAGE)]; //��Ϣʵ��ĸ�����ʵ���Ͼ���_BSP_MESSAGE�ṹ��ĸ���
static INT8U GPSTempBuf[GPS_TEMP_BUF_LEN];            // ��ʾ����һ֡���ݻ���

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//ר����GPRS���մ���
/*****************************************************************************
* Function     : GPRSRecv_HardwareInit
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��6��14��       
*****************************************************************************/
static INT8U GPSRecv_HardwareInit(void)
{
    _BSP_UART_SET UartGPSSet;                      
    
    GPSRECVTaskEvent = OSQCreate(GPSRECVOSQ, GPSRECV_Q_LEN);//������ʾ������Ϣ����
    if (GPSRECVTaskEvent == NULL)
    {
        OSTaskSuspend(OS_PRIO_SELF);    //����ǰ����
        return FALSE;
    }
    memset(GPSSendBuf, 0 ,sizeof(GPSSendBuf));
    memset(GPSRecvBuf, 0 ,sizeof(GPSRecvBuf));

    UartGPSSet.BaudRate = 115200;
    UartGPSSet.DataBits = BSPUART_WORDLENGTH_8;
    UartGPSSet.Parity   = BSPUART_PARITY_NO;
    UartGPSSet.StopBits = BSPUART_STOPBITS_1;
    UartGPSSet.RxBuf = GPSRecvBuf;
    UartGPSSet.RxBufLen = sizeof(GPSRecvBuf);
    UartGPSSet.TxBuf = GPSSendBuf;
    UartGPSSet.TxBufLen = sizeof(GPSSendBuf);
    UartGPSSet.Mode = UART_DEFAULT_MODE | UART_MSG_MODE;
    UartGPSSet.RxQueue = GPSRECVTaskEvent;
    UartGPSSet.RxMsgMemBuf = GPSRecvSMem;
    UartGPSSet.RxMsgMemLen = sizeof(GPSRecvSMem);
    if (BSP_UARTConfigInit(GPS_UART, &UartGPSSet) == FALSE)
    {
        return FALSE;
    }
	return  TRUE;
}

//ר����GPRS���մ���
/*****************************************************************************
* Function     : TaskGPRSRecv
* Description  : ���ڲ�������
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��6��16��        
*****************************************************************************/
#if(BSP_GPSENABLE > 0)				
void TaskGPSTask(void *pdata)
{
    INT8U err;    
    _BSP_MESSAGE *pMsg;
    GPSRecv_HardwareInit();
	//�����*�������������У�飬��$PCAS01,5*19,'P' ^ 'C' ^ 'A' ^ 'S'^ '0' ^ '1' ^ ',' ^ '5' == 0x19;
  // char *str_B115200 = "$PCAS01,5*19\r\n";
    char *str_B9600  = "$PCAS01,1*1D\r\n";
    char *str_save = "$PCAS00*01\r\n";//��ǰ����д��flash
	BSP_UARTWrite(GPS_UART,(INT8U*)str_B9600,strlen(str_B9600));
	OSTimeDly(SYS_DELAY_500ms); 
	BSP_UARTWrite(GPS_UART,(INT8U*)str_save,strlen(str_save));
	OSTimeDly(SYS_DELAY_500ms); 
	while(1)
    {
		OSTimeDly(SYS_DELAY_50ms); 
        //�Ӵ��ڶ�ȡһ����Ϣ
        pMsg = OSQPend(GPSRECVTaskEvent, SYS_DELAY_500ms, &err);
        if (OS_ERR_NONE == err)
        {
            switch (pMsg->MsgID)
            {
                case BSP_MSGID_UART_RXOVER:
					if (UART_MsgDeal(&pMsg, GPSTempBuf, sizeof(GPSTempBuf)) == TRUE)
                    {

						BSP_UARTWrite(GPS_UART,pMsg->pData,pMsg->DataLen);
					}

                    UART_FreeOneMsg(GPS_UART, pMsg);
                    break;
                default:
                    break;
            }
        }
	}	
}
#endif

/************************(C)COPYRIGHT 2018 ���ݿ��*****END OF FILE****************************/

