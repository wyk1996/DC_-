/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
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
//#include "W5500Main.h"
#include <string.h>
#include "sysconfig.h"
#include "w5500_conf.h"
#include "w5500.h"
#include "tcp_demo.h"
#include "socket.h"
/* Private define-----------------------------------------------------------------------------*/
#define ETHMAIN_Q_LEN  15
static void *ETHMAINOSQ[ETHMAIN_Q_LEN];					// ��Ϣ����
static OS_EVENT *ETHMainTaskEvent;				            // ʹ�õ��¼�

INT8U ETHStatus = SOCK_CLOSED;
#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif //MIN
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : APP_GetETHLinKState
* Description  : ��ȡ��̫������״̬
* Input        : void
* Output       : TRUE    ���ӳɹ�
				 FALSE   ����ʧ�� 
* Return       :
* Note(s)      :
* Contributor  : 2018��6��16��        
*****************************************************************************/
INT8U APP_GetETHLinKState(void)
{
	if(ETHStatus == SOCK_ESTABLISHED)
	{
		return TRUE;
	}
	return FALSE;
}

//ר����GPRS���մ���
/*****************************************************************************
* Function     : HardReset_W5500
* Description  : Ӳ����λw5500
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��6��16��        
*****************************************************************************/
void HardReset_W5500(void)
{
	reset_w5500();												/*Ӳ��λW5500*/
	set_w5500_mac();											/*����MAC��ַ*/
	set_w5500_ip();												/*����IP��ַ*/
	socket_buf_init(txsize, rxsize);			/*��ʼ��8��Socket�ķ��ͽ��ջ����С*/
}

/*****************************************************************************
* Function     : ETH_DisposeRecvData
* Description  : ��̫���������ݴ���
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��6��16��        
*****************************************************************************/
static void ETH_DisposeRecvData(INT8U *pdata,INT8U len)
{
	NOP();
}

/*****************************************************************************
* Function     : ETH_DisposeRecvData
* Description  : ��̫���������ݴ���
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��6��16��        
*****************************************************************************/
static void ETH_PeriodSendData(void)
{
	NOP();
}



INT8U ETHRecvBuf[200];
/*****************************************************************************
* Function     : W5500_GetRecvDataDispose
* Description  : ��ȡ�������ݴ���
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018��6��16��        
*****************************************************************************/
static INT8U W5500_GetRecvDataDispose(void)
{
	INT16U len;

	len=getSn_RX_RSR(SOCK_TCPC); 								  	         /*����lenΪ�ѽ������ݵĳ���*/
	if(len>0 )
	{
		recv(SOCK_TCPC,ETHRecvBuf,MIN(len,200)); 							   		         /*��������Server������*/
	}
	return len;
}


/*****************************************************************************
* Function     : TaskW5500Connect
* Description  : ��̫������
* Input        : void
* Output       : None
* Return       :
* Note(s)      :����������ѯ�ڴ������н��У�������һ��ѭ��ʱ�䲻��̫��
* Contributor  : 2020��12��14��        
*****************************************************************************/
void TaskW5500Connect(void *pdata)
{
 
	INT8U err;   
	_BSP_MESSAGE *pMsg;
	static INT32U count;
	INT8U recvlen = 0;			//��̫�����ݽ��ճ���
	
	pdata =  pdata; 
	ETHMainTaskEvent = OSQCreate(ETHMAINOSQ, ETHMAIN_Q_LEN);//
	if (ETHMainTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //����ǰ����
		return;
	}	
	
	gpio_for_w5500_config();							/*��ʼ��MCU�������*/
	HardReset_W5500();									//Ӳ����λw5500
	while(1)
    {  
		pMsg = OSQPend(ETHMainTaskEvent, SYS_DELAY_1ms, &err);
        if (OS_ERR_NONE == err || pMsg != NULL)
		{
			if(ETHStatus == SOCK_ESTABLISHED)				//˵���Ѿ���������
			{
				;//��Ϣ����
			}
		}
		if(PHY_check())   //�����Ѿ�����
		{
			if(ETHStatus == SOCK_ESTABLISHED)				//˵���Ѿ���������
			{
				count = 0;
				//��ȡ���ݴ���
				recvlen = W5500_GetRecvDataDispose();
				if(recvlen > 0)
				{
					ETH_DisposeRecvData(ETHRecvBuf,recvlen);
					send(SOCK_TCPC,ETHRecvBuf,recvlen);
				}
				//��������
				ETH_PeriodSendData();
			}
			
			else
			{
				//���߽��볤ʱ��Ϊ���ӳɹ�����ҪӲ����ʼ��
				if(++count  >= 10000)//���Զ�ʱ10s,�������Ը���ʵ���������
				{
					HardReset_W5500();									//Ӳ����λw5500
				}
			}
		}
	}	
}


/*****************************************************************************
* Function     : TaskW5500Connect
* Description  : ��̫������
* Input        : void
* Output       : None
* Return       :
* Note(s)      :����������ѯ�ڴ������н��У�������һ��ѭ��ʱ�䲻��̫��
* Contributor  : 2020��12��14��        
*****************************************************************************/
void TaskW5500Priod(void *pdata)
{
	pdata =  pdata; 
	while(1)
	{
		if(PHY_check())   //�����Ѿ�����
		{
			ETHStatus = do_tcp_client();                    /*���ӷ�����*/
		}
		OSTimeDly(SYS_DELAY_500ms); 
	}
	
}

/************************(C)COPYRIGHT 2020 �����Ƽ�*****END OF FILE****************************/

