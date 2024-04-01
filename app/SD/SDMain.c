/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: USBMain.c
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
#include "SDInterFace.h"
#include "Disinterface.h"
#include "sysconfig.h"
#include "FlashDataDeal.h"
#include "ff.h"
/* Private define-----------------------------------------------------------------------------*/
#define SD_PATH                "0:"
#define SD_OSQ_LEN             30
#define SD_BUF_SIZE            512
#define  SD_FILEPATH_SIZE      80         
/* Private typedef----------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
 OS_EVENT *SDEvent;               //SD卡事件控制块
 void *SDOSQ[SD_OSQ_LEN];         //SD卡消息队列
OS_EVENT *SDMutex;                 	//sd互斥锁

const char Catalog[] = "0:/HistoryTrade";
const char FileName[] = "0:/HistoryTrade/HistoryTrade.txt";  //交易记录
const char Title[] = "交易记录如下:";
const char Title1[] = "StartTime:";
const char Title2[] = "    EndTime:";
const char Title3[] = "    GunNum:";
const char Title4[] = "    CardNum:";
const char Title5[] = "    ChargeType:";
const char Title6[] = "    ChargeTime:";
const char Title7[] = "    CostPower:";
const char Title8[] = "    CostMoney:";
const char Title9[] = "    SettlementSign:";
const char Title10[] = "    CardStatus:";
const char Title11[] = "    GreyLockTimes:";
const char Title12[] = "    BeforeBalance:";
const char Title13[] = "    AfterBalance:";
const char Title14[] = "    SOC:";
const char Title15[] = "    Startup mode:";
const char Title16[] = "    StartfailReason:";
const char Title17[] = "    StopReason:";
const char Title18[] = "    VINCode:";
const char Title19[] = "    TradeNum:";
const INT8U last[2] = {0x0d, 0x0a};

FATFS SDFatFs;
FIL SDObj;
SDSYSINFO SDInfo;
UINT br;
FRESULT res;
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

/*****************************************************************************
* Function     : SetSDCardSTATUS
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
void SetSDCardSTATUS(SDSTATUS state)
{
	SDInfo.SDstatus = state;
}
/*****************************************************************************
* Function     : SetSDCardCatalogSign
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
void SetSDCardCatalogSign(SDVALUE state)
{
	SDInfo.CreatCatalogSign = state;
}
/*****************************************************************************
* Function     : GetSDCardCatalogSign
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
SDVALUE GetSDCardCatalogSign(void)
{
	return SDInfo.CreatCatalogSign;
}
/*****************************************************************************
* Function     : GetNeedSaveTradeSign
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
INT8U GetNeedSaveTradeSign(void)
{
	if(SDInfo.NeedTradeNum)
	{
		return TRUE;
	}
	return FALSE;
}
/*****************************************************************************
* Function     : CreatSDCatalog
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
INT8U CreatSDCatalog(void)
{
	f_mount(&SDFatFs, SD_PATH, 1);   //挂载0:盘符		
  res = f_mkdir(Catalog);	       //创建目录
	
	if ((res == FR_OK) || (res == FR_EXIST))
	{
		res = f_open( &SDObj , FileName , FA_CREATE_NEW);
		f_close(&SDObj);	
		if(res == FR_OK)                                  
		{		                                             
			res = f_open( &SDObj , FileName , FA_WRITE);
			res = f_write(&SDObj, Title, sizeof(Title),&br);  
      res = f_lseek(&SDObj, f_size(&SDObj));	
      res = f_write(&SDObj, last, sizeof(last), &br);			
			res = f_close(&SDObj);		
		}		
    		
   	return  TRUE;	
	}		
	return FALSE;
}
/*****************************************************************************
* Function     : SaveTradeRecord
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年9月3日   ZL
*****************************************************************************/
void SaveTradeRecord(void)
{
	INT16U i, j;
	_FLASH_OPERATION  FlashOper;
	_CHARGE_RECODE  TempRecord;
	
	for(i=1; i<=SDInfo.NeedTradeNum; i++)
	{
		APP_RWChargeRecode(((SDInfo.SaveTradeNum+i)%1000), FLASH_ORDER_READ, &TempRecord);
		
		res = f_open( &SDObj , FileName , FA_WRITE );
	  res = f_lseek(&SDObj, f_size(&SDObj));	
	
	  res = f_write(&SDObj, Title1, sizeof(Title1), &br);
	  f_printf(&SDObj, "%2x%2x%2x%2x%2x%2x", TempRecord.StartTime.Year,  TempRecord.StartTime.Month, \
	  TempRecord.StartTime.Day, TempRecord.StartTime.Hour, TempRecord.StartTime.Minute ,TempRecord.StartTime.Second);
	
	  res = f_write(&SDObj, Title2, sizeof(Title2), &br);
	  f_printf(&SDObj, "%2x%2x%2x%2x%2x%2x", TempRecord.EndTime.Year,  TempRecord.EndTime.Month, \
	  TempRecord.EndTime.Day, TempRecord.EndTime.Hour, TempRecord.EndTime.Minute ,TempRecord.EndTime.Second);	
	
	  res = f_write(&SDObj, Title3, sizeof(Title3), &br);
	  f_printf(&SDObj, "%d", TempRecord.Gun);
	
	  res = f_write(&SDObj, Title4, sizeof(Title4), &br);
	  f_printf(&SDObj, "%2x%2x%2x%2x%2x%2x%2x%2x", TempRecord.CardNum[0], TempRecord.CardNum[1], TempRecord.CardNum[2], \
		TempRecord.CardNum[3], TempRecord.CardNum[4], TempRecord.CardNum[5], TempRecord.CardNum[6], TempRecord.CardNum[7]);	
		
	  res = f_write(&SDObj, Title5, sizeof(Title5), &br);
	  f_printf(&SDObj, "%2d", TempRecord.ChargeType);

	  res = f_write(&SDObj, Title6, sizeof(Title6), &br);
	  f_printf(&SDObj, "%4d", TempRecord.ChargeTime);
		
	  res = f_write(&SDObj, Title7, sizeof(Title7), &br);
	  f_printf(&SDObj, "%6u", TempRecord.TotPower);		

	  res = f_write(&SDObj, Title8, sizeof(Title8), &br);
	  f_printf(&SDObj, "%6u", TempRecord.TotMoney);		
		
	  res = f_write(&SDObj, Title9, sizeof(Title9), &br);
	  f_printf(&SDObj, "%d", TempRecord.BillingStatus);	
		
	  res = f_write(&SDObj, Title10, sizeof(Title10), &br);
	  f_printf(&SDObj, "%d", TempRecord.CardState);	
		
	  res = f_write(&SDObj, Title11, sizeof(Title11), &br);
	  f_printf(&SDObj, "%d", TempRecord.CardLockNum);	

	  res = f_write(&SDObj, Title12, sizeof(Title12), &br);
	  f_printf(&SDObj, "%6u", TempRecord.BeforeCardBalance);	
		
	  res = f_write(&SDObj, Title13, sizeof(Title13), &br);
	  f_printf(&SDObj, "%6u", TempRecord.AfterCardBalance);
		
	  res = f_write(&SDObj, Title14, sizeof(Title14), &br);
	  f_printf(&SDObj, "%d", TempRecord.SOC);
		
	  res = f_write(&SDObj, Title15, sizeof(Title15), &br);
	  f_printf(&SDObj, "%d", TempRecord.StartType);	

	  res = f_write(&SDObj, Title16, sizeof(Title16), &br);
	  f_printf(&SDObj, "%d", TempRecord.StartFailReason);	
		
	  res = f_write(&SDObj, Title17, sizeof(Title17), &br);
	  f_printf(&SDObj, "%d", TempRecord.StopChargeReason);	
		
	  res = f_write(&SDObj, Title18, sizeof(Title18), &br);
		for(j=0;j<17;j++)
	    f_printf(&SDObj, "%2x", TempRecord.CarVin[j]);		
			
	  res = f_write(&SDObj, Title19, sizeof(Title19), &br);
		for(j=0;j<17;j++)
	    f_printf(&SDObj, "%2x", TempRecord.TransNum[j]);			
		
	  res = f_write(&SDObj, last, sizeof(last), &br);
		res = f_close(&SDObj);
		
		SDInfo.SaveTradeNum++;
	  OSTimeDly(SYS_DELAY_10ms);		
	}
	

	FlashOper.DataID = PARA_1202_ID;
	FlashOper.Len = PARA_1202_FLLEN;
	FlashOper.ptr = (INT8U *)&SDInfo.SaveTradeNum;
	FlashOper.RWChoose = FLASH_ORDER_WRITE;
	if(APP_FlashOperation(&FlashOper) == FALSE)
	{
		printf("Read Charge Mode err");
	}
}
/*****************************************************************************
* Function     : SDCard_STATUS
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年9月2日   ZL
*****************************************************************************/
void GetTadeTotalNum(void)
{
	  _FLASH_OPERATION  FlashOper;
	
		FlashOper.DataID = PARA_1200_ID;
		FlashOper.Len = PARA_1200_FLLEN;
		FlashOper.ptr = (INT8U *)&SDInfo.CurTradeNum;
		FlashOper.RWChoose = FLASH_ORDER_READ;
		if(APP_FlashOperation(&FlashOper) == FALSE)
		{
			printf("Read Charge Mode err");
		}
		if(SDInfo.CurTradeNum == 0xFFFFFFFF)
			 SDInfo.CurTradeNum = 0;
		
		if(SDInfo.CurTradeNum >= SDInfo.SaveTradeNum)
		{
		  SDInfo.NeedTradeNum = SDInfo.CurTradeNum -SDInfo.SaveTradeNum;			
		}

}
/*****************************************************************************
* Function     : SDCard_STATUS
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
INT8U SDCard_STATUS(void)
{
	if (SD_Detect() == SD_PRESENT) //SD卡插入
	{
		SetSDCardSTATUS(_INSEART);
     return TRUE;
	}
	SetSDCardCatalogSign(_SET);	
	SetSDCardSTATUS(_NOINSEART); 
	return FALSE;
}
/*****************************************************************************
* Function     : SDStorage_Init
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
void SDStorage_Init(void)
{
	INT8U err;
	SDMutex = OSMutexCreate(PRI_SD_MUTEX, &err);      //创建usb互斥锁	
	if(SDMutex == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    			  //挂起当前任务
				return;
	}
	SDEvent = OSQCreate(SDOSQ, SD_OSQ_LEN);     	 //创建消息队列
	if(SDEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);				 //挂起当前任务
		return;					
	}
	
	//memset(&SDInfo, 0 ,sizeof(SDSYSINFO));
  SDInfo.SDstatus = _NOINSEART;
	SDInfo.CreatCatalogSign = _RESET;
	SDInfo.NeedTradeNum = 0;
	SDInfo.TotalSpace = 0;
	SDInfo.SDUsedSpace = 0;
	
	SetSDCardCatalogSign(_SET);	
}
/*****************************************************************************
* Function     : exf_getfree
* Description  : 
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年8月31日   ZL
*****************************************************************************/
INT8U exf_getfree(INT32U *total, INT32U *free)
{
 FATFS *fs1;
 INT8U res;
 INT32U fre_clust=0, fre_sect=0, tot_sect=0;
  //得到磁盘信息及空闲簇数量
 res =(INT32U)f_getfree(SD_PATH, (DWORD*)&fre_clust, &fs1);
 if(res==0)
 {              
     tot_sect=(fs1->n_fatent-2)*fs1->csize; //得到总扇区数
     fre_sect=fre_clust*fs1->csize;   //得到空闲扇区数    
#if _MAX_SS!=512          //扇区大小不是512字节,则转换为512字节
  tot_sect*=fs1->ssize/512;
  fre_sect*=fs1->ssize/512;
#endif   
  *total=tot_sect>>1; //单位为KB
  *free=fre_sect>>1; //单位为KB 
  }
 return res;
} 
/*****************************************************************************
* Function     : TaskSDMaster
* Description  : SD卡任务
* Input        : void *pdata  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年6月15日
*****************************************************************************/
void TaskSDMaster(void *pdata)
{  
	_BSP_MESSAGE *pMsg;						
	INT8U err;
	pdata = pdata;
	SDStorage_Init();
	OSTimeDly(SYS_DELAY_5s);
	while(1)
	{	
		pMsg = OSQPend(SDEvent, SYS_DELAY_1s, &err);       	
		if (OS_ERR_NONE == err)					//收到消息
		{
			 switch(pMsg->MsgID)
			 {
				 case BSP_MSGID_DISP:            //来自显示任务消息	
							break;
				 default:
							break;
			 }
		}  
		
		if(SDCard_STATUS() == FALSE)         //判断卡是否插入
			 continue;
		
		if(GetSDCardCatalogSign() == _SET)  //判断插入后，建立目录
		{
			if(CreatSDCatalog() == TRUE)      //目录创建成功，或目录已存在都认为TRUE
			  SetSDCardCatalogSign(_RESET);					 
		}

		
		//判断桩是否空闲
		
		
		GetTadeTotalNum();//空闲获取FLASH内交易记录条数，当前记录的交易记录条数，对比差值后，写入差值记录

		
		if(GetNeedSaveTradeSign()==TRUE)//存入数据至SD卡
		{
			SaveTradeRecord();
		}
    
		//exf_getfree(&SDInfo.TotalSpace, &SDInfo.SDUsedSpace);
 }

}
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
