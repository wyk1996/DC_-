/*****************************************Copyright(C)******************************************
*******************************************浙江快电*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_RX8025.c
**创    建    人: 
**创  建  日  期:2018年7月3日
**最  新  版  本: V0.1
**描          述: RX8025驱动，8025时钟芯片用的SIIC是PB6 PB7，对应I2C驱动的0.
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include	"sysconfig.h"
#include	"bsp_i2c.h"
#include	"bsp_RX8025.h"
#include "bsp_rtc.h"
/* Private define-----------------------------------------------------------------------------*/
// 设备读写地址
#define	RX8025_ADDR_READ		0x65
#define	RX8025_ADDR_WRITE		0x64
// 设备寄存器地址
#define	RX8025_ADDR_SECONDS		0x00
#define	RX8025_ADDR_WEEK		0x30
#define	RX8025_ADDR_DATES		0x40
#define	RX8025_ADDR_MONTH		0x50
#define	RX8025_ADDR_MINUTES		0x80
#define	RX8025_ADDR_CONTROL1	0xE0
#define	RX8025_ADDR_CONTROL2	0xF0
// 设备操作模式
#define	RX8025_WRITE_MODE		0xF0
#define	RX8025_READ_MODE		0xF0 
#define	RX8025_SIMP_READ_MODE	0x04
/* Private typedef----------------------------------------------------------------------------*/
// 寄存器结构
// CONTROL1,结构定义
typedef struct
{
	INT8U CT:3;
	INT8U TEST:1;
	INT8U nCLEN2:1;
	INT8U HOUR24:1;
	INT8U DALE:1;
	INT8U WALE:1;
}_RX8025_REG_CONTROL1_BITS;
typedef union
{
	INT8U byte;
	_RX8025_REG_CONTROL1_BITS bits;
}_RX8025_REG_CONTROL1;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
static void RX8025Write(INT8U addr,INT8U *pData,INT8U len)
{
	INT8U i;
	IIC_Start();
	if(IIC_SendByte(RX8025_ADDR_WRITE)==FALSE)
	{
		IIC_Stop();
		return;
	}
	if(IIC_SendByte(addr)==FALSE)
	{
		IIC_Stop();
		return;
	}
	for(i=0;i<len;i++)
	{
		if(IIC_SendByte(pData[i])==FALSE)
		{
			IIC_Stop();
			return;
		}
	}
	IIC_Stop();
}
static void RX8025Read(INT8U addr,INT8U *pData,INT8U len)
{
	INT8U i;
	IIC_Start();
	if(IIC_SendByte(RX8025_ADDR_WRITE)==FALSE)
	{
		IIC_Stop();
		return;
	}
	if(IIC_SendByte(addr)==FALSE)
	{
		IIC_Stop();
		return;
	}
	IIC_Start();
	if(IIC_SendByte(RX8025_ADDR_READ)==FALSE)
	{
		IIC_Stop();
		return;
	}
	for(i=0;i<len-1;i++)
	{
		pData[i] = IIC_RecvByte();
		IIC_Ack();
	}
	pData[i] = IIC_RecvByte();
	IIC_NAck();
	IIC_Stop();
}
/***********************************************************************************************
* Function		: BSP_RX8025Write
* Description	: 向时钟芯片写数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月3日
***********************************************************************************************/
static void BSP_RX8025Write(INT8U *pData,INT8U len)
{
	RX8025Write((RX8025_ADDR_SECONDS&RX8025_WRITE_MODE),pData,len);
}
/***********************************************************************************************
* Function		: BSP_RX8025Read
* Description	: 从时钟芯片读取数据
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	:2018年7月3日
***********************************************************************************************/
static void BSP_RX8025Read(INT8U *pData,INT8U len)
{
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),pData,len);
}
/***********************************************************************************************
* Function		: BSP_RX8025ControlINTA
* Description	: 控制芯片的INTA脚状态
* Input			: state:状态(_BSPRX8025_INTAOUT)
					BSPRX8025_INTAOUT_HIZ:高阻
					BSPRX8025_INTAOUT_LOW:输出低
					BSPRX8025_INTAOUT_2HZ:输出2Hz(50%)的脉冲
					BSPRX8025_INTAOUT_1HZ:输出1Hz(50%)的脉冲
					BSPRX8025_INTAOUT_SEC:每秒的第0秒翻转?还是脉冲?
					BSPRX8025_INTAOUT_MIN:每分的第0秒翻转?还是脉冲?
					BSPRX8025_INTAOUT_HOUR:每时的第0秒翻转?还是脉冲?
					BSPRX8025_INTAOUT_MONTH:每月的第0秒翻转?还是脉冲?
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月3日
***********************************************************************************************/
//static void BSP_RX8025ControlINTA(_BSPRX8025_INTAOUT state)
//{
//	_RX8025_REG_CONTROL1 data;
//	
//	RX8025Read((RX8025_ADDR_CONTROL1&RX8025_READ_MODE),(INT8U *)(&data),1);
//	data.bits.CT = state;							// INTA输出1Hz方波
//	RX8025Write((RX8025_ADDR_CONTROL1&RX8025_WRITE_MODE),(INT8U *)(&data),1);
//}
/***********************************************************************************************
* Function		: BSP_RX8025Init
* Description	: RX8025初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月3日
***********************************************************************************************/
static void BSP_RX8025Init(void)
{
    INT8U buf[16];
	IIC_Init();
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),buf,8);
	buf[0] = 0x20;
	RX8025Write((RX8025_ADDR_CONTROL1&RX8025_WRITE_MODE),buf,1);	//24小时制
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),buf,16);
}

static void RTCRead(_BSPRTC_TIME *pTime)
{
	INT8U temp[7];
	
	OSSchedLock();
	BSP_RX8025Read(temp,7);
	OSSchedUnlock();
	pTime->Second = temp[0];
	pTime->Minute = temp[1];
	pTime->Hour = temp[2];
	pTime->Day = temp[4];
	pTime->Month = temp[5];
	pTime->Year = temp[6];
	pTime->Week = temp[3];
	if((pTime->Week == 0x0) || (pTime->Week > 0x07))
		pTime->Week = 0x07;
}

/***********************************************************************************************
* Function		: BSP_RTCGetTime
* Description	: 得到RTC时间
* Input			: *pTime:时间数据保存指针
* Output		: TRUE/FALSE
* Note(s)		: 失败自动重试2次,还是不行返回FALSE
* Contributor	: 2018年7月3日
***********************************************************************************************/
static INT8U BSP_RX8025GetTime(_BSPRTC_TIME *pTime)
{
	INT8U i;
	_BSPRTC_TIME time;
	
	i=3;
	while(i--)
	{
		RTCRead(&time);
		if(BSP_RTCCheckTime(&time)==TRUE)		// 数据合法
		{
			INT8U j,*pTemp0,*pTemp1;
			
			pTemp0 = (INT8U *)&time;
			pTemp1 = (INT8U *)pTime;
			// 保存数据
			for(j=0;j<sizeof(_BSPRTC_TIME);j++)
				pTemp1[j] = pTemp0[j];
			return TRUE;
		}
		// 读数据错
		BSP_RTCInit();
	}
	return FALSE;
}
static void RTCWrite(_BSPRTC_TIME *pTime)
{
	INT8U temp[7];
	if(pTime->Week >= 0x07)
		pTime->Week = 0x00;
	temp[0] = pTime->Second;
	temp[1] = pTime->Minute;
	temp[2] = pTime->Hour;
	temp[3] = pTime->Week;
	temp[4] = pTime->Day;
	temp[5] = pTime->Month;
	temp[6] = pTime->Year;
	OSSchedLock();
	BSP_RX8025Write(temp,7);
	OSSchedUnlock();
}
/***********************************************************************************************
* Function		: BSP_RTCSetTime
* Description	: 重新设置RTC时间
* Input			: *pTime:要设的时间数据指针
* Output		: TRUE/FALSE
* Note(s)		: 失败自动重试2次,还是不行返回FALSE
* Contributor	: 2018年7月3日
***********************************************************************************************/
INT8U BSP_RX8025SetTime(_BSPRTC_TIME *pTime)
{   
	INT8U i;
	_BSPRTC_TIME time;

	if(BSP_RTCCheckTime((_BSPRTC_TIME *)pTime)==FALSE)	// 数据合法
		return FALSE;
	i=3;
	while(i--)
	{
		RTCWrite(pTime);
		if(BSP_RTCGetTime(&time)==TRUE)
		{
			if(memcmp((INT8U *)&time,(INT8U *)pTime,sizeof(_BSPRTC_TIME)-1)==0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

static _BSP_RTC_OPERATION RX8025_Operation =
{
    .RTCInit        =   BSP_RX8025Init,
    .RTCGetTime     =   BSP_RX8025GetTime,
    .RTCSetTime     =   BSP_RX8025SetTime,
};

/*****************************************************************************
* Function     : BSP_GetPCF8563Operation
* Description  : 
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月3日     
*****************************************************************************/
_BSP_RTC_OPERATION* BSP_GetRX8025Operation(void)
{
    return &RX8025_Operation;
}
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
