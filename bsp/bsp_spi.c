
#include "bsp_spi.h" 
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"


/*
*********************************************************************************************************
*	函 数 名: bsp_InitSPI2_IO
*	功能说明: 初始化串行Flash硬件接口（配置STM32的SPI时钟、GPIO)
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitSPI2_IO(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能GPIO 时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD, ENABLE);

    /* 配置 SCK, MISO 、 MOSI 为复用功能 */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 配置片选口线为推挽输出模式 */
    SPI_FLASH_CS_HIGH;		/* 片选置高，不选中 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = SF_CS_PIN;
    GPIO_Init(SF_CS_GPIO, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CfgSPIForSFlash
*	功能说明: 配置STM32内部SPI硬件的工作模式、速度等参数，用于访问SPI接口的串行Flash。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_CfgSPI2(void)
{
    SPI_InitTypeDef  SPI_InitStructure;

	/* 打开SPI时钟 */
	ENABLE_SPI_RCC();

	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */

	/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI_FLASH_SPI, &SPI_InitStructure);

	SPI_Cmd(SPI_FLASH_SPI, DISABLE);			/* 先禁止SPI  */

	SPI_Cmd(SPI_FLASH_SPI, ENABLE);				/* 使能SPI  */
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSPI2
*	功能说明: 配置STM32内部SPI硬件的工作模式、速度等参数，用于访问SPI接口的串行Flash。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitSPI2(void)
{
    bsp_InitSPI2_IO();

    bsp_CfgSPI2();
}


/*
*********************************************************************************************************
*	函 数 名: bsp_InitSPI2
*	功能说明: 初始化spi1
*	形    参:  无
*	返 回 值: 无
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


    //IO复用
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);                
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);               
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);                
    
    
	//设置SPI1
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;      //全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                           //SPI主机模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                       //字节传输
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                             //空闲状态下为高字节
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                            //在SCK的第二个跳变沿数据被采集
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                               //NSS信号由软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;      /* 84M / 8 */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                      //数据从MSB位开始传输
	SPI_InitStructure.SPI_CRCPolynomial = 7;                               
	SPI_Init(SPI1, &SPI_InitStructure);
    
	SPI_Cmd(SPI1, ENABLE);					                                //使能SPI1
}


/***********************************************************************************************
* Function		: SPI_SendRcvByte
* Description	: 读写spi数据
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
    
	//等待数据寄存器空
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
	{
	    //如果非空，等待发送结束
		if( (count1++) >= 0x30000 )
		{
			//延时总现出错，复位总线
			RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);        //复位SPI1外设
            DelayUs(5);
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);       //停止复位SPI1外设
            DelayUs(5);

			count1 = 0;
			*statu = FALSE;
			return FALSE;
		}	
	}

	//发送数据
	SPI_I2S_SendData(SPI1, Data);

	/* 等待接收到一个字节的数据 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
	{
	    //如果空，等待接收结束
		if( (count1++) >= 0x30000)
		{
			//延时总现出错，复位总线
			RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);        //复位SPI1外设
            DelayUs(5);
            RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, DISABLE);       //停止复位SPI1外设
            DelayUs(5);

			*statu = FALSE;
			return FALSE;
		} 
	}

	/* 返回接收到的数据 */
    *statu = TRUE;
    data = SPI_I2S_ReceiveData(SPI1);
	return data;
}
/*********************************************END OF FILE**********************/
