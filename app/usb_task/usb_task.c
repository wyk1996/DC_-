/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: usb_test.c
**硬          件: STM32
**创    建    人:  
**创  建  日  期:  
**最  新  版  本: V0.1
**描          述: usb程序
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "usb_task.h"
#include "usbh_usr.h" 	
#include "ff.h"
#include 	"sysconfig.h"
#include "FlashDataDeal.h"
/* Private macro------------------------------------------------------------------------------*/
#define USB_Q_LEN       20
static void *USBTaskOSQ[USB_Q_LEN];					// 消息队列
static OS_EVENT *USBTaskEvent;				            // 使用的事件

OS_EVENT *USBMutex;                 //USB互斥锁，同一时刻只能有一个任务进行临界点访问
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/

/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function	    : ReadFilesInfoinMSCUSB
* Description	: 
* Input		    :
* Return	    :
* Note(s)	    :
* Contributor	:
***********************************************************************************************/
#if 1
#define USB_BUF_SIZE   			 (512u*1)         //USB
#define USB_FILEPATH_SIZE        80                 //文件路基缓存大小
static char USBFilePathBuff[USB_FILEPATH_SIZE] = USBPATH"/kuaidian";
static char pfilename[30] = {USBPATH"/kuaidian/usbtest.txt"};
static FIL g_USBFileObject;
INT8U USB_Test(void)
{
	FRESULT fresult;
	INT8U err;
	INT32U lseek_len;				//偏移量长度
	INT32U write_num;				//写入的字节数

	OSMutexPend(USBMutex, 0, &err);					//获取锁
    fresult = f_mkdir(USBFilePathBuff);     
    if (fresult != FR_OK)
    {
        if (fresult != FR_EXIST) 					//目录创建不成功
        {
            printf("f_mkdir err f_mkdir %s error %d", USBFilePathBuff, fresult);
            OSMutexPost(USBMutex); 					//释放锁
            return FALSE;
        }
		 OSMutexPost(USBMutex); 					//释放锁
        //说明目录已经创建
//		printf("dir  Already created !");
    }
	fresult = f_open(&g_USBFileObject, pfilename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	if(fresult != FR_OK)
	{     
		printf("f_open error %d", fresult);
		OSMutexPost(USBMutex); //释放锁
		return FALSE;
	}
	lseek_len = g_USBFileObject.fsize;     //获取文件大小
	fresult = f_lseek(&g_USBFileObject,lseek_len);   //调整指针头
	if(fresult != FR_OK)
	{
		f_close(&g_USBFileObject);
		printf("f_lseek error %d", fresult);
		OSMutexPost(USBMutex); 
		return FALSE;
	}
	fresult = f_write(&g_USBFileObject, "0123456789 ",11,(UINT*)&write_num);
	if(fresult != FR_OK)
	{
		f_close(&g_USBFileObject);
		printf("f_write error %d", fresult);
		OSMutexPost(USBMutex); 
		return FALSE;
	}
	f_close(&g_USBFileObject);
	OSMutexPost(USBMutex); 					//释放锁
	return TRUE;
}
#endif

#if (BSP_USBHOSTENABLE > 0)	
INT8U UsbTestState;
INT8U updataflag = 0xAA;
INT8U USBUPDATA = 0;
#include "bsp_io.h"
/*****************************************************************************
* Function     : TaskUsbTest
* Description  : USB测试任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年5月22日       
*****************************************************************************/
void TaskUsbTask(void *pdata)
{
	OSTimeDly(SYS_DELAY_2s);
	while(1)
	{
		OSTimeDly(SYS_DELAY_2s);
		if(USBUPDATA)
		{
			updataflag = 0x66;	//U盘升级 0x66 远程升级 0x55
			BootLoadWrite(&updataflag,0,1);
			BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
			OSTimeDly(SYS_DELAY_50ms);	
			BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
			OSTimeDly(SYS_DELAY_50ms);	
			BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
			OSTimeDly(SYS_DELAY_500ms);	
			BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
			IWDG_ReloadCounter();
			jump_to_app(LoadCodeAddress);	
		}
	}
	
//	INT8U err;
//	_BSP_MESSAGE *pMsg;
//	static INT8U UsbTestState;
//	
//	pMsg = pMsg;
//	USBMutex = OSMutexCreate(PRI_USB_MUTEX, &err);      //创建互斥锁
//	USBTaskEvent = OSQCreate(USBTaskOSQ, USB_Q_LEN);//创建显示接收消息队列
//    if (USBTaskEvent == NULL)
//    {
//        OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
//        return;
//    }
//	OSTimeDly(SYS_DELAY_2s);
//	while(1)
//	{			
//			OSTimeDly(SYS_DELAY_5ms);
//			UsbTestState = Bsp_GetUSBState();
//			if(UsbTestState  && USBUPDATA)
//			{
//					USBUPDATA = 0;
////					USB_Test();
//				
//				
//					OSTimeDly(SYS_DELAY_500ms);
//					updataflag = 0x66;	//U盘升级 0x66 远程升级 0x55
//					BootLoadWrite(&updataflag,0,1);
//					BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
//					OSTimeDly(SYS_DELAY_50ms);	
//					BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
//					OSTimeDly(SYS_DELAY_50ms);	
//					BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
//					OSTimeDly(SYS_DELAY_500ms);	
//					BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
//					IWDG_ReloadCounter();
//					jump_to_app(LoadCodeAddress);		
//			}	
//		}
//		  //从串口读取一个消息
////        pMsg = OSQPend(USBTaskEvent, SYS_DELAY_500ms, &err);
////        if ((OS_ERR_NONE == err) && UsbTestState)  
////        {
////			;
////			switch(pMsg->MsgID)
////			{
////				;						//数据处理
////			}
////		}		
}

#endif
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
