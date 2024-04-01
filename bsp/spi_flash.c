

#include "spi_flash.h"
//#include "Algorithm.h"
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
* 变量定义
*******************************************************************************/
static uint8_t s_spiBuf[LIMIT_BUF_SIZE];    /* 用于写函数，先读出整个page，修改缓冲区后，再整个page回写 */
uint32_t DeviceID=0;

/*******************************************************************************
* 函数声明
*******************************************************************************/
static void SST26_WaitForWriteEnd(void);


/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Delay_ms(u32 ms)
{
    u32 len;
    
    for (;ms > 0; ms --)
        for (len = 0; len < 100; len++ );
}

/*******************************************************************************
* Function Name  : SST26_SendByte
* Description    : Sends a byte through the SPI interface and return the byte 
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
u8 SST26_SendByte(u8 byte)
{
    /* Loop while DR register in not emplty */
    while(SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);
    //SST26_CS_HIGH;  while(1);
    /* Send byte through the SPI2 peripheral */
    SPI_I2S_SendData(SPI_FLASH_SPI, byte);

    /* Wait to receive a byte */
    while(SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

    /* Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData(SPI_FLASH_SPI);
}

/*
*********************************************************************************************************
*   函 数 名: SST26_WriteEnable
*   功能说明: 向器件发送写使能命令
*   形    参：无
*   返 回 值: 无
*********************************************************************************************************
*/
static void SST26_WriteEnable(void)
{
    SPI_FLASH_CS_LOW;                         /* 使能片选 */
    SST26_SendByte(CMD_WREN);                 /* 发送命令 */
    SPI_FLASH_CS_HIGH;                        /* 禁能片选 */  

    SST26_WaitForWriteEnd();                  /* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*   函 数 名: SST26_WaitForWriteEnd
*   功能说明: 采用循环查询的方式等待器件内部写操作完成
*   形    参：无
*   返 回 值: 无
*********************************************************************************************************
*/  
static void SST26_WaitForWriteEnd(void)
{
    SPI_FLASH_CS_LOW;                                 /* 使能片选 */
    SST26_SendByte(CMD_RDSR);                           /* 发送命令， 读状态寄存器 */

    while((SST26_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET);  /* 判断状态寄存器的忙标志位 */
   
    SPI_FLASH_CS_HIGH;                                    /* 禁能片选 */  
}

/*
*********************************************************************************************************
*   函 数 名: SST26_EraseSector
*   功能说明: 擦除指定的扇区
*   形    参：_uiSectorAddr : 扇区地址
*   返 回 值: 无
*********************************************************************************************************
*/
void SST26_EraseSector(uint32_t _uiSectorAddr)
{
    SST26_WriteEnable();                                /* 发送写使能命令 */
    
    /* 擦除扇区操作 */
    SPI_FLASH_CS_LOW;                                 /* 使能片选 */
    SST26_SendByte(CMD_SE);                             /* 发送擦除命令 */
    SST26_SendByte((_uiSectorAddr & 0xFF0000) >> 16);   /* 发送扇区地址的高8bit */
    SST26_SendByte((_uiSectorAddr & 0xFF00) >> 8);      /* 发送扇区地址中间8bit */
    SST26_SendByte(_uiSectorAddr & 0xFF);               /* 发送扇区地址低8bit */
    SPI_FLASH_CS_HIGH;                                /* 禁能片选 */
    
    SST26_WaitForWriteEnd();                            /* 等待串行Flash内部写操作完成 */
}
/*
*********************************************************************************************************
*   函 数 名: SST26_EraseChip
*   功能说明: 擦除整个芯片
*   形    参：无
*   返 回 值: 无
*********************************************************************************************************
*/  
void SST26_EraseChip(void)
{
    SST26_WriteEnable();        /* 发送写使能命令 */
    
    /* 擦除扇区操作 */
    SPI_FLASH_CS_LOW;         /* 使能片选 */
    SST26_SendByte(CMD_BE);     /* 发送擦除命令 */
    SPI_FLASH_CS_HIGH;        /* 禁能片选 */
    
    SST26_WaitForWriteEnd();    /* 等待串行Flash内部写操作完成 */
}

/*
*********************************************************************************************************
*   函 数 名: sSST26_PageWrite
*   功能说明: 向一个page内写入若干字节。字节个数不能超出页面大小（4K)
*   形    参：  _pBuf : 数据源缓冲区；
*               _uiWriteAddr ：目标区域首地址
*               _usSize ：数据个数，不能超过页面大小
*   返 回 值: 无
*********************************************************************************************************
*/  
void SST26_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
    uint32_t i, num;
    
     /* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
    if (_usSize == 0)
    {
        return;
    }

    num = 0;
    
    while((num < _usSize) && (num < SEC_SIZE))
    {
        SST26_WriteEnable();                                /* 发送写使能命令 */
    
        /* 擦除扇区操作 */
        SPI_FLASH_CS_LOW;                                   /* 使能片选 */
        SST26_SendByte(CMD_WRPAGE);                         /* 发送页编程命令(256Byte) */
        SST26_SendByte((_uiWriteAddr & 0xFF0000) >> 16);    /* 发送扇区地址的高8bit */
        SST26_SendByte((_uiWriteAddr & 0xFF00) >> 8);       /* 发送扇区地址中间8bit */
        SST26_SendByte(_uiWriteAddr & 0xFF);                /* 发送扇区地址低8bit */    

        for (i = 0; i < PAGE_SIZE; i++)
        {
            if(i < _usSize)
            {
                SST26_SendByte(*_pBuf++);                   /* 发送256个字节数据 */
            }
            else
                SST26_SendByte(0);                          /* 发送256个字节数据 */
        }
        num += PAGE_SIZE;                                   /* 计入已发送256个字节 */
        _uiWriteAddr += PAGE_SIZE;
        SPI_FLASH_CS_HIGH;                                  /* 禁能片选 */
        
        SST26_WaitForWriteEnd();                            /* 等待串行Flash内部写操作完成 */
    }
}

/*
*********************************************************************************************************
*   函 数 名: SST26_ReadBuffer
*   功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*   形    参：  _pBuf : 数据源缓冲区；
*               _uiReadAddr ：首地址
*               _usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*   返 回 值: 无
*********************************************************************************************************
*/
void SST26_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
    /* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
    if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > SF_TOTAL_SIZE)
    {
        return;
    }
        
    SST26_WriteEnable();                              /* 发送写使能命令 */
    
    SPI_FLASH_CS_LOW;                                 /* 使能片选 */
    SST26_SendByte(CMD_READ);                           /* 发送读命令 */
//  SST26_SendByte(CMD_READFAST);                           /* 发送读命令 */
    
    SST26_SendByte((_uiReadAddr & 0xFF0000) >> 16); /* 发送扇区地址的高8bit */
    SST26_SendByte((_uiReadAddr & 0xFF00) >> 8);        /* 发送扇区地址中间8bit */
    SST26_SendByte(_uiReadAddr & 0xFF);             /* 发送扇区地址低8bit */    
    while (_uiSize--)
    {
        *_pBuf++ = SST26_SendByte(DUMMY_BYTE);          /* 读一个字节并存储到pBuf，读完后指针自加1 */
    }
    SPI_FLASH_CS_HIGH;                                    /* 禁能片选 */

    SST26_WaitForWriteEnd();        /* 等待串行Flash内部操作完成 */
}

/*
*********************************************************************************************************
*   函 数 名: SST26_CmpData
*   功能说明: 比较Flash的数据.
*   形    参：  _ucpTar : 数据缓冲区
*               _uiSrcAddr ：Flash地址
*               _uiSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*   返 回 值: 0 = 相等, 1 = 不等
*********************************************************************************************************
*/
static uint8_t SST26_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
    uint8_t ucValue;

    /* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
    if ((_uiSrcAddr + _uiSize) > SF_TOTAL_SIZE)
    {
        return 1;
    }

    if (_uiSize == 0)   
    {
        return 0;
    }
        
    SPI_FLASH_CS_LOW;                                 /* 使能片选 */
    SST26_SendByte(CMD_READ);                           /* 发送读命令 */

    SST26_SendByte((_uiSrcAddr & 0xFF0000) >> 16);      /* 发送扇区地址的高8bit */
    SST26_SendByte((_uiSrcAddr & 0xFF00) >> 8);     /* 发送扇区地址中间8bit */
    SST26_SendByte(_uiSrcAddr & 0xFF);                  /* 发送扇区地址低8bit */    
    while (_uiSize--)
    {
        /* 读一个字节 */
        ucValue = SST26_SendByte(DUMMY_BYTE);       
        if (*_ucpTar++ != ucValue)
        {
            SPI_FLASH_CS_HIGH;
            return 1;
        }
    }
    SPI_FLASH_CS_HIGH;    
    return 0;
}

/*
*********************************************************************************************************
*   函 数 名: SST26_NeedErase
*   功能说明: 判断写PAGE前是否需要先擦除。
*   形    参： _ucpOldBuf ： 旧数据
*              _ucpNewBuf ： 新数据
*              _uiLen ：数据个数，不能超过页面大小
*   返 回 值: 0 : 不需要擦除， 1 ：需要擦除
*********************************************************************************************************
*/ 
static uint8_t SST26_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
    uint16_t i;
    uint8_t ucOld;

    /*
    算法第1步：old 求反, new 不变
          old    new
          1101   0101
    ~     1111
        = 0010   0101

    算法第2步: old 求反的结果与 new 位与
          0010   old
    &     0101   new
         =0000

    算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
    */

    for (i = 0; i < _usLen; i++)
    {
        ucOld = *_ucpOldBuf++;
        ucOld = ~ucOld;

        /* 注意错误的写法: if (ucOld & (*_ucpNewBuf++) != 0) */
        if ((ucOld & (*_ucpNewBuf++)) != 0)
        {
            return 1;
        }
    }
    return 0;
}

/*
*********************************************************************************************************
*   函 数 名: SST26_AutoWritePage
*   功能说明: 写1个PAGE并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*   形    参：  _pBuf : 数据源缓冲区；
*               _uiWriteAddr ：目标区域首地址
*               _usSize ：数据个数，不能超过页面大小
*   返 回 值: 0 : 错误， 1 ： 成功
*********************************************************************************************************
*/ 
static uint8_t SST26_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
    uint16_t i;
    uint16_t j;                 /* 用于延时 */
    uint32_t uiFirstAddr;       /* 扇区首址 */
    uint8_t ucNeedErase;        /* 1表示需要擦除 */
    //uint8_t ucaFlashBuf[SF_PAGE_SIZE];
    uint8_t cRet;

    /* 长度为0时不继续操作,直接认为成功 */
    if (_usWrLen == 0)
    {
        return 1;
    }   
    
    /* 如果偏移地址超过芯片容量则退出 */
    if (_uiWrAddr >= SF_TOTAL_SIZE)
    {
        return 0;
    }

    /* 如果数据长度大于扇区容量，则退出 */
    if (_usWrLen > SF_PAGE_SIZE)
    {
        return 0;
    }

    /* 如果FLASH中的数据没有变化,则不写FLASH */
    SST26_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);
    if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
    {
        return 1;
    }

    /* 判断是否需要先擦除扇区 */
    /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
    ucNeedErase = 0;
    if (SST26_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
    {
        ucNeedErase = 1;
    }

    uiFirstAddr = _uiWrAddr & (~(SF_PAGE_SIZE - 1));

    //注意有问题:扇区的大小已经超过了s_spiBuf的大小
    if (_usWrLen == SF_PAGE_SIZE)       /* 整个扇区都改写 */
    {
        for (i = 0; i < SF_PAGE_SIZE; i++)
        {
            s_spiBuf[i] = _ucpSrc[i];
        }
    }
    else                        /* 改写部分数据 */
    {
        /* 先将整个扇区的数据读出 */
        SST26_ReadBuffer(s_spiBuf, uiFirstAddr, SF_PAGE_SIZE);

        /* 再用新数据覆盖 */
        i = _uiWrAddr & (SF_PAGE_SIZE - 1);
        memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
    }

    /* 写完之后进行校验，如果不正确则重写，最多3次 */
    cRet = 0;
    for (i = 0; i < 3; i++)
    {

        /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
        if (ucNeedErase == 1)
        {           
            SST26_EraseSector(uiFirstAddr);     /* 擦除1个扇区 */
        }

        /* 编程一个PAGE */
        SST26_PageWrite(s_spiBuf, uiFirstAddr, SF_PAGE_SIZE);
    
        if (SST26_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
        {
            cRet = 1;
            break;
        }
        else
        {
            if (SST26_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
            {
                cRet = 1;
                break;
            }

            /* 失败后延迟一段时间再重试 */
            for (j = 0; j < 10000; j++);
        }
    }

    return cRet;
}

/*
*********************************************************************************************************
*   函 数 名: SST26_WriteBuffer
*   功能说明: 写1个扇区并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*   形    参：  _pBuf : 数据源缓冲区；
*               _uiWrAddr ：目标区域首地址
*               _usSize ：数据个数，不能超过页面大小
*   返 回 值: 1 : 成功， 0 ： 失败
*********************************************************************************************************
*/ 
uint8_t SST26_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
    uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    
    Addr = _uiWriteAddr % SF_PAGE_SIZE;
    count = SF_PAGE_SIZE - Addr;
    NumOfPage =  _usWriteSize / SF_PAGE_SIZE;
    NumOfSingle = _usWriteSize % SF_PAGE_SIZE;
    
    if (Addr == 0) /* 起始地址是页面首地址  */
    {
        if (NumOfPage == 0) /* 数据长度小于页面大小 */
        {
            if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
            {
                return 0;
            }
        }
        else    /* 数据长度大于等于页面大小 */
        {
            while (NumOfPage--)
            {
                if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, SF_PAGE_SIZE) == 0)
                {
                    return 0;
                }               
                _uiWriteAddr +=  SF_PAGE_SIZE;
                _pBuf += SF_PAGE_SIZE;
            }
            if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
            {
                return 0;
            }               
        }
    }
    else  /* 起始地址不是页面首地址  */
    {
        if (NumOfPage == 0) /* 数据长度小于页面大小 */
        {
            if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_PAGESIZE */
            {
                temp = NumOfSingle - count;
                
                if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
                {
                    return 0;
                }
                                
                _uiWriteAddr +=  count;
                _pBuf += count;
            
                if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, temp) == 0)
                {
                    return 0;
                }               
            }
            else
            {
                if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
                {
                    return 0;
                }
            }
        }
        else    /* 数据长度大于等于页面大小 */
        {
            _usWriteSize -= count;
            NumOfPage =  _usWriteSize / SF_PAGE_SIZE;
            NumOfSingle = _usWriteSize % SF_PAGE_SIZE;
            
            if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, count) == 0)
            {
                return 0;
            }           
                
            _uiWriteAddr +=  count;
            _pBuf += count;
            
            while (NumOfPage--)
            {
                if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, SF_PAGE_SIZE) == 0)
                {
                    return 0;
                }               
                _uiWriteAddr +=  SF_PAGE_SIZE;
                _pBuf += SF_PAGE_SIZE;
            }
            
            if (NumOfSingle != 0)
            {
                if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, NumOfSingle) == 0)
                {
                    return 0;
                }               
            }
        }
    }
    return 1;   /* 成功 */
}


/*
*********************************************************************************************************
*   函 数 名: SS26_ReadID
*   功能说明: 读取器件ID
*   形    参：无
*   返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）
*********************************************************************************************************
*/  
uint32_t SST26_ReadID(void)
{
    uint32_t uiID; 
    uint8_t id1, id2, id3;
    
    SST26_WriteEnable();                            /* 发送写使能命令 */
    
    /* 擦除扇区操作 */
    SPI_FLASH_CS_LOW;                         /* 使能片选 */
    SST26_SendByte(CMD_RDID);                   /* 发送读ID命令 */
    id1 = SST26_SendByte(DUMMY_BYTE);       /* 读ID的第1个字节 */
    id2 = SST26_SendByte(DUMMY_BYTE);       /* 读ID的第2个字节 */
    id3 = SST26_SendByte(DUMMY_BYTE);       /* 读ID的第3个字节 */
    SPI_FLASH_CS_HIGH;                        /* 禁能片选 */

    SST26_WaitForWriteEnd();                            /* 等待串行Flash内部写操作完成 */

    uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;
    
    return uiID;
}       

/*
*********************************************************************************************************
*   函 数 名: SST26_UnlockGlobal
*   功能说明: 写状态寄存器
*   形    参：_ucValue : 状态寄存器的值
*   返 回 值: 无
*********************************************************************************************************
*/  
static void SST26_UnlockGlobal(void)
{
    SPI_FLASH_CS_LOW;               /* 使能片选 */
    SST26_SendByte(CMD_UNLOCK);     /* 发送命令， 解锁全片写保护 */
    SPI_FLASH_CS_HIGH;              /* 禁能片选 */  
    
    SST26_WaitForWriteEnd();        /* 等待串行Flash内部操作完成 */
}   

/*******************************************************************************
* Function Name  : SST26_ReadID_Test
* Description    : 读取SST25VF016B ID 
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void SST26_ReadID_Test(void)
{
    DeviceID = SST26_ReadID();          /*  仅保留低24位数据*/
    if(DeviceID == SST26_JEDEC_ID)
    {
        /*  芯片类型识别*/
    }
}

/*
*********************************************************************************************************
*   函 数 名: SST26_Init
*   功能说明: SST26VF032B初始化函数
*   形    参: null
*   返 回 值: no
*********************************************************************************************************
*/
void SST26_Init(void)
{    
	bsp_InitSPI2();
    SST26_ReadID_Test();    /*  芯片类型识别*/

    SST26_UnlockGlobal();   /* 解锁全片写保护 */
}

       
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

