/*****************************************Copyright(H)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: connectbmsinterface.h
* Author			  :      
* Date First Issued	: 10/6/2015
* Version			  : V0.1
* Description		: 大部分驱动都可以在这里初始化
*----------------------------------------历史版本信息-------------------------------------------
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
    INT32U DataID; //指令的ID
    INT8U *pdata;  //数据域
    INT8U len;     //数据域有效长度
    //INT8U offset;  //表格里的偏移量
}CanInterfaceStruct; 

//RTS帧有效内容，刚好8个字节
__packed typedef struct
{
    INT8U 	rtscontrolflag ;	//指定目标地址的请求发送，此值固定为0x10--RTS专用
    INT16U 	messagebytenum;		//多包的总字节数，从1开始计算
    INT8U 	messagepagenum; 	//数据总包数，从1开始计算
    INT8U 	SAEdefaultbyte;	 	//SAE设定使用，最大包数上限，0xff代表没有限制
    INT8U	paranumber[3];  	//打包消息的参数组编号，及3个字节的PGN
}_BMS_RTS_FRAME_INF;

extern _BMS_RTS_FRAME_INF  BMS_RTS_FrameInf[GUN_MAX];//BMS传输过来的帧信息
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

/***********************************************************************************************
* Function		: CanProtocolOperationRTS
* Description	: RTS帧接收处理
* Input			: 
* Return		:
* Note(s)		: 
* Contributor	: 160523	叶喜雨
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
/************************(H)COPYRIGHT 2010 汇誉科技*****END OF FILE****************************/
