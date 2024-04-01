/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_pcf8563.c
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp_i2c.h"
#include "bsp_pcf8563.h"
/* Private define-----------------------------------------------------------------------------*/
//读写从地址
#define PCF8563_ADDR_READ       (0xA3)      //读地址
#define PCF8563_ADDR_WRITE      (0xA2)      //写地址

#define PCF8563_TIME_LEN        (PCF8563_REGISTER_YEAR - PCF8563_REGISTER_SECOND + 1)
/* Private typedef----------------------------------------------------------------------------*/
//寄存器定义
typedef enum 
{
    PCF8563_REGISTER_CONTROL_STATE1,        //0x00：控制状态1寄存器       
    PCF8563_REGISTER_CONTROL_STATE2,        //0x01：控制状态2寄存器       
    PCF8563_REGISTER_SECOND,                //0x02：秒寄存器
    PCF8563_REGISTER_MINUTE,                //0x03：分寄存器
    PCF8563_REGISTER_HOUR,                  //0x04：时寄存器
    PCF8563_REGISTER_DAY,                   //0x05：日寄存器
    PCF8563_REGISTER_WEEK,                  //0x06：周寄存器
    PCF8563_REGISTER_MONTH,                 //0x07：月寄存器
    PCF8563_REGISTER_YEAR,                  //0x08：年寄存器
    PCF8563_REGISTER_MINUTE_ALARM,          //0x09：分闹铃寄存器       
    PCF8563_REGISTER_HOUR_ALARM,            //0x0A：时闹铃寄存器       
    PCF8563_REGISTER_DAY_ALARM,             //0x0B：天闹铃寄存器
    PCF8563_REGISTER_WEEKDAY_ALARM,         //0x0C：星期闹铃寄存器
    PCF8563_REGISTER_CLKOUT_CONTROL,        //0x0D：CLKOUT控制寄存器
    PCF8563_REGISTER_TIMER_CONTROL,         //0x0E：倒计数定时器控制寄存器
    PCF8563_REGISTER_TIMER,                 //0x0F：倒计数定时器值寄存器
    PCF8563_REGISTER_MAX,                   //0x10：寄存器最大地址
}_BSP_PCF8563_REGISTER;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/

/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : BSP_PCF8563Read
* Description  : 从PCFF8563读取数据一个或多个数据
* Input        : _BSP_PCF8563_REGISTER RegAddr  : 需要读取的寄存器起始地址
                 INT8U ReadRegNum               : 需要读取的寄存器个数
                 INT8U *pData                   : 读取到的数据存放的buf
                 INT8U Len                      : pData对应的buf大小
* Output       : None
* Return       : 
* Note(s)      : 采用了Master reads after setting register address (write register address; READ data)读模式
* Contributor  : 2018年6月14日        
*****************************************************************************/
static INT8U BSP_PCF8563Read(_BSP_PCF8563_REGISTER RegAddr, INT8U ReadRegNum, INT8U *pData, INT8U Len)
{
    INT8U i = 0;

    //寄存器是否合法
    if ( (RegAddr + ReadRegNum >= PCF8563_REGISTER_MAX) || !ReadRegNum)
    {
        return FALSE;
    }

    //Len必须要大于等于ReadRegNum，否则数据会存不全，当错误返回
    if ( (pData == NULL) || (Len < ReadRegNum) )
    {
        return FALSE;
    }
    
    //产生一个起始条件
    IIC_Start(); 
    //发送写从地址
    if (IIC_SendByte(PCF8563_ADDR_WRITE) == FALSE)
    {
        IIC_Stop(); 
        return FALSE;
    }
    //发送寄存器地址
    if (IIC_SendByte(RegAddr) == FALSE)
    {
        IIC_Stop();
        return FALSE;
    }
    //再次发送一个起始地址
    IIC_Start(); 
    //发送读从地址
    if (IIC_SendByte(PCF8563_ADDR_READ) == FALSE)
    {
        IIC_Stop(); 
        return FALSE;
    }
    while (ReadRegNum)
    {
        //保存读到的数据
        pData[i++] = IIC_RecvByte();
        //寄存器--
        if (--ReadRegNum == 0)
        {
            //最后一个寄存器，回复一个NO ack
            IIC_NAck();
        }
        else
        {
            //不是最后一个寄存器，回复一个ACK
            IIC_Ack();
        }
    }
    IIC_Stop(); 
    return TRUE;
}

/*****************************************************************************
* Function     : BSP_PCF8563Write
* Description  : 从PCFF8563写入一个或者多个数据
* Input        : _BSP_PCF8563_REGISTER RegAddr  : 需要写入的寄存器起始地址
                 INT8U ReadRegNum               : 需要写入的寄存器个数
                 INT8U *pData                   : 写入的数据buf
                 INT8U Len                      : pData对应的buf大小
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月14日        
*****************************************************************************/
static INT8U BSP_PCF8563Write(_BSP_PCF8563_REGISTER RegAddr, INT8U ReadRegNum, INT8U *pData, INT8U Len)
{
    INT8U i = 0;
    
    //寄存器是否合法
    if ( (RegAddr + ReadRegNum >= PCF8563_REGISTER_MAX) || !ReadRegNum)
    {
        return FALSE;
    }
    //Len必须要大于等于ReadRegNum，否则数据会写不全，当错误返回
    if ( (pData == NULL) || (Len < ReadRegNum) )
    {
        return FALSE;
    }

    //产生一个起始条件
    IIC_Start(); 
    //发送写从地址
    if (IIC_SendByte(PCF8563_ADDR_WRITE) == FALSE)
    {
        IIC_Stop(); 
        return FALSE;
    }
    //发送寄存器地址
    if (IIC_SendByte(RegAddr) == FALSE)
    {
        IIC_Stop();
        return FALSE;
    }
    //发送多个数据
    for (i = 0 ; i < Len; i++)
    {
        //发送数据
        if (IIC_SendByte(pData[i]) == FALSE)
        {
            IIC_Stop();
            return FALSE;
        }
    }
    return TRUE;
}


/*****************************************************************************
* Function     : BSP_PCF8563SetTime
* Description  : 设置时间
* Input        : INT8U *pData     
* Output       : None
* Return       : 
* Note(s)      : PCF8563内部存储格式为秒 分 时 日 星期 月 年，跟_BSPRTC_TIME不能
                 一一对应，先做一下转换
* Contributor  : 2017年6月14日        
*****************************************************************************/
static INT8U BSP_PCF8563SetTime(_BSPRTC_TIME *pData)
{
    INT8U temp[PCF8563_TIME_LEN];
    INT8U ret;
    
    if ( (pData == NULL))
    {
        return FALSE;
    }
    
    temp[0] = pData->Second;
    temp[1] = pData->Minute;
    temp[2] = pData->Hour;
    temp[3] = pData->Day;
    temp[4] = pData->Week;
    temp[5] = pData->Month;
    temp[6] = pData->Year;
    
    OSSchedLock();
    ret = BSP_PCF8563Write(PCF8563_REGISTER_SECOND, PCF8563_TIME_LEN, temp, sizeof(temp) ); 
    OSSchedUnlock();
    return ret;
}

/*****************************************************************************
* Function     : BSP_PCF8563GetTime
* Description  : 获取时间
* Input        : INT8U *pData  
                 INT8U Len     
* Output       : None
* Return       : 
* Note(s)      : PCF8563内部存储格式为秒 分 时 日 星期 月 年，跟_BSPRTC_TIME不能
                 一一对应，获取完后需要做一下转换
* Contributor  : 2018年6月14日       
*****************************************************************************/
static INT8U BSP_PCF8563GetTime(_BSPRTC_TIME *pData)
{
    INT8U temp[PCF8563_TIME_LEN];
    
    if (pData == NULL)
    {
        return FALSE;
    }

    OSSchedLock();
    //读取RTC时间
    if (BSP_PCF8563Read(PCF8563_REGISTER_SECOND, PCF8563_TIME_LEN, temp, sizeof(temp) ) == FALSE)
    {
        OSSchedUnlock();
        return FALSE;
    }
    OSSchedUnlock();
    pData->Second = temp[0] & 0x7f;  //秒只有0~6位有效
    pData->Minute = temp[1] & 0x7f;  //分只有0~6位有效
    pData->Hour   = temp[2] & 0x3f;  //时只有0~5位有效
    pData->Day    = temp[3] & 0x3f;  //日只有0~5位有效
    pData->Week   = temp[4] & 0x07;  //星期只有低三位有效
    pData->Month  = temp[5] & 0x1f;  //月只有0~4位有效
    pData->Year   = temp[6];
    return TRUE;
}

/*****************************************************************************
* Function     : BSP_PCF8563Init
* Description  : PCF8563初始化
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月14日        
*****************************************************************************/
static void BSP_PCF8563Init(void)
{
    IIC_Init();     //GPIO端口时钟初始化
}

//PCF8563操作结构体
static _BSP_RTC_OPERATION PFC8563_Operation =
{
    .RTCInit        =   BSP_PCF8563Init,
    .RTCGetTime     =   BSP_PCF8563GetTime,
    .RTCSetTime     =   BSP_PCF8563SetTime,
};

/*****************************************************************************
* Function     : BSP_GetPCF8563Operation
* Description  : 返回PCF8563操作函数
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月14日        
*****************************************************************************/
_BSP_RTC_OPERATION* BSP_GetPCF8563Operation(void)
{
    return &PFC8563_Operation;
}


/************************(C)COPYRIGHT 2018 杭州快电****END OF FILE****************************/
