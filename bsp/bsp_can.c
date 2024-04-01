/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bsp_can.c
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
#define __BSP_CAN_C_
/* Includes-----------------------------------------------------------------------------------*/
#include "stm32f4xx_gpio.h"
#include "bsp_can.h"
/* Private define-----------------------------------------------------------------------------*/
#define MAX_MAIL_NUM        3  // 邮箱个数
#define MAX_CAN_NUM         3  // CAN的个数
/* Private typedef----------------------------------------------------------------------------*/
_BSPCAN_Control BSPCAN_Control[MAX_CAN_NUM];   
static INT16U RxBuffPosition[MAX_CAN_NUM]={0};	          // 位置
static INT8U NextIsRearPosition[MAX_CAN_NUM]={0};	          // 上一个位置
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static INT8U BSPCAN_MSG_Num[MAX_CAN_NUM][MAX_MAIL_NUM]={0};   // 发送邮箱标记
/* Private function prototypes----------------------------------------------------------------*/
void CAN_Configuration(INT8U num);
INT8U IsEmptyFormation(INT8U num);
INT8U IsFullBSPCAN_Control(INT8U num);
void SetHeadBSPCAN_Control(INT16U head);
INT8U GetFormation(INT8U num,INT16U head,CanRxMsg *element);
void BSP_CanDataProcess(INT8U num);
/* Private functions--------------------------------------------------------------------------*/

#define CAN1_CLK                    RCC_APB1Periph_CAN1
#define CAN1_RX_PIN                 GPIO_Pin_8
#define CAN1_TX_PIN                 GPIO_Pin_9
#define CAN1_GPIO_TX_PORT           GPIOB
#define CAN1_GPIO_RX_PORT           GPIOB
#define CAN1_GPIO_TX_CLK            RCC_AHB1Periph_GPIOB
#define CAN1_GPIO_RX_CLK            RCC_AHB1Periph_GPIOB
#define CAN1_AF_PORT                GPIO_AF8_CAN1
#define CAN1_RX_SOURCE              GPIO_PinSource8
#define CAN1_TX_SOURCE              GPIO_PinSource9


#define CAN2_CLK                    RCC_APB1Periph_CAN2
#define CAN2_RX_PIN                 GPIO_Pin_5
#define CAN2_TX_PIN                 GPIO_Pin_6
#define CAN2_GPIO_TX_PORT           GPIOB
#define CAN2_GPIO_RX_PORT           GPIOB
#define CAN2_GPIO_TX_CLK            RCC_AHB1Periph_GPIOB
#define CAN2_GPIO_RX_CLK            RCC_AHB1Periph_GPIOB
#define CAN2_AF_PORT                GPIO_AF_CAN2
#define CAN2_RX_SOURCE              GPIO_PinSource5
#define CAN2_TX_SOURCE              GPIO_PinSource6


#define CAN3_CLK                    RCC_APB1Periph_CAN3
#define CAN3_RX_PIN                 GPIO_Pin_3
#define CAN3_TX_PIN                 GPIO_Pin_4
#define CAN3_GPIO_TX_PORT           GPIOB
#define CAN3_GPIO_RX_PORT           GPIOB
#define CAN3_GPIO_TX_CLK            RCC_AHB1Periph_GPIOB
#define CAN3_GPIO_RX_CLK            RCC_AHB1Periph_GPIOB
#define CAN3_AF_PORT                GPIO_AF11_CAN3
#define CAN3_RX_SOURCE              GPIO_PinSource3
#define CAN3_TX_SOURCE              GPIO_PinSource4
/*******************************************************************************
* Function Name  : BSP_CAN_GPIOInit(void)
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BSP_CAN_GPIOInit(INT8U num)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if(num == 0) //CAN1
    {
		  /* Enable GPIO clock */
		RCC_AHB1PeriphClockCmd(CAN1_GPIO_TX_CLK | CAN1_GPIO_RX_CLK, ENABLE);

		/* Connect CAN pins to AF8 */
		GPIO_PinAFConfig(CAN1_GPIO_RX_PORT, CAN1_RX_SOURCE, CAN1_AF_PORT);
		GPIO_PinAFConfig(CAN1_GPIO_TX_PORT, CAN1_TX_SOURCE, CAN1_AF_PORT);

		/* Configure CAN TX pins */
		GPIO_InitStructure.GPIO_Pin = CAN1_TX_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
		GPIO_Init(CAN1_GPIO_TX_PORT, &GPIO_InitStructure);

		/* Configure CAN RX  pins */
		GPIO_InitStructure.GPIO_Pin = CAN1_RX_PIN ;
		GPIO_Init(CAN1_GPIO_RX_PORT, &GPIO_InitStructure);
	}
	else if(num == 1) //CAN2
	{

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(CAN2_GPIO_TX_CLK | CAN2_GPIO_RX_CLK, ENABLE);

	/* Connect CAN pins to AF9 */
    GPIO_PinAFConfig(CAN2_GPIO_RX_PORT, CAN2_RX_SOURCE, CAN2_AF_PORT);
    GPIO_PinAFConfig(CAN2_GPIO_TX_PORT, CAN2_TX_SOURCE, CAN2_AF_PORT);

	/* Configure CAN TX pins */
    GPIO_InitStructure.GPIO_Pin = CAN2_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		//
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;		//
    GPIO_Init(CAN2_GPIO_TX_PORT, &GPIO_InitStructure);

	/* Configure CAN RX  pins */
    GPIO_InitStructure.GPIO_Pin = CAN2_RX_PIN ;
    GPIO_Init(CAN2_GPIO_RX_PORT, &GPIO_InitStructure);
	}
	else
	{

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(CAN3_GPIO_TX_CLK | CAN3_GPIO_RX_CLK, ENABLE);

	/* Connect CAN pins to AF11 */
    GPIO_PinAFConfig(CAN3_GPIO_RX_PORT, CAN3_RX_SOURCE, CAN3_AF_PORT);
    GPIO_PinAFConfig(CAN3_GPIO_TX_PORT, CAN3_TX_SOURCE, CAN3_AF_PORT);

	/* Configure CAN TX pins */
    GPIO_InitStructure.GPIO_Pin = CAN3_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(CAN3_GPIO_TX_PORT, &GPIO_InitStructure);

	/* Configure CAN RX  pins */
    GPIO_InitStructure.GPIO_Pin = CAN3_RX_PIN ;
    GPIO_Init(CAN3_GPIO_RX_PORT, &GPIO_InitStructure);
	}
	
}
/*******************************************************************************
* Function Name  : CAN_Configuration
* Description    : Configures the CAN.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
void CAN_Configuration(INT8U num)
{
	CAN_InitTypeDef        CAN_InitStructure;
	/************************CAN通信参数设置**********************************/

	RCC_APB1PeriphClockCmd(CAN1_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(CAN2_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(CAN3_CLK, ENABLE);
	/* CAN register init */
	CAN_DeInit(CAN_HD_Table[num].CANx);
	//  CAN_StructInit(&CAN_InitStructure);
	
	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM=DISABLE;//禁止时间触发通信模式

    CAN_InitStructure.CAN_ABOM =  ENABLE;
	//11CAN_InitStructure.CAN_ABOM=DISABLE;//，软件对CAN_MCR寄存器的INRQ位进行置1随后清0后，一旦硬件检测
	                                   //到128次11位连续的隐性位，就退出离线状态。
	CAN_InitStructure.CAN_AWUM=DISABLE;//睡眠模式通过清除CAN_MCR寄存器的SLEEP位，由软件唤醒

	CAN_InitStructure.CAN_NART=DISABLE; //ENABLE;CAN报文只被发送1次，不管发送的结果如何（成功、出错或仲裁丢失）
	
    CAN_InitStructure.CAN_RFLM=DISABLE;//在接收溢出时FIFO未被锁定，当接收FIFO的报文未被读出，下一个收到的报文会覆盖原有
	                                   //的报文
	CAN_InitStructure.CAN_TXFP=DISABLE;//发送FIFO优先级由报文的标识符来决定
	
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;  	   //正常工作模式
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;		   //BTR-SJW 重新同步跳跃宽度 1个时间单元

	/* ss=1 bs1=8 bs2=7 位时间宽度为(1+8+7) 波特率即为时钟周期tq*(1+8+7)  */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq;		   //BTR-TS1 时间段1 占用了8个时间单元
	CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;		   //BTR-TS1 时间段2 占用了7个时间单元

	/* (CAN 时钟频率为 APB1 = 42 MHz) */
	CAN_InitStructure.CAN_Prescaler =BSPCAN_Control[num].pCanRxElem->BaundRate;		   //BTR-BRP 波特率分频器  定义了时间单元的时间长度 48/(1+8+7)/24=125 kbps
	
	if(num == 0)
	{
		CAN_Init(CAN1, &CAN_InitStructure);
		CAN1->FMR = 0x00000E01;	//过滤器组工作在初始化模式
		CAN1->FA1R &= ~(1<<0);	//过滤器 0 不激活
		CAN1->FS1R |= 1<<0;		//过滤器位宽为 32 位
		CAN1->FM1R |= 0<<0;		//过滤器 0 工作在标识符屏蔽位模式
		CAN1->FFA1R |= 0<<0;	//过滤器 0 关联到 FIFO0
		CAN1->sFilterRegister[0].FR1 = 0x00000000;//32 位 ID
		CAN1->sFilterRegister[0].FR2 = 0x00000000;//32 位 MASK
		CAN1->FA1R |= 1<<0;		//激活过滤器 0
		CAN1->FMR &= 0<<0;		//过滤器组进入正常模式
	}
	else if(num == 1)
	{
		CAN_Init(CAN2, &CAN_InitStructure);
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;
		CAN_FilterInitStructure.CAN_FilterNumber = 14;					   
		CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	    
		CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	
		CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;		       
		CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;                 
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;			  
		CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;			    
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
		CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;			   
		CAN_FilterInit(CAN2, &CAN_FilterInitStructure);
	}
	else
	{
			CAN_Init(CAN3, &CAN_InitStructure);
			CAN_FilterInitTypeDef  CAN_FilterInitStructure;
			CAN_FilterInitStructure.CAN_FilterNumber = 8;					   
			CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;	    
			CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	
			CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;		       
			CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;                 
			CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;			  
			CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;			    
			CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
			CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;			   
			CAN_FilterInit(CAN3, &CAN_FilterInitStructure);
	}

}


/***********************************************************************************************
* Function		: BSP_CAN_Init
* Description	: 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	:2018年5月25日
***********************************************************************************************/
void  BSP_CAN_Init(INT8U num,BSP_CAN_Set *pbxCAN,OS_EVENT *Mail_Queue)
{  
	if(pbxCAN == NULL || Mail_Queue == NULL)
	{
		return;
	}
	BSPCAN_Control[num].pCanRxElem = pbxCAN ;
	
	/*消息设定初始化*/
	BSPCAN_Control[num].MailOrQueue =  Mail_Queue;
 	

	/*can的终端使能配置*/

		//管脚硬件初始化完毕，进行CAN配置
	//CAN0必须先初始化
		BSP_CAN_GPIOInit(0);//硬件初始化
		CAN_Configuration(0);
		CAN_ITConfig(CAN1,CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);  		// fifo0中断
		//CAN_ITConfig(CAN_HD_Table[num].CANx,CAN_IT_FMP1 | CAN_IT_FF1 | CAN_IT_FOV1, ENABLE);  	// fifo1中断
		CAN_ITConfig(CAN1,CAN_IT_TME, DISABLE);                             		// 发送中断
		CAN_ITConfig(CAN1,CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC| CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);  // ERR中断

		if(num != 0)
		{
				BSP_CAN_GPIOInit(num);//硬件初始化
				CAN_Configuration(num);
				/*can的终端使能配置*/
				CAN_ITConfig(CAN_HD_Table[num].CANx,CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);  // fifo0中断
				//CAN_ITConfig(CAN_HD_Table[i].CANx,CAN_IT_FMP1 | CAN_IT_FF1 | CAN_IT_FOV1, ENABLE);  // fifo1中断
				CAN_ITConfig(CAN_HD_Table[num].CANx,CAN_IT_TME, DISABLE);                             // 发送中断
				CAN_ITConfig(CAN_HD_Table[num].CANx,CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC| CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);  // ERR中断
		}
	// CAN缓存初始化
 	memset(&BSPCAN_MSG_Num[num][0],0,sizeof(BSPCAN_MSG_Num)/MAX_CAN_NUM);
	
}

/***********************************************************************************************
* Function		: BSP_CAN_Write
* Description	: CAN发送数据函数
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	:2018年5月25日
***********************************************************************************************/
INT8U BSP_CAN_Write(INT8U num,BSP_CAN_Set *CANTxSet)
{
    INT8U TransmitMailbox = 0;
#if OS_CRITICAL_METHOD == 3    /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr = 0;
#endif	
	
	OS_ENTER_CRITICAL();
	    /* transmit */
    TransmitMailbox = CAN_Transmit(CAN_HD_Table[num].CANx,CANTxSet->TxMessage);
    if(CAN_NO_MB == TransmitMailbox) 
    {
         OS_EXIT_CRITICAL();            								//
		 return FALSE;
    }
	if(1 == TransmitMailbox)					//测试是否使用了第二个邮箱
	{
		NOP();
	}
//    else
//    {
//         BSPCAN_MSG_Num[num][TransmitMailbox] = 1;
//    }
 //   CAN_ITConfig(CAN_HD_Table[num].CANx,CAN_IT_TME, ENABLE);
	OS_EXIT_CRITICAL();            								//
	
    return TRUE;
}
/***********************************************************************************************
* Function		:BSP_CANTx_IRQHandler 
* Description	:can发送中断处理函数 
* Input			: 
* Output		: 
* Note(s)		: 非外部用户调用
* Contributor	: 2018年5月25日
***********************************************************************************************/
void  BSP_CANTx_IRQHandler(INT8U num)
{

	if(SET == CAN_GetITStatus(CAN_HD_Table[num].CANx,CAN_IT_TME))     //发送是否完成
	{
		CAN_ClearITPendingBit(CAN_HD_Table[num].CANx,CAN_IT_TME);      //清标志位
		//CAN_ITConfig(CAN_HD_Table[num].CANx,CAN_IT_TME, DISABLE);	  
		//BSPCAN_MSG_Num[num][0] = 0;
	}
}
/***********************************************************************************************
* Function		:BSP_CANRx_IRQHandler 
* Description	:can接收中断处理函数 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018年5月25日
***********************************************************************************************/
void  BSP_CANRx_IRQHandler(INT8U num)
{
	CanRxMsg RxMessage;

	
	if(SET == CAN_GetITStatus(CAN_HD_Table[num].CANx,CAN_IT_FF0))
	{
		CAN_ClearITPendingBit(CAN_HD_Table[num].CANx,CAN_IT_FF0);
	}
	else if(SET == CAN_GetITStatus(CAN_HD_Table[num].CANx,CAN_IT_FOV0))
	{
		CAN_ClearITPendingBit(CAN_HD_Table[num].CANx,CAN_IT_FOV0);
	}
	else
	{
		CAN_Receive(CAN_HD_Table[num].CANx, CAN_FIFO0, &RxMessage);
		/*插入接收数据队列*/
	    if( BSPCAN_Control[num].front == ((BSPCAN_Control[num].rear+1) % BSPCAN_Control[num].pCanRxElem->RxMaxNum))//是否为满
	    {
			NOP();
	        return;
	    }    
	    else //队列空
	    {
	        memcpy(&BSPCAN_Control[num].pCanRxElem->RxMessage[BSPCAN_Control[num].rear],&RxMessage,sizeof(CanRxMsg));
	        BSPCAN_Control[num].rear = ((BSPCAN_Control[num].rear + 1) %  BSPCAN_Control[num].pCanRxElem->RxMaxNum);//MAX_CAN_SIZE;        
	    }
	}
	if(NextIsRearPosition[num] == 0x55)
	{
		RxBuffPosition[num] = BSPCAN_Control[num].pCanRxElem->RxMaxNum;
		NextIsRearPosition[num] = 0;
	}
	else
		RxBuffPosition[num] = BSPCAN_Control[num].rear;	
	BSP_CanDataProcess(num);  	
}
/***********************************************************************************************
* Function		: BSP_GetRxBuffPosotion
* Description	: 获取本次接收的can数据再哪一个位置
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 10/12/2020 		叶喜雨
***********************************************************************************************/
INT16U BSP_GetRxBuffPosotion(INT8U num)
{
	return RxBuffPosition[num]-1;;
}

/***********************************************************************************************
* Function		: GetFormation
* Description	: 取对头
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/
INT8U GetFormation(INT8U num,INT16U head,CanRxMsg *element)
{
   
	if(head != BSPCAN_Control[num].rear) //到队列尾
	{
		memcpy(element,&BSPCAN_Control[num].pCanRxElem->RxMessage[head],sizeof(CanRxMsg));//得到数据    
		return 1;
	}
	else
	{
		return 0;  //无指定对头数据
	}        
}
/***********************************************************************************************
* Function		: BSP_CanDataProcess
* Description	:
* Input			: 
* Output		: 
* Note(s)		:
* Contributor	: 2018年5月25日
***********************************************************************************************/
void BSP_CanDataProcess(INT8U num)
{
  INT8U pos;
    INT16U head;
    CanRxMsg RxMessage;  
//    CanTxMsg TxMessage;
    _BSP_MESSAGE *TmpMsg;

    pos = BSP_GetRxBuffPosotion(num);
//	TmpMsg = &BSPCAN_Control[num].BspMsg;
    TmpMsg = &BSPCAN_Control[num].pCanRxElem->BspMsg[pos];    

    head = BSPCAN_Control[num].front;
    if(1 == GetFormation(num,head,&RxMessage))
    {
        head = (head + 1) %  BSPCAN_Control[num].pCanRxElem->RxMaxNum;// MAX_CAN_SIZE;     //²éÑ¯Í·Ç°¹ö
		/*重新设置队列头指针*/
	    if(BSPCAN_Control[num].front != BSPCAN_Control[num].rear)
	    {
	        BSPCAN_Control[num].front = head;
	    }
		/*填充消息*/			
		TmpMsg->MsgID  = BSP_MSGID_CAN_RXOVER;
//		TmpMsg->DivNum = 0x00;
		TmpMsg->DivNum = BSP_GetRxBuffPosotion(num);
		TmpMsg->pData  = &RxMessage.Data[0];
		TmpMsg->DataLen= 8;

		if(BSPCAN_Control[num].MailOrQueue->OSEventType == OS_EVENT_TYPE_Q )
			OSQPost(BSPCAN_Control[num].MailOrQueue, TmpMsg);
		else if(BSPCAN_Control[num].MailOrQueue->OSEventType == OS_EVENT_TYPE_MBOX )
			OSMboxPost(BSPCAN_Control[num].MailOrQueue, TmpMsg);
        
    }
	if(BSPCAN_Control[num].front == BSPCAN_Control[num].pCanRxElem->RxMaxNum-1)
		NextIsRearPosition[num] = 0x55;
 
}
/************************(C)COPYRIGHT 2018 杭州汇誉****END OF FILE****************************/
