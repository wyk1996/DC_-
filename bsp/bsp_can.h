/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_can.h
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BSP_CAN_H_
#define	__BSP_CAN_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#include "bsp_conf.h" 
#include "stm32f4xx_can.h"
/* Private define-----------------------------------------------------------------------------*/
//波特率设置，目前只做2个波特率
#define BSP_CAN_BAUNDRATE_125KBPS   24u   
#define BSP_CAN_BAUNDRATE_250KBPS   12u
//控制结构体
#define BSPCANTxMsg     CanTxMsg	//重新定义Can内部发送控制结构体，外部用户调用此
#define BSPCANRxMsg		CanRxMsg 	//重新定义Can内部接收控制结构体，外部用户调用此
#define MAX_CAN_SIZE    10			//接收最大队列数,该配置放入BSP_CAN_Set,由外部配置,先预留
/* Private typedef----------------------------------------------------------------------------*/
#ifdef __BSP_CAN_C_
// 引脚控制结构
typedef struct
{
	CAN_TypeDef *CANx;
	GPIO_TypeDef *GPIOx;
	INT16U Port_Pin_Tx;
	INT16U Port_Pin_Rx;
}_BSPCAN_HD;
const _BSPCAN_HD CAN_HD_Table[]=
{
	{CAN1,GPIOB,BIT(9),BIT(8)},
	{CAN2,GPIOB,BIT(6), BIT(5) },
	{CAN3,GPIOB,BIT(4), BIT(3) },
};
#endif	//__BSP_CAN_C_



typedef struct STRUCT_CAN_SET
{
    INT8U        BaundRate;                // 波特率
	BSPCANTxMsg  *TxMessage;               // 发送配置
	BSPCANRxMsg  *RxMessage;               // 接收配置，CanRxMsg结构数组形式
    _BSP_MESSAGE *BspMsg;                  // 消息实体指针  
	INT8U     RxMaxNum;                    // 接收元素最大个数，CanRxMsg数组大小值
}BSP_CAN_Set;

typedef struct CANCONTROL
{
//    CanRxMsg Elem[MAX_CAN_SIZE];
	BSP_CAN_Set *pCanRxElem;           //  接收元素控制体
    INT16U front;					   //接收存储头
    INT16U rear;					   //接收存储尾
	OS_EVENT      *MailOrQueue;
// 	_BSP_MESSAGE  BspMsg;

}_BSPCAN_Control;
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*******************************************************************************
* Function Name  : BSP_CAN_GPIOInit(void)
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BSP_CAN_GPIOInit(INT8U num);

/***********************************************************************************************
* Function		: BSP_CAN_Init
* Description	: can外部调用初始化
* Input			: 
* Output		: 
* Note(s)		: BSP_CAN_Set 里或许该吧波特率的配置也做上，由于不知道can现场使用是否会碰见多种波特率
                  暂时波特率固定的方式。在CAN_Configuration里直接配置好。配置方式见中文说明
* Contributor	: 2018年5月25日
***********************************************************************************************/
void  BSP_CAN_Init(INT8U num,BSP_CAN_Set *pbxCAN,OS_EVENT *Mail_Queue);

/***********************************************************************************************
* Function		:BSP_CANTx_IRQHandler 
* Description	:can发送中断处理函数 
* Input			: 
* Output		: 
* Note(s)		: 非外部用户调用
* Contributor	: 2018年5月25日
***********************************************************************************************/
void  BSP_CANTx_IRQHandler(INT8U num);

/***********************************************************************************************
* Function		:BSP_CANRx_IRQHandler 
* Description	:can接收中断处理函数 
* Input			: 
* Output		: 
* Note(s)		: 非外部用户调用
* Contributor	: 2018年5月25日
***********************************************************************************************/
void  BSP_CANRx_IRQHandler(INT8U num);

/***********************************************************************************************
* Function		: BSP_CAN_Write
* Description	: 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	:2018年5月25日
***********************************************************************************************/
INT8U BSP_CAN_Write(INT8U num,BSP_CAN_Set *CANTxSet);
#endif	//__BSP_CAN_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
