/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: FmDataDeal.c
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
#include "Bsp_Fm.h"
#include "FmDataDeal.h"
/* Private define-----------------------------------------------------------------------------*/
#define FM_TABLELEN 4   		
/* Private typedef----------------------------------------------------------------------------*/
const _FM_MANAGE_TABLE FMParaManageTable[FM_TABLELEN]=
{
	{FM_0100_ID,  	FM_0100_FLADDR, 		FM_0100_FLLEN}, //A枪储存当前时间
	 
	{FM_0101_ID,  	FM_0101_FLADDR, 		FM_0101_FLLEN}, //A枪储存记录
	 
	{FM_0102_ID,  	FM_0102_FLADDR, 		FM_0102_FLLEN}, //B枪储存当前时间
	 
	{FM_0103_ID,  	FM_0103_FLADDR, 		FM_0103_FLLEN}, //B枪储存记录
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
static INT16S SearchFmManagementTable(const INT16U DataId)
{
	const _FM_MANAGE_TABLE *p = FMParaManageTable;
    INT16U count;

	for (count = 0; count < FM_TABLELEN; count++)
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
static INT16U ProtocolFmOperation(INT16U offset, _FM_OPERATION *pOps)
{
    const _FM_MANAGE_TABLE *pTable = FMParaManageTable;
    
	if ( (offset >= FM_TABLELEN) || (pOps == NULL) || (pOps->ptr == NULL) \
        || (pOps->Len < pTable[offset].DataLen) )
	{
        return FALSE;
	}

    //根据命令来执行
	switch (pOps->RWChoose)
	{
		case FM_ORDER_WRITE:
			BSP_WriteDataToFm(pTable[offset].Addr, pOps->ptr, pTable[offset].DataLen);
			break;
	
		case FM_ORDER_READ:
			BSP_ReadDataFromFm(pTable[offset].Addr, pOps->ptr, pTable[offset].DataLen);
			break;
		default:
            break;
	}
	return TRUE;
}

/*****************************************************************************
* Function     : APP_FMOperation
* Description  : FM读写操作
* Input        : _FLASH_OPERATION *pOps  
* Output       : None
* Return       : 返回读写成功的字节数
* Note(s)      : 
* Contributor  : 2018年7月30日        
*****************************************************************************/
INT16U APP_FMOperation(_FM_OPERATION *pOps)
{
    INT16S offset;
    
	if (pOps == NULL)
	{
        return FALSE;
	}
	if (pOps->DataID != 0xFFFF)
	{
		offset = SearchFmManagementTable(pOps->DataID);  //通过ID查找偏移值
		if (offset < 0)
		{
			return FALSE;
		}
	}
	return ProtocolFmOperation(offset, pOps);
}

