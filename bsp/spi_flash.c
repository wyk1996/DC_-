

#include "spi_flash.h"
//#include "Algorithm.h"
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
* ��������
*******************************************************************************/
static uint8_t s_spiBuf[LIMIT_BUF_SIZE];    /* ����д�������ȶ�������page���޸Ļ�������������page��д */
uint32_t DeviceID=0;

/*******************************************************************************
* ��������
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
*   �� �� ��: SST26_WriteEnable
*   ����˵��: ����������дʹ������
*   ��    �Σ���
*   �� �� ֵ: ��
*********************************************************************************************************
*/
static void SST26_WriteEnable(void)
{
    SPI_FLASH_CS_LOW;                         /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_WREN);                 /* �������� */
    SPI_FLASH_CS_HIGH;                        /* ����Ƭѡ */  

    SST26_WaitForWriteEnd();                  /* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*   �� �� ��: SST26_WaitForWriteEnd
*   ����˵��: ����ѭ����ѯ�ķ�ʽ�ȴ������ڲ�д�������
*   ��    �Σ���
*   �� �� ֵ: ��
*********************************************************************************************************
*/  
static void SST26_WaitForWriteEnd(void)
{
    SPI_FLASH_CS_LOW;                                 /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_RDSR);                           /* ������� ��״̬�Ĵ��� */

    while((SST26_SendByte(DUMMY_BYTE) & WIP_FLAG) == SET);  /* �ж�״̬�Ĵ�����æ��־λ */
   
    SPI_FLASH_CS_HIGH;                                    /* ����Ƭѡ */  
}

/*
*********************************************************************************************************
*   �� �� ��: SST26_EraseSector
*   ����˵��: ����ָ��������
*   ��    �Σ�_uiSectorAddr : ������ַ
*   �� �� ֵ: ��
*********************************************************************************************************
*/
void SST26_EraseSector(uint32_t _uiSectorAddr)
{
    SST26_WriteEnable();                                /* ����дʹ������ */
    
    /* ������������ */
    SPI_FLASH_CS_LOW;                                 /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_SE);                             /* ���Ͳ������� */
    SST26_SendByte((_uiSectorAddr & 0xFF0000) >> 16);   /* ����������ַ�ĸ�8bit */
    SST26_SendByte((_uiSectorAddr & 0xFF00) >> 8);      /* ����������ַ�м�8bit */
    SST26_SendByte(_uiSectorAddr & 0xFF);               /* ����������ַ��8bit */
    SPI_FLASH_CS_HIGH;                                /* ����Ƭѡ */
    
    SST26_WaitForWriteEnd();                            /* �ȴ�����Flash�ڲ�д������� */
}
/*
*********************************************************************************************************
*   �� �� ��: SST26_EraseChip
*   ����˵��: ��������оƬ
*   ��    �Σ���
*   �� �� ֵ: ��
*********************************************************************************************************
*/  
void SST26_EraseChip(void)
{
    SST26_WriteEnable();        /* ����дʹ������ */
    
    /* ������������ */
    SPI_FLASH_CS_LOW;         /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_BE);     /* ���Ͳ������� */
    SPI_FLASH_CS_HIGH;        /* ����Ƭѡ */
    
    SST26_WaitForWriteEnd();    /* �ȴ�����Flash�ڲ�д������� */
}

/*
*********************************************************************************************************
*   �� �� ��: sSST26_PageWrite
*   ����˵��: ��һ��page��д�������ֽڡ��ֽڸ������ܳ���ҳ���С��4K)
*   ��    �Σ�  _pBuf : ����Դ��������
*               _uiWriteAddr ��Ŀ�������׵�ַ
*               _usSize �����ݸ��������ܳ���ҳ���С
*   �� �� ֵ: ��
*********************************************************************************************************
*/  
void SST26_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
    uint32_t i, num;
    
     /* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
    if (_usSize == 0)
    {
        return;
    }

    num = 0;
    
    while((num < _usSize) && (num < SEC_SIZE))
    {
        SST26_WriteEnable();                                /* ����дʹ������ */
    
        /* ������������ */
        SPI_FLASH_CS_LOW;                                   /* ʹ��Ƭѡ */
        SST26_SendByte(CMD_WRPAGE);                         /* ����ҳ�������(256Byte) */
        SST26_SendByte((_uiWriteAddr & 0xFF0000) >> 16);    /* ����������ַ�ĸ�8bit */
        SST26_SendByte((_uiWriteAddr & 0xFF00) >> 8);       /* ����������ַ�м�8bit */
        SST26_SendByte(_uiWriteAddr & 0xFF);                /* ����������ַ��8bit */    

        for (i = 0; i < PAGE_SIZE; i++)
        {
            if(i < _usSize)
            {
                SST26_SendByte(*_pBuf++);                   /* ����256���ֽ����� */
            }
            else
                SST26_SendByte(0);                          /* ����256���ֽ����� */
        }
        num += PAGE_SIZE;                                   /* �����ѷ���256���ֽ� */
        _uiWriteAddr += PAGE_SIZE;
        SPI_FLASH_CS_HIGH;                                  /* ����Ƭѡ */
        
        SST26_WaitForWriteEnd();                            /* �ȴ�����Flash�ڲ�д������� */
    }
}

/*
*********************************************************************************************************
*   �� �� ��: SST26_ReadBuffer
*   ����˵��: ������ȡ�����ֽڡ��ֽڸ������ܳ���оƬ������
*   ��    �Σ�  _pBuf : ����Դ��������
*               _uiReadAddr ���׵�ַ
*               _usSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*   �� �� ֵ: ��
*********************************************************************************************************
*/
void SST26_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
    /* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
    if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > SF_TOTAL_SIZE)
    {
        return;
    }
        
    SST26_WriteEnable();                              /* ����дʹ������ */
    
    SPI_FLASH_CS_LOW;                                 /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_READ);                           /* ���Ͷ����� */
//  SST26_SendByte(CMD_READFAST);                           /* ���Ͷ����� */
    
    SST26_SendByte((_uiReadAddr & 0xFF0000) >> 16); /* ����������ַ�ĸ�8bit */
    SST26_SendByte((_uiReadAddr & 0xFF00) >> 8);        /* ����������ַ�м�8bit */
    SST26_SendByte(_uiReadAddr & 0xFF);             /* ����������ַ��8bit */    
    while (_uiSize--)
    {
        *_pBuf++ = SST26_SendByte(DUMMY_BYTE);          /* ��һ���ֽڲ��洢��pBuf�������ָ���Լ�1 */
    }
    SPI_FLASH_CS_HIGH;                                    /* ����Ƭѡ */

    SST26_WaitForWriteEnd();        /* �ȴ�����Flash�ڲ�������� */
}

/*
*********************************************************************************************************
*   �� �� ��: SST26_CmpData
*   ����˵��: �Ƚ�Flash������.
*   ��    �Σ�  _ucpTar : ���ݻ�����
*               _uiSrcAddr ��Flash��ַ
*               _uiSize �����ݸ���, ���Դ���PAGE_SIZE,���ǲ��ܳ���оƬ������
*   �� �� ֵ: 0 = ���, 1 = ����
*********************************************************************************************************
*/
static uint8_t SST26_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
    uint8_t ucValue;

    /* �����ȡ�����ݳ���Ϊ0���߳�������Flash��ַ�ռ䣬��ֱ�ӷ��� */
    if ((_uiSrcAddr + _uiSize) > SF_TOTAL_SIZE)
    {
        return 1;
    }

    if (_uiSize == 0)   
    {
        return 0;
    }
        
    SPI_FLASH_CS_LOW;                                 /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_READ);                           /* ���Ͷ����� */

    SST26_SendByte((_uiSrcAddr & 0xFF0000) >> 16);      /* ����������ַ�ĸ�8bit */
    SST26_SendByte((_uiSrcAddr & 0xFF00) >> 8);     /* ����������ַ�м�8bit */
    SST26_SendByte(_uiSrcAddr & 0xFF);                  /* ����������ַ��8bit */    
    while (_uiSize--)
    {
        /* ��һ���ֽ� */
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
*   �� �� ��: SST26_NeedErase
*   ����˵��: �ж�дPAGEǰ�Ƿ���Ҫ�Ȳ�����
*   ��    �Σ� _ucpOldBuf �� ������
*              _ucpNewBuf �� ������
*              _uiLen �����ݸ��������ܳ���ҳ���С
*   �� �� ֵ: 0 : ����Ҫ������ 1 ����Ҫ����
*********************************************************************************************************
*/ 
static uint8_t SST26_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
{
    uint16_t i;
    uint8_t ucOld;

    /*
    �㷨��1����old ��, new ����
          old    new
          1101   0101
    ~     1111
        = 0010   0101

    �㷨��2��: old �󷴵Ľ���� new λ��
          0010   old
    &     0101   new
         =0000

    �㷨��3��: ���Ϊ0,���ʾ�������. �����ʾ��Ҫ����
    */

    for (i = 0; i < _usLen; i++)
    {
        ucOld = *_ucpOldBuf++;
        ucOld = ~ucOld;

        /* ע������д��: if (ucOld & (*_ucpNewBuf++) != 0) */
        if ((ucOld & (*_ucpNewBuf++)) != 0)
        {
            return 1;
        }
    }
    return 0;
}

/*
*********************************************************************************************************
*   �� �� ��: SST26_AutoWritePage
*   ����˵��: д1��PAGE��У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*   ��    �Σ�  _pBuf : ����Դ��������
*               _uiWriteAddr ��Ŀ�������׵�ַ
*               _usSize �����ݸ��������ܳ���ҳ���С
*   �� �� ֵ: 0 : ���� 1 �� �ɹ�
*********************************************************************************************************
*/ 
static uint8_t SST26_AutoWritePage(uint8_t *_ucpSrc, uint32_t _uiWrAddr, uint16_t _usWrLen)
{
    uint16_t i;
    uint16_t j;                 /* ������ʱ */
    uint32_t uiFirstAddr;       /* ������ַ */
    uint8_t ucNeedErase;        /* 1��ʾ��Ҫ���� */
    //uint8_t ucaFlashBuf[SF_PAGE_SIZE];
    uint8_t cRet;

    /* ����Ϊ0ʱ����������,ֱ����Ϊ�ɹ� */
    if (_usWrLen == 0)
    {
        return 1;
    }   
    
    /* ���ƫ�Ƶ�ַ����оƬ�������˳� */
    if (_uiWrAddr >= SF_TOTAL_SIZE)
    {
        return 0;
    }

    /* ������ݳ��ȴ����������������˳� */
    if (_usWrLen > SF_PAGE_SIZE)
    {
        return 0;
    }

    /* ���FLASH�е�����û�б仯,��дFLASH */
    SST26_ReadBuffer(s_spiBuf, _uiWrAddr, _usWrLen);
    if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
    {
        return 1;
    }

    /* �ж��Ƿ���Ҫ�Ȳ������� */
    /* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
    ucNeedErase = 0;
    if (SST26_NeedErase(s_spiBuf, _ucpSrc, _usWrLen))
    {
        ucNeedErase = 1;
    }

    uiFirstAddr = _uiWrAddr & (~(SF_PAGE_SIZE - 1));

    //ע��������:�����Ĵ�С�Ѿ�������s_spiBuf�Ĵ�С
    if (_usWrLen == SF_PAGE_SIZE)       /* ������������д */
    {
        for (i = 0; i < SF_PAGE_SIZE; i++)
        {
            s_spiBuf[i] = _ucpSrc[i];
        }
    }
    else                        /* ��д�������� */
    {
        /* �Ƚ��������������ݶ��� */
        SST26_ReadBuffer(s_spiBuf, uiFirstAddr, SF_PAGE_SIZE);

        /* ���������ݸ��� */
        i = _uiWrAddr & (SF_PAGE_SIZE - 1);
        memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
    }

    /* д��֮�����У�飬�������ȷ����д�����3�� */
    cRet = 0;
    for (i = 0; i < 3; i++)
    {

        /* ����������޸�Ϊ�����ݣ�����λ���� 1->0 ���� 0->0, ���������,���Flash���� */
        if (ucNeedErase == 1)
        {           
            SST26_EraseSector(uiFirstAddr);     /* ����1������ */
        }

        /* ���һ��PAGE */
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

            /* ʧ�ܺ��ӳ�һ��ʱ�������� */
            for (j = 0; j < 10000; j++);
        }
    }

    return cRet;
}

/*
*********************************************************************************************************
*   �� �� ��: SST26_WriteBuffer
*   ����˵��: д1��������У��,�������ȷ������д���Ρ��������Զ���ɲ���������
*   ��    �Σ�  _pBuf : ����Դ��������
*               _uiWrAddr ��Ŀ�������׵�ַ
*               _usSize �����ݸ��������ܳ���ҳ���С
*   �� �� ֵ: 1 : �ɹ��� 0 �� ʧ��
*********************************************************************************************************
*/ 
uint8_t SST26_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
    uint16_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    
    Addr = _uiWriteAddr % SF_PAGE_SIZE;
    count = SF_PAGE_SIZE - Addr;
    NumOfPage =  _usWriteSize / SF_PAGE_SIZE;
    NumOfSingle = _usWriteSize % SF_PAGE_SIZE;
    
    if (Addr == 0) /* ��ʼ��ַ��ҳ���׵�ַ  */
    {
        if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
        {
            if (SST26_AutoWritePage(_pBuf, _uiWriteAddr, _usWriteSize) == 0)
            {
                return 0;
            }
        }
        else    /* ���ݳ��ȴ��ڵ���ҳ���С */
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
    else  /* ��ʼ��ַ����ҳ���׵�ַ  */
    {
        if (NumOfPage == 0) /* ���ݳ���С��ҳ���С */
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
        else    /* ���ݳ��ȴ��ڵ���ҳ���С */
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
    return 1;   /* �ɹ� */
}


/*
*********************************************************************************************************
*   �� �� ��: SS26_ReadID
*   ����˵��: ��ȡ����ID
*   ��    �Σ���
*   �� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��
*********************************************************************************************************
*/  
uint32_t SST26_ReadID(void)
{
    uint32_t uiID; 
    uint8_t id1, id2, id3;
    
    SST26_WriteEnable();                            /* ����дʹ������ */
    
    /* ������������ */
    SPI_FLASH_CS_LOW;                         /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_RDID);                   /* ���Ͷ�ID���� */
    id1 = SST26_SendByte(DUMMY_BYTE);       /* ��ID�ĵ�1���ֽ� */
    id2 = SST26_SendByte(DUMMY_BYTE);       /* ��ID�ĵ�2���ֽ� */
    id3 = SST26_SendByte(DUMMY_BYTE);       /* ��ID�ĵ�3���ֽ� */
    SPI_FLASH_CS_HIGH;                        /* ����Ƭѡ */

    SST26_WaitForWriteEnd();                            /* �ȴ�����Flash�ڲ�д������� */

    uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;
    
    return uiID;
}       

/*
*********************************************************************************************************
*   �� �� ��: SST26_UnlockGlobal
*   ����˵��: д״̬�Ĵ���
*   ��    �Σ�_ucValue : ״̬�Ĵ�����ֵ
*   �� �� ֵ: ��
*********************************************************************************************************
*/  
static void SST26_UnlockGlobal(void)
{
    SPI_FLASH_CS_LOW;               /* ʹ��Ƭѡ */
    SST26_SendByte(CMD_UNLOCK);     /* ������� ����ȫƬд���� */
    SPI_FLASH_CS_HIGH;              /* ����Ƭѡ */  
    
    SST26_WaitForWriteEnd();        /* �ȴ�����Flash�ڲ�������� */
}   

/*******************************************************************************
* Function Name  : SST26_ReadID_Test
* Description    : ��ȡSST25VF016B ID 
* Input          : None
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
void SST26_ReadID_Test(void)
{
    DeviceID = SST26_ReadID();          /*  ��������24λ����*/
    if(DeviceID == SST26_JEDEC_ID)
    {
        /*  оƬ����ʶ��*/
    }
}

/*
*********************************************************************************************************
*   �� �� ��: SST26_Init
*   ����˵��: SST26VF032B��ʼ������
*   ��    ��: null
*   �� �� ֵ: no
*********************************************************************************************************
*/
void SST26_Init(void)
{    
	bsp_InitSPI2();
    SST26_ReadID_Test();    /*  оƬ����ʶ��*/

    SST26_UnlockGlobal();   /* ����ȫƬд���� */
}

       
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

