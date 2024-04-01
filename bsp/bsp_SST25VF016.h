//******************************************************************************
//*File name:           SST25VF016BDrv.h                                       *
//*Descriptions:        SPI下的SST25VF016B操作函数库                           *
//*Created date:        2011-03-29                                             *
//*Modified date:       2011-03-29                                             *
//*Version:             1.0                                                    *
//******************************************************************************

#ifndef __F016B_Driver_H 
#define __F016B_Driver_H

#include "stm32f4xx.h"
#include "bsp_spi.h" 
/* 定义操作SST25VF016B所需的数据 */
// enum PARAMETER_TYPE {Invalid};		// 定义1个常量,代表无效参数
//enum OptResult{ERROR2 = 0,OK};
//typedef enum ERTYPE{Sec1,Sec8,Sec16,Chip} ErType;  
//typedef enum IDTYPE{Manu_ID,Dev_ID,Jedec_ID} idtype;


#define  SST25_MAX_ADDR		0x1FFFFF	// 定义芯片内部最大地址 
#define	 SST25_SEC_MAX         512         // 定义最大扇区号
#define  SST25_SEC_SIZE		0x1000      // 扇区大小

#define SST25_ReadData                      0x03
#define SST25_FastReadData                  0x0B
#define SST25_4KByte_BlockERASE             0x20
#define SST25_32KByte_BlockErase            0x52
#define SST25_64KByte_BlockErase            0xD8
#define SST25_ChipErase                     0xC7

#define SST25_ByteProgram                   0x02
#define SST25_AAI_WordProgram               0xAD

#define SST25_ReadStatusReg                 0x05
#define SST25_EnableWriteStatusReg          0x50
#define SST25_WriteStatusReg                0x01
#define SST25_WriteEnable                   0x06
#define SST25_WriteDisable                  0x04
#define SST25_ManufactDeviceID              0x90
#define SST25_JedecDeviceID                 0x9F
#define SST25_EBSY                          0x70
#define SST25_DBSY                          0x80 

#define SST25_Dummy_Byte      0xFF        //哑字节



#define SST25_CS_LOW()       SPI_FLASH_CS_LOW
#define SST25_CS_HIGH()      SPI_FLASH_CS_HIGH

void SPI_FLASH_Init(void);
void SST25V_Init(void);


// u8 SST25V_ByteRead(u32 ReadAddr);
// void SST25V_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
// u8 SST25V_HighSpeedRead(u32 ReadAddr);
// void SST25V_HighSpeedBufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

// u8 SPI_Flash_SendByte(u8 byte);
// u8 SPI_Flash_ReceiveByte(void);
// void SST25V_ByteWrite(u8 Byte, u32 WriteAddr);
// void AutoAddressIncrement_WordProgramA(u8 Byte1, u8 Byte2, u32 Addr);
// void AutoAddressIncrement_WordProgramB(u8 state,u8 Byte1, u8 Byte2) ;

// void SST25V_Wait_Busy_AAI(void);
// void SST25V_SectorErase_4KByte(u32 Addr);
// void SST25V_BlockErase_32KByte(u32 Addr);
// void SST25V_BlockErase_64KByte(u32 Addr);
// void SST25V_ChipErase(void);

// u8 SST25V_ReadStatusRegister(void);
// void SST25V_WriteEnable(void);
// void SST25V_WriteDisable(void);

// void SST25V_EnableWriteStatusRegister(void);
// void SST25V_WriteStatusRegister(u8 Byte);
// void SST25V_WaitForWriteEnd(void);

// u32 SST25V_ReadJedecID(void);

// u16 SST25V_ReadManuID_DeviceID(u32 ReadManu_DeviceID_Addr);

// void SST25V_EBSY();
// void SST25V_DBSY();


//u8 SST25V_ByteRead(u32 ReadAddr);
void SST25V_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);

//void SSTF016B_WriteOneByte(unsigned long W_address, unsigned char byte);
//void SSTF016B_WriteSingleBlock(unsigned long W_address, unsigned char* SndbufPt, unsigned int Count);
void SSTF016B_WriteMultiBlock(unsigned long pW_address, unsigned char* pSndbufPt,unsigned int pCount);


#endif
