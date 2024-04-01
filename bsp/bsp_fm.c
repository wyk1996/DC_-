/*****************************************Copyright(C)******************************************
*******************************************���ݿ��********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: BSP_FM.c
* Author			: 
* Date First Issued	: 
* Version			: V
* Description		: ��Ӧ�����������磬ͬʱ��Ӧ��Ƭ����Ƭ���������
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
/* Note��ʹ�ö�ȡ����д���ʱ��Ҫ���� open �� close �Ĳ��� */

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
* Description    : FM25CL64 Ƭѡʹ��
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
* Description    : FM25CL64 Ƭѡ��ֹ
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
* Description    : FM25CL64 д����
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
* Description    : FM25CL64 д�����ر�
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
* Description    : ������������ܣ�������flash��������
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
* Description    : �ر������������ֹ���޲���
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
* Description    : ��ȡ״̬��
* Input          : None
* Output         : None
* Return         : ״̬��
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
* Description    : �ȴ���ȡ FM25CL64 ��ռ����
* Input          : None
* Output         : None
* Return         : 0 - ʧ��
*                  1 - �ɹ�
*******************************************************************************/
INT8U WaitFM(void)
{
	OSSchedLock();
	return 1; 
}
 
/*******************************************************************************
* Function Name  : FMFree
* Description    : FM25CL64 ���ʽ�������Դ�ͷ�
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
* Description    : FM25CL64 ��ʼ��
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
	  FmDisWP(0);   /* ȷ��״̬��д�ɹ� */
	  OpenFm(0);
	  SPI_SendRcvByte(BSPI_PORT, FM_WRSR,&SPI_statu);
	  SPI_SendRcvByte(BSPI_PORT, 0x80,&SPI_statu);  //WPEN = 0 ,else is 0;
	  CloseFm(0);
	  FmDisWP(0);

	  if(MAX_FM_LEN > FM_CHIPSIZE)
	  {/* ��Ƭ���� */
		  // set write enable
		  OpenFm(1);
		  SPI_SendRcvByte(BSPI_PORT, FM_WREN,&SPI_statu);
		  CloseFm(1);
		  
		  // set statue regist;
		  FmDisWP(1);	/* ȷ��״̬��д�ɹ� */
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
* Input          : FlashAddr - ��ַ��Χ FM_CHIPSIZE
*				   Len       - ���ݳ���
*                  DataAddr -  �����׵�ַ
* Output         : None.
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U WriteDataToFm(INT8U ChipNum, INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
	SPI_TypeDef *BSPI_PORT;
	INT8U SPI_statu = TRUE;
	if((FlashAddr >= FM_CHIPSIZE) || ((FlashAddr + Len) > FM_CHIPSIZE) || (DataAddr == NULL))
  	   return FALSE;
	assert_param(Len <= 2048);	/* �������ƣ����Ʒǵ�����ʱ�� */
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
* Input          : FlashAddr - ��ַ��Χ FM_CHIPSIZE
*				   Len       - ���ݳ���
* Output         : DataAddr -  ���ݴ���׵�ַ
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U ReadDataFromFm(INT8U ChipNum, INT32U FlashAddr,INT8U *DataAddr,INT32U Len)
{
  SPI_TypeDef *BSPI_PORT;
  u8 SPI_statu =TRUE;
  
  if((FlashAddr >= FM_CHIPSIZE) || ((FlashAddr + Len) > FM_CHIPSIZE) || (DataAddr == NULL))
  	   return FALSE;
  assert_param(Len <= 2048);	/* �������ƣ����Ʒǵ�����ʱ�� */
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
* Input          : FlashAddr - ��ַ��Χ MAX_FM_LEN
*				   Len       - ���ݳ���
*                  DataAddr -  �����׵�ַ
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
* Input          : FlashAddr - ��ַ��Χ MAX_FM_LEN
*				   Len       - ���ݳ���
* Output         : DataAddr -  ���ݴ���׵�ַ
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

/************************(C)COPYRIGHT 2018 ���ݿ��****END OF FILE****************************/



