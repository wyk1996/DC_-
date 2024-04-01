/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			:ModuleYouren.c
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "ModuleManage.h"
#include "bsp_uart.h"
/* Private define-----------------------------------------------------------------------------*/
__packed typedef struct
{
    _GPRS_STATE YOUREN_ExistState;                      //有人模块是否存在         	STATE_OK：存在  STATE_ERR：不存在
    _GPRS_STATE YOUREN_CfgIPPorttState;                    //有人是否配置完成连服务器
	_GPRS_STATE YOUREN_ConectState;						//获取连接状态
    _GPRS_STATE YOUREN_RegisterState;					//有人是否已经注册完成
	_GPRS_STATUS YOUREN_Status;							//是否为配置状态（AT），通信状态（数据传输）
	__packed union
    {
        INT8U OneByteBits;
        __packed struct
        {
        //进入AT指令需要2条指令
			INT8U ATCMD_ENTER_A   : 1;			 //有人模块必须先进入AT指令模式,发送AT指令才有效
			INT8U ATCMD_ENTER_OK  : 1;			 //有人模块必须先进入AT指令模式,发送AT指令才有效
			
			INT8U ATCMD_DATA      : 1;			 //退出AT指令模式，进入之前配置的模式（NET 透传模式）
			INT8U ATCMD_PRESER    : 1;			 //保存当前配置模式
			INT8U WKMOD_State     : 1;           //设置工作模式状态
            INT8U ReserveBits     : 3;           //预留
        }OneByte;
	}State0;
	__packed union
    {
		INT8U TwoByteBits;
		__packed struct
        {
			INT8U SOCKAEN         : 1;			 //socka使能状态
			INT8U SOCKBEN         : 1;			 //sockb使能状态
			INT8U SOCKCEN         : 1;			 //sockc使能状态
			INT8U SOCKDEN         : 1;			 //sockd使能状态
			INT8U SOCKA  	      : 1;			 //socka创建状态
			INT8U SOCKB           : 1;			 //sockb创建状态
			INT8U SOCKC           : 1;			 //sockc创建状态
			INT8U SOCKD           : 1;			 //sockd创建状态
        }TwoByte;
    }State1;
}_YOUREN_CMD_STATE;

/* Private typedef----------------------------------------------------------------------------*/
/*static */_YOUREN_CMD_STATE   YOURENCmdState;						//有人状态
//以下是AT指令发送处理
/*****************************************************************************
* Function     : YouRenReset
* Description  : 有人模块重启（保存当前模式，进入数据模式）
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
static INT8U YouRenReset()
{
		//保存当前配置配置，重启进入数据模式
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+S\r\n", strlen("AT+S\r\n") ) == strlen("AT+S\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);
		OSSchedLock();
		YOURENCmdState.State0.OneByteBits = 0;
		YOURENCmdState.State1.TwoByteBits = 0;
		YOURENCmdState.YOUREN_ConectState = STATE_ERR;
		YOURENCmdState.YOUREN_Status = STATE_DATA;
		OSSchedUnlock();		
        return TRUE;
    }

	return FALSE;
}
/*****************************************************************************
* Function     : APP_GetYouRenStatus
* Description  : 获取有人模块是否为配置状态（AT），通信状态（数据传输）
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
static _GPRS_STATUS APP_GetYouRenStatus()
{
	return YOURENCmdState.YOUREN_Status;
}

/*****************************************************************************
* Function     : APP_GetYouRenExistState
* Description  : 获取有人模块是否存在
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
static _GPRS_STATE APP_GetYouRenExistState()
{
	return YOURENCmdState.YOUREN_ExistState;
}

/*****************************************************************************
* Function     : APP_GetYouRenCfgIPPorttState
* Description  : 配置服务器是否成功
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static _GPRS_STATE APP_GetYouRenCfgIPPorttState()
{
	return YOURENCmdState.YOUREN_CfgIPPorttState;
}

/*****************************************************************************
* Function     : APP_GetYouRenConnectState
* Description  : 连接服务器状态
* Input        :     
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static _GPRS_STATE APP_GetYouRenConnectState()
{
	return YOURENCmdState.YOUREN_ConectState;
}

/*****************************************************************************
* Function     : APP_SetYouRenExistState
* Description  : 设置有人模块模块状态
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/	
static void APP_SetYouRenExistState(_GPRS_STATE state)
{
	OSSchedLock();
	YOURENCmdState.YOUREN_ExistState = state;
	OSSchedUnlock();
}

/*****************************************************************************
* Function     : GPRS_ENTER_ATCMD1
* Description  : 进入AT配置指令1
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_ENTER_ATCMD1(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"+++", strlen("+++") ) == strlen("+++") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}	

/*****************************************************************************
* Function     : GPRS_ENTER_ATCMD1
* Description  : 进入AT配置指令2
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_ENTER_ATCMD2(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"a", strlen("a") ) == strlen("a") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}	

/*****************************************************************************
* Function     : GPRS_ENTER_DATA
* Description  : 进入数据模式
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_ENTER_DATA(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+ENTM\r\n", strlen("AT+ENTM\r\n") ) == strlen("AT+ENTM\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}	
/*****************************************************************************
* Function     : GPRS_Command_WKMOD
* Description  : 设置当前为网络透传模式	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_WKMOD(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+WKMOD=NET\r\n", strlen("AT+WKMOD=NET\r\n") ) == strlen("AT+WKMOD=NET\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}	
/*****************************************************************************
* Function     : GPRS_Command_SOCKAEN
* Description  : 使能socketA
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKAEN(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKAEN=on\r\n", strlen("AT+SOCKAEN=on\r\n") ) == strlen("AT+SOCKAEN=on\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKBEN
* Description  : 使能socketB
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKBEN(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKBEN=on\r\n", strlen("AT+SOCKBEN=on\r\n") ) == strlen("AT+SOCKBEN=on\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKAEN
* Description  : 使能socketC
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKCEN(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKCEN=on\r\n", strlen("AT+SOCKCEN=on\r\n") ) == strlen("AT+SOCKCEN=on\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKDEN
* Description  : 使能socketA
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKDEN(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKDEN=on\r\n", strlen("AT+SOCKDEN=on\r\n") ) == strlen("AT+SOCKDEN=on\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}


/*****************************************************************************
* Function     : GPRS_Command_SOCKAEN
* Description  : 使能socketA
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKAOFF(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKAEN=off\r\n", strlen("AT+SOCKAEN=off\r\n") ) == strlen("AT+SOCKAEN=off\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKBEN
* Description  : 使能socketB
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKBOFF(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKBEN=off\r\n", strlen("AT+SOCKBEN=off\r\n") ) == strlen("AT+SOCKBEN=off\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKAEN
* Description  : 使能socketC
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKCOFF(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKCEN=off\r\n", strlen("AT+SOCKCEN=off\r\n") ) == strlen("AT+SOCKCEN=off\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKDEN
* Description  : 使能socketA
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKDOFF(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKDEN=off\r\n", strlen("AT+SOCKDEN=off\r\n") ) == strlen("AT+SOCKDEN=off\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}
/*****************************************************************************
* Function     : GPRS_Command_SOCKALK
* Description  : 查询socketA状态
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKALK(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKALK?\r\n", strlen("AT+SOCKALK?\r\n") ) == strlen("AT+SOCKALK?\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKDEN
* Description  :  查询socketB状态
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKBLK(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKBLK?\r\n", strlen("AT+SOCKBLK?\r\n") ) == strlen("AT+SOCKBLK?\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKDEN
* Description  : 查询socketC状态
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKCLK(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKCLK?\r\n", strlen("AT+SOCKCLK?\r\n") ) == strlen("AT+SOCKCLK?\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKDEN
* Description  : 查询socketD状态
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKDLK(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT+SOCKDLK?\r\n", strlen("AT+SOCKDLK?\r\n") ) == strlen("AT+SOCKDLK?\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}
/*****************************************************************************
* Function     : GPRS_Command_SOCKA
* Description  : 连接socketAtcp客户端
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKA(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT\r\n", strlen("AT\r\n") ) == strlen("AT\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }

    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKB
* Description  : 连接socketAtcp客户端
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKB(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT\r\n", strlen("AT\r\n") ) == strlen("AT\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKC
* Description  : 连接socketAtcp客户端
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKC(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT\r\n", strlen("AT\r\n") ) == strlen("AT\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKD
* Description  : 连接socketAtcp客户端
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKD(INT8U *pdata, INT16U len)
{
	if (BSP_UARTWrite(GPRS_UART, (INT8U*)"AT\r\n", strlen("AT\r\n") ) == strlen("AT\r\n") )
    {
		OSTimeDly(SYS_DELAY_5ms);	
        return TRUE;
    }
    return FALSE;
}



//以下是AT指令接收数据处理
/*****************************************************************************
* Function	   : GPRS_ENTER_ATCMD1_Ack
* Description  : 设置进入AT指令模式返回1	
* Input 	   : INT8U *pdata  
				 INT16U len    
* Output	   : None
* Return	   : 
* Note(s)	   : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_ENTER_ATCMD1_Ack(INT8U *pdata, INT16U len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	if(pdata[0] != 'a')				//需要返回A
	{
		return FALSE;
	}
	OSSchedLock();
	YOURENCmdState.State0.OneByte.ATCMD_ENTER_A = GPRS_RESPOND_OK;
	OSSchedUnlock();
	return TRUE;
}	

/*****************************************************************************
* Function	   : GPRS_ENTER_ATCMD2_Ack
* Description  : 设置进入AT指令模式返回2	
* Input 	   : INT8U *pdata  
				 INT16U len    
* Output	   : None
* Return	   : 
* Note(s)	   : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_ENTER_ATCMD2_Ack(INT8U *pdata, INT16U len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
	if (strstr((char *)pdata, "+ok") == NULL)
    {
        return FALSE;
    } 
	OSSchedLock();
	YOURENCmdState.State0.OneByte.ATCMD_ENTER_OK = GPRS_RESPOND_OK;
	OSSchedUnlock();
	return TRUE;
}	

/*****************************************************************************
* Function	   : GPRS_ENTER_DATA_Ack
* Description  : 设置进入数据模式	
* Input 	   : INT8U *pdata  
				 INT16U len    
* Output	   : None
* Return	   : 
* Note(s)	   : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_ENTER_DATA_Ack(INT8U *pdata, INT16U len)
{
	if ( (pdata == NULL) || !len)
	{
		return FALSE;
	}
//	if (strstr((char *)pdata, "OK") == NULL)
//    {
//        return FALSE;
//    } 
	OSSchedLock();
	YOURENCmdState.State0.OneByte.ATCMD_DATA = GPRS_RESPOND_OK;
	YOURENCmdState.YOUREN_Status = STATE_DATA;
	OSSchedUnlock();
	return TRUE;
}	
/*****************************************************************************
* Function     : GPRS_Command_WKMOD
* Description  : 设置当前为网络透传模式返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_WKMOD_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
    YOURENCmdState.State0.OneByte.WKMOD_State = GPRS_RESPOND_OK;
    OSSchedUnlock();
    return TRUE;
}	  

/*****************************************************************************
* Function     : GPRS_Command_SOCKAEN
* Description  : 使能socketA返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKAEN_Ack(INT8U *pdata, INT16U len)
 {
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
    YOURENCmdState.State1.TwoByte.SOCKAEN = GPRS_RESPOND_OK;
    OSSchedUnlock();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKBEN
* Description  : 使能socketB返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKBEN_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
    YOURENCmdState.State1.TwoByte.SOCKBEN = GPRS_RESPOND_OK;
    OSSchedUnlock();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKAEN
* Description  : 使能socketC返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKCEN_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
    YOURENCmdState.State1.TwoByte.SOCKCEN = GPRS_RESPOND_OK;
    OSSchedUnlock();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKDEN
* Description  : 使能socketA返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKDEN_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
    YOURENCmdState.State1.TwoByte.SOCKDEN = GPRS_RESPOND_OK;
    OSSchedUnlock();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKA
* Description  : 配置socketAtcp客户端返回	
* Input        : INT8U *pdata	  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKA_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
	YOURENCmdState.YOUREN_CfgIPPorttState = STATE_OK;
    YOURENCmdState.State1.TwoByte.SOCKA = GPRS_RESPOND_OK;
    OSSchedUnlock();
		//保存当前配置配置，重启进入数据模式
	YouRenReset();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKB
* Description  : 配置socketAtcp客户端返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKB_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
	YOURENCmdState.YOUREN_CfgIPPorttState = STATE_OK;
	YOURENCmdState.State1.TwoByte.SOCKB = GPRS_RESPOND_OK;
    OSSchedUnlock();
	YouRenReset();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKC
* Description  : 配置socketAtcp客户端返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKC_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
	YOURENCmdState.YOUREN_CfgIPPorttState = STATE_OK;
    YOURENCmdState.State1.TwoByte.SOCKC = GPRS_RESPOND_OK;
    OSSchedUnlock();
	YouRenReset();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKD
* Description  : 配置socketAtcp客户端返回	
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKD_Ack(INT8U *pdata, INT16U len)
{
    if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "OK") == NULL)
    {
        return FALSE;
    } 
    OSSchedLock();
	YOURENCmdState.YOUREN_CfgIPPorttState = STATE_OK;
    YOURENCmdState.State1.TwoByte.SOCKD = GPRS_RESPOND_OK;
    OSSchedUnlock();
	YouRenReset();
    return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKALK_Ack
* Description  : 获取连接是否成功
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKALK_Ack(INT8U *pdata, INT16U len)
{
	 if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "disconnected") != NULL)
    {
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_ERR;
		OSSchedUnlock();
		return TRUE;
    } 
	if(strstr((char *)pdata, "connected") != NULL)
	{
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_OK;
		OSSchedUnlock();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKBLK_Ack
* Description  : 获取连接是否成功
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKBLK_Ack(INT8U *pdata, INT16U len)
{
	 if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "connected") != NULL)
    {
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_OK;
		OSSchedUnlock();
    } 
	else
	{
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_ERR;
		OSSchedUnlock();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKCLK_Ack
* Description  : 获取连接是否成功
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKCLK_Ack(INT8U *pdata, INT16U len)
{
	 if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "connected") != NULL)
    {
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_OK;
		OSSchedUnlock();
    } 
	else
	{
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_ERR;
		OSSchedUnlock();
	}
	return TRUE;
}

/*****************************************************************************
* Function     : GPRS_Command_SOCKDLK_Ack
* Description  : 获取连接是否成功
* Input        : INT8U *pdata  
                 INT16U len    
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U GPRS_Command_SOCKDLK_Ack(INT8U *pdata, INT16U len)
{
	 if ( (pdata == NULL) || !len)
    {
        return FALSE;
    }
    if (strstr((char *)pdata, "connected") != NULL)
    {
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_OK;
		OSSchedUnlock();
    } 
	else
	{
		OSSchedLock();
		YOURENCmdState.YOUREN_ConectState = STATE_ERR;
		OSSchedUnlock();
	}
	return TRUE;
}
//GPRS发送表
static const _GPRS_AT_FRAME GPRS_SendFrameTable[GPRS_SEND_TABLE_LEN] = 
{
	{"+++"  					,	   GPRS_ENTER_ATCMD1						},		//进入AT指令配置 1

	{"a"  						,	   GPRS_ENTER_ATCMD2						},		//进入AT指令配置 2
	
	{"AT+WKMOD"         		,  	   GPRS_Command_WKMOD						},		//设置当前为网络透传模式

	{"AT+SOCKA"                 ,      GPRS_Command_SOCKA                       },		//连接socketAtcp客户端
	
	{"AT+SOCKB"                 ,      GPRS_Command_SOCKB                       },		//连接socketBtcp客户端
	
	{"AT+SOCKC"                 ,      GPRS_Command_SOCKC                       },		//连接socketCtcp客户端
	
	{"AT+SOCKD"                 ,      GPRS_Command_SOCKD                       },		//连接socketDtcp客户端	
	
	{"AT+SOCKAENON"               ,      GPRS_Command_SOCKAEN                     },		//使能socketA
	
	{"AT+SOCKBENON"               ,      GPRS_Command_SOCKBEN                     },		//使能socketB
	
	{"AT+SOCKCENON"               ,      GPRS_Command_SOCKCEN                     },		//使能socketC
	
	{"AT+SOCKDENON"               ,      GPRS_Command_SOCKDEN                     },		//使能socketD
	
	{"AT+SOCKAENOFF"               ,      GPRS_Command_SOCKAOFF                    },		//关闭socketA
	
	{"AT+SOCKBENOFF"               ,      GPRS_Command_SOCKBOFF                    },		//关闭socketB
	
	{"AT+SOCKCENOFF"               ,      GPRS_Command_SOCKCOFF                     },		//关闭socketC
	
	{"AT+SOCKDENOFF"               ,      GPRS_Command_SOCKDOFF                     },		//关闭socketD
	
	{"AT+SOCKALK"				,	   GPRS_Command_SOCKALK						},		//查询SOCKA状态
	
	{"AT+SOCKBLK"				,	   GPRS_Command_SOCKBLK						},		//查询SOCKB状态
	
	{"AT+SOCKCLK"				,	   GPRS_Command_SOCKCLK						},		//查询SOCKC状态
	
	{"AT+SOCKDLK"				,      GPRS_Command_SOCKDLK						},		//查询SOCKD状态
	
	{"AT+ENTM"					,      GPRS_ENTER_DATA							},		//进入数据模式
};

//GPRS接收表
static const _GPRS_AT_FRAME 	GPRS_RecvFrameTable[GPRS_RECV_TABLE_LEN] = 
{
	{"a"						,      GPRS_ENTER_ATCMD1_Ack						},		//进入AT指令配置 1

	{"+ok"						,      GPRS_ENTER_ATCMD2_Ack						},		//进入AT指令配置 2
	
	{"AT+WKMOD"         		,  	   GPRS_Command_WKMOD_Ack						},		//设置当前为网络透传模式
	
	{"AT+SOCKA"                 ,      GPRS_Command_SOCKA_Ack                       },		//连接socketAtcp客户端
	
	{"AT+SOCKB"                 ,      GPRS_Command_SOCKB_Ack                       },		//连接socketBtcp客户端
	
	{"AT+SOCKC"                 ,      GPRS_Command_SOCKC_Ack                       },		//连接socketCtcp客户端
	
	{"AT+SOCKD"                 ,      GPRS_Command_SOCKD_Ack                       },		//连接socketDtcp客户端
	
	{"AT+SOCKAEN"               ,      GPRS_Command_SOCKAEN_Ack                     },		//使能socketA
	
	{"AT+SOCKBEN"               ,      GPRS_Command_SOCKBEN_Ack                     },		//使能socketB
	
	{"AT+SOCKCEN"               ,      GPRS_Command_SOCKCEN_Ack                     },		//使能socketC
	
	{"AT+SOCKDEN"               ,      GPRS_Command_SOCKDEN_Ack                     },		//使能socketD
	
	{"AT+SOCKALK"               ,      GPRS_Command_SOCKALK_Ack                     },		//查询连接状态
	
	{"AT+SOCKBLK"               ,      GPRS_Command_SOCKBLK_Ack                     },		//查询连接状态
	
	{"AT+SOCKCLK"               ,      GPRS_Command_SOCKCLK_Ack                     },		//查询连接状态
	
	{"AT+SOCKDLK"               ,      GPRS_Command_SOCKDLK_Ack                     },		//查询连接状态
	
	{"AT+ENTM"					,      GPRS_ENTER_DATA_Ack       					},		//进入数据模式
	
};
/*****************************************************************************
* Function     :Module_QueryConectState
* Description  :查询连接状态
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U Module_QueryConectState(OS_EVENT* pevent)
{
	static _BSP_MESSAGE send_message;
    static INT8U i = 0;
	INT8U err;
    
	if(pevent == NULL)
	{
		return FALSE;
	}
	
	 send_message.MsgID = BSP_GPRS_AT;
	send_message.pData = (INT8U*)"AT+SOCKALK";	
	send_message.DataLen = strlen("AT+SOCKALK");

	if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
	{
		printf("Send message error %d",err);
		return FALSE;
	}
	
	
	//等待ec20回复AT命令，超时时间为2秒
	i = 0;
	while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
	{
		OSTimeDly(SYS_DELAY_100ms);
		//等待AT指令被回复
		if (YOURENCmdState.YOUREN_ConectState == STATE_OK)
		{
			break;
		}
		i++;
	}
	if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
	{
		printf("YOUREN respond cmd:ATCMD_ENTER_A timeout");
		return FALSE;
	}
	return TRUE;
	
}

/*****************************************************************************
* Function     :Module_YouRenTest
* Description  :有人模块测试是否存在
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U Module_YouRenTest(OS_EVENT* pevent)
{
	static _BSP_MESSAGE send_message;
    static INT8U i = 0;
	INT8U err;
	static INT8U count = 1;
    
	if(pevent == NULL)
	{
		return FALSE;
	}
	if(count == 1)
	{
		//第一次进入初始化
		memset(&YOURENCmdState,0,sizeof(YOURENCmdState));  //初始化有人模块状态
		count = 0;
	}
	//模块重启初始为之前配置的模式（NET 透传模式），应进入AT指令模式
//	从短信透传、 网络透传、 HTTPD 切换至指令模式的时序：
//1. 串口设备给模块连续发送“+++”， 模块收到“+++”后， 会给设备发送一个‘a’。
//在发送“+++”之前的 200ms 内不可发送任何数据。
//2. 当设备接收‘a’后， 必须在 3 秒内给模块发送一个‘a’。
//3. 模块在接收到‘a’后， 给设备发送“+ok”， 并进入“指令模式”。
//4. 设备接收到“+ok”后， 知道模块已进入“指令模式”， 可以向其发送 AT 指令。
	if(APP_GetYouRenStatus() != STATE_CFG)
	{
		 send_message.MsgID = BSP_GPRS_AT;
		send_message.pData = (INT8U*)"+++";	
		send_message.DataLen = strlen("+++");

		if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
		{
			printf("Send message error %d",err);
			return FALSE;
		}
		
		
		//等待ec20回复AT命令，超时时间为2秒
		i = 0;
		while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
		{
			OSTimeDly(SYS_DELAY_100ms);
			//等待AT指令被回复
			if (YOURENCmdState.State0.OneByte.ATCMD_ENTER_A == GPRS_RESPOND_OK)
			{
				break;
			}
			i++;
		}
		  //判断是否回复超时
		if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
		{
			printf("YOUREN respond cmd:ATCMD_ENTER_A timeout");
			return FALSE;
		}

		
		send_message.MsgID = BSP_GPRS_AT;
		send_message.pData = (INT8U*)"a";	
		send_message.DataLen = strlen("a");
		if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
		{
			printf("Send message error %d",err);
			return FALSE;
		}
		
		
		//等待ec20回复AT命令，超时时间为2秒
		i = 0;
		while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
		{
			OSTimeDly(SYS_DELAY_100ms);
			//等待AT指令被回复
			if (YOURENCmdState.State0.OneByte.ATCMD_ENTER_OK == GPRS_RESPOND_OK)
			{
				break;
			}
			i++;
		}
		//判断是否回复超时
		if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
		{
			printf("YOUREN respond cmd:ATCMD_ENTER_OK timeout");
			return FALSE;
		}
		YOURENCmdState.YOUREN_Status = STATE_CFG;				//配置模式
	}
	
	//设置当前网络模式	(透传)
	send_message.pData = (INT8U*)"AT+WKMOD";					
    send_message.DataLen = strlen("AT+WKMOD");
	if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
    {
        printf("Send message error %d",err);
        return FALSE;
    }
	//等待ec20回复AT命令，超时时间为2秒
    i = 0;
    while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
    {
        OSTimeDly(SYS_DELAY_100ms);
        //等待AT指令被回复
        if (YOURENCmdState.State0.OneByte.WKMOD_State == GPRS_RESPOND_OK)
        {
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
    {
        printf("YOUREN respond cmd:WKMOD_State timeout");
        return FALSE;
    }
	//使能socketA
	send_message.pData = (INT8U*)"AT+SOCKAENON";						
    send_message.DataLen = strlen("AT+SOCKAENON");
	if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
    {
        printf("Send message error %d",err);
        return FALSE;
    }
	//等待ec20回复AT命令，超时时间为2秒
    i = 0;
    while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
    {
        OSTimeDly(SYS_DELAY_100ms);
        //等待AT指令被回复
        if (YOURENCmdState.State1.TwoByte.SOCKAEN == GPRS_RESPOND_OK)
        {
            break;
        }
        i++;
    }
    //判断是否回复超时
    if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
    {
        printf("YOUREN respond cmd:SOCKAEN timeout");
        return FALSE;
    }
	return TRUE;
}

/*****************************************************************************
* Function     :Module_YouConnectServer
* Description  :有人模块连接服务器
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U Module_YouConnectServer(INT8U* pIP,INT16U port,OS_EVENT* pevent)
{
	INT8U buf[60];
     INT32S length;
	 if(pIP == NULL)
	 {
		 return FALSE;
	 }
	 //AT+SOCKA="TCP","test.usr.cn",2317
	length = snprintf((char *)buf, sizeof(buf), "AT+SOCKA=\"TCP\",\"%s\",%d,\r\n", pIP,port);
	if (length == -1)
	{
        printf("snprintf error, the len is %d", length);
		return FALSE;
	}
	if (BSP_UARTWrite(GPRS_UART, buf, strlen((char*)buf) ) == strlen((char*)buf) )
    {
		OSTimeDly(SYS_DELAY_5ms);	
    }
	return TRUE;
}	


/*****************************************************************************
* Function     :Module_YourenEnterData
* Description  :进入数据模式
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U Module_YourenEnterData(OS_EVENT* pevent)
{
	static _BSP_MESSAGE send_message;
    static INT8U i = 0;
	INT8U err;
	
	if(pevent == NULL)
	{
	 return FALSE;
	}
	 send_message.MsgID = BSP_GPRS_AT;
	send_message.pData = (INT8U*)"AT+ENTM";	
	send_message.DataLen = strlen("AT+ENTM");

	if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
	{
		printf("Send message error %d",err);
		return FALSE;
	}
	
	
	//等待ec20回复AT命令，超时时间为2秒
	i = 0;
	while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
	{
		OSTimeDly(SYS_DELAY_100ms);
		//等待AT指令被回复
		if (YOURENCmdState.State0.OneByte.ATCMD_DATA == GPRS_RESPOND_OK)
		{
			break;
		}
		i++;
	}
	if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
    {
        printf("YOUREN respond cmd:Module_YourenEnterData timeout");
        return FALSE;
    }
	return TRUE;
}

/*****************************************************************************
* Function     :Module_YourenEnterCfg
* Description  :进入配置模式
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static INT8U Module_YourenEnterCfg(OS_EVENT* pevent)
{	
	static _BSP_MESSAGE send_message;
    static INT8U i = 0;
	INT8U err;
	
	if(pevent == NULL)
	{
	 return FALSE;
	}
	send_message.MsgID = BSP_GPRS_AT;
	send_message.pData = (INT8U*)"+++";	
	send_message.DataLen = strlen("+++");

	if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
	{
		printf("Send message error %d",err);
		return FALSE;
	}
	
	
	//等待ec20回复AT命令，超时时间为2秒
	i = 0;
	while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
	{
		OSTimeDly(SYS_DELAY_100ms);
		//等待AT指令被回复
		if (YOURENCmdState.State0.OneByte.ATCMD_ENTER_A == GPRS_RESPOND_OK)
		{
			break;
		}
		i++;
	}
	  //判断是否回复超时
	if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
	{
		printf("YOUREN respond cmd:ATCMD_ENTER_A timeout");
		return FALSE;
	}

	
	send_message.MsgID = BSP_GPRS_AT;
	send_message.pData = (INT8U*)"a";	
	send_message.DataLen = strlen("a");
	if((err = OSQPost(pevent, &send_message)) != OS_ERR_NONE)
	{
		printf("Send message error %d",err);
		return FALSE;
	}
	
	
	//等待ec20回复AT命令，超时时间为2秒
	i = 0;
	while (i < SYS_DELAY_2s / SYS_DELAY_100ms)
	{
		OSTimeDly(SYS_DELAY_100ms);
		//等待AT指令被回复
		if (YOURENCmdState.State0.OneByte.ATCMD_ENTER_OK == GPRS_RESPOND_OK)
		{
			break;
		}
		i++;
	}
	//判断是否回复超时
	if (i == SYS_DELAY_2s / SYS_DELAY_100ms)
	{
		printf("YOUREN respond cmd:ATCMD_ENTER_OK timeout");
		return FALSE;
	}
	YOURENCmdState.YOUREN_Status = STATE_CFG;				//配置模式
	return TRUE;
}
/*****************************************************************************
* Function     :YOUREN_RecvDespose
* Description  :有人模块接收处理
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
static _BSP_MESSAGE * YOUREN_RecvDespose(_BSP_MESSAGE* pMsg)
{
	static  _BSP_MESSAGE  message;
		
	if(pMsg == NULL)
	{
		return NULL;
	}
	if((pMsg->pData == NULL) || (pMsg->DataLen == 0) )
	{
		return NULL;
	}
	if (!strncmp( (char *)pMsg->pData, "AT+SOCKALK", strlen("AT+SOCKALK") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+SOCKALK");
		return &message;
    }
	if (!strncmp( (char *)pMsg->pData, "AT+SOCKBLK", strlen("AT+SOCKBLK") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+SOCKBLK");
		return &message;
    }
	if (!strncmp( (char *)pMsg->pData, "AT+SOCKCLK", strlen("AT+SOCKCLK") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+SOCKCLK");
		return &message;
    }
	if (!strncmp( (char *)pMsg->pData, "AT+SOCKDLK", strlen("AT+SOCKDLK") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+SOCKDLK");
		return &message;
    }
	if (!strncmp( (char *)pMsg->pData, "AT+WKMOD", strlen("AT+WKMOD") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+WKMOD");
		return &message;
    }
	if (!strncmp( (char *)pMsg->pData, "AT+SOCKAEN", strlen("AT+SOCKAEN") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+SOCKAEN");
		return &message;
    }

	if (!strncmp( (char *)pMsg->pData, "AT+SOCKA", strlen("AT+SOCKA") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+SOCKA");
		return &message;
    }
	
	
	if (!strncmp( (char *)pMsg->pData, "AT+ENTM", strlen("AT+ENTM") ) )     
    {
    	message.pData = pMsg->pData;
		message.DataLen = strlen("AT+ENTM");
		return &message;
    }
	
	
	if (!strncmp( (char *)pMsg->pData, "+ok", strlen("+ok") ) )    
	{
		message.pData = pMsg->pData;
		message.DataLen = strlen("+ok");
		return &message;
	}
	if (!strncmp( (char *)pMsg->pData, "a", strlen("a") ) )    
	{
		message.pData = pMsg->pData;
		message.DataLen = strlen("a");
		return &message;
	}
	return NULL;
}

/* Private macro------------------------------------------------------------------------------*/	
_GPRS_SELECT_MODULE GPRSModuleYouRen = 					//有人模块节点
{
	.Link = {NULL,NULL},								//节点
	
	.Name = GPRS_MODULE_YOUREN,			
	
	.FunTest = Module_YouRenTest,				   		//模块测试 ，主要判断模块是否存在，让模块进入AT指令模式
	
	.FunConnectServer = Module_YouConnectServer,   		//配置服务器
	
	.FunEnterData = Module_YourenEnterData,				//进入数据模式
	
	.FunEnterCfg = Module_YourenEnterCfg,				//进入配置模式
	
	.SendFreamTab = GPRS_SendFrameTable,		   		//发送处理表
	
	.RecvFreamTab = GPRS_RecvFrameTable,				//接收处理表

	.FunGetExistState = APP_GetYouRenExistState,	
	
	.FunSetExistState = APP_SetYouRenExistState,

	.FunGetConnectState = APP_GetYouRenConnectState,
		
	.FunGetCfgState = APP_GetYouRenCfgIPPorttState,
		
	.FunGetStatus = APP_GetYouRenStatus,
	
	.FunRecvDispose = YOUREN_RecvDespose,				//接收处理
	
	.FunYouRenReset = YouRenReset,						//从新启动		

	.FunQueryConectState = Module_QueryConectState,		//查询模块是否连接成功
};

/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     :Register_YouRenNode
* Description  :注册有人模块节点
* Input        :
* Output       :
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月11日
*****************************************************************************/
INT8U Register_YouRenNode(void)
{
	INIT_LIST_HEAD(&GPRSModuleYouRen.Link);	
	return (GPRS_NodeRegister(&GPRSModuleYouRen.Link) );
}

/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/

