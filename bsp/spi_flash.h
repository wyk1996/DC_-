
#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "bsp_spi.h" 



/*******************************************************************************
* �궨��
*******************************************************************************/
/* Private typedef -----------------------------------------------------------*/
/* �������SST25VF016B��������� */
typedef enum ERTYPE{Sec1,Sec8,Sec16,Chip} ErType;  
typedef enum IDTYPE{Manu_ID,Dev_ID,Jedec_ID} idtype;

/* Private macro -------------------------------------------------------------*/

#define LIMIT_BUF_SIZE      4096     /*��д���ԣ�ʹ��256���ֽڵ�BUFFER�����BUFFER����̫��Ļ���RTT HEAP�޷�ִ��*/

//SST26VF032B 
#define MAX_ADDR            0x03FFFFF   /* ����оƬ�ڲ�����ַ 4 MByte*/
#define SEC_MAX             1024        /* ������������� */
#define SEC_SIZE            4096        /* ÿ������4KByte */
#define PAGE_SIZE           256         /* ÿ��д��ҳ256KByte */
#define SST26_JEDEC_ID      0xBF2642    /* JEDEC */
#define SF_TOTAL_SIZE       (256*16384) /* ��������4 MByte */
#define SF_PAGE_SIZE        (4096)      /* ҳ���С��ÿ������4 KByte*/

#define CMD_UNLOCK          0x98        /* ��������ȫ������ */
#define CMD_WRPAGE          0x02        /* ҳ���ָ�� */
#define CMD_DISWR           0x04        /* ��ֹд, �˳�AAI״̬ */
#define CMD_WRSR            0x01        /* д״̬�Ĵ������� */
#define CMD_WREN            0x06        /* дʹ������ */
#define CMD_READ            0x03        /* ������������ */
#define CMD_READFAST        0x0B        /* ���ٶ����������� */
#define CMD_RDSR            0x05        /* ��״̬�Ĵ������� */
#define CMD_RDID            0x9F        /* ������ID���� */
#define CMD_SE              0x20        /* 4KB���������������� */
#define CMD_BE              0xC7        /* chip-erase�������� */
#define DUMMY_BYTE          0x00        /* ���������Ϊ����ֵ�����ڶ����� */

#define WIP_FLAG            0x01        /* ״̬�Ĵ����е����ڱ�̱�־��WIP) */


/*************************************************************/
/*                      ��������                             */
/*************************************************************/


/*************************************************************/
/*                      ��������                             */
/*************************************************************/
void SST26_Init(void);
void SST26_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
uint8_t SST26_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);


#endif


