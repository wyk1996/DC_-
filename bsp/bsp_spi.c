
#include "bsp_spi.h" 
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSPI2_IO
*	����˵��: ��ʼ������FlashӲ���ӿڣ�����STM32��SPIʱ�ӡ�GPIO)
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitSPI2_IO(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ʹ��GPIO ʱ�� */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD, ENABLE);

    /* ���� SCK, MISO �� MOSI Ϊ���ù��� */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* ����Ƭѡ����Ϊ�������ģʽ */
    SPI_FLASH_CS_HIGH;		/* Ƭѡ�øߣ���ѡ�� */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = SF_CS_PIN;
    GPIO_Init(SF_CS_GPIO, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_CfgSPIForSFlash
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���SPI�ӿڵĴ���Flash��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_CfgSPI2(void)
{
    SPI_InitTypeDef  SPI_InitStructure;

	/* ��SPIʱ�� */
	ENABLE_SPI_RCC();

	/* ����SPIӲ������ */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ���ݷ���2��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32��SPI����ģʽ ������ģʽ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* ����λ���� �� 8λ */
	/* SPI_CPOL��SPI_CPHA���ʹ�þ���ʱ�Ӻ����ݲ��������λ��ϵ��
	   ��������: ���߿����Ǹߵ�ƽ,��2�����أ������ز�������)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* ʱ�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* ʱ�ӵĵ�2�����ز������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* Ƭѡ���Ʒ�ʽ��������� */

	/* ���ò�����Ԥ��Ƶϵ�� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI_FLASH_SPI, &SPI_InitStructure);

	SPI_Cmd(SPI_FLASH_SPI, DISABLE);			/* �Ƚ�ֹSPI  */

	SPI_Cmd(SPI_FLASH_SPI, ENABLE);				/* ʹ��SPI  */
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSPI2
*	����˵��: ����STM32�ڲ�SPIӲ���Ĺ���ģʽ���ٶȵȲ��������ڷ���SPI�ӿڵĴ���Flash��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitSPI2(void)
{
    bsp_InitSPI2_IO();

    bsp_CfgSPI2();
}


/*
*********************************************************************************************************
*	�� �� ��: bsp_InitSPI2
*	����˵��: ��ʼ��spi1
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitSPI1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);               
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);                

  	/* Configure SPI1 pins: SCK, MISO ,MOSI------------- */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;               //PA5 PA6 PA5
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                         
  	GPIO_Init(GPIOA, &GPIO_InitStructure);


	//CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                   
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                     
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);                


    //IO����
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);                
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);               
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);                
    
    
	//����SPI1
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;      //ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                           //SPI����ģʽ
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                       //�ֽڴ���
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                             //����״̬��Ϊ���ֽ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                            //��SCK�ĵڶ������������ݱ��ɼ�
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                               //NSS�ź����������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;      /* 84M / 8 */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                      //���ݴ�MSBλ��ʼ����
	SPI_InitStructure.SPI_CRCPolynomial = 7;                               
	SPI_Init(SPI1, &SPI_InitStructure);
    
	SPI_Cmd(SPI1, ENABLE);					                                //ʹ��SPI1
}


/***********************************************************************************************
* Function		: SPI_SendRcvByte
* Description	: ��дspi����
* Input			:
* Output		:
* Note(s)		: 
* Contributor	:
***********************************************************************************************/
INT8U SPI1_SendRcvByte( INT8U Data, INT8U *statu)
{
    INT8U data = 0;
	INT32U count1 = 0;
	 
	*statu = FALSE;
    
    if (!IS_SPI_ALL_PERIPH(SPI1) )
    {
        return FALSE;
    }
    
	//�ȴ����ݼĴ�����
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
	{
	    //����ǿգ��ȴ����ͽ���
		if( (count1++) >= 0x30000 )
		{
			//��ʱ���ֳ�����λ����
			RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);        //��λSPI1����
            DelayUs(5);
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);       //ֹͣ��λSPI1����
            DelayUs(5);

			count1 = 0;
			*statu = FALSE;
			return FALSE;
		}	
	}

	//��������
	SPI_I2S_SendData(SPI1, Data);

	/* �ȴ����յ�һ���ֽڵ����� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
	{
	    //����գ��ȴ����ս���
		if( (count1++) >= 0x30000)
		{
			//��ʱ���ֳ�����λ����
			RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);        //��λSPI1����
            DelayUs(5);
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);       //ֹͣ��λSPI1����
            DelayUs(5);

			*statu = FALSE;
			return FALSE;
		} 
	}

	/* ���ؽ��յ������� */
    *statu = TRUE;
    data = SPI_I2S_ReceiveData(SPI1);
	return data;
}
/*********************************************END OF FILE**********************/
