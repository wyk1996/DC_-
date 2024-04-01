/*****************************************Copyright(H)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: connectbmsinterface.h
* Author			  :      
* Date First Issued	: 10/6/2015
* Version			  : V0.1
* Description		: �󲿷������������������ʼ��
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History			  :
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__CONNECT_BMS_INTERFACE_H_
#define	__CONNECT_BMS_INTERFACE_H_
#include "stm32f4xx.h"
#include "ucos_ii.h"
#include "app_conf.h"
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/                                
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
    INT32U DataID; //ָ���ID
    INT8U *pdata;  //������
    INT8U len;     //��������Ч����
    //INT8U offset;  //������ƫ����
}CanInterfaceStruct; 

//RTS֡��Ч���ݣ��պ�8���ֽ�
__packed typedef struct
{
    INT8U 	rtscontrolflag ;	//ָ��Ŀ���ַ�������ͣ���ֵ�̶�Ϊ0x10--RTSר��
    INT16U 	messagebytenum;		//��������ֽ�������1��ʼ����
    INT8U 	messagepagenum; 	//�����ܰ�������1��ʼ����
    INT8U 	SAEdefaultbyte;	 	//SAE�趨ʹ�ã����������ޣ�0xff����û������
    INT8U	paranumber[3];  	//�����Ϣ�Ĳ������ţ���3���ֽڵ�PGN
}_BMS_RTS_FRAME_INF;

extern _BMS_RTS_FRAME_INF  BMS_RTS_FrameInf[GUN_MAX];//BMS���������֡��Ϣ
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

/***********************************************************************************************
* Function		: CanProtocolOperationRTS
* Description	: RTS֡���մ���
* Input			: 
* Return		:
* Note(s)		: 
* Contributor	: 160523	Ҷϲ��
***********************************************************************************************/
INT8U CanProtocolOperationRTSA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationRTSB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBHMA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBHMB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBRMA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBRMB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBCPA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBCPB(CanInterfaceStruct *controlstrcut);

INT8U CanProtocolOperationBROA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBROB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBCLA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBCLB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBCSA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBCSB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSMA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSMB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBMVA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBMVB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBMTA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBMTB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSPA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSPB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSTA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSTB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSDA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBSDB(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBEMA(CanInterfaceStruct *controlstrcut);
INT8U CanProtocolOperationBEMB(CanInterfaceStruct *controlstrcut);
#endif //__CONNECT_BMS_INTERFACE_H_
/************************(H)COPYRIGHT 2010 �����Ƽ�*****END OF FILE****************************/
