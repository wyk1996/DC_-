//******************************************************************************
//*File name:           bsp_SST25VF016.c                                       *
//*Descriptions:        SPI�µ�SST25VF016B����������                           *
//*Created date:        2011-03-29                                             *
//*Modified date:       2011-03-29                                             *
//*Version:             1.0                                                    *
//*note:                SST25VFXX д����������ʹ��д״̬��ͬʱ��д������λ����Ҫ
//*                     �ڲ���״̬����������4K,32K,64K��ʽ����������д�����轫Ҫ
//*                     �õ�ַ���ڵ�4K���ݶ�������ŵ�һ�����飬�޸������Ӧ����
//*                     �ݣ�Ȼ������洢���ڵĸ�4K���ݣ�����������д��洢��       
//******************************************************************************
#include "bsp_SST25VF016.h"


// u8 SST25V_ByteRead(u32 ReadAddr);
// void SST25V_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

u8 SST25V_HighSpeedRead(u32 ReadAddr);
void SST25V_HighSpeedBufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

u8 SPI_Flash_SendByte(u8 byte);
u8 SPI_Flash_ReceiveByte(void);
void SST25V_ByteWrite(u8 Byte, u32 WriteAddr);
void AutoAddressIncrement_WordProgramA(u8 Byte1, u8 Byte2, u32 Addr);
void AutoAddressIncrement_WordProgramB(u8 state,u8 Byte1, u8 Byte2) ;

void SST25V_Wait_Busy_AAI(void);
void SST25V_SectorErase_4KByte(u32 Addr);
void SST25V_BlockErase_32KByte(u32 Addr);
void SST25V_BlockErase_64KByte(u32 Addr);
void SST25V_ChipErase(void);

u8 SST25V_ReadStatusRegister(void);
void SST25V_WriteEnable(void);
void SST25V_WriteDisable(void);

void SST25V_EnableWriteStatusRegister(void);
void SST25V_WriteStatusRegister(u8 Byte);
void SST25V_WaitForWriteEnd(void);

u32 SST25V_ReadJedecID(void);

u16 SST25V_ReadManuID_DeviceID(u32 ReadManu_DeviceID_Addr);

void SST25V_EBSY(void);
void SST25V_DBSY(void);

/******************************************************************************
** ��������: SST25V_Init(void)						
** ��������: SST25 ��ʼ��
** ��ڲ���: ��		
** ���ڲ���: ��		
** ��    ע: �� 
*******************************************************************************/
void SST25V_Init(void)
{
    bsp_InitSPI2();
    SST25_CS_HIGH();
    SST25V_EnableWriteStatusRegister();
    SST25V_WriteStatusRegister(0x02);
    SST25V_DBSY();
}


/******************************************************************************
** ��������: SPI_FLASH_SendByte(u8 byte)					
** ��������: ����1���ֽ����ݣ����ؽ��յ�������		
** ��ڲ���: byte	
** ���ڲ���: byte	
** ��    ע: �� 
*******************************************************************************/
static u8 SPI_FLASH_SendByte(u8 byte)
{
    while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI_FLASH_SPI, byte);
    while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI_FLASH_SPI);
}
/******************************************************************************
** ��������: SPI_FLASH_ReadByte(void)					
** ��������: ����SPI����	
** ��ڲ���: ��		
** ���ڲ���: byte ���ؽ��յ�����
** ��    ע: �� 
*******************************************************************************/
static u8 SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(SST25_Dummy_Byte));
}
/************************************************************************
** ��������: SST25V_ReadStatusRegister(void)	
** ��������: SST25VF016B ��ȡ״̬�Ĵ���
** ��ڲ���: ��
** ���ڲ���: ״̬�Ĵ�������
** ע	 ��: 
************************************************************************/
u8 SST25V_ReadStatusRegister(void)
{
  u8 StatusRegister = 0;
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_ReadStatusReg);
  StatusRegister = SPI_FLASH_ReadByte();
  SST25_CS_HIGH();
  return StatusRegister;
}
/************************************************************************
** ��������: SST25V_WriteEnable(void)
** ��������: SST25VF016B дʹ��
** ��ڲ���: ��
** ���ڲ���: ��
** ע	 ��: 
************************************************************************/
void SST25V_WriteEnable(void)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_WriteEnable);
  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SST25V_WriteDisable(void)
** ��������: SST25VF016B ��ֹдʹ��
** ��ڲ���: ��
** ���ڲ���: ��
** ע	 ��: 
************************************************************************/
void SST25V_WriteDisable(void)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_WriteDisable);
  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SST25V_EnableWriteStatusRegister(void)
** ��������: SST25VF016B ʹ��д״̬�Ĵ���
** ��ڲ���: ��
** ���ڲ���: ��
** ע	 ��: 
************************************************************************/
void SST25V_EnableWriteStatusRegister(void)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_EnableWriteStatusReg);
  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SST25V_WriteStatusRegister(u8 Byte)
** ��������: SST25VF016B д״̬�Ĵ���ָ��
** ��ڲ���: u8 Byte :ָ��
** ���ڲ���: ��
** ע	 ��: 
************************************************************************/
void SST25V_WriteStatusRegister(u8 Byte)
{
  SST25V_EnableWriteStatusRegister();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_WriteStatusReg);
  SPI_FLASH_SendByte(Byte);
  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SST25V_WaitForWriteEnd(void)
** ��������: SST25VF016B �ȴ�����д��
** ��ڲ���: ��
** ���ڲ���: ��
** ע	 ��: 
************************************************************************/
void SST25V_WaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_ReadStatusReg);
  do
  {
    FLASH_Status = SPI_FLASH_SendByte(SST25_Dummy_Byte);

  } while((FLASH_Status & SST25_WriteStatusReg) == SET);

  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SST25V_ByteRead		
** ��������: SST25VF016B�Ķ�ȡ1���ֽ�����(����)			
** ��ڲ���:
**			 u32 ReadAddr��Ŀ���ַ,
             ��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��		
** ���ڲ���: ���ض�ȡ������	
** ��    ע: 
************************************************************************/
u8 SST25V_ByteRead(u32 ReadAddr)
{
  u8 Temp = 0;
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_ReadData);
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  SPI_FLASH_SendByte(ReadAddr & 0xFF);  
  Temp = SPI_FLASH_ReadByte();
  SST25_CS_HIGH();
  return Temp;
}
/************************************************************************
** ��������: SST25V_BufferRead		
** ��������: SST25VF016B�Ķ�ȡN���ֽ�����(����)			
** ��ڲ���:
**			 u32 ReadAddr��Ŀ���ַ,
             ��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**           pBuffer:  ������ݵ�����
**           NumByteToRead : ���ݸ���
** ���ڲ���: ���ض�ȡ������	
** ��    ע: 
************************************************************************/
void SST25V_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_ReadData);
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  SPI_FLASH_SendByte(ReadAddr & 0xFF);

  while(NumByteToRead--)
  {
    *pBuffer = SPI_FLASH_ReadByte();
    pBuffer++;
  }
  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SST25V_HighSpeedRead	
** ��������: SST25VF016B�Ķ����������ٶ�ȡ1�ֽ�		
** ��ڲ���:
**			 u32 ReadAddr��Ŀ���ַ,
**			 u8* pBuffer:���ջ����ָ��			
** ���ڲ���: 	
** ��    ע: 
************************************************************************/
u8 SST25V_HighSpeedRead(u32 ReadAddr)
{
  u32 Temp = 0;
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_FastReadData);
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
  SPI_FLASH_SendByte(SST25_Dummy_Byte);
  Temp = SPI_FLASH_ReadByte();
  SST25_CS_HIGH();
  return Temp;
}
/************************************************************************
** ��������: SST25V_HighSpeedBufferRead		
** ��������: SST25VF016B�Ķ����������ٶ�ȡN�ֽ�		
** ��ڲ���:
**			 u32 ReadAddr��Ŀ���ַ,
             ��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**      	 u16 NumByteToRead:	Ҫ��ȡ�������ֽ���
**			 u8* pBuffer:���ջ����ָ��			
** ���ڲ���: 	
** ��    ע: 
************************************************************************/
void SST25V_HighSpeedBufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_FastReadData);
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
  SPI_FLASH_SendByte(SST25_Dummy_Byte);

  while(NumByteToRead--)
  {
    *pBuffer = SPI_FLASH_ReadByte();
    pBuffer++;
  }
  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SST25V_Wait_Busy_AAI(void) 				
** ��������: SST25VF016B ��ѯAAIæ��
** ��ڲ���: ��
** ���ڲ���: ��	
** ע	 ��: 
************************************************************************/
void SST25V_Wait_Busy_AAI(void) 
{ 
  while (SST25V_ReadStatusRegister() == 0x43) /* ???? */
  SST25V_ReadStatusRegister(); 
}
/************************************************************************
** ��������: SST25V_ByteWrite				
** ��������: SST25VF016B��д��������д1�����ݵ�ָ����ַ	
** ��ڲ���:
**			 u32 WriteAddr��Ŀ���ַ,��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**			 u8 Byte:��������
** ���ڲ���: 	
** ע	 ��: 
************************************************************************/
void SST25V_ByteWrite(u8 Byte, u32 WriteAddr)
{
  SST25V_WriteEnable();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_ByteProgram);
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  SPI_FLASH_SendByte(WriteAddr & 0xFF);
  
  SPI_FLASH_SendByte(Byte);
  SST25_CS_HIGH();
  SST25V_WaitForWriteEnd();
}
/************************************************************************
** ��������: SST25V_ByteWrite				
** ��������: SST25VF016B��д��������д2�����ݵ�ָ����ַ (AAIд) ����ַ��д
**           �� ��һ����д��ַ���Ժ����2��2�����ݵĳ���д
** ��ڲ���:
**			 u32 WriteAddr��Ŀ���ַ,��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**			 u8 Byte1, u8 Byte2:���͵�2������
** ���ڲ���: 	
** ע	 ��: 
************************************************************************/
void AutoAddressIncrement_WordProgramA(u8 Byte1, u8 Byte2, u32 Addr)
{
  SST25V_WriteEnable();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_AAI_WordProgram);
  SPI_FLASH_SendByte((Addr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((Addr & 0xFF00) >> 8);
  SPI_FLASH_SendByte(Addr & 0xFF);

  SPI_FLASH_SendByte(Byte1);
  SPI_FLASH_SendByte(Byte2);

  SST25_CS_HIGH();
  SST25V_Wait_Busy_AAI();
  //SPI_FLASH_WaitForWriteEnd();
}
/************************************************************************
** ��������: SST25V_ByteWrite				
** ��������: SST25VF016B��д��������д2�����ݵ�ָ����ַ (AAIд) ������ַ��д
**           �� ��һ����д��ַ���Ժ����2��2�����ݵĳ���д
** ��ڲ���:
**			 u32 WriteAddr��Ŀ���ַ,��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**			 u8 Byte1, u8 Byte2:���͵�2������
**           u8 state :ģʽѡ��AAI�������ݵ����2������֮����Ҫ����һ��WRDI
** ���ڲ���: 	
** ע	 ��: 
************************************************************************/
void AutoAddressIncrement_WordProgramB(u8 state,u8 Byte1, u8 Byte2) 
{ 
  SST25V_WriteEnable();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_AAI_WordProgram);

  SPI_FLASH_SendByte(Byte1);
  SPI_FLASH_SendByte(Byte2);

  SST25_CS_HIGH();
  SST25V_Wait_Busy_AAI();
  
  if(state==1)
  {
    SST25V_WriteDisable();
  }
  SST25V_Wait_Busy_AAI();
}
/************************************************************************
** ��������: SST25V_SectorErase_4KByte				
** ��������: SST25VF016B �����õ�ַ���ڵ�4K�洢��������
** ��ڲ���: u32 Addr : ��ַ
** ���ڲ���: ��	
** ע	 ��: 
************************************************************************/
void SST25V_SectorErase_4KByte(u32 Addr)
{
  SST25V_WriteEnable();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_4KByte_BlockERASE);
  SPI_FLASH_SendByte((Addr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((Addr & 0xFF00) >> 8);
  SPI_FLASH_SendByte(Addr & 0xFF);
  
  SST25_CS_HIGH();
  SST25V_WaitForWriteEnd();
}
/************************************************************************
** ��������: SST25V_BlockErase_32KByte(u32 Addr)			
** ��������: SST25VF016B �����õ�ַ���ڵ�32K�洢��������
** ��ڲ���: u32 Addr : ��ַ
** ���ڲ���: ��	
** ע	 ��: 
************************************************************************/
void SST25V_BlockErase_32KByte(u32 Addr)
{
  SST25V_WriteEnable();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_32KByte_BlockErase);
  SPI_FLASH_SendByte((Addr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((Addr & 0xFF00) >> 8);
  SPI_FLASH_SendByte(Addr & 0xFF);
  
  SST25_CS_HIGH();
  SST25V_WaitForWriteEnd();
}
/************************************************************************
** ��������: SST25V_BlockErase_64KByte(u32 Addr)			
** ��������: SST25VF016B �����õ�ַ���ڵ�64K�洢��������
** ��ڲ���: u32 Addr : ��ַ
** ���ڲ���: ��	
** ע	 ��: 
************************************************************************/
void SST25V_BlockErase_64KByte(u32 Addr)
{
  SST25V_WriteEnable();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_64KByte_BlockErase);
  SPI_FLASH_SendByte((Addr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((Addr & 0xFF00) >> 8);
  SPI_FLASH_SendByte(Addr & 0xFF);
  
  SST25_CS_HIGH();
  SST25V_WaitForWriteEnd();
}
/************************************************************************
** ��������: SST25V_ChipErase(void)		
** ��������: SST25VF016B �����洢����ȫ������
** ��ڲ���: ��
** ���ڲ���: ��	
** ע	 ��: 
************************************************************************/
void SST25V_ChipErase(void)
{
  SST25V_WriteEnable();
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_ChipErase);
  SST25_CS_HIGH();
  SST25V_WaitForWriteEnd();
}
/************************************************************************
** ��������: SST25V_ReadJedecID(void)
** ��������: SST25VF016B ��ȡ�����̺�������Ϣ
** ��ڲ���: ��
** ���ڲ���: ��Ϣ����
** ע	 ��: 
************************************************************************/
u32 SST25V_ReadJedecID(void)
{
  u32 JEDECID = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_JedecDeviceID);
  Temp0 = SPI_FLASH_ReadByte();
  Temp1 = SPI_FLASH_ReadByte();
  Temp2 = SPI_FLASH_ReadByte();
  SST25_CS_HIGH();  
  JEDECID = (Temp0 << 16) | (Temp1 << 8) | Temp2;
  return JEDECID;
}
/************************************************************************
** ��������: SST25V_ReadManuID_DeviceID(u32 ReadManu_DeviceID_Addr)
** ��������: SST25VF016B ��ȡ������Ϣ
** ��ڲ���: u32 ReadManu_DeviceID_Addr ������ַ
** ���ڲ���: u16  ����ID
** ע	 ��: 
************************************************************************/
u16 SST25V_ReadManuID_DeviceID(u32 ReadManu_DeviceID_Addr)
{
  u16 ManuID_DeviceID = 0;
  u8 ManufacturerID = 0,  DeviceID = 0;
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_ManufactDeviceID);
  
  SPI_FLASH_SendByte((ReadManu_DeviceID_Addr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((ReadManu_DeviceID_Addr & 0xFF00) >> 8);
  SPI_FLASH_SendByte(ReadManu_DeviceID_Addr & 0xFF);
  
  if(ReadManu_DeviceID_Addr==1)
  {
    DeviceID = SPI_FLASH_ReadByte();
    ManufacturerID = SPI_FLASH_ReadByte();
  }
  else
  {
    ManufacturerID = SPI_FLASH_ReadByte();
    DeviceID = SPI_FLASH_ReadByte();
  }
  
  ManuID_DeviceID = ((ManufacturerID<<8) | DeviceID);
  SST25_CS_HIGH();
  
  return ManuID_DeviceID;
}
/************************************************************************
** ��������: SST25V_EBSY()
** ��������: SST25VF016B ����EBSY
** ��ڲ���: ��
** ���ڲ���: ��
** ע	 ��: 
************************************************************************/
void SST25V_EBSY()
{ 
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_EBSY);
  SST25_CS_HIGH();
} 
/************************************************************************
** ��������: SST25V_EBSY()
** ��������: SST25VF016B ����DBSY
** ��ڲ���: ��
** ���ڲ���: ��
** ע	 ��: 
************************************************************************/
void SST25V_DBSY()
{ 
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_DBSY);
  SST25_CS_HIGH();
}
/************************************************************************
** ��������: SSTF016B_WriteSingleBlock		
** ��������: ��1��������д�������
** ��ڲ���:
**			 unsigned long W_address��Ŀ���ַ,
**                ��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**			 unsigned char* SndbufPt:Ҫ���͵�����
**           unsigned int Count:�������ݸ���
** ���ڲ���: 	
** ע	 ��: 
************************************************************************/
void SSTF016B_WriteOneByte(unsigned long W_address, unsigned char byte)
{
    unsigned long Dst;
    unsigned long i;
    unsigned char SST25VFTemp[4096]={0,0,0,0,0,0,0,0,0,};
    Dst = W_address / 4096;    //�����������ڵ���������ʼ��ַ
    Dst = Dst * 4096;
    SST25V_BufferRead(SST25VFTemp,Dst,4096); //�Ƚ��������ڵ����ݶ���
    SST25V_SectorErase_4KByte(Dst);          //����������
    SST25VFTemp[W_address % 4096] = byte;   
//     for(i = 0;i < 4096;i ++)      //��ֵ����
// 	{
//         SST25V_ByteWrite(SST25VFTemp[i],Dst + i);
// 	}	
    AutoAddressIncrement_WordProgramA(SST25VFTemp[0],SST25VFTemp[1], Dst);    
    for(i = 2;i < 4094;i += 2)      //��ֵ����
	{
        AutoAddressIncrement_WordProgramB(0,SST25VFTemp[i],SST25VFTemp[i + 1]);
	}	
    AutoAddressIncrement_WordProgramB(1,SST25VFTemp[4094],SST25VFTemp[4095]);	
}

  unsigned char SST25VFTemp[4096]={0,0,0,0,0,0,0,0,0,};
/************************************************************************
** ��������: SSTF016B_WriteSingleBlock		
** ��������: ��1��������д�������
** ��ڲ���:
**			 unsigned long W_address��Ŀ���ַ,
**                ��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**			 unsigned char* SndbufPt:Ҫ���͵�����
**           unsigned int Count:�������ݸ���
** ���ڲ���: 	
** ע	 ��: 
************************************************************************/
void SSTF016B_WriteSingleBlock(unsigned long W_address, unsigned char* SndbufPt, unsigned int Count)
{
    unsigned long Sector ;
    unsigned long i = 0;
    unsigned long Dst = 0;

    unsigned int  m;   // �����ڸ�������ʣ�������
        
    m      = W_address % 4096;
    Sector = W_address / 4096;    //  ����512,������
    Dst = Sector * 4096;//����оƬ������ַ
    SST25V_BufferRead(SST25VFTemp,Dst,4096); //�Ƚ��������ڵ����ݶ���
    SST25V_SectorErase_4KByte(Dst);          //����������
    for(i = 0;i < Count;i ++)      //��ֵ����
	{
        SST25VFTemp[i + m] = SndbufPt[i];
	}
    AutoAddressIncrement_WordProgramA(SST25VFTemp[0],SST25VFTemp[1], Dst);    
    for(i = 2;i < 4094;i += 2)      //��ֵ����
	{
        AutoAddressIncrement_WordProgramB(0,SST25VFTemp[i],SST25VFTemp[i + 1]);
	}	
    AutoAddressIncrement_WordProgramB(1,SST25VFTemp[4094],SST25VFTemp[4095]); 		
}
/************************************************************************
** ��������: SSTF016B_WriteMultiBlock	
** ��������: �����ڵ�2��������д�������
** ��ڲ���:
**			 unsigned long W_address��Ŀ���ַ,
**                ��Χ 0x0 - SST25_MAX_ADDR��SST25_MAX_ADDR = 0x1FFFFF��
**			 unsigned char* SndbufPt:Ҫ���͵�����
**           unsigned int Count:�������ݸ���
** ���ڲ���: 	
** ע	 ��: 
************************************************************************/
void SSTF016B_WriteMultiBlock(unsigned long pW_address, unsigned char* pSndbufPt,unsigned int pCount)
{
    unsigned long Sector ;
    unsigned long Dst = 0;
    unsigned int  m;   // �����ڸ�������ʣ�������
        
    m      = pW_address % 4096;
    Sector = pW_address / 4096;    //  ����512,������
    
    if( (4096 - m) >= pCount)
    {
        SSTF016B_WriteSingleBlock(pW_address,pSndbufPt, pCount);
    }
    else
    {
        SSTF016B_WriteSingleBlock(pW_address,pSndbufPt, 4096 - m);
        Dst = Sector * 4096 + 4096;            
        SSTF016B_WriteSingleBlock(Dst,&pSndbufPt[4096 - m], pCount - ( 4096 - m));
	
    }	
}


