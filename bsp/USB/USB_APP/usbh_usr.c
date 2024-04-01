#include "usbh_usr.h" 
#include "usb_hcd_int.h"
#include "ff.h"  


_USB_STATE  USBState = USB_UNAVAILABLE ;       
static FATFS USBFatFs;   //�洢USB������Ϣ
static u8 AppState; 
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
//USB OTG �жϷ�����
//USB HOST �û��ص�����.
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
//����Ϊ���ص�����ʵ��.


//USB HOST ��ʼ�� 
void USBH_USR_Init(void)
{
	printf("USB OTG HS MSC Host\r\n");
	printf("> USB Host library started.\r\n");
	printf("  USB Host Library v2.1.0\r\n\r\n");
	
}
//��⵽U�̲���
void USBH_USR_DeviceAttached(void)//U�̲���
{
	USBState  = USB_UNAVAILABLE;			//USB״̬������
	printf("USB Dev Insert!\r\n");
}
//��⵽U�̰γ�
void USBH_USR_DeviceDisconnected (void)//U���Ƴ�
{
	USBState  = USB_UNAVAILABLE;			//USB״̬������
	printf("USB Dev Pull Out!\r\n");
}  
//��λ�ӻ�
void USBH_USR_ResetDevice(void)
{
	USBState  = USB_UNAVAILABLE;			//USB״̬������
	printf("Init Dev...\r\n");
}
//��⵽�ӻ��ٶ�
//DeviceSpeed:�ӻ��ٶ�(0,1,2 / ����)
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
//��⵽�ӻ���������
//DeviceDesc:�豸������ָ��
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{ 
	USBH_DevDesc_TypeDef *hs;
	hs=DeviceDesc;   
	printf("VID: %04Xh\r\n" , (uint32_t)(*hs).idVendor); 
	printf("PID: %04Xh\r\n" , (uint32_t)(*hs).idProduct); 
}
//�ӻ���ַ����ɹ�
void USBH_USR_DeviceAddressAssigned(void)
{
	printf("Slave Dev Add Distribution success!\r\n");   
}
//��������������Ч
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
//��ȡ���豸Manufacturer String
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
	printf("Manufacturer: %s\r\n",(char *)ManufacturerString);
}
//��ȡ���豸Product String 
void USBH_USR_Product_String(void *ProductString)
{
	printf("Product: %s\r\n",(char *)ProductString);  
}
//��ȡ���豸SerialNum String 
void USBH_USR_SerialNum_String(void *SerialNumString)
{
	printf("Serial Number: %s\r\n",(char *)SerialNumString);    
} 
//�豸USBö�����
void USBH_USR_EnumerationDone(void)
{ 
	printf("USB Dev enum OK!\r\n\r\n");    
} 
//�޷�ʶ���USB�豸
void USBH_USR_DeviceNotSupported(void)
{
	USBState  = USB_UNAVAILABLE;			//USB״̬������
	printf("unknown USB Dev \r\n\r\n");    
}  
//�ȴ��û����밴��,ִ����һ������
USBH_USR_Status USBH_USR_UserInput(void)
{ 
//	printf("�����û�ȷ�ϲ���!\r\n");
	return USBH_USR_RESP_OK;
} 
//USB�ӿڵ�������
void USBH_USR_OverCurrentDetected (void)
{
	USBState  = USB_UNAVAILABLE;			//USB״̬������
	printf("�˿ڵ�������!!!\r\n");
} 

//extern u8 USH_User_App(void);		//�û�����������
//USB HOST MSC���û�Ӧ�ó���
int USBH_USR_MSC_Application(void)
{
	static INT8U mountusb = TRUE;
  	switch(AppState)
  	{
    	case USH_USR_FS_INIT://��ʼ���ļ�ϵͳ 
			AppState=USH_USR_FS_TEST;
      		break;
    	case USH_USR_FS_TEST:								//ִ��USB OTG ����������
				while (HCD_IsDeviceConnected(&USB_OTG_Core))          
				{          
					if (mountusb)                                                
					{
						if (f_mount(&USBFatFs,USBPATH, 1) != FR_OK) //�����ļ�ϵͳ
						{
								mountusb = TRUE;
								printf("USB Ok Fatfs Fail");
								USBState  = USB_UNAVAILABLE;			//USB״̬������
								break;
						}
						
						mountusb = FALSE;
						printf("USB Fatfs All Ok!");
						USBState  = USB_AVAILABLE;						//ֻ��������USB״̬����
						OSTimeDlyHMSM(0, 0, 0, 500); 
					}
					OSTimeDlyHMSM(0, 0, 0, 500);
				}		
				AppState = USH_USR_FS_INIT;
				f_mount(0, USBPATH, 1);                                       //ж��usb
				mountusb = TRUE;
				break;
    	default:
				break;
  	} 
	return 0;
}
//�û�Ҫ�����³�ʼ���豸
void USBH_USR_DeInit(void)
{
	USBState  = USB_UNAVAILABLE;			//USB״̬������
  	AppState=USH_USR_FS_INIT;
}
//�޷��ָ��Ĵ���!!  
void USBH_USR_UnrecoveredError (void)
{
	printf("Unable to restore the error!!!\r\n\r\n");	
}
////////////////////////////////////////////////////////////////////////////////////////
//�û����庯��,ʵ��fatfs diskio�Ľӿں��� 
extern USBH_HOST              USB_Host;

//��ȡU��״̬
//����ֵ:0,U��δ����
//      1,����
u8 USBH_UDISK_Status(void)
{
	return HCD_IsDeviceConnected(&USB_OTG_Core);//����U��״̬
}

//��U��
//buf:�����ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;		 
u8 USBH_UDISK_Read(u8* buf,u32 sector,u32 cnt)
{
	u8 res=1;
	if(HCD_IsDeviceConnected(&USB_OTG_Core)&&AppState==USH_USR_FS_TEST)//���ӻ�����,����APP����״̬
	{  		    
		do
		{
			res=USBH_MSC_Read10(&USB_OTG_Core,buf,sector,512*cnt);
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);		      
			if(!HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				res=1;//��д����
				break;
			};   
		}while(res==USBH_MSC_BUSY);
	}else res=1;		  
	if(res==USBH_MSC_OK)res=0;	
	return res;
}

//дU��
//buf:д���ݻ�����
//sector:������ַ
//cnt:��������	
//����ֵ:����״̬;0,����;����,�������;		 
u8 USBH_UDISK_Write(u8* buf,u32 sector,u32 cnt)
{
	u8 res=1;
	if(HCD_IsDeviceConnected(&USB_OTG_Core)&&AppState==USH_USR_FS_TEST)//���ӻ�����,����APP����״̬
	{  		    
		do
		{
			res=USBH_MSC_Write10(&USB_OTG_Core,buf,sector,512*cnt); 
			USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);		      
			if(!HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				res=1;//��д����
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









