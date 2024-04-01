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
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __DWINPROTOCOL_H_
#define __DWINPROTOCOL_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "display.h"
#include  "bsp_rtc.h"
/* Private define-----------------------------------------------------------------------------*/
#define LCD_SEND_BUF_LEN            (300u)

#define DWIN_LCD_HEAD       	    (0x5AA5)                    //迪文屏数据帧头
#define KEY_VARI_ADDR       	    (0x0000)                    //键值变量地址
#define REGISTER_WRITE      	    (0x80)		                //写寄存器指令
#define REGISTER_READ       	    (0x81)		                //读寄存器指令
#define VARIABLE_WRITE      	    (0x82)		                //写变量存储器指令
#define VARIABLE_READ       	    (0x83)		                //读变量存储器指令

#define DWIN_RTC_REGISTER   	    (0x20)		                //RTC寄存器地址

#define DWIN_RESET_REGISTER   	    (0xEE)		                //复位寄存器地址
#define DWIN_KEY_CONTROL_REGISTER   (0x4F)		                //键控寄存器地址
#define DWIN_LED_STA_REGISTER       (0x1E)		                //背光亮度寄存器地址
#define DWIN_LED_OFF_VALUE          (0x00)		                //息屏状态背光亮度值（该值要与屏幕系统设置里的R7保持一致，修改时请慎重）
#define DWIN_INPUT_STATUS_REGISTER  (0xE9)		                //触摸屏录入状态寄存器地址

#define DWIN_RESET_CMD   	        (0x5AA5)		            //复位指令
#define DWIN_HIDE_CMD   	        (0xFF00)		            //隐藏指令
#define DWIN_LCD_COVER       	    (0xFFFF)                    //覆盖指令，在文本显示和ASCII显示时在帧尾添加，可以免去擦除操作
/* Private typedef----------------------------------------------------------------------------*/
typedef enum
 {
 	DISP_UART_BUSY = 0,
 	DISP_UART_IDLE,
	DISP_UART_MAX,
}_DISP_UART_STATUS;
typedef struct
{
    INT8U Buf[LCD_SEND_BUF_LEN];
    INT8U Len;
}_LCD_SEND_CONTROL;

typedef enum
{
	BLACK  = 0x0000,
	WHITE  = 0xFFFF,
	YELLOW = 0xFFE0,
	RED    = 0xF800,
	GREEN  = 0x07E0,
}_SHOW_COLOR;

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: printBackImage
* Description	: 显示背景图片(这里画面ID和图片保存在屏上的顺序保持一致)
* Input			:
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintBackImage(INT8U Image);

/***********************************************************************************************
* Function		: PrintNum32uVariable
* Description	: 显示类型为INT32的数字变量
* Input			: INT16U VariableAddress  变量地址
                  INT32U num			  显示数据            
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void PrintNum32uVariable(INT16U VariableAddress,INT32U num);

/***********************************************************************************************
* Function		: PrintNum16uVariable
* Description	: 显示类型为INT16的数字变量
* Input			: INT16U VariableAddress  变量地址
                  INT16U num			  显示数据            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintNum16uVariable(INT16U VariableAddress,INT16U num);

/***********************************************************************************************
* Function		: PrintNum16uVariable
* Description	: 显示类型为INT8U的数字变量
* Input			: INT16U VariableAddress  变量地址
                  INT8U num			  	  显示数据            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintNum8uVariable(INT16U VariableAddress,INT8U num);

/***********************************************************************************************
* Function		: PrintDecimalVariable
* Description	: 显示2位小数变量(范围0-99999.99)
* Input			: INT16U VariableAddress  变量地址
                  FP32 num			  	  显示数据            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintDecimalVariable(INT16U VariableAddress,FP32 num);

/***********************************************************************************************
* Function		: PrintIcon
* Description	: 显示图标变量
* Input			: INT16U VariableAddress  变量地址
                  INT16U num			  图标位置            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintIcon(INT16U VariableAddress,INT16U num);


/***********************************************************************************************
* Function		: PrintTime
* Description	: 显示时间变量
* Input			: INT32U VariableAddress  变量地址
                  _BSPRTC_TIME gRTC       RTC数据
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintTime(INT16U VariableAddress,_BSPRTC_TIME gRTC);


/***********************************************************************************************
* Function		: PrintStr
* Description	: 显示字符串
* Input			: INT16U VariableAddress  变量地址
                  INT8U *s  			  字符串内容          
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintStr(INT16U VariableAddress,INT8U *s,INT8U len);

/***********************************************************************************************
* Function		: PrintMAC
* Description	: 显示MAC地址
* Input			: INT16U VariableAddress  变量地址
                  INT8U *s  			  字符串内容            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void PrintMAC(INT16U VariableAddress,INT8U *s,INT8U len);

/***********************************************************************************************
* Function		: SetVariHide
* Description	: 隐藏变量
* Input			: INT16U DescrPointer  	  描述指针地址       
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void SetVariHide(INT16U DescrPointer);

/***********************************************************************************************
* Function		: SetVariCancelHide
* Description	: 取消隐藏变量
* Input			: INT16U DescrPointer  	  描述指针地址 
                  INT16U VariableAddress  变量实际地址
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void SetVariCancelHide(INT16U DescrPointer,INT16U VariableAddress);

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
void DisplayQRCode(INT16U VariableAddress, INT8U *str, INT8U len);
	
/***********************************************************************************************
* Function		: CleanSomePlace
* Description	: 清空某一地址段数据
* Input			: INT16U VariableAddress  变量地址
                  INT8U len  			  地址长度            
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void CleanSomePlace(INT16U VariableAddress,INT8U len);

/***********************************************************************************************
* Function		: KeyControl
* Description	: 键控函数
* Input			: INT8U keyval 键控值           
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void KeyControl(INT8U keyval);

/***********************************************************************************************
* Function		: ReadDwinInputStatus
* Description	: 读迪文屏输入状态
* Input			:           
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void ReadDwinInputStatus(void);

/***********************************************************************************************
* Function		: ReadLcdBackLight
* Description	: 读迪文屏背光亮度值
* Input			:         
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void ReadLcdBackLight(void);

/***********************************************************************************************
* Function		: LCD_Reset
* Description	: 显示屏复位
* Input			:          
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void LCD_Reset(void);

/***********************************************************************************************
* Function		: HEXtoBCD
* Description	: 将INT8U的HEX转换为对应的BCD码
* Input			: INT8U value   0~99
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
INT8U HEXtoBCD(INT8U value);

/***********************************************************************************************
* Function		: BCDtoHEX
* Description	: 将INT8U的BCD码转换为对应的HEX
* Input			: INT8U value   0~0x99
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
INT8U BCDtoHEX(INT8U value);

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
void ReadDwinVariable(INT16U VariableAddress,INT8U len);

/***********************************************************************************************
* Function		: SetVariColor
* Description	: 设置变量颜色
* Input			: INT16U DescrPointer  	  描述指针地址
                  _SHOW_COLOR color  		  字符串颜色           
* Output		:
* Note(s)		:
* Contributor	: 2018年6月14日  
***********************************************************************************************/
void SetVariColor(INT16U DescrPointer,_SHOW_COLOR color);

/***********************************************************************************************
* Function		: ReadDWRTC
* Description	: 读取迪文RTC
* Input			: 
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void ReadDWRTC(void);

/***********************************************************************************************
* Function		: ReadDWRTC
* Description	: 设置迪文RTC
* Input			: 
* Output		:
* Note(s)		:
* Contributor	:  2018年6月14日  
***********************************************************************************************/
void SetDWRTC(INT8U * ptime,INT8U len);
#endif
/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/


