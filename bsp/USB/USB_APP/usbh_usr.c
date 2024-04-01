#include "usbh_usr.h" 
#include "usb_hcd_int.h"
#include "ff.h"  


_USB_STATE  USBState = USB_UNAVAILABLE ;       
static FATFS USBFatFs;   //存储USB挂在信息
static u8 AppState; 
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
//USB OTG 中断服务函数
//USB HOST 用户回调函数.
USBH_Usr_cb_TypeDef USR_cb =
{
	USBH_USR_Init,
	USBH_USR_DeInit,
	USBH_USR_DeviceAttached,
	USBH_USR_ResetDevice,
	USBH_USR_DeviceDisconnected,
	USBH_USR_OverCurrentDetected,
	USBH_USR_DeviceSpeedDetected,
	USBH_USR_Device_DescAvailable,
	USBH_USR_DeviceAddressAssigned,
	USBH_USR_Configuration_DescAvailable,
	USBH_USR_Manufacturer_String,
	USBH_USR_Product_String,
	USBH_USR_SerialNum_String,
	USBH_USR_EnumerationDone,
	USBH_USR_UserInput,
	USBH_USR_MSC_Application,
	USBH_USR_DeviceNotSupported,
	USBH_USR_UnrecoveredError 
}; 
/////////////////////////////////////////////////////////////////////////////////
//以下为各回调函数实现.


//USB HOST 初始化 
void USBH_USR_Init(void)
{
	printf("USB OTG HS MSC Host\r\n");
	printf("> USB Host library started.\r\n");
	printf("  USB Host Library v2.1.0\r\n\r\n");
	
}
//检测到U盘插入
void USBH_USR_DeviceAttached(void)//U盘插入
{
	USBState  = USB_UNAVAILABLE;			//USB状态不可用
	printf("USB Dev Insert!\r\n");
}
//检测到U盘拔出
void USBH_USR_DeviceDisconnected (void)//U盘移除
{
	USBState  = USB_UNAVAILABLE;			//USB状态不可用
	printf("USB Dev Pull Out!\r\n");
}  
//复位从机
void USBH_USR_ResetDevice(void)
{
	USBState  = USB_UNAVAILABLE;			//USB状态不可用
	printf("Init Dev...\r\n");
}
//检测到从机速度
//DeviceSpeed:从机速度(0,1,2 / 其他)
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
	if(DeviceSpeed==HPRT0_PRTSPD_HIGH_SPEED)
	{
		printf("HS USB Dev!\r\n");
 	}  
	else if(DeviceSpeed==HPRT0_PRTSPD_FULL_SPEED)
	{
		printf("FS USB Dev!\r\n"); 
	}
	else if(DeviceSpeed==HPRT0_PRTSPD_LOW_SPEED)
	{
		printf("LS USB Dev!\r\n");  
	}
	else
	{
		printf("Dev Err!\r\n");  
	}
}
//检测到从机的描述符
//DeviceDesc:设备描述符指针
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{ 
	USBH_DevDesc_TypeDef *hs;
	hs=DeviceDesc;   
	printf("VID: %04Xh\r\n" , (uint32_t)(*hs).idVendor); 
	printf("PID: %04Xh\r\n" , (uint32_t)(*hs).idProduct); 
}
//从机地址分配成功
void USBH_USR_DeviceAddressAssigned(void)
{
	printf("Slave Dev Add Distribution success!\r\n");   
}
//配置描述符获有效
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{
	USBH_InterfaceDesc_TypeDef *id; 
	id = itfDesc;   
	if((*id).bInterfaceClass==0x08)
	{
		printf("Mobile storage device!\r\n"); 
	}else if((*id).bInterfaceClass==0x03)
	{
		printf("HID USB Dev!\r\n"); 
	}    
}
//获取到设备Manufacturer String
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
	printf("Manufacturer: %s\r\n",(char *)ManufacturerString);
}
//获取到设备Product String 
void USBH_USR_Product_String(void *ProductString)
{
	printf("Product: %s\r\n",(char *)ProductString);  
}
//获取到设备SerialNum String 
void USBH_USR_SerialNum_String(void *SerialNumString)
{
	printf("Serial Number: %s\r\n",(char *)SerialNumString);    
} 
//设备USB枚举完成
void USBH_USR_EnumerationDone(void)
{ 
	printf("USB Dev enum OK!\r\n\r\n");    
} 
//无法识别的USB设备
void USBH_USR_DeviceNotSupported(void)
{
	USBState  = USB_UNAVAILABLE;			//USB状态不可用
	printf("unknown USB Dev \r\n\r\n");    
}  
//等待用户输入按键,执行下一步操作
USBH_USR_Status USBH_USR_UserInput(void)
{ 
//	printf("跳过用户确认步骤!\r\n");
	return USBH_USR_RESP_OK;
} 
//USB接口电流过载
void USBH_USR_OverCurrentDetected (void)
{
	USBState  = USB_UNAVAILABLE;			//USB状态不可用
	printf("端口电流过大!!!\r\n");
} 

//extern u8 USH_User_App(void);		//用户测试主程序
//USB HOST MSC类用户应用程序
int USBH_USR_MSC_Application(void)
{
	static INT8U mountusb = TRUE;
  	switch(AppState)
  	{
    	case USH_USR_FS_INIT://初始化文件系统 
			AppState=USH_USR_FS_TEST;
      		break;
    	case USH_USR_FS_TEST:								//执行USB OTG 测试主程序
				while (HCD_IsDeviceConnected(&USB_OTG_Core))          
				{          
					if (mountusb)                                                
					{
						if (f_mount(&USBFatFs,USBPATH, 1) != FR_OK) //挂在文件系统
						{
								mountusb = TRUE;
								printf("USB Ok Fatfs Fail");
								USBState  = USB_UNAVAILABLE;			//USB状态不可用
								break;
						}
						
						mountusb = FALSE;
						printf("USB Fatfs All Ok!");
						USBState  = USB_AVAILABLE;						//只有在这里USB状态可用
						OSTimeDlyHMSM(0, 0, 0, 500); 
					}
					OSTimeDlyHMSM(0, 0, 0, 500);
				}		
				AppState = USH_USR_FS_INIT;
				f_mount(0, USBPATH, 1);                                       //卸载usb
				mountusb = TRUE;
				break;
    	default:
				break;
  	} 
	return 0;
}
//用户要求重新初始化设备
void USBH_USR_DeInit(void)
{
	USBState  = USB_UNAVAILABLE;			//USB状态不可用
  	AppState=USH_USR_FS_INIT;
}
//无法恢复的错误!!  
void USBH_USR_UnrecoveredError (void)
{
	printf("Unable to restore the error!!!\r\n\r\n");	
}
////////////////////////////////////////////////////////////////////////////////////////
//用户定义函数,实现fatfs diskio的接口函数 
extern USBH_HOST              USB_Host;

//获取U盘状态
//返回值:0,U盘未就绪
//      1,就绪
u8 USBH_UDISK_Status(void)
{
	return HCD_IsDeviceConnected(&USB_OTG_Core);//返回U盘状态
}

//读U盘
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;		 
u8 USBH_UDISK_Read(u8* buf,u32 sector,u32 cnt)
{
	u8 res=1;
	if(HCD_IsDeviceConnected(&USB_OTG_Core)&&AppState==USH_USR_FS_TEST)//连接还存在,且是APP测试状态
	{  		    
		do
		{
			res=USBH_MSC_Read10(&USB_OTG_Core,buf,sector,512*cnt);
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);		      
			if(!HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				res=1;//读写错误
				break;
			};   
		}while(res==USBH_MSC_BUSY);
	}else res=1;		  
	if(res==USBH_MSC_OK)res=0;	
	return res;
}

//写U盘
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数	
//返回值:错误状态;0,正常;其他,错误代码;		 
u8 USBH_UDISK_Write(u8* buf,u32 sector,u32 cnt)
{
	u8 res=1;
	if(HCD_IsDeviceConnected(&USB_OTG_Core)&&AppState==USH_USR_FS_TEST)//连接还存在,且是APP测试状态
	{  		    
		do
		{
			res=USBH_MSC_Write10(&USB_OTG_Core,buf,sector,512*cnt); 
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);		      
			if(!HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				res=1;//读写错误
				break;
			};   
		}while(res==USBH_MSC_BUSY);
	}else res=1;		  
	if(res==USBH_MSC_OK)res=0;	
	return res;
}

INT8U Bsp_GetUSBState(void)
{
	if(HCD_IsDeviceConnected(&USB_OTG_Core) && (USBState  == USB_AVAILABLE) )
	{
		return TRUE;
	}
	return FALSE;
}









