/**
******************************************************************************
* @file    			w5500_conf.c                                                                                              
* @version 			V1.2										  		
* @date    			2017-11-01								 
* @brief 				��3.5.0�汾�⽨�Ĺ���ģ��						  	
* @attention		����MCU����ֲw5500������Ҫ�޸ĵ��ļ�������w5500��MAC��IP��ַ 	
*				
* @company  		��������Ƽ����޹�˾
* @information  WIZnet W5500�ٷ������̣�ȫ�̼���֧�֣��۸�������ƣ�
* @website  		www.wisioe.com																							 	 				
* @forum        www.w5500.cn																								 		
* @qqGroup      383035001																										 
******************************************************************************
*/

#include <stdio.h> 
#include <string.h>
#include "w5500_conf.h"
//#include "bsp_i2c_ee.h"
//#include "bsp_spi_flash.h"
#include "utility.h"
#include "w5500.h"
#include "dhcp.h"
#include "UCOS_II.h"
#include "sysConfig.h"
#include "bsp_spi.h" 
//#include "bsp_TiMbase.h"

CONFIG_MSG  ConfigMsg;																	/*���ýṹ��*/
EEPROM_MSG_STR EEPROM_MSG;															/*EEPROM�洢��Ϣ�ṹ��*/

/*MAC��ַ���ֽڱ���Ϊż��*/
/*������W5500�����������ͬһ�ֳ���������ʹ�ò�ͬ��MAC��ַ*/
uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};

/*����Ĭ��IP��Ϣ*/
uint8 local_ip[4]={192,168,6,170};												/*����W5500Ĭ��IP��ַ*/
uint8 subnet[4]={255,255,255,0};												/*����W5500Ĭ����������*/
uint8 gateway[4]={192,168,6,1};													/*����W5500Ĭ������*/
uint8 dns_server[4]={114,114,114,114};									/*����W5500Ĭ��DNS*/

uint16 local_port=8000;	                    						/*���屾�ض˿�*/

/*����Զ��IP��Ϣ*/
//uint8  remote_ip[4]={122,114,122,174};										/*Զ��IP��ַ*/
//uint16 remote_port=41625;																/*Զ�˶˿ں�*/

uint8  remote_ip[4]={192,168,6,189};										/*Զ��IP��ַ*/
uint16 remote_port=5001;	
/*IP���÷���ѡ��������ѡ��*/
uint8  ip_from=IP_FROM_DEFINE;				

uint8  dhcp_ok=0;																				/*dhcp�ɹ���ȡIP*/
uint32 ms=0;																						/*�������*/
uint32 dhcp_time= 0;																		/*DHCP���м���*/
vu8    ntptimer = 0;																		/*NPT�����*/

/**
*@brief		����W5500��IP��ַ
*@param		��
*@return	��
*/
void set_w5500_ip(void)
{	
		
  /*���ƶ����������Ϣ�����ýṹ��*/
	memcpy(ConfigMsg.mac, mac, 6);
	memcpy(ConfigMsg.lip,local_ip,4);
	memcpy(ConfigMsg.sub,subnet,4);
	memcpy(ConfigMsg.gw,gateway,4);
	memcpy(ConfigMsg.dns,dns_server,4);
	if(ip_from==IP_FROM_DEFINE)	
		printf(" ʹ�ö����IP��Ϣ����W5500:\r\n");
	
//	/*ʹ��EEPROM�洢��IP����*/	
//	if(ip_from==IP_FROM_EEPROM)
//	{
//		/*��EEPROM�ж�ȡIP������Ϣ*/
//		read_config_from_eeprom();		
//		
//		/*�����ȡEEPROM��MAC��Ϣ,��������ã����ʹ��*/		
//		if( *(EEPROM_MSG.mac)==0x00&& *(EEPROM_MSG.mac+1)==0x08&&*(EEPROM_MSG.mac+2)==0xdc)		
//		{
//			printf(" IP from EEPROM\r\n");
//			/*����EEPROM������Ϣ�����õĽṹ�����*/
//			memcpy(ConfigMsg.lip,EEPROM_MSG.lip, 4);				
//			memcpy(ConfigMsg.sub,EEPROM_MSG.sub, 4);
//			memcpy(ConfigMsg.gw, EEPROM_MSG.gw, 4);
//		}
//		else
//		{
//			printf(" EEPROMδ����,ʹ�ö����IP��Ϣ����W5500,��д��EEPROM\r\n");
//			write_config_to_eeprom();	/*ʹ��Ĭ�ϵ�IP��Ϣ������ʼ��EEPROM������*/
//		}			
//	}

//	/*ʹ��DHCP��ȡIP�����������DHCP�Ӻ���*/		
//	if(ip_from==IP_FROM_DHCP)								
//	{
//		/*����DHCP��ȡ��������Ϣ�����ýṹ��*/
//		if(dhcp_ok==1)
//		{
//			printf(" IP from DHCP\r\n");		 
//			memcpy(ConfigMsg.lip,DHCP_GET.lip, 4);
//			memcpy(ConfigMsg.sub,DHCP_GET.sub, 4);
//			memcpy(ConfigMsg.gw,DHCP_GET.gw, 4);
//			memcpy(ConfigMsg.dns,DHCP_GET.dns,4);
//		}
//		else
//		{
//			printf(" DHCP�ӳ���δ����,���߲��ɹ�\r\n");
//			printf(" ʹ�ö����IP��Ϣ����W5500\r\n");
//		}
//	}
		
	/*����������Ϣ��������Ҫѡ��*/	
	ConfigMsg.sw_ver[0]=FW_VER_HIGH;
	ConfigMsg.sw_ver[1]=FW_VER_LOW;	

	/*��IP������Ϣд��W5500��Ӧ�Ĵ���*/	
	setSUBR(ConfigMsg.sub);
	setGAR(ConfigMsg.gw);
	setSIPR(ConfigMsg.lip);
	
	getSIPR (local_ip);			
	printf(" W5500 IP��ַ   : %d.%d.%d.%d\r\n", local_ip[0],local_ip[1],local_ip[2],local_ip[3]);
	getSUBR(subnet);
	printf(" W5500 �������� : %d.%d.%d.%d\r\n", subnet[0],subnet[1],subnet[2],subnet[3]);
	getGAR(gateway);
	printf(" W5500 ����     : %d.%d.%d.%d\r\n", gateway[0],gateway[1],gateway[2],gateway[3]);
}

/**
*@brief		����W5500��MAC��ַ
*@param		��
*@return	��
*/
void set_w5500_mac(void)
{
	uint32 sn0;
	sn0=*(vu32*)(0x1FFF7A10);//��ȡSTM32��ΨһID��ǰ24λ��ΪMAC��ַ�����ֽ�
	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	//�����ֽ�(IEEE��֮Ϊ��֯ΨһID,OUI)��ַ�̶�Ϊ:2.0.0
	/* set MAC hardware address */
	mac[3] =  (sn0>>16);//&0XFF;//�����ֽ���STM32��ΨһID
	mac[4] =  (sn0>>8);//&0XFFF;
	mac[5] =  sn0;//&0XFF;
	memcpy(ConfigMsg.mac, mac, 6);
	setSHAR(ConfigMsg.mac);	/**/
	//memcpy(DHCP_GET.mac, mac, 6);
}

/**
*@brief		����W5500��GPIO�ӿ�
*@param		��
*@return	��
*/
void gpio_for_w5500_config(void)
{
	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	bsp_InitSPI1();		//spi��ʼ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	/*����RESET����*/	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;						 /*ѡ��Ҫ���Ƶ�GPIO����*/		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 /*������������Ϊ50MHz*/	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);							 /*���ÿ⺯������ʼ��GPIO*/
	GPIO_SetBits(GPIOC, GPIO_Pin_5);		
	/*����INT����*/	
	//�͵�ƽ��W5500���ж���Ч
	//�ߵ�ƽ�����ж�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;						 /*ѡ��Ҫ���Ƶ�GPIO����*/		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 /*������������Ϊ50MHz */		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;				 /*��������ģʽΪͨ������ģ����������*/	
	GPIO_Init(GPIOC, &GPIO_InitStructure);							 /*���ÿ⺯������ʼ��GPIO*/

}

/**
*@brief		W5500Ƭѡ�ź����ú���
*@param		val: Ϊ��0����ʾƬѡ�˿�Ϊ�ͣ�Ϊ��1����ʾƬѡ�˿�Ϊ��
*@return	��
*/
void wiz_cs(uint8_t val)
{
	if (val == LOW) 
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_4); 
	}
	else if (val == HIGH)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_4); 
	}

	
}

/**
*@brief		����W5500��Ƭѡ�˿�SCSnΪ��
*@param		��
*@return	��
*/
void iinchip_csoff(void)
{
	wiz_cs(LOW);
}

/**
*@brief		����W5500��Ƭѡ�˿�SCSnΪ��
*@param		��
*@return	��
*/
void iinchip_cson(void)
{	
   wiz_cs(HIGH);
}

/**
*@brief		W5500��λ���ú���
*@param		��
*@return	��
*/
void reset_w5500(void)
{
		GPIO_SetBits(GPIOC, GPIO_Pin_5);
		OSTimeDly(SYS_DELAY_1s); 
		GPIO_ResetBits(GPIOC, GPIO_Pin_5);
		OSTimeDly(SYS_DELAY_1ms);  
		GPIO_SetBits(GPIOC, GPIO_Pin_5);
		//OSTimeDly(SYS_DELAY_5ms);
	
}

/**
*@brief		STM32 SPI1��д8λ����
*@param		dat��д���8λ����
*@return	��
*/
uint8  IINCHIP_SpiSendData(uint8 dat)
{
	uint8 status;
	return (SPI1_SendRcvByte(dat,&status));
//	return(SPI_FLASH_SendByte(dat));
}

/**
*@brief		д��һ��8λ���ݵ�W5500
*@param		addrbsb: д�����ݵĵ�ַ
*@param   data��д���8λ����
*@return	��
*/
void IINCHIP_WRITE( uint32 addrbsb,  uint8 data)
{
	iinchip_csoff();                              		
	IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);	
	IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
	IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4);  
	IINCHIP_SpiSendData(data);                   
	iinchip_cson();                            
}

/**
*@brief		��W5500����һ��8λ����
*@param		addrbsb: д�����ݵĵ�ַ
*@param   data����д��ĵ�ַ����ȡ����8λ����
*@return	��
*/
uint8 IINCHIP_READ(uint32 addrbsb)
{
	uint8 data = 0;
	iinchip_csoff();                            
	IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
	IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
	IINCHIP_SpiSendData( (addrbsb & 0x000000F8))    ;
	data = IINCHIP_SpiSendData(0x00);            
	iinchip_cson();                               
	return data;    
}

/**
*@brief		��W5500д��len�ֽ�����
*@param		addrbsb: д�����ݵĵ�ַ
*@param   buf��д���ַ���
*@param   len���ַ�������
*@return	len�������ַ�������
*/
uint16 wiz_write_buf(uint32 addrbsb,uint8* buf,uint16 len)
{
	uint16 idx = 0;
	if(len == 0) printf(" Unexpected2 length 0\r\n");
	iinchip_csoff();                               
	IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
	IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
	IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4); 
	for(idx = 0; idx < len; idx++)
	{
		IINCHIP_SpiSendData(buf[idx]);
	}
	iinchip_cson();                           
	return len;  
}

/**
*@brief		��W5500����len�ֽ�����
*@param		addrbsb: ��ȡ���ݵĵ�ַ
*@param 	buf����Ŷ�ȡ����
*@param		len���ַ�������
*@return	len�������ַ�������
*/
uint16 wiz_read_buf(uint32 addrbsb, uint8* buf,uint16 len)
{
  uint16 idx = 0;
  if(len == 0)
  {
    printf(" Unexpected2 length 0\r\n");
  }
  iinchip_csoff();                                
  IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
  IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
  IINCHIP_SpiSendData( (addrbsb & 0x000000F8));    
  for(idx = 0; idx < len; idx++)                   
  {
    buf[idx] = IINCHIP_SpiSendData(0x00);
  }
  iinchip_cson();                                  
  return len;
}

/**
*@brief		д������Ϣ��EEPROM
*@param		��
*@return	��
*/
//void write_config_to_eeprom(void)
//{
//	uint16 dAddr=0;
//	ee_WriteBytes(ConfigMsg.mac,dAddr,(uint8)EEPROM_MSG_LEN);				
//	delay_ms(10);																							
//}

///**
//*@brief		��EEPROM��������Ϣ
//*@param		��
//*@return	��
//*/
//void read_config_from_eeprom(void)
//{
//	ee_ReadBytes(EEPROM_MSG.mac,0,EEPROM_MSG_LEN);
//	delay_us(10);
//}

/**
*@brief		STM32��ʱ��2��ʼ��
*@param		��
*@return	��
*/
//void timer2_init(void)
//{
//	TIM2_Configuration();																		/* TIM2 ��ʱ���� */
//	TIM2_NVIC_Configuration();															/* ��ʱ�����ж����ȼ� */
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);		/* TIM2 ���¿�ʱ�ӣ���ʼ��ʱ */
//}

///**
//*@brief		dhcp�õ��Ķ�ʱ����ʼ��
//*@param		��
//*@return	��
//*/
//void dhcp_timer_init(void)
//{
//  timer2_init();																	
//}

///**
//*@brief		ntp�õ��Ķ�ʱ����ʼ��
//*@param		��
//*@return	��
//*/
//void ntp_timer_init(void)
//{
//  timer2_init();																	
//}

///**
//*@brief		��ʱ��2�жϺ���
//*@param		��
//*@return	��
//*/
//void timer2_isr(void)
//{
//  ms++;	
//  if(ms>=1000)
//  {  
//    ms=0;
//    dhcp_time++;			/*DHCP��ʱ��1S*/
//	#ifndef	__NTP_H__
//	ntptimer++;				/*NTP����ʱ���1S*/
//	#endif
//  }

//}
