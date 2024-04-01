/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: main.c
* Author			:
* Date First Issued	: 130722
* Version			: V
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2013	        : V
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "DwinProtocol.h"
#include "bsp_uart.h"
#include "DisplayMain.h"
#include "DataChangeInterface.h"

/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static _LCD_SEND_CONTROL LCD_SendControl; 
/* Private function prototypes----------------------------------------------------------------*/
/*****************************************************************************
* Function     : LCD_SendBufLenClear
* Description  : 清除LCD发送BUF长度
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月14日  
*****************************************************************************/
static void LCD_SendBufLenClear(void)
{
    OSSchedLock();
    LCD_SendControl.Len = 0;
    OSSchedUnlock();
}

/*****************************************************************************
* Function     : LCD_SendByteToBuf
* Description  : 往LCD缓冲区写入一个字节
* Input        : INT8U data  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :  2018年6月14日  
*****************************************************************************/
static void LCD_SendByteToBuf(INT8U data) //向串口发送一个字节
{
    OSSchedLock();
    LCD_SendControl.Buf[LCD_SendControl.Len++] = data;
    if (LCD_SendControl.Len >= LCD_SEND_BUF_LEN)
    {
        LCD_SendControl.Len = 0;
    }
    OSSchedUnlock();
}

/*****************************************************************************
* Function     : LCD_SendTwoBytes
* Description  : 往LCD缓冲区写入一个半字，低字节先写入，高字节后写入
* Input        : INT16U data  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :  2018年6月14日  
*****************************************************************************/
static void LCD_SendTwoBytes(INT16U data) //向串口发送一个字
{
    LCD_SendByteToBuf( ( (data >> 8) & 0xFF) );
    LCD_SendByteToBuf( (data & 0xFF) );
}

/*****************************************************************************
* Function     : LCD_SendTwoBytes
* Description  : 往LCD缓冲区写入一个字，低字节先写入，高字节后写入
* Input        : INT32U data  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :  2018年6月14日  
*****************************************************************************/
static void LCD_SendFourBytes(INT32U data) //向串口发送一个字
{
    LCD_SendTwoBytes( ( (data >> 16) & 0xFFFF) );
    LCD_SendTwoBytes( (data & 0xFFFF) );
}

/*****************************************************************************
* Function     : LCD_SendToUart
* Description  : LCD发送buf数据写入串口
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :  2018年6月14日  
*****************************************************************************/
static INT8U LCD_SendToUart(void)
{
	static INT16U i = 0;

	while(APP_GetDispUartStatus() == DISP_UART_BUSY)
	{
		OSTimeDly(SYS_DELAY_2ms);	
		if(++i > SYS_DELAY_5s/SYS_DELAY_2ms)
		{
			i = 0;
			printf("wait send data timeout!");
			return FALSE;
		}
	}
	
	i = 0;
	APP_SetDispUartStatus(DISP_UART_BUSY);
    INT8U sendbytes = BSP_UARTWrite(DISPLAY_UART, LCD_SendControl.Buf, LCD_SendControl.Len);
    LCD_SendBufLenClear();
    return sendbytes;
}

/*****************************************************************************
* Function     : Dwin_FrameHead
* Description  : 迪文屏通信帧头
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月14日 
*****************************************************************************/
static void Dwin_FrameHead(void) //发送帧开始0x5a 0xa5
{
    LCD_SendTwoBytes(DWIN_LCD_HEAD);
}

/***********************************************************************************************
* Function		: PrintNum32uVariable
* Description	: 显示类型为INT32的数字变量
* Input			: INT16U VariableAddress  变量地址
                  INT32U num			  显示数据            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintNum32uVariable(INT16U VariableAddress,INT32U num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

	LCD_SendTwoBytes(VariableAddress);				             //地址
	
	LCD_SendFourBytes(num);						                 //数据
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
    APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: PrintNum16uVariable
* Description	: 显示类型为INT16的数字变量
* Input			: INT16U VariableAddress  变量地址
                  INT16U num			  显示数据            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintNum16uVariable(INT16U VariableAddress,INT16U num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

	LCD_SendTwoBytes(VariableAddress);				             //地址
	
	LCD_SendTwoBytes(num); 						                 //数据
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: PrintNum8uVariable
* Description	: 显示类型为INT8U的数字变量
* Input			: INT16U VariableAddress  变量地址
                  INT8U num			      显示数据             
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintNum8uVariable(INT16U VariableAddress,INT8U num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令    

	LCD_SendTwoBytes(VariableAddress);				             //地址
	
	LCD_SendByteToBuf(num); 						             //数据
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: PrintDecimalVariable
* Description	: 显示2位小数变量(0-99999.99)
* Input			: INT16U VariableAddress  变量地址
                  FP32 num			  	  显示数据            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintDecimalVariable(INT16U VariableAddress,FP32 num)
{	
    if((num < 0) || (num >100000))
    {
        return;
    }
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令
    LCD_SendByteToBuf(sizeof(INT32U) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）

	LCD_SendTwoBytes(VariableAddress);			                 //地址
	
	LCD_SendFourBytes((INT32U)(num*100));						 //发送的数据为实际数据的100倍
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: PrintIcon
* Description	: 显示图标变量
* Input			: INT16U VariableAddress  变量地址
                  INT16U num			  图标位置            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintIcon(INT16U VariableAddress,INT16U num)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(sizeof(num) + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令    

	LCD_SendTwoBytes(VariableAddress);			                 //地址
	
	LCD_SendTwoBytes(num); 							             //图标位置
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: PrintTime
* Description	: 显示时间变量
* Input			: INT32U VariableAddress  变量地址
                  _BSPRTC_TIME gRTC       RTC数据
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void PrintTime(INT16U VariableAddress,_BSPRTC_TIME gRTC)
{	
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
	
    LCD_SendByteToBuf(1 + sizeof(VariableAddress) + 12);          //数据长度=指令（1byte）+地址长度+变量个数
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令    

	LCD_SendTwoBytes(VariableAddress);				             //地址
	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Year) + 2000);						         //年
	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Month));						         //月
	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Day));						         //日
	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Hour));						         //时
	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Minute));						         //分
	LCD_SendTwoBytes(ByteBcdToHex(gRTC.Second));						         //秒
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}


/***********************************************************************************************
* Function		: PrintStr
* Description	: 显示字符串
* Input			: INT16U VariableAddress  变量地址
                  INT8U *s  			  字符串内容            
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void PrintStr(INT16U VariableAddress,INT8U *s,INT8U len)
{
    INT8U i = 0;

    if((s == NULL) || !len)
    {
        return ;
    }
    
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(len + sizeof(VariableAddress) +1+2); 		 //数据长度=地址长度+变量长度+指令（1byte）+覆盖指令（2byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令
	LCD_SendTwoBytes(VariableAddress);			                 //地址

    for (i = 0; i < len; i++)
    {
        LCD_SendByteToBuf(s[i]);
    }
    LCD_SendTwoBytes(DWIN_LCD_COVER);                           //覆盖上次显示内容，只显示本次内容长度数据
    //设置串口发送忙
    LCD_SendToUart();         									//开始发送
    OSTimeDly(SYS_DELAY_5ms); 									//确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
    //设置串口发送空闲
}

/***********************************************************************************************
* Function		: PrintMAC
* Description	: 显示MAC地址
* Input			: INT16U VariableAddress  变量地址
                  INT8U *s  			  字符串内容            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintMAC(INT16U VariableAddress,INT8U *s,INT8U len)
{
    INT8U i = 0;

    if((s == NULL) || !len)
    {
        return ;
    }
    
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(len + sizeof(VariableAddress) +1); 		 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令
	LCD_SendTwoBytes(VariableAddress);			                 //地址

    for (i = 0; i < len; i++)
    {
        LCD_SendByteToBuf(s[i]);
    }
    
    LCD_SendToUart();         									//开始发送
    OSTimeDly(SYS_DELAY_5ms); 									//确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: CleanSomePlace
* Description	: 清空某一地址段数据
* Input			: INT16U VariableAddress  变量地址
                  INT8U len  			  地址长度            
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void CleanSomePlace(INT16U VariableAddress,INT8U len)
{
    INT8U i;
    
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(len + sizeof(VariableAddress) +1); //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令    

	LCD_SendTwoBytes(VariableAddress);				             //地址

    for(i = 0;i<len;i++)
    {
	    LCD_SendByteToBuf(0); 						             //数据
    }
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);    
}

/***********************************************************************************************
* Function		: DisplayQRCode
* Description	: 显示二维码
* Input			: INT16U VariableAddress  变量地址
                  INT8U *str,			  ASCII地址
                  INT8U len               ASCII长度
* Output		:
* Note(s)		:
* Contributor	: 20201013 显示二维码
***********************************************************************************************/
void DisplayQRCode(INT16U VariableAddress, INT8U *str, INT8U len)
{
    INT8U i;
    
    if ( (str == NULL) || (len == 0) )
    {
        return;
    }

    Dwin_FrameHead();        									//帧头0x5A 0xA5
    LCD_SendByteToBuf(sizeof(VariableAddress) + len +1);        //数据长度
    LCD_SendByteToBuf(VARIABLE_WRITE);   						//写变量存储器指令 
    LCD_SendTwoBytes(VariableAddress);			                //地址
    for (i = 0; i < len; i ++)
    {
        LCD_SendByteToBuf(str[i]);                              //数据内容
    }
    LCD_SendToUart();         									//开始发送
    OSTimeDly(SYS_DELAY_5ms); 									//确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: KeyControl
* Description	: 键控函数
* Input			: INT8U keyval 键控值           
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void KeyControl(INT8U keyval)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(3u);                                       //数据长度
    LCD_SendByteToBuf(REGISTER_WRITE);   						 //写变量存储器指令    

	LCD_SendByteToBuf(DWIN_KEY_CONTROL_REGISTER);				 //地址

	LCD_SendByteToBuf(keyval); 						             //键值
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: ReadDwinInputStatus
* Description	: 读迪文屏输入状态
* Input			:         
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void ReadDwinInputStatus(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(3u);                                       //指令数据长度
    LCD_SendByteToBuf(REGISTER_READ);                            //读寄存器
    
    LCD_SendByteToBuf(DWIN_INPUT_STATUS_REGISTER);   			 //寄存器地址

    LCD_SendByteToBuf(1u);                                       //数据长度
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: ReadLcdBackLight
* Description	: 读迪文屏背光亮度值
* Input			:         
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void ReadLcdBackLight(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(3u);                                       //指令数据长度
    LCD_SendByteToBuf(REGISTER_READ);                            //读寄存器
    
    LCD_SendByteToBuf(DWIN_LED_STA_REGISTER);   			     //寄存器地址

    LCD_SendByteToBuf(1u);                                       //数据长度
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: LCD_Reset
* Description	: 显示屏复位
* Input			:          
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void LCD_Reset(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(4u);                                       //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(REGISTER_WRITE);   						 //写寄存器  

	LCD_SendByteToBuf(DWIN_RESET_REGISTER);				         //地址

	LCD_SendTwoBytes(DWIN_RESET_CMD); 						     //复位指令
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}
/***********************************************************************************************
* Function		: HEXtoBCD
* Description	: 将INT8U的HEX转换为对应的BCD码
* Input			: INT8U value   0~99
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
INT8U HEXtoBCD(INT8U value)
{
    INT8U  value_bcd;
        
    if(value > 99)
    {
        return FALSE;
    }

    value_bcd = ((value/10)<<4) + (value%10);

    return value_bcd;
}

/***********************************************************************************************
* Function		: BCDtoHEX
* Description	: 将INT8U的BCD码转换为对应的HEX
* Input			: INT8U value   0~0x99
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
INT8U BCDtoHEX(INT8U value)
{
    INT8U  value_bcd;
        
    if(value > 0X99)
    {
        return FALSE;
    }

    value_bcd = (value>>4)*10 + (value&0X0F);

    return value_bcd;
}

/***********************************************************************************************
* Function		: ReadDwinVariable
* Description	: 读取变量
* Input			: INT32U VariableAddress  变量起始地址
                  INT8U len               读取数据长度（字节），每个单位长度内有2个字节数据，
                                            如：len == 6 ,则从变量开始地址连续读取12字节数据
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void ReadDwinVariable(INT16U VariableAddress,INT8U len)
{	
    Dwin_FrameHead();        									 //帧头0x5A 0xA5

    LCD_SendByteToBuf(1 + sizeof(VariableAddress) + sizeof(len));//数据长度=指令（1byte）+地址长度+读取数据长度
    LCD_SendByteToBuf(VARIABLE_READ);   						 //读变量存储器指令    

	LCD_SendTwoBytes(VariableAddress);				             //地址
	
	LCD_SendByteToBuf(len);						                 //长度
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);

}

/***********************************************************************************************
* Function		: SetVariColor
* Description	: 设置变量颜色
* Input			: INT16U DescrPointer  	  描述指针地址
                  _SHOW_COLOR color  		  字符串颜色            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void SetVariColor(INT16U DescrPointer,_SHOW_COLOR color)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(5); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

	LCD_SendTwoBytes(DescrPointer + 0x03);		                 //0x03是描述指针偏移地址，此处代表颜色

	LCD_SendTwoBytes(color);
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: SetVariHide
* Description	: 隐藏变量
* Input			: INT16U DescrPointer  	  描述指针地址       
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void SetVariHide(INT16U DescrPointer)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(5); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

	LCD_SendTwoBytes(DescrPointer);		                         //描述指针

	LCD_SendTwoBytes(DWIN_HIDE_CMD);                             //隐藏变量
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: SetVariCancelHide
* Description	: 取消隐藏变量
* Input			: INT16U DescrPointer  	  描述指针地址 
                  INT16U VariableAddress  变量实际地址
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void SetVariCancelHide(INT16U DescrPointer,INT16U VariableAddress)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(5); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(VARIABLE_WRITE);   						 //写变量存储器指令

	LCD_SendTwoBytes(DescrPointer);		                         //描述指针

	LCD_SendTwoBytes(VariableAddress);                           //取消隐藏变量
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: PraPerprintBackImage
* Description	: 显示背景图片(这里画面ID和图片保存在屏上的顺序保持一致)
* Input			: INT16U IMAGE 图片ID
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PraPerprintBackImage(INT8U IMAGE)
{
    Dwin_FrameHead();         		    //帧头0x5A 0xA5
    LCD_SendByteToBuf(0x03);  		    //0x03 表示数据的长度，以字节为单位
    LCD_SendByteToBuf(REGISTER_WRITE);  //写DGUS寄存器
    
    LCD_SendByteToBuf(0x04);  		    //0x03、0x04 存放图片ID寄存器
    
    LCD_SendByteToBuf(IMAGE); 			//显示图片
    
    LCD_SendToUart();         			//开始发送
    OSTimeDly(SYS_DELAY_5ms); 			//确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}


/***********************************************************************************************
* Function		: ReadDWRTC
* Description	: 读取迪文RTC
* Input			: 
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void ReadDWRTC(void)
{
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(3); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(REGISTER_READ);   						 //写变量存储器指令

	LCD_SendByteToBuf(0x02);		                        

	LCD_SendByteToBuf(0x07);                         
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: ReadDWRTC
* Description	: 设置迪文RTC
* Input			: 
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void SetDWRTC(INT8U * ptime,INT8U len)
{
	INT8U i;
    Dwin_FrameHead();        									 //帧头0x5A 0xA5
    
    LCD_SendByteToBuf(9); 		 								 //数据长度=地址长度+变量长度+指令（1byte）
    LCD_SendByteToBuf(REGISTER_WRITE);   						 //写变量存储器指令

	for(i = 0;i < len;i++)
	{
		LCD_SendByteToBuf(ptime[i]);
	}	
    
    LCD_SendToUart();         									 //开始发送
    OSTimeDly(SYS_DELAY_5ms); 									 //确保发送完毕，必须有
	APP_SetDispUartStatus(DISP_UART_IDLE);
}

/***********************************************************************************************
* Function		: printBackImage
* Description	: 显示背景图片(这里画面ID和图片保存在屏上的顺序保持一致)
* Input			:
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintBackImage(INT8U Image)
{
    PraPerprintBackImage(Image);
}

/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/
