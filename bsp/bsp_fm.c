/*****************************************Copyright(C)******************************************
*******************************************杭州快电********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: BSP_FM.c
* Author			: 
* Date First Issued	: 
* Version			: V
* Description		: 适应大容量的铁电，同时适应单片或两片铁电情况。
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		: V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp_spi.h"
#include "Bsp_Fm.h"
/* Private define-----------------------------------------------------------------------------*/
/* Note：使用读取或者写入的时候，要进行 open 和 close 的操作 */

#define FM_WREN 0x06
#define FM_WRDI 0x04
#define FM_RDSR 0x05
#define FM_WRSR 0x01
#define FM_READ 0x03
#define FM_WRITE 0x02
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*******************************************************************************
* Function Name  : BSP_FmCS
* Description    : FM25CL64 片选使能
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FmCS(INT8U ChipNum)
{
	if(ChipNum == 0)
	{
		SPI_CSEnable(SPI1);
	}

	if(ChipNum == 1)
	{
		SPI_CSEnable(SPI2);
	}
}

/*******************************************************************************
* Function Name  : BSP_FmDisCS
* Description    : FM25CL64 片选禁止
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FmDisCS(INT8U ChipNum)
{
	if(ChipNum == 0)
	{
		SPI_CSDisable(SPI1);
	}
}

/*******************************************************************************
* Function Name  : BSP_FmWP
* Description    : FM25CL64 写保护
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FmWP(INT8U ChipNum)
{
	if(ChipNum == 0)
	{
		GPIO_ResetBits(SPI_FM_64_PORT, SPI_FM_64_NCS_PIN);
	}	
}

/*******************************************************************************
* Function Name  : FmDisWP
* Description    : FM25CL64 写保护关闭
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FmDisWP(INT8U ChipNum)
{
	if(ChipNum == 0)
	{
		GPIO_SetBits(SPI_FM_64_PORT, SPI_FM_64_NCS_PIN);
	}
}

/*******************************************************************************
* Function Name  : BSP_OpenFm
* Description    : 打开铁电操作功能，做到与flash操作互斥
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void OpenFm(INT8U ChipNum)
{
	FmCS(ChipNum);
}

/*******************************************************************************
* Function Name  : CloseFm
* Description    : 关闭铁电操作，防止被无操作
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CloseFm(INT8U ChipNum)
{
	FmDisCS(ChipNum);
}

/*******************************************************************************
* Function Name  : BSP_FmReadStatue
* Description    : 读取状态字
* Input          : None
* Output         : None
* Return         : 状态字
*******************************************************************************/
u8 FmReadStatue(INT8U ChipNum)
{
	 SPI_TypeDef *BSPI_PORT = SPI1;
    INT8U SpiStatue = TRUE;
    INT8U statue;
  
    OpenFm(ChipNum);
  
    SPI_SendRcvByte(BSPI_PORT,FM_RDSR,&SpiStatue);
    statue = SPI_SendRcvByte(BSPI_PORT,0xff,&SpiStatue);
  
    CloseFm(ChipNum);
    return statue;
}

/*******************************************************************************
* Function Name  : WaitFM
* Description    : 等待获取 FM25CL64 独占访问
* Input          : None
* Output         : None
* Return         : 0 - 失败
*                  1 - 成功
*******************************************************************************/
INT8U WaitFM(void)
{
	OSSchedLock();
	return 1; 
}
 
/*******************************************************************************
* Function Name  : FMFree
* Description    : FM25CL64 访问结束，资源释放
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/ 
void FMFree(void)
{ 
	OSSchedUnlock();
}


/*******************************************************************************
* Function Name  : BSP_InitFm
* Description    : FM25CL64 初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BSP_InitFm(void)
{
  SPI_TypeDef *BSPI_PORT;
  INT8U       SPI_statu = TRUE;
	
   BSPI_PORT = SPI1;
	
  if(WaitFM())
  {
	  BSP_SPI1Init();	  
	  DelayUS(500);

	// set write enable
	  OpenFm(0);
	  SPI_SendRcvByte(BSPI_PORT, FM_WREN,&SPI_statu);
	  CloseFm(0);
	  
	// set statue regist;
	  FmDisWP(0);   /* 确保状态字写成功 */
	  OpenFm(0);
	  SPI_SendRcvByte(BSPI_PORT, FM_WRSR,&SPI_statu);
	  SPI_SendRcvByte(BSPI_PORT, 0x80,&SPI_statu);  //WPEN = 0 ,else is 0;
	  CloseFm(0);
	  FmDisWP(0);

	  if(MAX_FM_LEN > FM_CHIPSIZE)
	  {/* 两片铁电 */
		  // set write enable
		  OpenFm(1);
		  SPI_SendRcvByte(BSPI_PORT, FM_WREN,&SPI_statu);
		  CloseFm(1);
		  
		  // set statue regist;
		  FmDisWP(1);	/* 确保状态字写成功 */
		  OpenFm(1);
		  SPI_SendRcvByte(BSPI_PORT, FM_WRSR,&SPI_statu);
		  SPI_SendRcvByte(BSPI_PORT, 0x80,&SPI_statu);  //WPEN = 0 ,else is 0;
		  CloseFm(1);
		  FmDisWP(1);
	  }

	  FMFree();
  }
}

/*******************************************************************************
* Function Name  : WriteDataToFm
* Description    : Write Data To Fm25CL64
* Input          : FlashAddr - 地址范围 FM_CHIPSIZE
*				   Len       - 数据长度
*                  DataAddr -  数据首地址
* Output         : None.
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U WriteDataToFm(INT8U ChipNum, INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
	SPI_TypeDef *BSPI_PORT;
	INT8U SPI_statu = TRUE;
	if((FlashAddr >= FM_CHIPSIZE) || ((FlashAddr + Len) > FM_CHIPSIZE) || (DataAddr == NULL))
  	   return FALSE;
	assert_param(Len <= 2048);	/* 长度限制，限制非调度区时间 */
	if(WaitFM())
	{
		  BSPI_PORT = SPI1 ; 
		OpenFm(ChipNum);
		SPI_SendRcvByte(BSPI_PORT, FM_WREN,&SPI_statu);
		CloseFm(ChipNum);
		 
		OpenFm(ChipNum);
#if	FM25CL64_ENABLE>0
		SPI_SendRcvByte(BSPI_PORT, FM_WRITE,&SPI_statu);
#endif
#if FM25L04_ENABLE>0
		if(FlashAddr>=0x100)
			SPI_SendRcvByte(BSPI_PORT, FM_WRITE|0x08,&SPI_statu);
		else
			SPI_SendRcvByte(BSPI_PORT, FM_WRITE|0x00,&SPI_statu);
#endif
#if	FM25CL64_ENABLE>0
		if(FM_CHIPSIZE > 0x10000)
		{
			SPI_SendRcvByte(BSPI_PORT, FlashAddr>>16,&SPI_statu);
		}
		SPI_SendRcvByte(BSPI_PORT, FlashAddr>>8,&SPI_statu);
		SPI_SendRcvByte(BSPI_PORT, FlashAddr,&SPI_statu);
#endif 
#if	FM25L04_ENABLE>0
		SPI_SendRcvByte(BSPI_PORT, FlashAddr,&SPI_statu);
#endif 
		SPI_statu = SPI_BufferSend1(BSPI_PORT,DataAddr,Len);  
	    CloseFm(ChipNum);
		FMFree();
		return SPI_statu;
	}
	return FALSE;
}

/*******************************************************************************
* Function Name  : ReadDataFromFm
* Description    : Read Data From Fm25CL64
* Input          : FlashAddr - 地址范围 FM_CHIPSIZE
*				   Len       - 数据长度
* Output         : DataAddr -  数据存放首地址
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U ReadDataFromFm(INT8U ChipNum, INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
  SPI_TypeDef *BSPI_PORT;
  u8 SPI_statu =TRUE;
  
  if((FlashAddr >= FM_CHIPSIZE) || ((FlashAddr + Len) > FM_CHIPSIZE) || (DataAddr == NULL))
  	   return FALSE;
  assert_param(Len <= 2048);	/* 长度限制，限制非调度区时间 */
  if(WaitFM())
  {
	   BSPI_PORT = SPI1; 
	   OpenFm(ChipNum);
	 
#if	FM25CL64_ENABLE>0
		SPI_SendRcvByte(BSPI_PORT, FM_READ,&SPI_statu);
#endif
#if FM25L04_ENABLE>0
		if(FlashAddr>=0x100)
			SPI_SendRcvByte(BSPI_PORT, FM_READ|0x08,&SPI_statu);
		else
			SPI_SendRcvByte(BSPI_PORT, FM_READ|0x00,&SPI_statu);
#endif
#if	FM25CL64_ENABLE>0
		if(FM_CHIPSIZE > 0x10000)
		{
			SPI_SendRcvByte(BSPI_PORT, FlashAddr>>16,&SPI_statu);
		}
		SPI_SendRcvByte(BSPI_PORT, FlashAddr>>8,&SPI_statu);
		SPI_SendRcvByte(BSPI_PORT, FlashAddr,&SPI_statu);
#endif
#if	FM25L04_ENABLE>0
		SPI_SendRcvByte(BSPI_PORT, FlashAddr,&SPI_statu);
#endif
	   SPI_statu = SPI_BufferReceive1(BSPI_PORT,DataAddr,Len);  
	     
	   CloseFm(ChipNum);
	   FMFree();
	   return SPI_statu;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : BSP_WriteDataToFm
* Description    : Write Data To Fm25CL64
* Input          : FlashAddr - 地址范围 MAX_FM_LEN
*				   Len       - 数据长度
*                  DataAddr -  数据首地址
* Output         : None.
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_WriteDataToFm(INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
	INT8U   ChipNum, ret;
	INT32U  Addr, FirstLen, SecondLen;

	if(Len >= FM_CHIPSIZE || FlashAddr + Len > MAX_FM_LEN)
		return FALSE;
	OSSchedLock();
	ChipNum = FlashAddr / FM_CHIPSIZE;
	Addr    = FlashAddr % FM_CHIPSIZE;
	FirstLen= ((Addr + Len)>FM_CHIPSIZE? (FM_CHIPSIZE - Addr):Len);
	ret = WriteDataToFm(ChipNum, Addr, DataAddr, FirstLen);

	if(ret == FALSE)
	{
		OSSchedUnlock();
  	    return FALSE;
	}
	if(FirstLen == Len)
	{
	    OSSchedUnlock();
		return TRUE;
	}

	SecondLen = Len -  FirstLen;
	ret = WriteDataToFm(ChipNum + 1, 0, DataAddr + FirstLen, SecondLen);
	OSSchedUnlock();
	return ret;
}

/*******************************************************************************
* Function Name  : ReadDataFromFm
* Description    : Read Data From Fm25CL64
* Input          : FlashAddr - 地址范围 MAX_FM_LEN
*				   Len       - 数据长度
* Output         : DataAddr -  数据存放首地址
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_ReadDataFromFm(INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
	INT8U   ChipNum, ret;
	INT32U  Addr, FirstLen, SecondLen;

	if(Len >= FM_CHIPSIZE || FlashAddr + Len > MAX_FM_LEN)
		return FALSE;
	OSSchedLock();
	ChipNum = FlashAddr / FM_CHIPSIZE;
	Addr    = FlashAddr % FM_CHIPSIZE;
	FirstLen= ((Addr + Len)>FM_CHIPSIZE? (FM_CHIPSIZE - Addr):Len);
	ret = ReadDataFromFm(ChipNum, Addr, DataAddr, FirstLen);

	if(ret == FALSE)
	{
		OSSchedUnlock();
  	    return FALSE;
	}
	if(FirstLen == Len)
	{
	    OSSchedUnlock();
		return TRUE;
	}

	SecondLen = Len -  FirstLen;
	ret = ReadDataFromFm(ChipNum + 1, 0, DataAddr + FirstLen, SecondLen);
	OSSchedUnlock();
	return ret;
}

/************************(C)COPYRIGHT 2018 杭州快电****END OF FILE****************************/



