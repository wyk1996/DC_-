
#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "bsp_spi.h" 



/*******************************************************************************
* 宏定义
*******************************************************************************/
/* Private typedef -----------------------------------------------------------*/
/* 定义操作SST25VF016B所需的数据 */
typedef enum ERTYPE{Sec1,Sec8,Sec16,Chip} ErType;  
typedef enum IDTYPE{Manu_ID,Dev_ID,Jedec_ID} idtype;

/* Private macro -------------------------------------------------------------*/

#define LIMIT_BUF_SIZE      4096     /*读写测试，使用256个字节的BUFFER，这个BUFFER定义太大的话，RTT HEAP无法执行*/

//SST26VF032B 
#define MAX_ADDR            0x03FFFFF   /* 定义芯片内部最大地址 4 MByte*/
#define SEC_MAX             1024        /* 定义最大扇区号 */
#define SEC_SIZE            4096        /* 每个扇区4KByte */
#define PAGE_SIZE           256         /* 每次写入页256KByte */
#define SST26_JEDEC_ID      0xBF2642    /* JEDEC */
#define SF_TOTAL_SIZE       (256*16384) /* 总容量，4 MByte */
#define SF_PAGE_SIZE        (4096)      /* 页面大小，每个扇区4 KByte*/

#define CMD_UNLOCK          0x98        /* 所有区块全部解锁 */
#define CMD_WRPAGE          0x02        /* 页编程指令 */
#define CMD_DISWR           0x04        /* 禁止写, 退出AAI状态 */
#define CMD_WRSR            0x01        /* 写状态寄存器命令 */
#define CMD_WREN            0x06        /* 写使能命令 */
#define CMD_READ            0x03        /* 读数据区命令 */
#define CMD_READFAST        0x0B        /* 高速读数据区命令 */
#define CMD_RDSR            0x05        /* 读状态寄存器命令 */
#define CMD_RDID            0x9F        /* 读器件ID命令 */
#define CMD_SE              0x20        /* 4KB扇区擦除扇区命令 */
#define CMD_BE              0xC7        /* chip-erase擦除命令 */
#define DUMMY_BYTE          0x00        /* 哑命令，可以为任意值，用于读操作 */

#define WIP_FLAG            0x01        /* 状态寄存器中的正在编程标志（WIP) */


/*************************************************************/
/*                      变量声明                             */
/*************************************************************/


/*************************************************************/
/*                      函数声明                             */
/*************************************************************/
void SST26_Init(void);
void SST26_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
uint8_t SST26_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);


#endif


