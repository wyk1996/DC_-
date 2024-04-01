//******************************************************************************
//*File name:           bsp_SST25VF016.c                                       *
//*Descriptions:        SPI下的SST25VF016B操作函数库                           *
//*Created date:        2011-03-29                                             *
//*Modified date:       2011-03-29                                             *
//*Version:             1.0                                                    *
//*note:                SST25VFXX 写操作必须先使能写状态，同时被写的数据位置需要
//*                     在擦除状态，而擦除是4K,32K,64K方式擦除，所以写数据需将要
//*                     该地址所在的4K数据读出，存放到一个数组，修改数组对应的数
//*                     据，然后擦除存储器内的该4K数据，将数组重新写入存储器       
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
** 函数名称: SST25V_Init(void)						
** 函数功能: SST25 初始化
** 入口参数: 无		
** 出口参数: 无		
** 备    注: 无 
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
** 函数名称: SPI_FLASH_SendByte(u8 byte)					
** 函数功能: 发送1个字节数据，返回接收到的数据		
** 入口参数: byte	
** 出口参数: byte	
** 备    注: 无 
*******************************************************************************/
static u8 SPI_FLASH_SendByte(u8 byte)
{
    while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI_FLASH_SPI, byte);
    while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI_FLASH_SPI);
}
/******************************************************************************
** 函数名称: SPI_FLASH_ReadByte(void)					
** 函数功能: 接收SPI数据	
** 入口参数: 无		
** 出口参数: byte 返回接收的数据
** 备    注: 无 
*******************************************************************************/
static u8 SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(SST25_Dummy_Byte));
}
/************************************************************************
** 函数名称: SST25V_ReadStatusRegister(void)	
** 函数功能: SST25VF016B 读取状态寄存器
** 入口参数: 无
** 出口参数: 状态寄存器数据
** 注	 意: 
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
** 函数名称: SST25V_WriteEnable(void)
** 函数功能: SST25VF016B 写使能
** 入口参数: 无
** 出口参数: 无
** 注	 意: 
************************************************************************/
void SST25V_WriteEnable(void)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_WriteEnable);
  SST25_CS_HIGH();
}
/************************************************************************
** 函数名称: SST25V_WriteDisable(void)
** 函数功能: SST25VF016B 禁止写使能
** 入口参数: 无
** 出口参数: 无
** 注	 意: 
************************************************************************/
void SST25V_WriteDisable(void)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_WriteDisable);
  SST25_CS_HIGH();
}
/************************************************************************
** 函数名称: SST25V_EnableWriteStatusRegister(void)
** 函数功能: SST25VF016B 使能写状态寄存器
** 入口参数: 无
** 出口参数: 无
** 注	 意: 
************************************************************************/
void SST25V_EnableWriteStatusRegister(void)
{
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_EnableWriteStatusReg);
  SST25_CS_HIGH();
}
/************************************************************************
** 函数名称: SST25V_WriteStatusRegister(u8 Byte)
** 函数功能: SST25VF016B 写状态寄存器指令
** 入口参数: u8 Byte :指令
** 出口参数: 无
** 注	 意: 
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
** 函数名称: SST25V_WaitForWriteEnd(void)
** 函数功能: SST25VF016B 等待数据写完
** 入口参数: 无
** 出口参数: 无
** 注	 意: 
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
** 函数名称: SST25V_ByteRead		
** 函数功能: SST25VF016B的读取1个字节数据(低速)			
** 入口参数:
**			 u32 ReadAddr：目标地址,
             范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）		
** 出口参数: 返回读取的数据	
** 备    注: 
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
** 函数名称: SST25V_BufferRead		
** 函数功能: SST25VF016B的读取N个字节数据(低速)			
** 入口参数:
**			 u32 ReadAddr：目标地址,
             范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**           pBuffer:  存放数据的数组
**           NumByteToRead : 数据个数
** 出口参数: 返回读取的数据	
** 备    注: 
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
** 函数名称: SST25V_HighSpeedRead	
** 函数功能: SST25VF016B的读函数，高速读取1字节		
** 入口参数:
**			 u32 ReadAddr：目标地址,
**			 u8* pBuffer:接收缓存的指针			
** 出口参数: 	
** 备    注: 
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
** 函数名称: SST25V_HighSpeedBufferRead		
** 函数功能: SST25VF016B的读函数，高速读取N字节		
** 入口参数:
**			 u32 ReadAddr：目标地址,
             范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**      	 u16 NumByteToRead:	要读取的数据字节数
**			 u8* pBuffer:接收缓存的指针			
** 出口参数: 	
** 备    注: 
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
** 函数名称: SST25V_Wait_Busy_AAI(void) 				
** 函数功能: SST25VF016B 查询AAI忙？
** 入口参数: 无
** 出口参数: 无	
** 注	 意: 
************************************************************************/
void SST25V_Wait_Busy_AAI(void) 
{ 
  while (SST25V_ReadStatusRegister() == 0x43) /* ???? */
  SST25V_ReadStatusRegister(); 
}
/************************************************************************
** 函数名称: SST25V_ByteWrite				
** 函数功能: SST25VF016B的写函数，可写1个数据到指定地址	
** 入口参数:
**			 u32 WriteAddr：目标地址,范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**			 u8 Byte:发送数据
** 出口参数: 	
** 注	 意: 
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
** 函数名称: SST25V_ByteWrite				
** 函数功能: SST25VF016B的写函数，可写2个数据到指定地址 (AAI写) 带地址的写
**           即 第一次需写地址，以后就是2个2个数据的持续写
** 入口参数:
**			 u32 WriteAddr：目标地址,范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**			 u8 Byte1, u8 Byte2:发送的2个数据
** 出口参数: 	
** 注	 意: 
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
** 函数名称: SST25V_ByteWrite				
** 函数功能: SST25VF016B的写函数，可写2个数据到指定地址 (AAI写) 不带地址的写
**           即 第一次需写地址，以后就是2个2个数据的持续写
** 入口参数:
**			 u32 WriteAddr：目标地址,范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**			 u8 Byte1, u8 Byte2:发送的2个数据
**           u8 state :模式选择，AAI发送数据的最后2个数据之后需要发送一次WRDI
** 出口参数: 	
** 注	 意: 
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
** 函数名称: SST25V_SectorErase_4KByte				
** 函数功能: SST25VF016B 擦除该地址所在的4K存储器的数据
** 入口参数: u32 Addr : 地址
** 出口参数: 无	
** 注	 意: 
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
** 函数名称: SST25V_BlockErase_32KByte(u32 Addr)			
** 函数功能: SST25VF016B 擦除该地址所在的32K存储器的数据
** 入口参数: u32 Addr : 地址
** 出口参数: 无	
** 注	 意: 
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
** 函数名称: SST25V_BlockErase_64KByte(u32 Addr)			
** 函数功能: SST25VF016B 擦除该地址所在的64K存储器的数据
** 入口参数: u32 Addr : 地址
** 出口参数: 无	
** 注	 意: 
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
** 函数名称: SST25V_ChipErase(void)		
** 函数功能: SST25VF016B 擦除存储器的全部数据
** 入口参数: 无
** 出口参数: 无	
** 注	 意: 
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
** 函数名称: SST25V_ReadJedecID(void)
** 函数功能: SST25VF016B 读取制造商和器件信息
** 入口参数: 无
** 出口参数: 信息数据
** 注	 意: 
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
** 函数名称: SST25V_ReadManuID_DeviceID(u32 ReadManu_DeviceID_Addr)
** 函数功能: SST25VF016B 读取器件信息
** 入口参数: u32 ReadManu_DeviceID_Addr 器件地址
** 出口参数: u16  器件ID
** 注	 意: 
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
** 函数名称: SST25V_EBSY()
** 函数功能: SST25VF016B 发送EBSY
** 入口参数: 无
** 出口参数: 无
** 注	 意: 
************************************************************************/
void SST25V_EBSY()
{ 
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_EBSY);
  SST25_CS_HIGH();
} 
/************************************************************************
** 函数名称: SST25V_EBSY()
** 函数功能: SST25VF016B 发送DBSY
** 入口参数: 无
** 出口参数: 无
** 注	 意: 
************************************************************************/
void SST25V_DBSY()
{ 
  SST25_CS_LOW();
  SPI_FLASH_SendByte(SST25_DBSY);
  SST25_CS_HIGH();
}
/************************************************************************
** 函数名称: SSTF016B_WriteSingleBlock		
** 函数功能: 在1个扇区内写多个数据
** 入口参数:
**			 unsigned long W_address：目标地址,
**                范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**			 unsigned char* SndbufPt:要发送的数组
**           unsigned int Count:发送数据个数
** 出口参数: 	
** 注	 意: 
************************************************************************/
void SSTF016B_WriteOneByte(unsigned long W_address, unsigned char byte)
{
    unsigned long Dst;
    unsigned long i;
    unsigned char SST25VFTemp[4096]={0,0,0,0,0,0,0,0,0,};
    Dst = W_address / 4096;    //计算数据所在的扇区的起始地址
    Dst = Dst * 4096;
    SST25V_BufferRead(SST25VFTemp,Dst,4096); //先将改扇区内的数据读出
    SST25V_SectorErase_4KByte(Dst);          //擦除改扇区
    SST25VFTemp[W_address % 4096] = byte;   
//     for(i = 0;i < 4096;i ++)      //赋值数组
// 	{
//         SST25V_ByteWrite(SST25VFTemp[i],Dst + i);
// 	}	
    AutoAddressIncrement_WordProgramA(SST25VFTemp[0],SST25VFTemp[1], Dst);    
    for(i = 2;i < 4094;i += 2)      //赋值数组
	{
        AutoAddressIncrement_WordProgramB(0,SST25VFTemp[i],SST25VFTemp[i + 1]);
	}	
    AutoAddressIncrement_WordProgramB(1,SST25VFTemp[4094],SST25VFTemp[4095]);	
}

  unsigned char SST25VFTemp[4096]={0,0,0,0,0,0,0,0,0,};
/************************************************************************
** 函数名称: SSTF016B_WriteSingleBlock		
** 函数功能: 在1个扇区内写多个数据
** 入口参数:
**			 unsigned long W_address：目标地址,
**                范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**			 unsigned char* SndbufPt:要发送的数组
**           unsigned int Count:发送数据个数
** 出口参数: 	
** 注	 意: 
************************************************************************/
void SSTF016B_WriteSingleBlock(unsigned long W_address, unsigned char* SndbufPt, unsigned int Count)
{
    unsigned long Sector ;
    unsigned long i = 0;
    unsigned long Dst = 0;

    unsigned int  m;   // 数据在该扇区内剩余的数据
        
    m      = W_address % 4096;
    Sector = W_address / 4096;    //  除以512,扇区数
    Dst = Sector * 4096;//计算芯片扇区地址
    SST25V_BufferRead(SST25VFTemp,Dst,4096); //先将改扇区内的数据读出
    SST25V_SectorErase_4KByte(Dst);          //擦除改扇区
    for(i = 0;i < Count;i ++)      //赋值数组
	{
        SST25VFTemp[i + m] = SndbufPt[i];
	}
    AutoAddressIncrement_WordProgramA(SST25VFTemp[0],SST25VFTemp[1], Dst);    
    for(i = 2;i < 4094;i += 2)      //赋值数组
	{
        AutoAddressIncrement_WordProgramB(0,SST25VFTemp[i],SST25VFTemp[i + 1]);
	}	
    AutoAddressIncrement_WordProgramB(1,SST25VFTemp[4094],SST25VFTemp[4095]); 		
}
/************************************************************************
** 函数名称: SSTF016B_WriteMultiBlock	
** 函数功能: 在相邻的2个扇区内写多个数据
** 入口参数:
**			 unsigned long W_address：目标地址,
**                范围 0x0 - SST25_MAX_ADDR（SST25_MAX_ADDR = 0x1FFFFF）
**			 unsigned char* SndbufPt:要发送的数组
**           unsigned int Count:发送数据个数
** 出口参数: 	
** 注	 意: 
************************************************************************/
void SSTF016B_WriteMultiBlock(unsigned long pW_address, unsigned char* pSndbufPt,unsigned int pCount)
{
    unsigned long Sector ;
    unsigned long Dst = 0;
    unsigned int  m;   // 数据在该扇区内剩余的数据
        
    m      = pW_address % 4096;
    Sector = pW_address / 4096;    //  除以512,扇区数
    
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


