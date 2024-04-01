/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: FlashDataDeal.c
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
#include "bsp_spi.h"
#include "spi_flash.h"
#include "FlashDataDeal.h"
#include "bsp_SST25VF016.h"
#include "w25qxx.h"

#ifndef MIN
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#endif //MIN
/* Private define-----------------------------------------------------------------------------*/
// typedef enum
// {
// 	MX25_FLASH_BUSY = 0,
// 	MX25_FLASH_IDLE,
// 	MX25_FLASH_MAX,
// }_MX25_FLASH_STATE;

//_MX25_FLASH_STATE MX25FlashState;
#define  BOATLOAD_BASE     1024*1024u
/* Private typedef----------------------------------------------------------------------------*/
const _FLASH_MANAGE_TABLE FlashParaManageTable[PARATABLELEN]=
{	
    //一下是配置信息
    

	//交易记录
	{PARA_1200_ID,  	PARA_1200_FLADDR,  		PARA_1200_FLLEN	    },   //当前交易记录个数，若超过1000，则直接覆盖前面的

	{PARA_1201_ID,  	PARA_1201_FLADDR,  		PARA_1201_FLLEN	    }, 	 //当前故障/告警记录个数，若超过1000，则直接覆盖前面的

	{PARA_1202_ID,  	PARA_1202_FLADDR,  		PARA_1202_FLLEN	    },   //留给操作sd卡使用的buf1

    {PARA_1203_ID,  	PARA_1203_FLADDR,  		PARA_1203_FLLEN	    },   //留给操作sd卡使用的buf2
	
	{PARA_1250_ID,  	PARA_1250_FLADDR,  		PARA_1250_FLLEN	    },	//交易记录

	{PARA_PRICALL_ID,  PARA_PRICALL_FLADDR,		PARA_PRICALL_FLLEN},	
	
	{PARA_JUST_ID,  PARA_JUST_FLADDR,		PARA_JUST_FLLEN},

	//存储A\B枪订单
	{PARA_BILLGUNA_ID,  	PARA_BILLGUNA_FLADDR,  		PARA_BILLGUNA_FLLEN	 },   //A枪订单
	
	{PARA_BILLGUNB_ID,  	PARA_BILLGUNB_FLADDR,  		PARA_BILLGUNB_FLLEN	 },   //A枪订单
	
	{PARA_CARDWHITEL_ID,  	PARA_CARDWHITEL_FLADDR,  	PARA_CARDWHITEL_FLLEN	    },   //卡白名单储存	
	
	{PARA_VINWHITEL_ID,  	PARA_VINWHITEL_FLADDR,  	PARA_VINWHITEL_FLLEN	    },   //VIN白名单储存	
	
	{PARA_OFFLINEBILLNUM_ID,PARA_OFFLINEBILLNUM_FLADDR, 1	},		//离线交易记录个数，最多100个

};

/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : SearchParaManagementTable
* Description  : 搜索flash参数配置表格
* Input        : INT16U DataId  
                 INT8U flag     
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年7月30日
*****************************************************************************/
static INT16S SearchParaManagementTable(const INT16U DataId)
{
	const _FLASH_MANAGE_TABLE *p = FlashParaManageTable;
    INT16U count;

	for (count = 0; count < PARATABLELEN; count++)
	{
	 	if (DataId == p[count].DataID)
		{
            return count;
		}
	}
	return -1;
}

/*****************************************************************************
* Function     : ProtocolFlashOperation
* Description  : flash真正读写函数
* Input        : INT8U offset         
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年7月30日
*****************************************************************************/
INT8U BootLoadWrite(INT8U* pBuffer,INT32U WriteAddr,INT16U NumByteToWrite)
{
	//程序  1  0x55 表示需要升级  非 0x55表示需要升级；2 - 5  表示程序长度    6以后全部是程序  
	W25QXX_Write(pBuffer,  WriteAddr+BOATLOAD_BASE,NumByteToWrite);
	return TRUE;
}

/*****************************************************************************
* Function     : ProtocolFlashOperation
* Description  : flash真正读写函数
* Input        : INT8U offset         
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年7月30日
*****************************************************************************/
INT8U BootLoadRead(INT8U* pBuffer,INT32U WriteAddr,INT16U NumByteToWrite)
{
	//程序  1  0x55 表示需要升级  非 0x55表示需要升级；2 - 5  表示程序长度    6以后全部是程序  
	W25QXX_Read(pBuffer,WriteAddr+BOATLOAD_BASE, NumByteToWrite);
	return TRUE;
}

/*****************************************************************************
* Function     : ProtocolFlashOperation
* Description  : flash真正读写函数
* Input        : INT8U offset         
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年7月30日
*****************************************************************************/
static INT16U ProtocolFlashOperation(INT16U offset, _FLASH_OPERATION *pOps)
{
    const _FLASH_MANAGE_TABLE *pTable = FlashParaManageTable;
    
	if ( (offset >= PARATABLELEN) || (pOps == NULL) || (pOps->ptr == NULL) )
	{
        return FALSE;
	}

    //根据命令来执行
	switch (pOps->RWChoose)
	{
		case FLASH_ORDER_WRITE:
			#if (BSP_FLASH25ENABLE > 0)			
			W25QXX_Write(pOps->ptr,pTable[offset].Addr,   MIN(pTable[offset].DataLen,pOps->Len));
			#else		
			SST26_WriteBuffer(pOps->ptr,pTable[offset].Addr,  MIN(pTable[offset].DataLen,pOps->Len));
			#endif 	
			break;
	
		case FLASH_ORDER_READ:
			#if (BSP_FLASH25ENABLE > 0)			
			W25QXX_Read(pOps->ptr,pTable[offset].Addr,   MIN(pTable[offset].DataLen,pOps->Len));
			#else		
			SST26_ReadBuffer(pOps->ptr,pTable[offset].Addr,  MIN(pTable[offset].DataLen,pOps->Len));
			#endif 		
			break;
		default:
            break;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_ChargeRecodeFlashOperation
* Description  : 读写交易记录
* Input        :
				count  读写在第几条 1 - 1000条
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_ChargeRecodeFlashOperation(INT16U count,_FLASH_OPERATION *pOps)
{
	if((pOps == NULL) || (count > 1000) )
	{
		return FALSE;
	}
	
	if(pOps->DataID != PARA_RECODE_ID || pOps->ptr == NULL)
	{
		return FALSE;
	}
	switch (pOps->RWChoose)
	{
		case FLASH_ORDER_WRITE:
				#if (BSP_FLASH25ENABLE > 0)			
				W25QXX_Write(pOps->ptr,PARA_RECODE_FLADDR + ((count - 1) * PARA_RECODE_FLLEN),  PARA_RECODE_FLLEN);
				#else		
				SST26_WriteBuffer(pOps->ptr,PARA_RECODE_FLADDR + ((count - 1) * PARA_RECODE_FLLEN),  PARA_RECODE_FLLEN);
				#endif 	
			break;
	
		case FLASH_ORDER_READ:
			#if (BSP_FLASH25ENABLE > 0)			
			W25QXX_Read( pOps->ptr,PARA_RECODE_FLADDR + ((count - 1) * PARA_RECODE_FLLEN), PARA_RECODE_FLLEN);
			#else		
			SST26_ReadBuffer( pOps->ptr,PARA_RECODE_FLADDR + ((count - 1) * PARA_RECODE_FLLEN), PARA_RECODE_FLLEN);
			#endif 		
			break;
		default:
            break;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_ChargeOffLineFlashOperation
* Description  : 读写离线交易激励
* Input        :
				count  读写在第几条 1 - 100条
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_ChargeOffLineFlashOperation(INT16U count,_FLASH_OPERATION *pOps)
{
	if((pOps == NULL) || (count > 100) )
	{
		return FALSE;
	}
	
	if(((pOps->DataID != PARA_OFFLINEBILL_ID) && (pOps->DataID != PARA_OFFLINEBILLFS_ID)) || pOps->ptr == NULL)
	{
		return FALSE;
	}
	switch (pOps->RWChoose)
	{
		case FLASH_ORDER_WRITE:
				#if (BSP_FLASH25ENABLE > 0)		
				if(pOps->DataID == PARA_OFFLINEBILLFS_ID)
				{
					W25QXX_Write(pOps->ptr,PARA_OFFLINEBILLFS_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN),  PARA_OFFLINEBILL_FLLEN);
				}
				else
				{
					W25QXX_Write(pOps->ptr,PARA_OFFLINEBILL_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN),  PARA_OFFLINEBILL_FLLEN);
				}
				#else
				if(pOps->DataID == PARA_OFFLINEBILLFS_ID)
				{
					SST26_WriteBuffer(pOps->ptr,PARA_OFFLINEBILLFS_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN),  PARA_OFFLINEBILL_FLLEN);
				}
				else
				{
					SST26_WriteBuffer(pOps->ptr,PARA_OFFLINEBILL_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN),  PARA_OFFLINEBILL_FLLEN);
				}				
				#endif 	
			break;
	
		case FLASH_ORDER_READ:
			#if (BSP_FLASH25ENABLE > 0)		
			if(pOps->DataID == PARA_OFFLINEBILLFS_ID)
			{
				W25QXX_Read( pOps->ptr,PARA_OFFLINEBILLFS_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN), PARA_OFFLINEBILL_FLLEN);
			}
			else
			{
				W25QXX_Read( pOps->ptr,PARA_OFFLINEBILL_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN), PARA_OFFLINEBILL_FLLEN);
			}		
			#else
			if(pOps->DataID == PARA_OFFLINEBILLFS_ID)
			{
				SST26_ReadBuffer( pOps->ptr,PARA_OFFLINEBILLFS_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN), PARA_OFFLINEBILL_FLLEN);
			}
			else
			{
				SST26_ReadBuffer( pOps->ptr,PARA_OFFLINEBILL_FLADDR + ((count - 1) * PARA_OFFLINEBILL_FLLEN), PARA_OFFLINEBILL_FLLEN);
			}			
			#endif 		
			break;
		default:
            break;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_ErrWarningFlashOperation
* Description  : 读写故障/告警记录
* Input        :
				cmd = 0 写全部整条故障/告警记录  cmd = 1只写故障消除时间
				count  读写在第几条 1 - 1000条
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_ErrWarningFlashOperation(INT8U cmd,INT16U count,_FLASH_OPERATION *pOps)
{
	if((pOps == NULL) || (count > 1000) )
	{
		return FALSE;
	}
	
	if(pOps->DataID != PARA_ERRWARN_ID || pOps->ptr == NULL)
	{
		return FALSE;
	}
	switch (pOps->RWChoose)
	{
		case FLASH_ORDER_WRITE:
			if(cmd == 0)   //写全部交易记录
			{
				#if (BSP_FLASH25ENABLE > 0)			
				W25QXX_Write(pOps->ptr,PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN),  PARA_ERRWARN_FLLEN);
				#else		
				SST26_WriteBuffer(pOps->ptr,PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN),  PARA_ERRWARN_FLLEN);
				#endif 	
			}
			else
			{
				//交易记录结束时间存储偏移18 长度7
				#if (BSP_FLASH25ENABLE > 0)			
				W25QXX_Write(pOps->ptr + 18,PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN + 18),  7);
				#else		
				SST26_WriteBuffer(pOps->ptr + 18,PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN + 18),  7);
				#endif 	
			}
			break;
	
		case FLASH_ORDER_READ:
			if(cmd == 0)   //读全部交易记录	
			{
				#if (BSP_FLASH25ENABLE > 0)			
				W25QXX_Read( pOps->ptr, PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN),PARA_ERRWARN_FLLEN);
				#else		
				SST26_ReadBuffer( pOps->ptr, PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN),PARA_ERRWARN_FLLEN);
				#endif 		
			}
			else
			{
				//交易记录结束时间存储偏移18 长度7
				#if (BSP_FLASH25ENABLE > 0)			
				W25QXX_Read(pOps->ptr + 18,PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN + 18), 7);
				#else		
				SST26_ReadBuffer(pOps->ptr + 18,PARA_ERRWARN_FLADDR + ((count - 1) * PARA_ERRWARN_FLLEN + 18), 7);	
				#endif 		
			}
			break;
		default:
            break;
	}
	return TRUE;
}


/****************************************擦除flash  必须关闭喂狗*********************************/
#include "stm32f4xx_flash.h"
#define FLASH_BASE_ADD  0x080F0000


#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
 
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0; 
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1; 
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2; 
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3; 
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4; 
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5; 
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6; 
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7; 
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8; 
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9; 
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10; 
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11; 
  }

  return sector;
}



INT8U write_flash(uint32_t add,uint16_t *FlashWriteBuf,uint16_t len)
{
	if((add < ADDR_FLASH_SECTOR_0) || (add > 0x080FFFFF) )
	{
		return FALSE;
	}

	FLASH_Unlock();	//解锁
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

	if (FLASH_COMPLETE != FLASH_EraseSector(GetSector(add),VoltageRange_3)) //擦除扇区内容
    {		
		return FALSE;
	}
	
	for (int i = 0; i < len/2; i++)
	{
		if (FLASH_COMPLETE != FLASH_ProgramHalfWord(add, FlashWriteBuf[i]))	//写入16位数据
		{			
			return FALSE;
		}
		add += 2;	//16位数据偏移两个位置
	}

	FLASH_Lock();	//上锁
     
	return TRUE;
}

INT16U read_flash(INT32U add)
{
	return *(INT16U*)add;
}

INT8U read_flashbuf(INT32U add,INT16U * buf,INT16U len)
{
	INT16U num;
	
	for(num = 0;num < len/2;num++)
	{
		buf[num] = read_flash(add);
		add +=2;
	}
}
INT16U STFlashbuf[300] = {0};



INT16U WFlashbuf[5] = {0x1111,0x2222,0x3333,0x4444,0x5555};
INT16U RFlashbuf[5] = {0};

INT8U Nflash_test(void)
{
	write_flash(ADDR_FLASH_SECTOR_11,WFlashbuf,sizeof(WFlashbuf));
	read_flashbuf(ADDR_FLASH_SECTOR_11,RFlashbuf,sizeof(RFlashbuf));
	__NOP();
}
/*************************************flash  end*************************************************/

/*****************************************************************************
* Function     : APP_FlashOperation
* Description  : FLASH读写操作
* Input        : _FLASH_OPERATION *pOps  
* Output       : None
* Return       : 返回读写成功的字节数
* Note(s)      : 
* Contributor  : 2018年7月30日        
*****************************************************************************/
INT16U APP_FlashOperation(_FLASH_OPERATION *pOps)
{
    INT16S offset;
    
	if (pOps == NULL)
	{
        return FALSE;
	}
	if(pOps->DataID == PARA_CFGALL_ID)  //系统配置  需要写到内部flash里面  20230131
	{
		__set_PRIMASK(1);   //关闭总中断
		if(pOps->RWChoose == FLASH_ORDER_WRITE)
		{
			IWDG_ReloadCounter();
			memcpy(STFlashbuf,pOps->ptr,pOps->Len);
			write_flash(ADDR_FLASH_SECTOR_11,STFlashbuf,sizeof(STFlashbuf));
			IWDG_Config(IWDG_Prescaler_64 ,625*6);	  //6s	
		}
		else
		{
			read_flashbuf(ADDR_FLASH_SECTOR_11,STFlashbuf,sizeof(STFlashbuf));
			memcpy(pOps->ptr,STFlashbuf,pOps->Len);
		}	
		__set_PRIMASK(0);   //开启总中断
		return  TRUE;
	}
	if(pOps->DataID == PARA_PRESETVOL_ID)  //系统配置  需要写到内部flash里面  20230131
	{
		__set_PRIMASK(1);
		if(pOps->RWChoose == FLASH_ORDER_WRITE)
		{
			IWDG_ReloadCounter();
			memcpy(STFlashbuf,pOps->ptr,pOps->Len);
			write_flash(ADDR_FLASH_SECTOR_10,STFlashbuf,sizeof(STFlashbuf));
			IWDG_Config(IWDG_Prescaler_64 ,625*6);	  //6s	
		}
		else
		{
			read_flashbuf(ADDR_FLASH_SECTOR_10,STFlashbuf,sizeof(STFlashbuf));
			memcpy(pOps->ptr,STFlashbuf,pOps->Len);
		}
		__set_PRIMASK(0);
		return  TRUE;
	}
	
	
	
	if (pOps->DataID != 0xFFFF)
	{
		offset = SearchParaManagementTable(pOps->DataID);  //通过ID查找偏移值
		if (offset < 0)
		{
			return FALSE;
		}
	}
	return ProtocolFlashOperation(offset, pOps);
	
}

/*****************************************************************************
* Function     : APP_FlashOperation
* Description  : FLASH读写操作
* Input        : _FLASH_OPERATION *pOps  
* Output       : None
* Return       : 返回读写成功的字节数
* Note(s)      : 
* Contributor  : 2018年7月30日        
*****************************************************************************/
//INT16U APP_FlashOperation(_FLASH_OPERATION *pOps)
//{
//	static INT16U i = 0;
//	INT16U res;
//	
//	if (pOps == NULL)
//	{
//        return FALSE;
//	}
//	while(MX25FlashState == MX25_FLASH_BUSY)
//	{
//		OSTimeDly(SYS_DELAY_5ms); 	
//		if(++i > SYS_DELAY_2s/SYS_DELAY_5ms)
//		{
//			i = 0;
//			return FALSE;
//		}
//	}
//	i = 0;
//	MX25FlashState =  MX25_FLASH_BUSY;	
//	res = FlashOperation(pOps);
//	MX25FlashState = MX25_FLASH_IDLE;
//	return res;
//}


