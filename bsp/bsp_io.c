/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_RLC.c
**硬          件: STM32
**创    建    人:
**创  建  日  期: 2018年6月5日
**最  新  版  本: V0.2
**描          述: RLC控制程序 ,统一输出格式
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: V0.1
**描          述: 
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#include	"sysconfig.h"
#include	"bsp_io.h"
/* Includes-----------------------------------------------------------------------------------*/
/* Private define-----------------------------------------------------------------------------*/




/* Private typedef----------------------------------------------------------------------------*/
// MPLS硬件配置结构
typedef struct
{
	GPIO_TypeDef *GPIOx;						// 引脚
	INT16U CtlPout;								// 控制脚,1:开
}_RLC_HD;


//=====================集成老板子===============
#if(USER_OLDandNEW == 0)
#if(USER_GUN != USER_SINGLE_GUN)
const _RLC_HD RLC_HD_Table[IO_MAX]=    // RLC硬件配置表---老双枪
{
	{GPIOC,BIT(5)},  
	{GPIOB,BIT(0)},  
	{GPIOB,BIT(1)},  
	{GPIOB,BIT(2)},  
	{GPIOE,BIT(7)},  
	{GPIOE,BIT(8)},  
	{GPIOE,BIT(9)},  
	{GPIOE,BIT(10)},  
	{GPIOE,BIT(11)},  
	{GPIOE,BIT(12)}, 

	{GPIOE,BIT(5)},  
	{GPIOE,BIT(4)},  
	{GPIOE,BIT(3)},  
	{GPIOE,BIT(2)},  
	{GPIOE,BIT(1)},  
	{GPIOE,BIT(0)},	
	{GPIOB,BIT(7)},
	{GPIOE,BIT(6)},	
	
	{GPIOC,BIT(13)},
	{GPIOA,BIT(6)},	
	{GPIOA,BIT(7)},
	{GPIOC,BIT(4)},	
	{GPIOA,BIT(8)},  
	{GPIOC,BIT(8)},  
	{GPIOC,BIT(9)},  
	
	{GPIOD,BIT(7)},    //485使能 打印串口
};
#else
const _RLC_HD RLC_HD_Table[IO_MAX]=     //---老单枪
{
	{GPIOC,BIT(4)},  //风扇
	{GPIOC,BIT(5)},   //A枪K1
	{GPIOB,BIT(0)},   //A枪电子锁
	{GPIOE,BIT(7)},  //A枪辅助电源
	{GPIOB,BIT(1)}, //交流输入接触器
	{GPIOE,BIT(8)},  //备用
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)}, 

	{GPIOE,BIT(5)},  //A枪绿灯
	{GPIOE,BIT(4)},  //A枪红灯
	{GPIOE,BIT(3)},  //A枪黄灯
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)}, 
	{GPIOE,BIT(2)}, //运行灯
	{GPIOE,BIT(6)},	//蜂鸣器
	
	{GPIOA,BIT(7)}, //不平衡桥
	{GPIOA,BIT(6)},	//平衡桥
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)}, 
	{GPIOD,BIT(12)},  // IO_4G_PWR,  
	{GPIOD,BIT(10)},  //IO_4G_RES
	{GPIOD,BIT(11)},  //IO_4G_ONOFF
	
	{GPIOC,BIT(0)},    //485使能 打印串口  用到一个悬空的引脚
};
#endif
#endif



//=====================新板子===============
#if(USER_OLDandNEW == 1)
#if(USER_GUN != USER_SINGLE_GUN)
//RLC硬件配置表
const _RLC_HD RLC_HD_Table[IO_MAX]=    //==新板子双枪
{
    {GPIOD,BIT(15)}, 
	{GPIOE,BIT(8)},   //A枪K1
	{GPIOB,BIT(2)},  //电子锁A
	{GPIOE,BIT(7)},  //A枪辅助电源
	{GPIOD,BIT(15)}, 
	
	{GPIOD,BIT(15)},  
	{GPIOE,BIT(9)},  //电子锁B
	{GPIOC,BIT(5)},  //辅助电源B
	{GPIOE,BIT(11)}, //B枪K1 
	{GPIOE,BIT(12)}, //母联

	{GPIOE,BIT(5)},  
	{GPIOE,BIT(4)},  
	{GPIOE,BIT(3)},  
	{GPIOE,BIT(2)},  
	{GPIOE,BIT(1)}, 
	
	{GPIOE,BIT(0)},	
	{GPIOB,BIT(7)},
	{GPIOE,BIT(6)},	
	{GPIOC,BIT(13)},
	{GPIOA,BIT(6)},
	
	{GPIOA,BIT(7)},
	{GPIOC,BIT(4)},	
	{GPIOC,BIT(9)},  
	{GPIOC,BIT(8)},  
	{GPIOA,BIT(8)},  
	
	
	{GPIOD,BIT(7)},    //485使能 打印串口
};
#else
const _RLC_HD RLC_HD_Table[IO_MAX]=
{
	{GPIOC,BIT(4)},   //风机==PC4
	{GPIOB,BIT(2)},   //A枪K1K2==PB2
	{GPIOE,BIT(8)},   //A电子锁==PE8
	{GPIOC,BIT(5)},   //备用====PC5====A枪辅源
	{GPIOE,BIT(7)},   //PE7-----交流接触器
	
//	{GPIOC,BIT(4)},  //风扇
//	{GPIOC,BIT(5)},   //A枪K1
//	{GPIOB,BIT(0)},   //A枪电子锁
//	{GPIOE,BIT(7)},  //A枪辅助电源
//	{GPIOB,BIT(1)}, //交流输入接触器
	
	
	{GPIOE,BIT(8)},  //备用
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)}, 

	{GPIOE,BIT(5)},  //A枪绿灯
	{GPIOE,BIT(4)},  //A枪红灯
	{GPIOE,BIT(3)},  //A枪黄灯
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)}, 
	{GPIOE,BIT(2)}, //运行灯
	{GPIOE,BIT(6)},	//蜂鸣器
	
	{GPIOA,BIT(7)}, //不平衡桥
	{GPIOA,BIT(6)},	//平衡桥
	{GPIOE,BIT(0)},  
	{GPIOE,BIT(0)}, 
//	{GPIOD,BIT(12)},  // IO_4G_PWR,  
//	{GPIOD,BIT(10)},  //IO_4G_RES
//	{GPIOD,BIT(11)},  //IO_4G_ONOFF
	
	{GPIOD,BIT(11)},  //IO_4G_ONOFF
	{GPIOD,BIT(10)},  //IO_4G_RES
	{GPIOD,BIT(12)},  // IO_4G_PWR,  

	
	{GPIOC,BIT(0)},    //485使能 打印串口  用到一个悬空的引脚
};
#endif
#endif


typedef struct
{
	INT32U RLCState;								// RLC状态,1:开;0:关
}_RLC_CONTROL;
/* Private variables--------------------------------------------------------------------------*/
static _RLC_CONTROL RLC_Control;				// 控制寄存器
_MSP_DI MSPIo = {0};
/* Private functions--------------------------------------------------------------------------*/


/***********************************************************************************************
* Function		: BSP_RLCWrite
* Description	: 继电器控制。
* Input			: num:继电器号(0 ~ BSPRLC_MAX_NUM-1)
				  state:输出状态_BSPRLC_STATE
				  	BSPRLC_STATE_CLOSE		关
					BSPRLC_STATE_OPEN		开
					BSPRLC_STATE_OVERTURN	翻转
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月5日
***********************************************************************************************/
void BSP_IOClose(INT8U num)
{
	if(num >= IO_MAX)
		return;
	GPIO_WriteBit(RLC_HD_Table[num].GPIOx,RLC_HD_Table[num].CtlPout,Bit_RESET);
	RLC_Control.RLCState &= (~(0x00000001 << num));
}
void BSP_IOOpen(INT8U num)
{
	if(num >= IO_MAX)
		return;
	GPIO_WriteBit(RLC_HD_Table[num].GPIOx,RLC_HD_Table[num].CtlPout,Bit_SET);
	RLC_Control.RLCState |= (0x00000001 << num);
}

INT8U BSP_GetIOState(INT8U num)
{
	if(num >= IO_MAX)
		return 0;
	if(RLC_Control.RLCState & (0x00000001 << num) )
	{
		return 1;
	}
	return 0;
}

void BSP_IOTurnover(INT8U num)
{

	if(num >= IO_MAX)
		return;
		if(RLC_Control.RLCState & (0x00000001 << num))		// 如果已经是开着,那么关掉
		BSP_IOClose(num);
	else
		BSP_IOOpen(num);
}
void BSP_RLCWrite(INT8U num,_BSPRLC_STATE state)
{
	if(num >= IO_MAX)
		return;
	switch(state)
	{
		case BSPRLC_STATE_OPEN:
			BSP_IOOpen(num);
			break;
		case BSPRLC_STATE_CLOSE:
			BSP_IOClose(num);
			break;
		case BSPRLC_STATE_OVERTURN:
			BSP_IOTurnover(num);
		default:
			break;
	}
}





/***********************************************************************************************
* Function		: BSP_RLCInit
* Description	: 驱动初始化
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年7月5日
***********************************************************************************************/
void BSP_IOInit(void)
{
	INT8U i;
	GPIO_InitTypeDef 	GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |RCC_AHB1Periph_GPIOC |RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE, ENABLE);
			/*设置引脚模式为输出模式*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;   
    
    /*设置引脚的输出类型为推挽输出*/
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    
    /*设置引脚为上拉模式*/
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

		/*设置引脚速率为2MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
	for(i=0;i<IO_MAX;i++)
	{
		
		// 控制脚
		GPIO_InitStructure.GPIO_Pin = RLC_HD_Table[i].CtlPout;


		GPIO_Init(RLC_HD_Table[i].GPIOx,&GPIO_InitStructure);
		BSP_IOClose(i);			//初始化关闭
	}
	
	GPIO_SetBits(GPIOD,GPIO_Pin_7); //打印485使能
}


/***********************************************************************************************
* Function		: BSP_OpenPG1
* Description	: 绝缘检测
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenPG1(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOOpen(IO_JY_PGA);
	}
	else
	{
		BSP_IOOpen(IO_JY_PGB);
	}
}

/***********************************************************************************************
* Function		: BSP_ClosePG1
* Description	:  绝缘检测
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_ClosePG1(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOClose(IO_JY_PGA);
	}
	else
	{
		BSP_IOClose(IO_JY_PGB);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenNG2
* Description	: 绝缘检测
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenNG2(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOOpen(IO_JY_NGA);
	}
	else
	{
		BSP_IOOpen(IO_JY_NGB);
	}
}

/***********************************************************************************************
* Function		: BSP_CloseNG2
* Description	:  绝缘检测
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_CloseNG2(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOClose(IO_JY_NGA);
	}
	else
	{
		BSP_IOClose(IO_JY_NGB);
	}
}


/***********************************************************************************************
* Function		: BSP_OpenLOCK
* Description	: 打开电子锁
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenLOCK(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOOpen(IO_RLY_LOCKA);
	}
	else
	{
		BSP_IOOpen(IO_RLY_LOCKB);
	}
}

/***********************************************************************************************
* Function		: BSP_CloseLOCK
* Description	: 关闭电子锁
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_CloseLOCK(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOClose(IO_RLY_LOCKA);
	}
	else
	{
		BSP_IOClose(IO_RLY_LOCKB);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 打开K1K2
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenK1K2(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		MSPIo.SWDC_A_State = 1;
		BSP_IOOpen(IO_RLY_K1K2A);
	}
	else
	{
		MSPIo.SWDC_B_State = 1;
		BSP_IOOpen(IO_RLY_K1K2B);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 关闭K1K2
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_CloseK1K2(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		MSPIo.SWDC_A_State = 0;
		BSP_IOClose(IO_RLY_K1K2A);
	}
	else
	{
		MSPIo.SWDC_B_State = 0;
		BSP_IOClose(IO_RLY_K1K2B);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 打开运行灯
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenRunLed(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOClose(IO_LEDA_G);
	}
	else
	{
		BSP_IOClose(IO_LEDB_G);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 关闭运行灯
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_CloseRunLed(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOOpen(IO_LEDA_G);
	}
	else
	{
		BSP_IOOpen(IO_LEDB_G);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 打开故障灯
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenFailLed(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOClose(IO_LEDA_Y);
	}
	else
	{
		BSP_IOClose(IO_LEDB_Y);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 关闭故障灯
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_CloseFailLed(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOOpen(IO_LEDA_Y);
	}
	else
	{
		BSP_IOOpen(IO_LEDB_Y);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 打开告警灯
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenWarnLed(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOClose(IO_LEDA_R);
	}
	else
	{
		BSP_IOClose(IO_LEDB_R);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 关闭告警灯
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_CloseWarnLed(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		BSP_IOOpen(IO_LEDA_R);
	}
	else
	{
		BSP_IOOpen(IO_LEDB_R);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 打开辅助电源
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_OpenBMSPower(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		#if(USER_GUN != USER_SINGLE_GUN)
		BSP_IOOpen(IO_RLY_BMSPOWERA);
		#else
		BSP_IOOpen(IO_RLY_BMSPOWERA);   //辅助电源正
		BSP_IOOpen(IO_RLY_RES1);		 //辅助电源负
		#endif
	}
	else
	{
		BSP_IOOpen(IO_RLY_BMSPOWERB);
	}
}

/***********************************************************************************************
* Function		: BSP_OpenBMSPower
* Description	: 关闭辅助电源
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2021年7月14日
***********************************************************************************************/
void BSP_CloseBMSPower(INT8U gun)
{
	if(gun >= 2)
	{
		return;
	}
	if(gun == 0)
	{
		#if(USER_GUN != USER_SINGLE_GUN)
		BSP_IOClose(IO_RLY_BMSPOWERA);
		#else
		BSP_IOClose(IO_RLY_BMSPOWERA);   //辅助电源正
		BSP_IOClose(IO_RLY_RES1);		 //辅助电源负
		#endif
	

	}
	else
	{
		BSP_IOClose(IO_RLY_BMSPOWERB);
	}
}
/************************(C)COPYRIGHT 2020汇誉科技*****END OF FILE****************************/
