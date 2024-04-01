/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------历史版本---------------------------------------------
* FileName			: card_cpu.c
* Author			:
* Date First Issued	:
* Version			: V1.0
* Description		:
*----------------------------------------历史版本-------------------------------------------
* History			:
* //2013	        :
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
//周立功M1卡

#include "bsp_conf.h"
#include "bsp_uart.h"
#include 	"sysconfig.h"
#include "card_Interface.h"
#include "Des.h"
#include "Disinterface.h"
#include "waittimeout.h"
#include "DisplayMain.h"
#include "4GMain.h"
#include "bsp.h"
/* Private define-----------------------------------------------------------------------------*/
#define    CARD_UART                   USART6     
#define    CARD_SEND_BUF_LEN          (100u)     
#define    CARD_RECV_BUF_LEN          (100u)           
#define    CARD_Q_LEN                  20
#define    CARDRec_Q_LEN                  15
#define    CARDFunction_LEN               10
#define    CARDGreyLock_CNT               5
/* Private macro------------------------------------------------------------------------------*/
static void *CPUCardOSQ[CARD_Q_LEN];	
static void *CardReciveOSQ[CARDRec_Q_LEN];	
static OS_EVENT *CPUCardTaskEvent;	          //用于接收别的任务发来的消息
static OS_EVENT *CardReciveEvent;             //用于串口接收
static INT8U CPUCardRecvSMem[CARD_Q_LEN * sizeof(_BSP_MESSAGE)]; 

static INT8U CPUCardSendBuf[CARD_SEND_BUF_LEN];           
INT8U  CPUCardRecvBuf[CARD_RECV_BUF_LEN];
INT8U  CPUCardTempBuf[CARD_RECV_BUF_LEN];


#define KEY_A     0x60
#define KEY_B     0x61
#define CARD_CMD				0x02			//命令
#define CARD_SUCCESS_STATUS  	0x00			//状态
#define BLOCK_POSSWORD			4				//密码块
#define BLOCK_STATUS			5				//卡状态块
#define BLOCK_STATUS1			9				//卡状态块备份1
#define BLOCK_STATUS2			14				//卡状态块备份2
#define BLOCK_STARTMONEY		10				//开始充电的时候写入一次。  等待下一次充电判断金额是否变化，若金额发生了变化，则判断为卡已经解锁
#define BLOCK_MONEY				6				//金额块
#define BLOCK_MONEY1			8				//金额块备份1
#define BLOCK_MONEY2			13				//金额块备份2
/* Private typedef----------------------------------------------------------------------------*/
SYSCARDINFO     SysCardInfo;        //系统参数，只允许本任务写
USERCARDINFO    UserCardInfo;       //临时用户卡信息
USERCARDINFO    A_UserCardInfo;     //A枪用户卡信息
USERCARDINFO    B_UserCardInfo;     //B枪用户卡信息
RECEIVEDATA     A_ReceiveData;        //用于接收来自别的消息的数据
RECEIVEDATA     B_ReceiveData;        //用于接收来自别的消息的数据
RECEIVEDATA     Query_ReceiveData;    //用于接收来自别的消息的数据
INT8U Dispose_LocalUnLock(_GUN_NUM gun);
INT32U StartChargeMoney = 0;   //开始充电的时候写入一次。  等待下一次充电判断金额是否变化，若金额发生了变化，则判断为卡已经解锁
/* Private variables--------------------------------------------------------------------------*/
INT8U Key[6] = {0x11,0x11,0x11,0x11,0x11,0x11};

//2次读取的数据一致才算成功
static INT8U ReadData1[6] = {0};		
static INT8U ReadData2[6] = {0};
/*****************************************************************************
* Function     : CPUCardInit
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月1日  YXY
*****************************************************************************/
void Card_Init(void)
{
	  CPUCardTaskEvent = OSQCreate(CPUCardOSQ, CARD_Q_LEN);     //控制消息创建
    if (CPUCardTaskEvent == NULL)
    {
        OSTaskSuspend(OS_PRIO_SELF);   
    }
		
		memset(&SysCardInfo, 0, sizeof(SYSCARDINFO));       //结构体初始化
		memset(&UserCardInfo, 0, sizeof(USERCARDINFO));		
		memset(&A_UserCardInfo, 0, sizeof(USERCARDINFO));
		memset(&B_UserCardInfo, 0, sizeof(USERCARDINFO));
		memset(&A_ReceiveData, 0, sizeof(RECEIVEDATA));
		memset(&B_ReceiveData, 0, sizeof(RECEIVEDATA));
}

/*****************************************************************************
* Function     : GetCardWorkStatus
* Description  : 获取卡任务当前工作状态：空闲 工作
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
CARDWORK_STATUS GetCardWorkStatus(void)
{
	  return SysCardInfo.Card_Status;
}


/*****************************************************************************
* Function     : GetCardInfo
* Description  : 获取相应命令对应的信息
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
USERCARDINFO* GetCardInfo(void)
{
	return &UserCardInfo;
}


/*****************************************************************************
* Function     : GetCardInfo
* Description  : 获取相应命令对应的信息
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 20200629
*****************************************************************************/
USERCARDINFO* GetGunCardInfo(_GUN_NUM gun)
{
	if(gun == GUN_A)
	{
		return &A_UserCardInfo;
	}
	else
	{
		return &B_UserCardInfo;
	}
}

/***********************************************************************************************
* Function      : CardUart_MsgFree
* Description   : 多余串口消息释放
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2018-06-03 yxy
***********************************************************************************************/
INT8U  CardUart_MsgFree(void)
{
	_BSP_MESSAGE *pMsg;  
	INT8U err;
	while(1)
	{
		pMsg = OSQPend(CardReciveEvent, SYS_DELAY_1ms, &err);
		if (OS_ERR_NONE == err && (pMsg->MsgID == BSP_MSGID_UART_RXOVER)) //接收到串口数据
		{				
		
			if (UART_MsgDeal(&pMsg, CPUCardTempBuf, sizeof(CPUCardTempBuf)) == TRUE)
			{	
				UART_FreeOneMsg(CARD_UART, pMsg);
			}
		}else{
			break;
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function     : GetCardInfoStatus
* Description  : 获取卡信息是否被读取
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月3日  叶喜雨
*****************************************************************************/
GETCARD_INFO GetCardInfoStatus(void)
{
	  return SysCardInfo.Get_Info;	
}
/*****************************************************************************
* Function     : SetCardInfoStatus
* Description  : 设置卡信息获取状态：获取  未获取
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月20日  叶喜雨
*****************************************************************************/
void SetCardInfoStatus(GETCARD_INFO state)
{
	SysCardInfo.Get_Info = state;
}
/*****************************************************************************
* Function     : SetCardWorkStatus
* Description  : 设置卡任务当前工作状态
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年8月3日  YXY
*****************************************************************************/
void SetCardWorkStatus(CARDWORK_STATUS state)
{
	  SysCardInfo.Card_Status = state;
}


/*********************************************************************************************************
* Function     : DispMsg_Send
* Description  : 给显示任务发送消息
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200702
*********************************************************************************************************/	
static void DispMsg_Send(_SEND_TO_CARD_CMD cmd)
{
	static _BSP_MESSAGE message[CARDCMD_MAX];
	OS_EVENT* pevent = APP_GetDispEvent();
						 							 

	 message[cmd].MsgID = BSP_MSGID_CARD;	
	 message[cmd].DivNum = cmd;
	 message[cmd].pData = (INT8U*)&SysCardInfo.Card_Result;
	 message[cmd].DataLen = sizeof(SysCardInfo.Card_Result);
	 OSQPost(pevent, &message[cmd]);				
}
/****************************************************************************
* Function     : Password_Judge
* Description  : 密码判断
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
//static INT8U Password_Judge(_GUN_NUM gun)
//{
//	INT8U password[6],i;
//	//密码-C3  123456(C4 C5 C6 C7 C8 C9)
//	for(i = 0;i < 6;i++)
//	{
//		if(UserCardInfo.CardCode[i] < 0xC3 ||  UserCardInfo.CardCode[i] > (0xC3 + 9))
//		{
//			SysCardInfo.Card_Result = _FailNotIssuer;				 //非发行方卡	
//			return FALSE;
//			//密码格式有误
//		}
//		password[i] = UserCardInfo.CardCode[i] - 0xC3 + '0';
//	}
//	//比较密码是否一致
//	if(gun == GUN_A)
//		
//	{
//		if(strncmp((const char* )password,(const char *)A_ReceiveData.password,6) == 0)
//		{
//			return TRUE;
//		}	
//	}else if(gun == GUN_B)
//	{
//		if(strncmp((const char *)password,(const char *)B_ReceiveData.password,6) == 0)
//		{
//			return TRUE;
//		}	
//	}else{
//		if(strncmp((const char *)password,(const char *)Query_ReceiveData.password,6) == 0)
//		{
//			return TRUE;
//		}	
//	}
//	SysCardInfo.Card_Result = _FailPIN;				 //密码错误
//	return FALSE;
//}


/****************************************************************************
* Function     : CardNum_Judge
* Description  : 卡号判断
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U CardNum_Judge(INT8U gun)
{
	if(gun == GUN_A)
	{
		if(strncmp((const char* )UserCardInfo.CardID,(const char *)A_UserCardInfo.CardID,4) == 0)
		{
			return TRUE;
		}
	}
	else
	{
		if(strncmp((const char* )UserCardInfo.CardID,(const char *)B_UserCardInfo.CardID,4) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/****************************************************************************
* Function     : Money_Cal
* Description  : 余额计算
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT32U Money_Cal(_GUN_NUM gun)
{
	//设置余额
	INT32U money;
	if(gun == GUN_A)
	{
		if(A_ReceiveData.money > 50000)
		{
			A_ReceiveData.money = 50000;  //一次充电不可能大于500
		}
		if( A_UserCardInfo.balance > A_ReceiveData.money )
		{
			money = A_UserCardInfo.balance - A_ReceiveData.money;
		}
		else
		{
			money = 0;
		}
	}
	else
	{
		if(B_ReceiveData.money > 50000)
		{
			B_ReceiveData.money = 50000;  //一次充电不可能大于500
		}
		if( B_UserCardInfo.balance > B_ReceiveData.money )
		{
			money = B_UserCardInfo.balance - B_ReceiveData.money;
		}
		else
		{
			money = 0;
		}
	}
	return money;
}


/*****************************************************************************
* Function     : CalcBCC
* Description  : 异或计算
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月30日  叶喜雨
*****************************************************************************/
INT8U CalcBCC( INT8U* parastr,INT16U len)
{
	INT16U i;
	INT8U bcc = 0;
	for( i = 0; i < len; i++ )
	{
		bcc ^= parastr[i];
	}
	bcc = ~bcc;
	return bcc;
}


/*****************************************************************************
* Function     : Send_AuthKey
* Description  : 密钥验证
* Input        : 
				keymode:密钥A/密钥B
				pkey :密钥
				cardid：卡号
				block：块

* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200628 叶喜雨
*****************************************************************************/
static INT8U Send_AuthKey(INT8U* pkey,INT8U len)
{
	INT8U buf[21];
	INT8U key[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0x80,0x69,0xff,0xff,0xff,0xff,0xff,0xff};
	
	memcpy(key,pkey,len);
	buf[0] = 0x03;				//命令
	buf[1] = 0x15;				//包长度
	buf[2] = 0xC3;				//命令
	buf[3] = 0x20;				//地址
	memcpy(&buf[4],key,16);
	buf[20] = CalcBCC(buf,20);		
	BSP_UARTWrite(CARD_UART, buf, sizeof(buf));
	OSTimeDly(SYS_DELAY_20ms);	
	return TRUE;
}

/*****************************************************************************
* Function     : Send_WriterBlock
* Description  : 块写入
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200628 叶喜雨
*****************************************************************************/
static INT8U Send_WriterBlock(INT8U num,INT8U* pdata)
{
	INT8U buf[23];
	
	buf[0] = 0x01;				//命令
	buf[1] = 0x17;				//包长度
	buf[2] = 0xA4;				//命令
	buf[3] = 0x20;				//地址
	buf[4] = num;				//块号
	buf[5] = 0x00;				//蜂鸣器
	memcpy(&buf[6],pdata,16);
	buf[22] = CalcBCC(buf,22);	

	BSP_UARTWrite(CARD_UART, buf, sizeof(buf));
	OSTimeDly(SYS_DELAY_20ms);	
	return TRUE;
}

/*****************************************************************************
* Function     : Send_ReadBlock
* Description  : 块读取
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200628 叶喜雨
*****************************************************************************/
static INT8U Send_ReadBlock(INT8U num)
{
	INT8U buf[8];
	
	buf[0] = 0x01;				//命令
	buf[1] = 0x08;				//包长度
	buf[2] = 0xA3;				//命令
	buf[3] = 0x20;				//地址
	buf[4] = num;				//块号
	buf[5] = 0x00;				//蜂鸣器
	buf[6] = 0x00;
	buf[7] = CalcBCC(buf,7);
	BSP_UARTWrite(CARD_UART, buf, sizeof(buf));
	OSTimeDly(SYS_DELAY_10ms);	
	return TRUE;
}


/*****************************************************************************
* Function     : Send_InitWallet
* Description  : 初始化钱包
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200628 叶喜雨
*****************************************************************************/
static INT8U Send_InitWallet(INT8U num,INT32U data)
{
	
	INT8U buf[11];
	
	buf[0] = 0x01;				//命令
	buf[1] = 0x0B;				//包长度
	buf[2] = 0xA6;				//命令
	buf[3] = 0x20;				//地址
	buf[4] = num;				//块号
	buf[5] = 0x00;				//蜂鸣器
	memcpy(&buf[6],&data,4);
	buf[10] = CalcBCC(buf,10);	
	BSP_UARTWrite(CARD_UART, buf, sizeof(buf));
	OSTimeDly(SYS_DELAY_20ms);	
	return TRUE;
}

/*****************************************************************************
* Function     : Send_ActiveCard
* Description  : 获取卡号
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200628 叶喜雨
*****************************************************************************/
static INT8U Send_ActiveCard()
{
	INT8U buf[8];
	
	buf[0] = 0x01;				//命令
	buf[1] = 0x08;				//包长度
	buf[2] = 0xA1;				//命令
	buf[3] = 0x20;				//地址
	buf[4] = 0x00;				
	buf[5] = 0x00;				//蜂鸣器
	buf[6] = 0x00;			
	buf[7] = CalcBCC(buf,7);			
	BSP_UARTWrite(CARD_UART, buf, sizeof(buf));
	OSTimeDly(SYS_DELAY_10ms);
	return TRUE;
}

/***********************************************************************************************
* Function      : WaitCardmessage
* Description   : 等待卡返回
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2018-06-03 yxy
***********************************************************************************************/
INT8U  WaitCardmessage(CARD_STEP step,INT32U time,INT8U data,INT8U count)
{
    INT8U err = 0,cmd,datalen;
	_BSP_MESSAGE *pMsg;  
	pMsg = OSQPend(CardReciveEvent, time, &err);
	if (OS_ERR_NONE == err && (pMsg->MsgID == BSP_MSGID_UART_RXOVER)) //接收到串口数据
	{				
		if (UART_MsgDeal(&pMsg, CPUCardTempBuf, sizeof(CPUCardTempBuf)) == TRUE)
		{	
			if(pMsg->DataLen < 6)
			{
				UART_FreeOneMsg(CARD_UART, pMsg);
				return FALSE;
			}
			if (pMsg->pData[3] != 0x20) 
			{
				UART_FreeOneMsg(CARD_UART, pMsg);
				return FALSE;
			}
			
			cmd = pMsg->pData[2];
			datalen = pMsg->pData[1];
			if(CalcBCC(pMsg->pData,datalen - 1 ) != pMsg->pData[pMsg->DataLen - 1])
			{
				UART_FreeOneMsg(CARD_UART, pMsg);
				return FALSE;
			}
			switch(step)
			{
				case CARD_SEARCH_STEP:  //寻卡
					if((cmd == 0xA1) && (datalen == 0x0C) && (pMsg->pData[4] == 0x00))
					{
							//memcpy(UserCardInfo.CardID,&pMsg->pData[7],4);
//								memset(ReadData1,0x88,6);
//								memset(ReadData2,0x88,6);
								if(count == 1)
								{
									memcpy(ReadData1,&pMsg->pData[7],4);
								}
								else
								{
									memcpy(ReadData2,&pMsg->pData[7],4);
								}
							UART_FreeOneMsg(CARD_UART, pMsg);
							return TRUE;
						}
						else{
							NOP();
						}
					break;
				case CARD_KEY_STEP:  	//密钥验证
					if(cmd == 0xc3)
					{
						if(pMsg->pData[4] == 0x00)
						{
							UART_FreeOneMsg(CARD_UART, pMsg);
							return TRUE;
						}else{
							NOP();
						}						
					}
					break;
				case CARD_READ_STEP:  	//读
					if((cmd == 0xA3) && (datalen == 0x16) && (pMsg->pData[4] == 0x00))
					{
							if(BLOCK_POSSWORD == data)  //读取密码
							{
//								for(i = 0;i < 6;i++)
//								{
//									UserCardInfo.CardCode[i] = pMsg->pData[5+i];
//								}
//								memset(ReadData1,0x88,6);
//								memset(ReadData2,0x88,6);
								if(count == 1)
								{
									memcpy(ReadData1,&pMsg->pData[5],6);
								}
								else
								{
									memcpy(ReadData2,&pMsg->pData[5],6);
								}
							}
							if(BLOCK_STATUS == data) //卡状态
							{
								if((pMsg->pData[5] != 0x00) && (pMsg->pData[5] != 0xff) )
								{
									UART_FreeOneMsg(CARD_UART, pMsg);
									return FALSE;
								}
								//UserCardInfo.lockstate = pMsg->pData[5];
//								memset(ReadData1,0x88,6);
//								memset(ReadData2,0x88,6);
								if(count == 1)
								{
									ReadData1[0] = pMsg->pData[5];
								}
								else
								{
									ReadData2[0] = pMsg->pData[5];
								}
								
							}
							if(BLOCK_MONEY == data)  //读取金额
							{
//								money = ((pMsg->pData[5]) | (pMsg->pData[6] << 8) | (pMsg->pData[7] << 16) | (pMsg->pData[8] << 24));
//								UserCardInfo.balance = money;
//								memset(ReadData1,0x88,6);
//								memset(ReadData2,0x88,6);
								if(count == 1)
								{
									memcpy(ReadData1,&pMsg->pData[5],6);
								}
								else
								{
									memcpy(ReadData2,&pMsg->pData[5],6);
								}
							}
							UART_FreeOneMsg(CARD_UART, pMsg);
							return TRUE;
						}
						else{
							NOP();
						}
						break;
				case CARD_WRITE_STEP:  	//写
					if((cmd == 0xA4) && (datalen == 0x08) && (pMsg->pData[4] == 0x00))
					{
						UART_FreeOneMsg(CARD_UART, pMsg);
						return TRUE;
					}
					else{
						NOP();
					}

					break;
				case CARD_SET_STEP:  	//设置
					if((cmd == 0xA6) && (datalen == 0x0A) && (pMsg->pData[4] == 0x00))
					{
						UART_FreeOneMsg(CARD_UART, pMsg);
						return TRUE;
					}
					else{
						NOP();
					}
					break;
				default:
					break;
			}
		}
		UART_FreeOneMsg(CARD_UART, pMsg);
		return FALSE;
	}	
	return FALSE;
}
/****************************************************************************
* Function     : M1_ReadCardID
* Description  : 读取卡号
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_ReadCardID(void)
{
	INT8U result;
	INT8U count = 1;
	/******************************************寻卡***********************************/
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_SEARCH_STEP,SYS_DELAY_1s);
	Send_ActiveCard();
	while(1)
	{	
		result = WaitCardmessage(CARD_SEARCH_STEP,SYS_DELAY_50ms,0,count);
		if(result == TRUE)
		{ 	
			//判断数据返回是否正确  
			(count == 1)?(count = 2):(count = 1); 
			if(strncmp((const char* )ReadData1,(const char *)ReadData2,4) == 0)
			{
				memcpy(UserCardInfo.CardID,ReadData1,4);  //获取卡号
				return TRUE;
			}	
		}
		if (CARD_TIMEOUT_CHECK(CARD_SEARCH_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_ActiveCard();
	}
}
/****************************************************************************
* Function     : M1_ReadCardCode
* Description  : 读取卡密码
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
//static INT8U M1_ReadCardCode(void)
//{
//	INT8U result;
//	INT8U count = 1;
//	CardUart_MsgFree();
//	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
//	Send_ReadBlock(BLOCK_POSSWORD);  //密码
//	while(1)
//	{
//		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_POSSWORD,count);
//		if(result == TRUE)
//		{
//			//判断数据返回是否正确 
//			//判断数据返回是否正确  
//			(count == 1)?(count = 2):(count = 1); 
//			if(strncmp((const char* )ReadData1,(const char *)ReadData2,4) == 0)
//			{
//				memcpy(UserCardInfo.CardCode,ReadData1,6);  //获取卡密码
//				break;
//			}	
//		}
//		//设置卡为空闲状态
//		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
//		{
//			return FALSE;
//		}
//		Send_ReadBlock(BLOCK_POSSWORD);  //密码
//	
//	}
//	return TRUE;
//}


/****************************************************************************
* Function     : M1_ReadCardMoney
* Description  : 读取余额
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_ReadCardMoney(void)
{
	INT8U result;
	INT8U count = 1;
	INT32U money = 0;
	
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
	Send_ReadBlock(BLOCK_MONEY);  //余额; 
	while(1)
	{
		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_MONEY,count);
		if(result == TRUE)
		{
			//判断数据返回是否正确  
			(count == 1)?(count = 2):(count = 1); 
			if(strncmp((const char* )ReadData1,(const char *)ReadData2,4) == 0)
			{
				money = ((ReadData1[0]) | (ReadData1[1] << 8) | (ReadData1[2] << 16) | (ReadData1[3] << 24));
				UserCardInfo.balance = money;
				return TRUE;
			}	
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_ReadBlock(BLOCK_MONEY);  //余额;
	}
}

/****************************************************************************
* Function     : M1_ReadCardStartMoney
* Description  : 开始充电金额
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_ReadCardStartMoney(void)
{
	INT8U result;
	INT8U count = 1;
	INT32U money = 0;
	
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
	Send_ReadBlock(BLOCK_STARTMONEY);  //余额; 
	while(1)
	{
		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_MONEY,count);
		if(result == TRUE)
		{
			//判断数据返回是否正确  
			(count == 1)?(count = 2):(count = 1); 
			if(strncmp((const char* )ReadData1,(const char *)ReadData2,4) == 0)
			{
				money = ((ReadData1[0]) | (ReadData1[1] << 8) | (ReadData1[2] << 16) | (ReadData1[3] << 24));
				StartChargeMoney = money;
				return TRUE;
			}	
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_ReadBlock(BLOCK_STARTMONEY);  //余额;
	}
}

/****************************************************************************
* Function     : M1_ReadCardMoney1
* Description  : 读取余额备份1
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_ReadCardMoney1(void)
{
	INT8U result;
	INT8U count = 1;
	INT32U money = 0;
	
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
	Send_ReadBlock(BLOCK_MONEY1);  //余额; 
	while(1)
	{
		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_MONEY,count);
		if(result == TRUE)
		{
			//判断数据返回是否正确  
			(count == 1)?(count = 2):(count = 1); 
			if(strncmp((const char* )ReadData1,(const char *)ReadData2,4) == 0)
			{
				money = ((ReadData1[0]) | (ReadData1[1] << 8) | (ReadData1[2] << 16) | (ReadData1[3] << 24));
				UserCardInfo.balance = money;
				return TRUE;
			}	
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_ReadBlock(BLOCK_MONEY1);  //余额;
	}
}

/****************************************************************************
* Function     : M1_ReadCardMoney2
* Description  : 读取余额2
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
//static INT8U M1_ReadCardMoney2(void)
//{
//	INT8U result;
//	INT8U count = 1;
//	INT32U money = 0;
//	
//	CardUart_MsgFree();
//	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
//	Send_ReadBlock(BLOCK_MONEY2);  //余额; 
//	while(1)
//	{
//		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_MONEY,count);
//		if(result == TRUE)
//		{
//			//判断数据返回是否正确  
//			(count == 1)?(count = 2):(count = 1); 
//			if(strncmp((const char* )ReadData1,(const char *)ReadData2,4) == 0)
//			{
//				money = ((ReadData1[0]) | (ReadData1[1] << 8) | (ReadData1[2] << 16) | (ReadData1[3] << 24));
//				UserCardInfo.balance = money;
//				return TRUE;
//			}	
//		}
//		//设置卡为空闲状态
//		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
//		{
//			return FALSE;
//		}
//		Send_ReadBlock(BLOCK_MONEY2);  //余额;
//	}
//}

/****************************************************************************
* Function     : M1_ReadCardState
* Description  : 读取卡状态
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_ReadCardState(void)
{
	INT8U result;
	INT8U count = 1;
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
	Send_ReadBlock(BLOCK_STATUS);  //卡状态
	while(1)
	{
		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_STATUS,count);
		if(result == TRUE)
		{
			//判断数据返回是否正确  
			(count == 1)?(count = 2):(count = 1); 
			if(strncmp((const char* )ReadData1,(const char *)ReadData2,1) == 0)
			{
				UserCardInfo.lockstate = ReadData1[0];
				return TRUE;
			}	
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_ReadBlock(BLOCK_STATUS);  //卡状态
	}
}

/****************************************************************************
* Function     : M1_ReadCardState
* Description  : 读取卡状态备份1
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_ReadCardState1(void)
{
	INT8U result;
	INT8U count = 1;
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
	Send_ReadBlock(BLOCK_STATUS);  //卡状态
	while(1)
	{
		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_STATUS,count);
		if(result == TRUE)
		{
			//判断数据返回是否正确  
			(count == 1)?(count = 2):(count = 1); 
			if(strncmp((const char* )ReadData1,(const char *)ReadData2,1) == 0)
			{
				UserCardInfo.lockstate = ReadData1[0];
				return TRUE;
			}	
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_ReadBlock(BLOCK_STATUS);  //卡状态
	}
}


/****************************************************************************
* Function     : M1_ReadCardState
* Description  : 读取卡状态
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_ReadCardState2(void)
{
	INT8U result;
	INT8U count = 1;
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
	Send_ReadBlock(BLOCK_STATUS2);  //卡状态
	while(1)
	{
		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_STATUS,count);
		if(result == TRUE)
		{
			//判断数据返回是否正确  
			(count == 1)?(count = 2):(count = 1); 
			if(strncmp((const char* )ReadData1,(const char *)ReadData2,1) == 0)
			{
				UserCardInfo.lockstate = ReadData1[0];
				return TRUE;
			}	
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_ReadBlock(BLOCK_STATUS2);  //卡状态
	}
}

/****************************************************************************
* Function     : M1_WriterCardStartMoney
* Description  : 开始充电金额
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_WriterCardStartMoney(INT32U money)
{
	INT8U result;

	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_SET_STEP,SYS_DELAY_1s);
	Send_InitWallet(BLOCK_STARTMONEY,money);  //余额
	while(1)
	{
		result = WaitCardmessage(CARD_SET_STEP,SYS_DELAY_50ms,BLOCK_MONEY,0);
		if(result == TRUE)
		{
			break;
		}
				//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_SET_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_InitWallet(BLOCK_STARTMONEY,money);  //余额
	}
	return TRUE;
}



/****************************************************************************
* Function     : M1_WriterCardMoney
* Description  : 写入余额
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_WriterCardMoney(INT32U money)
{
	INT8U result;

	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_SET_STEP,SYS_DELAY_1s);
	Send_InitWallet(BLOCK_MONEY,money);  //余额
	while(1)
	{
		result = WaitCardmessage(CARD_SET_STEP,SYS_DELAY_50ms,BLOCK_MONEY,0);
		if(result == TRUE)
		{
			break;
		}
				//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_SET_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_InitWallet(BLOCK_MONEY,money);  //余额
	}
	
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_SET_STEP,SYS_DELAY_1s);
	Send_InitWallet(BLOCK_MONEY1,money);  //备份余额1
	while(1)
	{
		result = WaitCardmessage(CARD_SET_STEP,SYS_DELAY_50ms,BLOCK_MONEY,0);
		if(result == TRUE)
		{
			break;
		}
				//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_SET_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_InitWallet(BLOCK_MONEY1,money);  //余额
	}
	
	CardUart_MsgFree();
	CARD_TIMEOUT_ENTER(CARD_SET_STEP,SYS_DELAY_1s);
	Send_InitWallet(BLOCK_MONEY2,money);  //备份余额2
	while(1)
	{
		result = WaitCardmessage(CARD_SET_STEP,SYS_DELAY_50ms,BLOCK_MONEY,0);
		if(result == TRUE)
		{
			break;
		}
				//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_SET_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_InitWallet(BLOCK_MONEY2,money);  //余额
	}
	return TRUE;
}

/****************************************************************************
* Function     : M1_WriterCardLock
* Description  : 卡上锁
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_WriterCardLock()
{
	INT8U result;
	INT8U cardlock[16] = {0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //卡上锁命令
	//给该卡上锁
	CARD_TIMEOUT_ENTER(CARD_WRITE_STEP,SYS_DELAY_1s);
	CardUart_MsgFree();
	Send_WriterBlock(BLOCK_STATUS,cardlock);			//写上锁
	while(1)
	{
		result = WaitCardmessage(CARD_WRITE_STEP,SYS_DELAY_50ms,BLOCK_POSSWORD,0);
		if(result == TRUE)
		{
			break;
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_WRITE_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_WriterBlock(BLOCK_STATUS,cardlock);			//写上锁
	}
	CARD_TIMEOUT_ENTER(CARD_WRITE_STEP,SYS_DELAY_1s);
	CardUart_MsgFree();
	Send_WriterBlock(BLOCK_STATUS,cardlock);			//写上锁
	while(1)
	{
		result = WaitCardmessage(CARD_WRITE_STEP,SYS_DELAY_50ms,BLOCK_POSSWORD,0);
		if(result == TRUE)
		{
			break;
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_WRITE_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_WriterBlock(BLOCK_STATUS,cardlock);			//写上锁
	}
	CARD_TIMEOUT_ENTER(CARD_WRITE_STEP,SYS_DELAY_1s);
	CardUart_MsgFree();
	Send_WriterBlock(BLOCK_STATUS2,cardlock);			//写上锁备份2
	while(1)
	{
		result = WaitCardmessage(CARD_WRITE_STEP,SYS_DELAY_50ms,BLOCK_POSSWORD,0);
		if(result == TRUE)
		{
			break;
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_WRITE_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_WriterBlock(BLOCK_STATUS2,cardlock);			//写上锁备份2
	}
	return TRUE;
}

/****************************************************************************
* Function     : M1_WriterCardUnLock
* Description  : 卡解锁
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U M1_WriterCardUnLock()
{
	INT8U result;
	INT8U cardlock[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //解锁锁命令
	//给该卡解锁
	CARD_TIMEOUT_ENTER(CARD_WRITE_STEP,SYS_DELAY_1s);
	CardUart_MsgFree();
	Send_WriterBlock(BLOCK_STATUS,cardlock);			//写解锁
	while(1)
	{
		result = WaitCardmessage(CARD_WRITE_STEP,SYS_DELAY_50ms,BLOCK_STATUS,0);
		if(result == TRUE)
		{
			break;
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_WRITE_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_WriterBlock(BLOCK_STATUS,cardlock);			//写上锁

	}
	//给该卡解锁
	CARD_TIMEOUT_ENTER(CARD_WRITE_STEP,SYS_DELAY_1s);
	CardUart_MsgFree();
	Send_WriterBlock(BLOCK_STATUS1,cardlock);			//写解锁
	while(1)
	{
		result = WaitCardmessage(CARD_WRITE_STEP,SYS_DELAY_50ms,BLOCK_STATUS,0);
		if(result == TRUE)
		{
			break;
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_WRITE_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_WriterBlock(BLOCK_STATUS1,cardlock);			//写上锁

	}
	//给该卡解锁
	CARD_TIMEOUT_ENTER(CARD_WRITE_STEP,SYS_DELAY_1s);
	CardUart_MsgFree();
	Send_WriterBlock(BLOCK_STATUS2,cardlock);			//写解锁
	while(1)
	{
		result = WaitCardmessage(CARD_WRITE_STEP,SYS_DELAY_50ms,BLOCK_STATUS,0);
		if(result == TRUE)
		{
			break;
		}
		//设置卡为空闲状态
		if (CARD_TIMEOUT_CHECK(CARD_WRITE_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
		{
			return FALSE;
		}
		Send_WriterBlock(BLOCK_STATUS2,cardlock);			//写上锁

	}
	return TRUE;
}
/****************************************************************************
* Function     : Charge_Query_Card
* Description  : 查询卡
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U Charge_Query_Card(void)
{
	//INT8U readnum = 0;
	
	if (M1_ReadCardID() == FALSE)   //获取卡号
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	
	
	/********************************读取扇区*****************************/
//	CARD_TIMEOUT_ENTER(CARD_READ_STEP,SYS_DELAY_1s);
//	CardUart_MsgFree();
//	Send_ReadBlock(BLOCK_POSSWORD);  //密码
//	while(1)
//	{
//		result = WaitCardmessage(CARD_READ_STEP,SYS_DELAY_50ms,BLOCK_POSSWORD);
//		if(result == TRUE)
//		{
//			//判断数据返回是否正确 
//			break;
//		}
//			//设置卡为空闲状态
//		if (CARD_TIMEOUT_CHECK(CARD_READ_STEP, SYS_DELAY_1s) == CARD_TIME_OUT)
//		{
//			SetCardWorkStatus(_Card_IDLE);	
//			return FALSE;
//		}
//		Send_ReadBlock(BLOCK_POSSWORD);  //密码
//	}
//	
	//读取余额
	if(M1_ReadCardMoney() == FALSE)		//读取余额
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	
	if(UserCardInfo.balance > 10000000) //大于10w元则有问题
	{
		if(M1_ReadCardMoney1() == FALSE)		//读取余额备份1
		{
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
		if(UserCardInfo.balance > 10000000) //大于10w元则有问题
		{
			if(M1_ReadCardMoney1() == FALSE)		//读取余额备份2
			{
				SetCardWorkStatus(_Card_IDLE);	
				return FALSE;
			}
		}
		if(UserCardInfo.balance <= 10000000) //正常卡
		{
			M1_WriterCardMoney(UserCardInfo.balance);			//更新余额
		}
	}

	if (M1_ReadCardState() == FALSE)   //读取卡状态
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
		
	/**********************************卡信息判断**************************/
	if(UserCardInfo.balance > 10000000) //大于10w元则有问题
	{
		SysCardInfo.Card_Result = _FailNotIssuer;				 //非发行方卡	
	}else{
		SysCardInfo.Card_Result = _SuccessRtu;				 //刷卡成功	
	}
	//密码校验
	#if(CARD_USER_CODE == 1)
        	//密码校验
		if(Password_Judge(gun) == FALSE)
		{
			//返回原因已经在密码校验中填写了
			NOP();
			//return FALSE;
			
		}
	#else
			NOP();
	#endif
	/*******************************给显示任务发送消息*******************/
	DispMsg_Send(CARDCMD_QUERY);	
	
	/*******************************等待显示任务处理完成*******************/
	//给显示任务返回消息，需要等待显示任务处理完消息，才可以设置为空闲状态，否则可能存在显示任务再次下发启动刷卡指令
	CARD_TIMEOUT_ENTER(CARD_INFODIPOSE_STEP,SYS_DELAY_5s);
	while(1)
	{
		if(GetCardInfoStatus() == _Already_Obtain)
		{
			break;
		}
		if (CARD_TIMEOUT_CHECK(CARD_INFODIPOSE_STEP, SYS_DELAY_5s) == CARD_TIME_OUT)
		{
			//设置卡为空闲状态
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
	}
	SetCardWorkStatus(_Card_IDLE); 
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
	OSTimeDly(SYS_DELAY_50ms);	
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	return TRUE;
	//设置卡为空闲状态
}

/****************************************************************************
* Function     : NetCharge_StartCard
* Description  :  网络开始充电，只要获取卡号
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U NetCharge_StartCard(_GUN_NUM gun)
{

	/******************************************寻卡***********************************/
	if(M1_ReadCardID() == FALSE)		//读取卡号
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	if(gun == GUN_A)
	{
		memcpy(A_UserCardInfo.CardID,UserCardInfo.CardID,4);
	}else{
		memcpy(B_UserCardInfo.CardID,UserCardInfo.CardID,4);
	}
	SysCardInfo.Card_Result = _SuccessRtu;  //刷卡成功
	if(gun == GUN_A)
	{
		DispMsg_Send(NETCARDCMD_STARTA);
	}else{
		DispMsg_Send(NETCARDCMD_STARTB);
	}
	/*******************************等待显示任务处理完成*******************/
	//给显示任务返回消息，需要等待显示任务处理完消息，才可以设置为空闲状态，否则可能存在显示任务再次下发启动刷卡指令
	CARD_TIMEOUT_ENTER(CARD_INFODIPOSE_STEP,SYS_DELAY_5s);
	while(1)
	{
		if(GetCardInfoStatus() == _Already_Obtain)
		{
			break;
		}
		if (CARD_TIMEOUT_CHECK(CARD_INFODIPOSE_STEP, SYS_DELAY_5s) == CARD_TIME_OUT)
		{
			//设置卡为空闲状态
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
	}
	SetCardWorkStatus(_Card_IDLE); 
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
	OSTimeDly(SYS_DELAY_50ms);	
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);
	return TRUE;
	
}

/****************************************************************************
* Function     : Charge_StartCard
* Description  : 开始充电刷卡
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U Charge_StartCard(_GUN_NUM gun)
{

	/******************************************寻卡***********************************/
	if(M1_ReadCardID() == FALSE)		//读取卡号
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	if(gun == GUN_A)
	{
		memcpy(A_UserCardInfo.CardID,UserCardInfo.CardID,4);
	}else{
		memcpy(B_UserCardInfo.CardID,UserCardInfo.CardID,4);
	}
	
	#if(CARD_USER_CODE == 1)
	if(M1_ReadCardCode() == FALSE)		//读取卡密码
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	if(gun == GUN_A)
	{
		memcpy(A_UserCardInfo.CardCode,UserCardInfo.CardCode,sizeof(UserCardInfo.CardCode));
	}else{
		memcpy(B_UserCardInfo.CardCode,UserCardInfo.CardCode,sizeof(UserCardInfo.CardCode));
	}
	#endif
	
	
	//读取余额
	if(M1_ReadCardMoney() == FALSE)		//读取余额
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	
	if(UserCardInfo.balance > 10000000) //大于10w元则有问题
	{
		if(M1_ReadCardMoney1() == FALSE)		//读取余额备份1
		{
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
		if(UserCardInfo.balance > 10000000) //大于10w元则有问题
		{
			if(M1_ReadCardMoney1() == FALSE)		//读取余额备份2
			{
				SetCardWorkStatus(_Card_IDLE);	
				return FALSE;
			}
		}
		if(UserCardInfo.balance <= 10000000) //正常卡
		{
			M1_WriterCardMoney(UserCardInfo.balance);			//更新余额
		}
	}
	
	
	//读取上一次开始充电余额
	if(M1_ReadCardStartMoney() == FALSE)		//读取余额
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	if(UserCardInfo.balance <= 10000000)  //正常卡
	{
		if(StartChargeMoney != UserCardInfo.balance)  //余额发送了变化 说明结算了
		{
			UserCardInfo.lockstate = 0;
			if(M1_WriterCardStartMoney(UserCardInfo.balance) == FALSE)
			{
				SetCardWorkStatus(_Card_IDLE);	
				return FALSE;
			}
		}
		else
		{
			//读取状态
			if(M1_ReadCardState() == FALSE)		//读取卡状态
			{
				SetCardWorkStatus(_Card_IDLE);	
				return FALSE;
			}
			if(UserCardInfo.lockstate == 0xff)
			{
				if(M1_ReadCardState1() == FALSE)		//读取卡状态备份1
				{
					SetCardWorkStatus(_Card_IDLE);	
					return FALSE;
				}
				if(UserCardInfo.lockstate == 0xff)
				{
					if(M1_ReadCardState2() == FALSE)		//读取卡状态备份2
					{
						SetCardWorkStatus(_Card_IDLE);	
						return FALSE;
					}
				}
				//后续如果成了，会写卡状态，所以再这里不需要更新
			}
		}
	}

	
	/**********************************卡信息判断**************************/
	if(UserCardInfo.balance > 10000000) //大于10w元则有问题
	{
		SysCardInfo.Card_Result = _FailNotIssuer;				 //非发行方卡	
	}else if(UserCardInfo.balance == 0){
		SysCardInfo.Card_Result = _BalanceLack;				 //非发行方卡	
	}
	else if((UserCardInfo.lockstate != 0xff) && (UserCardInfo.lockstate != 0x00))
	{
		SysCardInfo.Card_Result = _FailRtu;				 //非发行方卡	
	}else{
		SysCardInfo.Card_Result = _SuccessRtu;				 //刷卡成功	
	}
	
	if(gun == GUN_A)
	{
		A_UserCardInfo.balance = UserCardInfo.balance;
	}else{
		B_UserCardInfo.balance = UserCardInfo.balance;
	}
	#if(CARD_USER_CODE == 1)
        	//密码校验
		if(Password_Judge(gun) == FALSE)
		{
			//返回原因已经在密码校验中填写了
			NOP();
			//return FALSE;
			
		}
	#else
			NOP();
	#endif
	if(UserCardInfo.lockstate == 0xff)
	{
		if(Dispose_LocalUnLock(gun) == FALSE)   //本地扣除之前未结算的
		{
			SysCardInfo.Card_Result = _LockCard;				 //卡已经被锁
		}
	}
//	if(SysCardInfo.Card_Result == _SuccessRtu)  //刷卡成功
//	{
//		if(M1_WriterCardLock() ==  FALSE)		//写卡状态
//		{
//			SetCardWorkStatus(_Card_IDLE);	
//			return FALSE;
//		}
//	}
	/*******************************给显示任务发送消息*******************/
	OSTimeDly(SYS_DELAY_500ms);   //主要是为了防止别人提前拿卡
	if(gun == GUN_A)
	{
		DispMsg_Send(CARDCMD_STARTA);
	}else{
		DispMsg_Send(CARDCMD_STARTB);
	}
	/*******************************等待显示任务处理完成*******************/
	//给显示任务返回消息，需要等待显示任务处理完消息，才可以设置为空闲状态，否则可能存在显示任务再次下发启动刷卡指令
	CARD_TIMEOUT_ENTER(CARD_INFODIPOSE_STEP,SYS_DELAY_5s);
	while(1)
	{
		if(GetCardInfoStatus() == _Already_Obtain)
		{
			break;
		}
		if (CARD_TIMEOUT_CHECK(CARD_INFODIPOSE_STEP, SYS_DELAY_5s) == CARD_TIME_OUT)
		{
			//设置卡为空闲状态
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
	}
	SetCardWorkStatus(_Card_IDLE); 
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
	OSTimeDly(SYS_DELAY_50ms);	
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	return TRUE;
	//设置卡为空闲状态
}

/****************************************************************************
* Function     : NetCharge_StopCard
* Description  :  网络停止充电，只要获取卡号
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U NetCharge_StopCard()
{
	INT8U gun = 0;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	/******************************************寻卡***********************************/
	if(M1_ReadCardID() == FALSE)		//读取卡号
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	SysCardInfo.Card_Result = _NOStartCard;				 //非启动卡
	for(gun = 0;gun < GUN_MAX;gun++)
	{
		if(pdisp_conrtol->NetGunState[gun] == GUN_CHARGEING)  //充电状态
		{
			if(_4G_GetStartType(gun) == _4G_APP_CARD )//刷卡启动才比较
			{
				if(CardNum_Judge(gun) == TRUE)    //卡号一致
				{
					
					SysCardInfo.Card_Result = _SuccessRtu;  //刷卡成功
					break;
				}
				else
				{
					SysCardInfo.Card_Result = _NOStartCard;				 //非启动卡
				}
			}
			if(_4G_GetStartType(gun) == _4G_APP_BCCARD)
			{
				SysCardInfo.Card_Result = _SuccessRtu;  //刷卡成功
				break;
			}
		}
	}

	if(SysCardInfo.Card_Result == _SuccessRtu)
	{
	
		if(_4G_GetStartType(gun) == _4G_APP_BCCARD)
		{
				DispMsg_Send(NETCARDCMD_STOPA);
				OSTimeDly(SYS_DELAY_250ms); 
				DispMsg_Send(NETCARDCMD_STOPB);
		}
		else
		{
			if(gun == GUN_A)
			{
				DispMsg_Send(NETCARDCMD_STOPA);
			}
			if(gun == GUN_B)
			{
				DispMsg_Send(NETCARDCMD_STOPB);
			}
		}
		/*******************************等待显示任务处理完成*******************/
		//给显示任务返回消息，需要等待显示任务处理完消息，才可以设置为空闲状态，否则可能存在显示任务再次下发启动刷卡指令
		CARD_TIMEOUT_ENTER(CARD_INFODIPOSE_STEP,SYS_DELAY_5s);
		while(1)
		{
			if(GetCardInfoStatus() == _Already_Obtain)
			{
				break;
			}
			if (CARD_TIMEOUT_CHECK(CARD_INFODIPOSE_STEP, SYS_DELAY_5s) == CARD_TIME_OUT)
			{
				//设置卡为空闲状态
				SetCardWorkStatus(_Card_IDLE);	
				return FALSE;
			}
		}
		SetCardWorkStatus(_Card_IDLE);
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
		OSTimeDly(SYS_DELAY_50ms);	
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	}
	else
	{
		SetCardWorkStatus(_Card_IDLE);
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
		OSTimeDly(SYS_DELAY_50ms);	
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
		OSTimeDly(SYS_DELAY_50ms);
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
		OSTimeDly(SYS_DELAY_500ms);	
		BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	}
	return TRUE;
}

/****************************************************************************
* Function     : Charge_StopCard
* Description  : 结束充电刷卡
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
static INT8U Charge_StopCard(_GUN_NUM gun)
{
	INT32U money,usermoney;
	if (M1_ReadCardID() == FALSE)   //获取卡号
	{
		SetCardWorkStatus(_Card_IDLE);	
		return FALSE;
	}
	
	/*********************设置卡内余额**********************************/
	if(CardNum_Judge(gun) == TRUE)    //卡号一致
	{
		//给该卡上解锁
		if(M1_WriterCardUnLock() ==  FALSE)		//写卡状态
		{
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
		
		//设置余额
		money = Money_Cal(gun);
		//money = 10;
		if (M1_WriterCardMoney(money) == FALSE)   
		{
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
		
		usermoney = money;
		
		SysCardInfo.Card_Result = _SuccessRtu;				 //刷卡成功	
		if(gun == GUN_B)
		{
			B_UserCardInfo.balance = usermoney;
		}else{
			A_UserCardInfo.balance = usermoney;
		}
	
	}else{
		SysCardInfo.Card_Result = _NOStartCard;				 //非启动卡
	}
	
	
	/*******************************给显示任务发送消息*******************/
	OSTimeDly(SYS_DELAY_100ms);   //主要是为了防止别人提前拿卡
	if(gun == GUN_A)
	{
		DispMsg_Send(CARDCMD_STOPA);
	}else{
		DispMsg_Send(CARDCMD_STOPB);
	}
	/*******************************等待显示任务处理完成*******************/
	//给显示任务返回消息，需要等待显示任务处理完消息，才可以设置为空闲状态，否则可能存在显示任务再次下发启动刷卡指令
	CARD_TIMEOUT_ENTER(CARD_INFODIPOSE_STEP,SYS_DELAY_5s);
	while(1)
	{
		if(GetCardInfoStatus() == _Already_Obtain)
		{
			break;
		}
		if (CARD_TIMEOUT_CHECK(CARD_INFODIPOSE_STEP, SYS_DELAY_5s) == CARD_TIME_OUT)
		{
			//设置卡为空闲状态
			SetCardWorkStatus(_Card_IDLE);	
			return FALSE;
		}
	}
	SetCardWorkStatus(_Card_IDLE);
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
	OSTimeDly(SYS_DELAY_50ms);	
	BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
	return TRUE;
}

/****************************************************************************
* Function     : CardAnalysis
* Description  : 来自显示任务的消息解析
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
void CardAnalysis(INT8U div, INT8U *data, INT16U len)
{ 
		SetCardWorkStatus(_Card_Work);                       //卡任务进入工作中 
		memset(&UserCardInfo, 0, sizeof(USERCARDINFO));      //指令开始前清空临时用户信息				
	
		SysCardInfo.Card_Result = _DefaultCard;				 //返回结果	
		switch(div)                      //根据指令接收pdata数据
		{
			case NETCARDCMD_UNLOCK:
				if(M1_WriterCardUnLock())
				{
					BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_OPEN);
					OSTimeDly(SYS_DELAY_50ms);	
					BSP_RLCWrite(IO_LED_BEEP,(_BSPRLC_STATE)BSPRLC_STATE_CLOSE);	
				}
				SetCardWorkStatus(_Card_IDLE);		//卡位空闲状态		
				break;
			case CARDCMD_QUERY:		 
				memcpy(Query_ReceiveData.password, data, len);		
				Charge_Query_Card();			
				break;
			case CARDCMD_STARTA:
				SetCardInfoStatus(_Not_Obtain);  	//显示任务未获取信息
				SetCardWorkStatus(_Card_Work);		//卡为工作中		
				memcpy(A_ReceiveData.password, data, len);
				Charge_StartCard(GUN_A);
				break;
			case CARDCMD_STARTB:
				SetCardInfoStatus(_Not_Obtain);  	//显示任务未获取信息
				SetCardWorkStatus(_Card_Work);		//卡为工作中
				memcpy(B_ReceiveData.password, data, len);				                                	//密码接收
				Charge_StartCard(GUN_B);
				break;
			case CARDCMD_STOPA:
				SetCardInfoStatus(_Not_Obtain);  	//显示任务未获取信息
				SetCardWorkStatus(_Card_Work);		//卡为工作中
				memcpy((INT8U*)&A_ReceiveData.money, data, len);      //消费数据接收
				A_ReceiveData.money = A_ReceiveData.money/100;
				Charge_StopCard(GUN_A);
				break;
			case CARDCMD_STOPB:
				SetCardInfoStatus(_Not_Obtain);  	//显示任务未获取信息
				SetCardWorkStatus(_Card_Work);		//卡为工作中
				memcpy((INT8U*)&B_ReceiveData.money, data,len);      //消费数据接收
				B_ReceiveData.money = B_ReceiveData.money/100;	 //传进来是小数点后4位
				Charge_StopCard(GUN_B);
				break;	
			case NETCARDCMD_STARTA:
				SetCardInfoStatus(_Not_Obtain);  	//显示任务未获取信息
				SetCardWorkStatus(_Card_Work);		//卡为工作中
				NetCharge_StartCard(GUN_A);
				break;
			case NETCARDCMD_STARTB:
				SetCardInfoStatus(_Not_Obtain);  	//显示任务未获取信息
				SetCardWorkStatus(_Card_Work);		//卡为工作中
				NetCharge_StartCard(GUN_B);
				break;
			case  NETCARDCMD_STOPA:
			case  NETCARDCMD_STOPB:
				SetCardInfoStatus(_Not_Obtain);  	//显示任务未获取信息
				SetCardWorkStatus(_Card_Work);		//卡为工作中
				NetCharge_StopCard();
				break;
			default:
					break;			
		}

}
/*****************************************************************************
* Function     : APP_GetCardEvent
* Description  : 获取显示事件控制块指针
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 2018年7月27日
*****************************************************************************/
OS_EVENT* APP_GetCardEvent(void)
{
    return CPUCardTaskEvent;
}

/*****************************************************************************
* Function     : Card_HardwareInit
* Description  : 
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日       
*****************************************************************************/
static INT8U Card_HardwareInit(void)
{ 
	_BSP_UART_SET UartGPRSSet;
    CardReciveEvent = OSQCreate(CardReciveOSQ, CARDRec_Q_LEN);
    if (CardReciveEvent == NULL)
    {
        OSTaskSuspend(OS_PRIO_SELF);   
        return FALSE;
    }
    memset(CPUCardSendBuf, 0 ,sizeof(CPUCardSendBuf));
    memset(CPUCardRecvBuf, 0 ,sizeof(CPUCardRecvBuf));

    UartGPRSSet.BaudRate = 9600;
    UartGPRSSet.DataBits = BSPUART_WORDLENGTH_8;
    UartGPRSSet.Parity   = BSPUART_PARITY_NO;
    UartGPRSSet.StopBits = BSPUART_STOPBITS_1;
    UartGPRSSet.RxBuf = CPUCardRecvBuf;
    UartGPRSSet.RxBufLen = sizeof(CPUCardRecvBuf);
    UartGPRSSet.TxBuf = CPUCardSendBuf;
    UartGPRSSet.TxBufLen = sizeof(CPUCardSendBuf);
    UartGPRSSet.Mode = UART_DEFAULT_MODE | UART_MSG_MODE;
    UartGPRSSet.RxQueue = CardReciveEvent;
    UartGPRSSet.RxMsgMemBuf = CPUCardRecvSMem;
    UartGPRSSet.RxMsgMemLen = sizeof(CPUCardRecvSMem);
    if (BSP_UARTConfigInit(CARD_UART, &UartGPRSSet) == FALSE)
    {
        return FALSE;
    }   
    return TRUE;
}
/****************************************************************************
* Function     : Dispose_LocalUnLock
* Description  : 本地解锁处理
* Input        : 
* Output       : 
* Return       : 
* Note(s)      : 
* Contributor  : 20200629
******************************************************************************/
INT8U Dispose_LocalUnLock(_GUN_NUM gun)
{
	INT32U i;
	INT32U RecodeNum = APP_GetRecodeCurNum();
	_CHARGE_RECODE CurRecode;
	INT32U CardNun;
	INT32U money;
	
	memcpy(&CardNun,UserCardInfo.CardID,sizeof(INT32U));
	for(i = RecodeNum;i > RECODE_DISPOSE2(RecodeNum) ;i--)
	{
		//充最新的一条开始查找
		APP_RWChargeRecode( RECODE_DISPOSE1(i%1000),FLASH_ORDER_READ,&CurRecode);
		if(CurRecode.BillingStatus == RECODE_UNSETTLEMENT)
		{
		//查询卡号是否一致
			if( CurRecode.CardNum == CardNun)
			{
				if(UserCardInfo.balance > CurRecode.TotMoney)
				{
					money = UserCardInfo.balance - CurRecode.TotMoney;
				}else{
					money = 0;
				}
					CardUart_MsgFree();
					CARD_TIMEOUT_ENTER(CARD_SET_STEP,SYS_DELAY_500ms);
					Send_InitWallet(BLOCK_MONEY,money);  //余额
//					OSQFlush(CardReciveEvent);
					CardUart_MsgFree();
					while(1)
					{

						if (M1_WriterCardMoney(money) == FALSE)   
						{
							SetCardWorkStatus(_Card_IDLE);	
							return FALSE;
						}
						//余额写成功，余额改变了
						if(gun == GUN_B)
						{
							B_UserCardInfo.balance = money;
						}else{
							A_UserCardInfo.balance = money;
						}
						CurRecode.BillingStatus = RECODE_REPLSETTLEMENT; //设置未补充结算
						APP_RWChargeRecode( RECODE_DISPOSE1(i%1000),FLASH_ORDER_WRITE,&CurRecode);
						//判断数据返回是否正确
						return TRUE;
					}
						
			}
		}
	}
	return FALSE;
}

INT8U testcart;
/*****************************************************************************
* Function     : TaskCardStk
* Description  : 
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日   
*****************************************************************************/
void TaskCardStk(void *pdata)
{	
	INT8U err,result;  
	_BSP_MESSAGE *pMsg; 
	OSTimeDly(SYS_DELAY_500ms);
	Card_HardwareInit(); 
	Card_Init();
	OSTimeDly(SYS_DELAY_500ms);
	Send_AuthKey(Key,6);
	while(1)
	{
		result = WaitCardmessage(CARD_KEY_STEP,SYS_DELAY_100ms,1,1);
		if(result == TRUE)
		{
			//判断数据返回是否正确
			break;
		}
		OSTimeDly(SYS_DELAY_500ms);
		Send_AuthKey(Key,6);
	}	
	CardUart_MsgFree();
	while(1)
    {							
		pMsg = OSQPend(CPUCardTaskEvent, SYS_DELAY_500ms, &err);
		if ((OS_ERR_NONE == err) && (pMsg->MsgID == BSP_MSGID_DISP))   //来自显示任务的消息
		{				
			CardAnalysis(pMsg->DivNum, pMsg->pData, pMsg->DataLen);		
			OSQFlush(CPUCardTaskEvent);
		}	
//		if(testcart == 0)
//		{
//			Charge_StartCard(GUN_A);
//		}
//		else
//		{
//			Charge_StopCard(GUN_A);
//		}
	} 
}

/************************(C)COPYRIGHT 2020 汇誉科技*****END OF FILE****************************/
