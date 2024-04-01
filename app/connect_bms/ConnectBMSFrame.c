/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: connectbmsframe.c
* Author			: 叶喜雨
* Date First Issued	: 23/05/2016
* Version			: V0.1
* Description		: 连接BMS的组帧等处理文件，所有发送帧的组织都放在此文件中 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "gbstandard.h"
#include "connectbms.h"
#include "controlmain.h"
/* Private define-----------------------------------------------------------------------------*/
#define CTS_CONTROL_BYTE       17     //CTS的控制字
#define EMA_CONTROL_BYTE       19     //EMA的控制字 End of Msg ACK;
#define CRM_DEVICE_CODENUM      8     //充电机编号固定为8
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
extern INT8U CRMReadyflag[GUN_MAX];
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: Reply_BMSFrame_CTS
* Description	: 回复BMS的RTS的回复帧CTS
* Input			: 
* Return		: 
* Note(s)		: 该CTS区别与充电握手配置阶段的CTS，该CTS为J1939协议的多帧处理里的回复RTS的CTS
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
void Reply_BMSFrame_CTSA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //发送帧ID 1C EC F4 56
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;
    
    BMSCANTxbuffA.Data[0] = CTS_CONTROL_BYTE; //充电机发送给BMS的控制字，CTS固定17(0x11)
    BMSCANTxbuffA.Data[1] = BMS_RTS_FrameInf[GUN_A].messagepagenum; //可发送的数据包数
    BMSCANTxbuffA.Data[2] = 0x01;//固定为下一个要发送的数据包编号 
    BMSCANTxbuffA.Data[3] = 0xFF;
    BMSCANTxbuffA.Data[4] = 0xFF;//SAE预留的两个字节
    memcpy(&BMSCANTxbuffA.Data[5], BMS_RTS_FrameInf[GUN_A].paranumber, 3);//接收到的RTS的打包消息的参数组编号
    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: Reply_BMSFrame_CTS
* Description	: 回复BMS的RTS的回复帧CTS
* Input			: 
* Return		: 
* Note(s)		: 该CTS区别与充电握手配置阶段的CTS，该CTS为J1939协议的多帧处理里的回复RTS的CTS
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
void Reply_BMSFrame_CTSB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //发送帧ID 1C EC F4 56
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;
    
    BMSCANTxbuffB.Data[0] = CTS_CONTROL_BYTE; //充电机发送给BMS的控制字，CTS固定17(0x11)
    BMSCANTxbuffB.Data[1] = BMS_RTS_FrameInf[GUN_B].messagepagenum; //可发送的数据包数
    BMSCANTxbuffB.Data[2] = 0x01;//固定为下一个要发送的数据包编号 
    BMSCANTxbuffB.Data[3] = 0xFF;
    BMSCANTxbuffB.Data[4] = 0xFF;//SAE预留的两个字节
    memcpy(&BMSCANTxbuffB.Data[5], BMS_RTS_FrameInf[GUN_B].paranumber, 3);//接收到的RTS的打包消息的参数组编号
    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: Reply_TPCM_EndofMsgAckFrame
* Description	: 发送多帧消息结束应答，表示整个消息已经被接收并正确重组
* Input			: 
* Output		: 
* Note(s)		: CRM报文，PGN=236 P=7
* Contributor	: 20101210          
***********************************************************************************************/
void Reply_TPCM_EndofMsgAckFrameA(void)
{
    INT16U messagebytenum = 0;
    
    OSSchedLock();
    messagebytenum = BMS_RTS_FrameInf[GUN_A].messagebytenum;
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //发送帧ID
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;
    
    BMSCANTxbuffA.Data[0] = EMA_CONTROL_BYTE; //控制字,固定为19(0x13)
    BMSCANTxbuffA.Data[1] = messagebytenum&0xFF; //多包的总有效字节数
    BMSCANTxbuffA.Data[2] = (messagebytenum>>8)&0xFF; //
    BMSCANTxbuffA.Data[3] = BMS_RTS_FrameInf[GUN_A].messagepagenum; //全部数据包的数
    BMSCANTxbuffA.Data[4] = 0xFF;//SAE预留的两个字节
    memcpy( &BMSCANTxbuffA.Data[5],BMS_RTS_FrameInf[GUN_A].paranumber,3);//接收到的RTS的打包消息的参数组编号
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
    OSSchedUnlock();   
}

/***********************************************************************************************
* Function		: Reply_TPCM_EndofMsgAckFrame
* Description	: 发送多帧消息结束应答，表示整个消息已经被接收并正确重组
* Input			: 
* Output		: 
* Note(s)		: CRM报文，PGN=236 P=7
* Contributor	: 20101210          
***********************************************************************************************/
void Reply_TPCM_EndofMsgAckFrameB(void)
{
    INT16U messagebytenum = 0;
    
    OSSchedLock();
    messagebytenum = BMS_RTS_FrameInf[GUN_B].messagebytenum;
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_7, GB_RTS_PGN_236);  //发送帧ID
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;
    
    BMSCANTxbuffB.Data[0] = EMA_CONTROL_BYTE; //控制字,固定为19(0x13)
    BMSCANTxbuffB.Data[1] = messagebytenum&0xFF; //多包的总有效字节数
    BMSCANTxbuffB.Data[2] = (messagebytenum>>8)&0xFF; //
    BMSCANTxbuffB.Data[3] = BMS_RTS_FrameInf[GUN_B].messagepagenum; //全部数据包的数
    BMSCANTxbuffB.Data[4] = 0xFF;//SAE预留的两个字节
    memcpy( &BMSCANTxbuffB.Data[5],BMS_RTS_FrameInf[GUN_B].paranumber,3);//接收到的RTS的打包消息的参数组编号
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
    OSSchedUnlock();   
}
/***********************************************************************************************
* Function		: CRM_SendShakeHandIdentifyFrame
* Description	: 发送充电机辨识报文
* Input			: 
* Output		: 
* Note(s)		: CRM报文，PGN=256 P=6
* Contributor	: 20101210          
***********************************************************************************************/
void CRM_SendShakeHandIdentifyFrameA(void)
{
    OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CRM_PGN_256);  //发送帧ID
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;
#if 0
    if(BMS_CONNECT_Control.currentstate == BMS_BRM_SUCCESS)//BRM已经全部接收
        BMSCANTxbuff.Data[0] = 0xAA; //辨识结果
    else
        BMSCANTxbuff.Data[0] = 0x00; //辨识结果
#endif
    if (CRMReadyflag[GUN_A])
	{
		BMSCANTxbuffA.Data[0] = 0xAA; //辨识结果
	}
	else 
	{
		BMSCANTxbuffA.Data[0] = 0x00; //辨识结果
	}
    BMSCANTxbuffA.Data[1] = CRM_DEVICE_CODENUM; //充电机编号
    BMSCANTxbuffA.Data[2] = 0x00; 
    BMSCANTxbuffA.Data[3] = 0x00;
    BMSCANTxbuffA.Data[4] = 0x00;
    BMSCANTxbuffA.Data[5] = 0xFF;
    BMSCANTxbuffA.Data[6] = 0xFF;
    BMSCANTxbuffA.Data[7] = 0xFF;
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
    OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRM_SendShakeHandIdentifyFrame
* Description	: 发送充电机辨识报文
* Input			: 
* Output		: 
* Note(s)		: CRM报文，PGN=256 P=6
* Contributor	: 20101210          
***********************************************************************************************/
void CRM_SendShakeHandIdentifyFrameB(void)
{
    OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CRM_PGN_256);  //发送帧ID
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;
#if 0
    if(BMS_CONNECT_Control.currentstate == BMS_BRM_SUCCESS)//BRM已经全部接收
        BMSCANTxbuff.Data[0] = 0xAA; //辨识结果
    else
        BMSCANTxbuff.Data[0] = 0x00; //辨识结果
#endif
    if (CRMReadyflag[GUN_B])
	{
		BMSCANTxbuffB.Data[0] = 0xAA; //辨识结果
	}
	else 
	{
		BMSCANTxbuffB.Data[0] = 0x00; //辨识结果
	}
    BMSCANTxbuffB.Data[1] = CRM_DEVICE_CODENUM; //充电机编号
    BMSCANTxbuffB.Data[2] = 0x00; 
    BMSCANTxbuffB.Data[3] = 0x00;
    BMSCANTxbuffB.Data[4] = 0x00;
    BMSCANTxbuffB.Data[5] = 0xFF;
    BMSCANTxbuffB.Data[6] = 0xFF;
    BMSCANTxbuffB.Data[7] = 0xFF;
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
    OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CHM_SendShakeHandFrame
* Description	: 发送充电机辨识报文
* Input			: 
* Output		: 
* Note(s)		: CHM报文，PGN=9278 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CHM_SendShakeHandFrameA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CHM_PGN_9728);  //发送帧
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 3;
    
    BMSCANTxbuffA.Data[0] = 0x01; //版本V1.1
    BMSCANTxbuffA.Data[1] = 0x01;
    BMSCANTxbuffA.Data[2] = 0x00; 
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}


/***********************************************************************************************
* Function		: CHM_SendShakeHandFrame
* Description	: 发送充电机辨识报文
* Input			: 
* Output		: 
* Note(s)		: CHM报文，PGN=9278 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CHM_SendShakeHandFrameB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6, GB_CHM_PGN_9728);  //发送帧
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 3;
    
    BMSCANTxbuffB.Data[0] = 0x01; //版本V1.1
    BMSCANTxbuffB.Data[1] = 0x01;
    BMSCANTxbuffB.Data[2] = 0x00; 
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CTS_SendSynTimeMessage
* Description	: 发送时间同步信息CTS
* Input			: 
* Output		: 
* Note(s)		: CTS报文，PGN=1792 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CTS_SendSynTimeMessageA(void)
{
    _BSPRTC_TIME rtcTime;
	
	OSSchedLock();
    if (BSP_RTCGetTime(&rtcTime) == FALSE)
    {
        rtcTime.Year   = 0x16;
        rtcTime.Month  = 0x07;
        rtcTime.Day    = 0x13;
        rtcTime.Hour   = 0x08;
        rtcTime.Minute = 0x00;
        rtcTime.Second = 0x00;
        rtcTime.Week   = 0x00;
    }
	rtcTime.Week = 0x20;  //这里把Week字节默认为年的高位。

    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CTS_PGN_1792);  //发送帧
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 7;//数据长度7
    //CTS报文发送的时间都是BCD码
    * ( (_BSPRTC_TIME *)BMSCANTxbuffA.Data) = rtcTime;
    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CTS_SendSynTimeMessage
* Description	: 发送时间同步信息CTS
* Input			: 
* Output		: 
* Note(s)		: CTS报文，PGN=1792 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CTS_SendSynTimeMessageB(void)
{
    _BSPRTC_TIME rtcTime;
	
	OSSchedLock();
    if (BSP_RTCGetTime(&rtcTime) == FALSE)
    {
        rtcTime.Year   = 0x16;
        rtcTime.Month  = 0x07;
        rtcTime.Day    = 0x13;
        rtcTime.Hour   = 0x08;
        rtcTime.Minute = 0x00;
        rtcTime.Second = 0x00;
        rtcTime.Week   = 0x00;
    }
	rtcTime.Week = 0x20;  //这里把Week字节默认为年的高位。

    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CTS_PGN_1792);  //发送帧
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 7;//数据长度7
    //CTS报文发送的时间都是BCD码
    * ( (_BSPRTC_TIME *)BMSCANTxbuffB.Data) = rtcTime;
    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CML_SendMaxVolandCur
* Description	: 充电机最大能力输出报文，用来估计剩余充电时间
* Input			: 
* Output		: 
* Note(s)		: CML报文，PGN=2048 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CML_SendMaxVolandCurA(void)
{
    //计算充电机最大输出能力报文CML
	
    APP_GetCMLContext(&BMS_CML_Context[GUN_A]);
    
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CML_PGN_2048);  //发送帧
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    if (APP_GetGBType(GUN_A) == BMS_GB_2015)  //新国标 
        BMSCANTxbuffA.DLC = 8;//数据长度8
    else
        BMSCANTxbuffA.DLC = 6;//数据长度6

    APP_GetCMLContext(&BMS_CML_Context[GUN_A]);
    
    *( (_BMS_CML_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CML_Context[GUN_A];
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CML_SendMaxVolandCur
* Description	: 充电机最大能力输出报文，用来估计剩余充电时间
* Input			: 
* Output		: 
* Note(s)		: CML报文，PGN=2048 P=6
* Contributor	: 20160524        
***********************************************************************************************/
void CML_SendMaxVolandCurB(void)
{
    //计算充电机最大输出能力报文CML
    APP_GetCMLContext(&BMS_CML_Context[GUN_B]);
    
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CML_PGN_2048);  //发送帧
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    if (APP_GetGBType(GUN_B) == BMS_GB_2015)  //新国标 
        BMSCANTxbuffB.DLC = 8;//数据长度8
    else
        BMSCANTxbuffB.DLC = 6;//数据长度6

    APP_GetCMLContext(&BMS_CML_Context[GUN_B]);
    
    *( (_BMS_CML_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CML_Context[GUN_B];
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevUnstandby
* Description	: 发送CRO(0x00)报文
* Input			: 
* Output		: 
* Note(s)		: Cro报文，PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevUnstandbyA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //发送帧
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 1;//数据长度1
    
    //充电机未准备好
    BMSCANTxbuffA.Data[0] = GB_CHARGESTATE_NOTREADY; 
    
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevUnstandby
* Description	: 发送CRO(0x00)报文
* Input			: 
* Output		: 
* Note(s)		: Cro报文，PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevUnstandbyB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //发送帧
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 1;//数据长度1
    
    //充电机未准备好
    BMSCANTxbuffB.Data[0] = GB_CHARGESTATE_NOTREADY; 
    
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevStandby
* Description	: 发送CRO(0xAA)报文
* Input			: 
* Output		: 
* Note(s)		: Cro报文，PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevStandbyA(void)
{
	OSSchedLock();
    BMSCANTxbuffA.StdId=0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //发送帧
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 1;//数据长度1
    
    //充电机准备好
    BMSCANTxbuffA.Data[0] = GB_CHARGESTATE_STANDBY; 
    
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CRO_SendChargeDevStandby
* Description	: 发送CRO(0xAA)报文
* Input			: 
* Output		: 
* Note(s)		: Cro报文，PGN=2560 P=4
* Contributor	: 20160524        
***********************************************************************************************/
void CRO_SendChargeDevStandbyB(void)
{
	OSSchedLock();
    BMSCANTxbuffB.StdId=0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CRO_PGN_2560);  //发送帧
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 1;//数据长度1
    
    //充电机准备好
    BMSCANTxbuffB.Data[0] = GB_CHARGESTATE_STANDBY; 
    
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}
/***********************************************************************************************
* Function		: CCS_DeviceChargeState
* Description	: 发送时间同步信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CCS_DeviceChargeStateA(void)
{
	
    //获取CCS内容
    APP_GetCSSContext(GUN_A,&BMS_CCS_Context[GUN_A]);
    
	OSSchedLock();
    BMSCANTxbuffA.StdId=0x00;
    BMSCANTxbuffA.ExtId=GB_CONFIGSEND_ID(GB_PRIO_6,GB_CCS_PGN_4608);  //发送帧
    BMSCANTxbuffA.RTR=CAN_RTR_DATA;
    BMSCANTxbuffA.IDE=CAN_ID_EXT;
    if (APP_GetGBType(GUN_A) == BMS_GB_2015)  //新国标 
        BMSCANTxbuffA.DLC = 7;//数据长度8
    else
        BMSCANTxbuffA.DLC = 6;//数据长度6

    
    //或许数据
    * ( (_BMS_CCS_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CCS_Context[GUN_A];
    
    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CCS_DeviceChargeState
* Description	: 发送时间同步信息
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CCS_DeviceChargeStateB(void)
{

    //获取CCS内容
    APP_GetCSSContext(GUN_B,&BMS_CCS_Context[GUN_B]);
    
	OSSchedLock();
    BMSCANTxbuffB.StdId=0x00;
    BMSCANTxbuffB.ExtId=GB_CONFIGSEND_ID(GB_PRIO_6,GB_CCS_PGN_4608);  //发送帧
    BMSCANTxbuffB.RTR=CAN_RTR_DATA;
    BMSCANTxbuffB.IDE=CAN_ID_EXT;
    if (APP_GetGBType(GUN_B) == BMS_GB_2015)  //新国标 
        BMSCANTxbuffB.DLC = 7;//数据长度8
    else
        BMSCANTxbuffB.DLC = 6;//数据长度6

    
    //或许数据
    * ( (_BMS_CCS_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CCS_Context[GUN_B];
    
    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CST_DeviceEndCharge
* Description	: 中止充电
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CST_DeviceEndChargeA(void)
{
    //获取CST报文的内容
    APP_GetCSTContext(GUN_A,&BMS_CST_Context[GUN_A]);
    
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CST_PGN_6656);  //发送帧
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 4;//数据长度4
    
    if (APP_GetGBType(GUN_A) == BMS_GB_2011)  //旧国标 
        BMS_CST_Context[GUN_A].State0.EndChargeReason |= 0xc0;

    BMS_CST_Context[GUN_A].State1.EndChargeTroubleReason |= 0xf000;
    BMS_CST_Context[GUN_A].State2.EndChargeErroReason |= 0xf0;
    *( (_BMS_CST_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CST_Context[GUN_A]; 
    
    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
	
}

/***********************************************************************************************
* Function		: CST_DeviceEndCharge
* Description	: 中止充电
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CST_DeviceEndChargeB(void)
{
    //获取CST报文的内容
    APP_GetCSTContext(GUN_B,&BMS_CST_Context[GUN_B]);
    
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_4,GB_CST_PGN_6656);  //发送帧
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 4;//数据长度4
    
    if (APP_GetGBType(GUN_B) == BMS_GB_2011)  //旧国标 
        BMS_CST_Context[GUN_B].State0.EndChargeReason |= 0xc0;

    BMS_CST_Context[GUN_B].State1.EndChargeTroubleReason |= 0xf000;
    BMS_CST_Context[GUN_B].State2.EndChargeErroReason |= 0xf0;
    *( (_BMS_CST_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CST_Context[GUN_B]; 
    
    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
	
}

/***********************************************************************************************
* Function		: CSD_DeviceChargeEndStatisticsE
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CSD_DeviceChargeEndStatisticsA(void)
{
    //获取CSD报文
    APP_GetCSDContext(GUN_A,&BMS_CSD_Context[GUN_A]);
    OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CSD_PGN_7424);  //发送帧
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 8;//数据长度8
    
    *( (_BMS_CSD_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CSD_Context[GUN_A]; 

    BMSCANTxbuffA.Data[4] = CRM_DEVICE_CODENUM; //充电机编号
    BMSCANTxbuffA.Data[5] = 0x00; 
    BMSCANTxbuffA.Data[6] = 0x00;
    BMSCANTxbuffA.Data[7] = 0x00;

    BSP_CAN_Write(BMS_CAN_A, &bxBMSCANSetA);
	OSSchedUnlock();
}

/***********************************************************************************************
* Function		: CSD_DeviceChargeEndStatisticsE
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 20160524        
***********************************************************************************************/
void CSD_DeviceChargeEndStatisticsB(void)
{
    //获取CSD报文
    APP_GetCSDContext(GUN_B,&BMS_CSD_Context[GUN_B]);
    OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_6,GB_CSD_PGN_7424);  //发送帧
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 8;//数据长度8
    
    *( (_BMS_CSD_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CSD_Context[GUN_B]; 

    BMSCANTxbuffB.Data[4] = CRM_DEVICE_CODENUM; //充电机编号
    BMSCANTxbuffB.Data[5] = 0x00; 
    BMSCANTxbuffB.Data[6] = 0x00;
    BMSCANTxbuffB.Data[7] = 0x00;

    BSP_CAN_Write(BMS_CAN_B, &bxBMSCANSetB);
	OSSchedUnlock();
}
/*****************************************************************************
* Function      : CEM_DeviceChargeErr
* Description   : CEM报文发送
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月6日  叶喜雨
*****************************************************************************/
void CEM_DeviceChargeErrA(void)
{   
	OSSchedLock();
    BMSCANTxbuffA.StdId = 0x00;
    //发送帧
    BMSCANTxbuffA.ExtId = GB_CONFIGSEND_ID(GB_PRIO_2, GB_CEM_PGN_7936);  
    BMSCANTxbuffA.RTR = CAN_RTR_DATA;
    BMSCANTxbuffA.IDE = CAN_ID_EXT;
    BMSCANTxbuffA.DLC = 4;//数据长度4
    
    //CEM报文结构体内容已经在整个BMS交互过程的超时处理时候赋值，这里只管发送即可
    *( (_BMS_CEM_CONTEXT *)BMSCANTxbuffA.Data) = BMS_CEM_Context[GUN_A]; 

    BSP_CAN_Write(BMS_CAN_A,&bxBMSCANSetA);
	OSSchedUnlock();

}

/*****************************************************************************
* Function      : CEM_DeviceChargeErr
* Description   : CEM报文发送
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月6日  叶喜雨
*****************************************************************************/
void CEM_DeviceChargeErrB(void)
{   
	OSSchedLock();
    BMSCANTxbuffB.StdId = 0x00;
    //发送帧
    BMSCANTxbuffB.ExtId = GB_CONFIGSEND_ID(GB_PRIO_2, GB_CEM_PGN_7936);  
    BMSCANTxbuffB.RTR = CAN_RTR_DATA;
    BMSCANTxbuffB.IDE = CAN_ID_EXT;
    BMSCANTxbuffB.DLC = 4;//数据长度4
    
    //CEM报文结构体内容已经在整个BMS交互过程的超时处理时候赋值，这里只管发送即可
    *( (_BMS_CEM_CONTEXT *)BMSCANTxbuffB.Data) = BMS_CEM_Context[GUN_B]; 

    BSP_CAN_Write(BMS_CAN_B,&bxBMSCANSetB);
	OSSchedUnlock();

}
/************************(C)COPYRIGHT 2016 汇誉科技*****END OF FILE****************************/
