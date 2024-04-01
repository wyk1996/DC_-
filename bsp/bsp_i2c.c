/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件   名: bsp_IIC.c
**创    建   人: 
**创  建  日  期: 
**最  新  版  本: 
**描        述: 软件I2C
**---------------------------------------历史版本信息-------------------------------------------
**修    改   人: 
**日        期: 
**版        本: 
**描        述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "stm32f4xx_gpio.h"
#include "bsp_i2c.h"
/* Private define-----------------------------------------------------------------------------*/
// IIC引脚定义
#define	SIIC0_GPIO				GPIOH   
#define SIIC0_GPIO_SCL			BIT(7)
#define SIIC0_GPIO_SDA			BIT(8)

#define	IIC_DELAY_TIME			1
#define	IIC_DELAY_TIME_LONG		2
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: 
* Description	: 软件IIC驱动
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	:2018年5月25日
***********************************************************************************************/
static void IIC_DelayUs(INT8U time)
{
	DelayUs(time);
}

/*****************************************************************************
* Function     : IIC_CLK_High
* Description  : I2C的CLK引脚为高
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  :2018年5月25日
*****************************************************************************/
static void IIC_CLK_High(void)
{
    GPIO_SetBits(SIIC0_GPIO, SIIC0_GPIO_SCL);  //SCL = 1
}

/*****************************************************************************
* Function     : IIC_CLK_Low
* Description  : I2C的CLK引脚为低
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  :2018年5月25日
*****************************************************************************/
static void IIC_CLK_Low(void)
{
    GPIO_ResetBits(SIIC0_GPIO, SIIC0_GPIO_SCL);  //SCL = 0
}

/*****************************************************************************
* Function     : IIC_SDA_High
* Description  : 设置I2C的SDA为高
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void IIC_SDA_High(void)
{
    GPIO_SetBits(SIIC0_GPIO, SIIC0_GPIO_SDA);  
}

/*****************************************************************************
* Function     : IIC_SDA_Low
* Description  : 设置I2C的SDA为低
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void IIC_SDA_Low(void)
{
	GPIO_ResetBits(SIIC0_GPIO, SIIC0_GPIO_SDA);  
}

/*****************************************************************************
* Function     : IIC_SDA_In
* Description  : I2C的SDA引脚设置为输入
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void IIC_SDA_In(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	// 配置SDA
	GPIO_InitStructure.GPIO_Pin = SIIC0_GPIO_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;            //输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;        //不上下拉
	GPIO_Init(SIIC0_GPIO, &GPIO_InitStructure);	
}

/*****************************************************************************
* Function     : IIC_SDA_Out
* Description  : I2C的SDA引脚设置为输出
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static void IIC_SDA_Out(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    //配置SDA为输出脚
	GPIO_InitStructure.GPIO_Pin = SIIC0_GPIO_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       //设置为输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //使能上拉
	GPIO_Init(SIIC0_GPIO, &GPIO_InitStructure);         //初始化GPIO
}



/*****************************************************************************
* Function     : IIC_SDA_Read
* Description  : 读取I2C的SDA电平
* Input        : void  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
static INT8U IIC_SDA_Read(void)
{
	IIC_DelayUs(IIC_DELAY_TIME);
	return GPIO_ReadInputDataBit(SIIC0_GPIO,SIIC0_GPIO_SDA);
}

/*****************************************************************************
* Function     : IIC_Init
* Description  : I2C初始化
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    
	RCC->AHB1ENR|=1<<7;    //Ê¹ÄÜPORTHÊ±ÖÓ
	//配置SCL、SDA为输出脚，初始化为1
	GPIO_InitStructure.GPIO_Pin = SIIC0_GPIO_SCL | SIIC0_GPIO_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                   //设置为输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                  //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                    //使能上拉
	GPIO_Init(SIIC0_GPIO, &GPIO_InitStructure);                     //初始化GPIO

    IIC_SDA_High();                                                 //SDA = 1
	IIC_CLK_High();                                                 //SCL = 1
}
/*****************************************************************************
* Function     : IIC_Start
* Description  : I2C起始条件:SDA在SCL为高电平期间产生一个下降沿
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void IIC_Start(void)
{ 
    IIC_SDA_Out();   //设置SDA为输出
    
	IIC_SDA_High();  //SDA = 1
	IIC_CLK_High();  //SCL = 1
	IIC_DelayUs(2);  
	IIC_SDA_Low();   //SDA = 0,产生一个起始条件
	IIC_DelayUs(2);  
	IIC_CLK_Low();   //SCL = 0，准备发送或者接收数据
}

/*****************************************************************************
* Function     : IIC_Stop
* Description  : I2C停止条件:SDA在SCL为高电平期间产生了一个上升沿
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void IIC_Stop(void)
{
    IIC_CLK_Low();   //SCL = 0,避免在SCL=1时操作了SDA
    IIC_SDA_Out();   //设置SDA为输出
	IIC_SDA_Low();   //SDA = 0
    IIC_DelayUs(2);
	IIC_CLK_High();  //SCL = 1
	IIC_DelayUs(2);
	IIC_SDA_High();  //SDA = 1,产生一个上升沿
	IIC_DelayUs(2);
}


/*****************************************************************************
* Function     : IIC_Ack
* Description  : 接收8位数据后产生一个ACK(发送SDA=0),SDA只能在SCL=0是发生变化，
                 否则将产生了起始条件或者停止条件
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
void IIC_Ack(void)
{
    IIC_CLK_Low();   //SCL = 0
    IIC_SDA_Out();   //设置SDA为输出
	IIC_SDA_Low();   //SDA = 0
    IIC_DelayUs(2);  
	IIC_CLK_High();  //SCL = 1,发送一位0
	IIC_DelayUs(2); 
    IIC_CLK_Low();   //SCL = 0
}

/*****************************************************************************
* Function     : IIC_NAck
* Description  : 接收到数据后不产生一个ACK(发送SDA=1)
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  :2018年5月25日
*****************************************************************************/
void IIC_NAck(void)
{
    IIC_CLK_Low();   //SCL = 0
    IIC_SDA_Out();   //设置SDA为输出
    IIC_DelayUs(2);
    IIC_SDA_High();  //SDA = 1
    IIC_DelayUs(2);
	IIC_CLK_High();  //SCL = 1,发送一位1
    IIC_DelayUs(2);
    IIC_CLK_Low();   //SCL = 0
}

/*****************************************************************************
* Function     : IIC_SendByte
* Description  : I2C发送一个字节数据,按位发送，先发送MSB
* Input        : INT8U val  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U IIC_SendByte(INT8U val)
{
	INT8U i;
	
	IIC_CLK_Low();               //SCL = 0
	IIC_SDA_Out();               //设置SDA为输出
	IIC_DelayUs(2);
	for (i = 0; i < 8; i++)
	{
		if (val & 0x80)
		{
			IIC_SDA_High();
		}
		else
		{
			IIC_SDA_Low();
		}
        IIC_DelayUs(2);          //等待SDA稳定
		IIC_CLK_High();          //SCL = 1，发送数据
		IIC_DelayUs(2);          //等待数据发送完成
		IIC_CLK_Low();    
        IIC_DelayUs(2);          //等待CLK=0稳定
		val <<= 1;               //数据左移一位
	}
	//等待应答
	IIC_SDA_In();                //SDA设置为输入，准备接收ACK      
	IIC_CLK_High();              //CLK = 1
	IIC_DelayUs(1);              //等待CLK = 1稳定
	i = 0;
	while (IIC_SDA_Read() )      //读SDA，直到SDA = 0才表示成功        
	{
		if (++i > 12)
		{
			IIC_CLK_Low();
            IIC_DelayUs(2);
			return FALSE;
		}
	}
	IIC_CLK_Low();
    IIC_DelayUs(2);
	return TRUE;
}

/*****************************************************************************
* Function     : IIC_RecvByte
* Description  : I2C接收一个字节
* Input        : void  
* Output       : None
* Return       : 
* Note(s)      : 
* Contributor  : 2018年5月25日
*****************************************************************************/
INT8U IIC_RecvByte(void)
{
	INT8U i, val = 0;
    
	IIC_CLK_Low();                 //SCL = 0，准备读取最高位数据
	IIC_SDA_In();                  //设置SDA为输入
    IIC_DelayUs(2);
	for (i = 0; i < 8; i++)
	{
		IIC_CLK_High();            //SCL = 1,读取数据
        IIC_DelayUs(2);
		val <<= 1;                 //i=0时，是进行了空左移，左移7次就够了，否则最高字节丢失
		val |= IIC_SDA_Read();     //读取SDA上的数据            
		IIC_CLK_Low();             //SCL = 0,继续读取下一个位
        IIC_DelayUs(2);
	}
	return val;
}
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
