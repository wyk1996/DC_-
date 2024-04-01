/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: datachangeinterface.h
* Author			: 
* Date First Issued	: 
* Version			: V2.1
* Description		: 包含主程序需要使用的常规函数，包括字符结构更改，时间计算函数
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2008		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef _DATA_CHANGE_INTERFACE_H_
#define _DATA_CHANGE_INTERFACE_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "time.h"
#include "bsp_rtc.h"

/* Private define-----------------------------------------------------------------------------*/
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) < (b) ? (b) : (a) )
/* Private typedef----------------------------------------------------------------------------*/
//时间结构体
typedef struct 
{
    u8 hour;
    u8 min;
    u8 sec;			
    //公历日月年周
    u16 w_year;
    u8  w_month;
    u8  w_date;
    u8  week;		 
}tm;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/

INT16U TwoByteToShort(INT8U *Point);
INT32U ThreeByteToInt(INT8U *Point);
INT32U FourByteToInt(INT8U *Point);
INT64U FiveByteToLong(INT8U *Point);

//整型拆分成单字节数据
void ShortToByte(INT16U Source, INT8U *Target);
void IntToFourByte(INT32U Source, INT8U *Target);
void LongToFiveByte(INT64U Source, INT8U *Target);

//BCD码转化成Hex
INT8U ByteBcdToHex(INT8U Source);
INT16U TwoByteBcdToHex(INT8U *Point);
INT32U ThreeByteBcdToHex(INT8U *Point);
INT32U FourByteBcdToHex(INT8U *Point);
INT64U FiveByteBcdToHex(INT8U *Point);

//Hex转化成BCD码
INT8U ByteHexToBcd(INT8U Source);
INT16U ShortHexToBCD(INT16U Source);
INT32U IntHexToBcd(INT32U Source);
INT64U LongHexToBcd(INT64U Source);

//BCD与Hex相加或相减
INT8U BcdAddHex(INT8U YuBcd,INT8U MuHex);
INT8U BcdDccHex(INT8U YuBcd,INT8U MuHex);
void  HextoAscii(INT32U hexdata,INT8U *data);
//国网专用数据格式转换
INT64S Dada02ToINT64S(INT16U Source); 
INT16U INT64SToDada02(INT64S Source);
INT32S Dada03ToINT32S(INT32U Source);
INT32U INT64SToDada03(INT64S Source);
INT32U INT32UToDada03(INT32U Source);
INT8U  CmpNBuf(INT8U*  pbuf1,INT8U*  pbuf2,INT8U len);
//任务索引数据校验
INT8U TeskCS(INT8U*Buff,INT16U len);

//取绝对值
INT32U Labs(long v);
/*****************************************************************************
* Function     : DigitBits
* Description  : 计算一个无符号整数的位数
* Input        : INT32U digit  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U DigitBits(INT32U digit);

/*****************************************************************************
* Function     : little2bigs
* Description  : 小端转大端，两个字节,最后一个s是short的意思
* Input        : INT16U l  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT16U little2bigs(INT16U l);

/*****************************************************************************
* Function     : big2littles
* Description  : 大端转小端,两个字节,最后一个s是short的意思
* Input        : INT16U b  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT16U big2littles(INT16U b);

/*****************************************************************************
* Function     : little2bigl
* Description  : 小端转大端，4个字节,最后一个l是long的意思
* Input        : INT32U l  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :2018年7月25日
*****************************************************************************/
INT32U little2bigl(INT32U l);

/*****************************************************************************
* Function     : big2littlel
* Description  : 大端转小端，4个字节,最后一个l是long的意思
                 
* Input        : INT32U b  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT32U big2littlel(INT32U b);

/*****************************************************************************
* Function     : HexToStr
* Description  : 十六进制转字符串
* Input        : INT8U* const pDest       ：字符串存储地址
                 const INT32U DestLen     ：字符串存储缓存大小
                 const INT8U* const pSrc  ：需要转的十六进制数
                 const INT32U SrcLen      ：需要转的十六进制数长度
* Output       : None
* Return       : 
* Note(s)      : 此函数转成功后会自动添加\0
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U HexToStr(INT8U* const pDest, const INT32U DestLen, const INT8U* const pSrc, const INT32U SrcLen);

void GetTheTime(_BSPRTC_TIME *Ptr_RtcTim);//获取当前时间
void AdjustTime(_BSPRTC_TIME *pTime, INT32U numb, BOOLEAN state);//计算某个时间点的倒退或将来的时间
/*****************************************************************************
* Function     : CompareTime
* Description  : 比较两个时间是否一致，一致返回0
* Input        : _BSPRTC_TIME Time1  
                 _BSPRTC_TIME Time2  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U CompareTime(_BSPRTC_TIME Time1, _BSPRTC_TIME Time2);

INT32U CmpTheTime(_BSPRTC_TIME *TimeBuff,_BSPRTC_TIME *TimeBuff2,INT8U Type);//计算两个时间点的时间差
INT8U CmpTeskTime(INT8U *TimeBuff);//与当前时间比大小 



/*****************************************************************************
* Function     : Is_Leap_Year
* Description  : 判断是否是闰年函数
* Input        : u16 year  ：要判断的年份
* Output       : None
* Return       : 该年份是不是闰年.1,是.0,不是
* Note(s)      : 
                月份     1  2  3  4  5  6  7  8  9  10 11 12
                闰年     31 29 31 30 31 30 31 31 30 31 30 31
                非闰年 31 28 31 30 31 30 31 31 30 31 30 31
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U Is_Leap_Year(INT16U year);

/*****************************************************************************
* Function     : RTC_Get_Week
* Description  : 获得现在是星期几，输入公历日期得到星期(只允许1901-2099年)
* Input        : u16 year  ：公历年
                 u8 month  ：公历月
                 u8 day    ：公历日 
* Output       : None
* Return       : 返回值：星期号(1~7,代表周1~周日)
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U RTC_Get_Week(INT16U year, INT8U month, INT8U day);

/*****************************************************************************
* Function     : Change2Unixstamp
* Description  : unix时间戳转换为北京时间，注意北京时间为GMT+8时区，输入的参数都为16进制数
* Input        : unsigned int year  
                 unsigned int mon   
                 unsigned int day   
                 unsigned int hour  
                 unsigned int min   
                 unsigned int sec   
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
time_t Change2Unixstamp(INT32U year, INT32U mon, INT32U day, INT32U hour,
                        INT32U min, INT32U sec);

/*****************************************************************************
* Function     : Unixstamp2RTC
* Description  : unix时间戳转tm格式时间
* Input        : INT32U unixstamp  
                 tm* ptime         
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U Unixstamp2RTC(INT32U unixstamp, tm* ptime);

/***********************************************************************************************
* Function		: ASC2BCD
* Description	: ASCII转BCD
* Input			: INT8U *asc        ASCII
                  INT8U asclen      ASCII长度
* Output		: INT8U *bcd        BCD
* Note(s)		: 仅支持纯数字的ASCII转换
* Contributor	: 2018年7月25日
***********************************************************************************************/
INT8U ASC2BCD(INT8U *asc, INT8U asclen, INT8U *bcd);
    
/* Private functions--------------------------------------------------------------------------*/
#endif // _DATA_CHANGE_INTERFACE_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
